#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <queue>
#include <stdexcept>

using namespace std;

//============================================
// Base Class: IODevice
//============================================
class IODevice {
protected:
    string deviceName;
    bool isOpen;

public:
    IODevice(string name) : deviceName(name), isOpen(false) {}
    virtual ~IODevice() {}

    virtual bool open() = 0;
    virtual void close() = 0;
    virtual string getDeviceType() = 0;
    virtual void displayInfo() = 0;

    string getName() { return deviceName; }
    bool getStatus() { return isOpen; }
};

//============================================
// Block Device
//============================================
class BlockDevice : public IODevice {
private:
    size_t blockSize;
    size_t totalBlocks;
    vector<vector<uint8_t>> blocks;

public:
    BlockDevice(string name, size_t blockSz, size_t totalBlk)
        : IODevice(name), blockSize(blockSz), totalBlocks(totalBlk) {
        blocks.resize(totalBlocks, vector<uint8_t>(blockSize, 0));
    }

    bool open() override {
        isOpen = true;
        cout << "[Block Device] " << deviceName << " opened\n";
        return true;
    }

    void close() override {
        isOpen = false;
        cout << "[Block Device] " << deviceName << " closed\n";
    }

    vector<uint8_t> readBlock(size_t blockNum) {
        if (!isOpen) throw runtime_error("Device not open");
        if (blockNum >= totalBlocks) throw out_of_range("Block out of range");
        return blocks[blockNum];
    }

    void writeBlock(size_t blockNum, const vector<uint8_t>& data) {
        if (!isOpen) throw runtime_error("Device not open");
        if (blockNum >= totalBlocks) throw out_of_range("Block out of range");
        if (data.size() != blockSize)
            throw invalid_argument("Data size mismatch");

        blocks[blockNum] = data;
        cout << "[Block Device] Block " << blockNum << " written\n";
    }

    string getDeviceType() override { return "Block Device"; }

    void displayInfo() override {
        cout << "Device: " << deviceName << endl;
        cout << "Type: " << getDeviceType() << endl;
        cout << "Block Size: " << blockSize << " bytes\n";
        cout << "Total Blocks: " << totalBlocks << endl;
        cout << "Total Capacity: "
             << (blockSize * totalBlocks) << " bytes\n";
    }
};

//============================================
// Character Device
//============================================
class CharacterDevice : public IODevice {
private:
    queue<char> inputBuffer;
    string outputBuffer;

public:
    CharacterDevice(string name) : IODevice(name) {}

    bool open() override {
        isOpen = true;
        cout << "[Char Device] " << deviceName << " opened\n";
        return true;
    }

    void close() override {
        isOpen = false;
        cout << "[Char Device] " << deviceName << " closed\n";
    }

    char getChar() {
        if (!isOpen) throw runtime_error("Device not open");
        if (inputBuffer.empty()) return '\0';

        char c = inputBuffer.front();
        inputBuffer.pop();
        return c;
    }

    void putChar(char c) {
        if (!isOpen) throw runtime_error("Device not open");
        outputBuffer += c;
        cout << "[Char Device] Output: " << c << endl;
    }

    void simulateInput(string input) {
        for (char c : input)
            inputBuffer.push(c);
    }

    string getDeviceType() override { return "Character Device"; }

    void displayInfo() override {
        cout << "Device: " << deviceName << endl;
        cout << "Type: " << getDeviceType() << endl;
        cout << "Buffer size: " << inputBuffer.size() << " chars\n";
    }
};

//============================================
// Network Device
//============================================
class NetworkDevice : public IODevice {
private:
    string ipAddress;
    int port;
    bool connected;
    vector<string> receivedPackets;

public:
    NetworkDevice(string name, string ip, int p)
        : IODevice(name), ipAddress(ip), port(p), connected(false) {}

    bool open() override {
        isOpen = true;
        cout << "[Network Device] " << deviceName << " opened\n";
        return true;
    }

    void close() override {
        isOpen = false;
        connected = false;
        cout << "[Network Device] " << deviceName << " closed\n";
    }

    void connect() {
        if (!isOpen) throw runtime_error("Device not open");
        connected = true;
        cout << "[Network Device] Connected to "
             << ipAddress << ":" << port << endl;
    }

    void sendPacket(string data) {
        if (!connected)
            throw runtime_error("Not connected to network");
        cout << "[Network Device] Sending packet: "
             << data << endl;
    }

    string getDeviceType() override { return "Network Device"; }

    void displayInfo() override {
        cout << "Device: " << deviceName << endl;
        cout << "Type: " << getDeviceType() << endl;
        cout << "IP: " << ipAddress << endl;
        cout << "Port: " << port << endl;
        cout << "Connected: "
             << (connected ? "Yes" : "No") << endl;
    }
};

//============================================
// Device Manager
//============================================
class DeviceManager {
private:
    map<string, shared_ptr<IODevice>> deviceTable;

public:
    void registerDevice(shared_ptr<IODevice> device) {
        deviceTable[device->getName()] = device;
        cout << "[DevMgr] Registered: "
             << device->getName()
             << " (" << device->getDeviceType() << ")\n";
    }

    shared_ptr<IODevice> getDevice(string name) {
        if (deviceTable.find(name) == deviceTable.end())
            throw runtime_error("Device not found: " + name);
        return deviceTable[name];
    }

    void listAllDevices() {
        cout << "\n=== Registered Devices ===\n";
        for (auto& pair : deviceTable) {
            cout << "- " << pair.first
                 << " [" << pair.second->getDeviceType() << "] "
                 << "Status: "
                 << (pair.second->getStatus() ? "Open" : "Closed")
                 << endl;
        }
    }

    void openDevice(string name) {
        getDevice(name)->open();
    }

    void closeDevice(string name) {
        getDevice(name)->close();
    }
};

//============================================
// MAIN
//============================================
int main() {
    cout << "=== Application I/O Interface Demo ===\n";

    DeviceManager devMgr;

    auto disk = make_shared<BlockDevice>("sda", 512, 100);
    auto keyboard = make_shared<CharacterDevice>("keyboard");
    auto ethernet = make_shared<NetworkDevice>("eth0", "192.168.1.1", 8080);

    devMgr.registerDevice(disk);
    devMgr.registerDevice(keyboard);
    devMgr.registerDevice(ethernet);

    devMgr.listAllDevices();

    // Block Device Test
    cout << "\n--- Block Device Test ---\n";
    devMgr.openDevice("sda");

    auto diskDevice =
        dynamic_pointer_cast<BlockDevice>(devMgr.getDevice("sda"));

    vector<uint8_t> testData(512, 0xAB);
    diskDevice->writeBlock(0, testData);

    auto readData = diskDevice->readBlock(0);
    cout << "Read block 0, first byte: "
         << hex << (int)readData[0] << dec << endl;

    diskDevice->displayInfo();

    // Character Device Test
    cout << "\n--- Character Device Test ---\n";
    devMgr.openDevice("keyboard");

    auto kbDevice =
        dynamic_pointer_cast<CharacterDevice>(
            devMgr.getDevice("keyboard"));

    kbDevice->simulateInput("Hello OS!");

    for (int i = 0; i < 5; i++) {
        char c = kbDevice->getChar();
        if (c != '\0')
            cout << "Read char: " << c << endl;
    }

    // Network Device Test
    cout << "\n--- Network Device Test ---\n";
    devMgr.openDevice("eth0");

    auto netDevice =
        dynamic_pointer_cast<NetworkDevice>(
            devMgr.getDevice("eth0"));

    netDevice->connect();
    netDevice->sendPacket("GET / HTTP/1.1");

    devMgr.listAllDevices();

    return 0;
}
