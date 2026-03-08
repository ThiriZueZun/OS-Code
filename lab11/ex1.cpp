#include <iostream>
#include <memory>
#include <map>
#include <string>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <chrono>
#include <thread>
using namespace std;
// ■■ Abstract Base ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
class IODevice {
protected:
string name;
bool opened = false;
public:
explicit IODevice(string n) : name(move(n)) {}
virtual ~IODevice() = default;
virtual bool open() = 0;
virtual void close() = 0;
virtual string read(size_t n) = 0;
virtual bool write(const string&) = 0;
virtual string getInfo() const = 0;
const string& getName() const { return name; }
bool isOpen() const { return opened; }
};
// ■■ KeyboardDevice ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
class KeyboardDevice : public IODevice {
string buffer = "Hello OS! ";
public:
explicit KeyboardDevice(string n) : IODevice(move(n)) {}
bool open() override { opened = true;
cout << "[KB] " << name << " opened\n"; return true; }
void close() override { opened = false;
cout << "[KB] " << name << " closed\n"; }
string read(size_t n) override {
if (!opened) throw runtime_error(name + ": not open");
string out = buffer.substr(0, min(n, buffer.size()));
cout << "[KB] read " << out.size() << " bytes: '" << out << "'\n";
return out;
}
bool write(const string& s) override {
cout << "[KB] keyboards are input-only\n"; return false;
}
string getInfo() const override {
return "KeyboardDevice '" + name + "' | rate: ~10 B/s | char-stream";
}
};
// ■■ DiskDevice ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
class DiskDevice : public IODevice {
static const size_t SECTOR = 512;
vector<string> sectors;
size_t totalSectors;
public:
DiskDevice(string n, size_t sectors_):
IODevice(move(n)), sectors(sectors_, string(SECTOR,'\0')),
totalSectors(sectors_) {}
bool open() override {
opened = true;
cout << "[DISK] " << name << " spun up\n"; return true;
}
void close() override {
opened = false;
cout << "[DISK] " << name << " parked\n";
}
string read(size_t sector) override {
if (!opened) throw runtime_error(name + ": not open");
if (sector >= totalSectors) throw out_of_range("bad sector");
cout << "[DISK] read sector " << sector << "\n";
return sectors[sector];
}
bool write(const string& data) override {
if (!opened) return false;
sectors[0] = data.substr(0, SECTOR);
cout << "[DISK] wrote " << data.size() << " bytes to sector 0\n";
return true;
}
string getInfo() const override {
ostringstream ss;
ss << "DiskDevice '" << name << "' | sectors: " << totalSectors
<< " | capacity: " << totalSectors * SECTOR / 1024 << " KB";
return ss.str();
}
};
// ■■ NetworkDevice ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
class NetworkDevice : public IODevice {
string ip;
int port;
bool connected = false;
public:
NetworkDevice(string n, string ip_, int port_):
IODevice(move(n)), ip(move(ip_)), port(port_) {}
bool open() override {
opened = true;
cout << "[NET] " << name << " socket opened\n"; return true;
}
void close() override {
opened = connected = false;
cout << "[NET] " << name << " disconnected\n";
}
string read(size_t n) override {
if (!opened) throw runtime_error(name + ": not open");
string pkt = "<packet:" + to_string(n) + "bytes>";
cout << "[NET] received: " << pkt << "\n";
return pkt;
}
bool write(const string& data) override {
if (!opened) return false;
cout << "[NET] sent " << data.size() << " bytes to "
<< ip << ":" << port << "\n";
return true;
}
string getInfo() const override {
return "NetworkDevice '" + name + "' | " + ip + ":" + to_string(port)
+ " | rate: up to 1 Gb/s";
}
};
// ■■ DeviceRegistry ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
class DeviceRegistry {
map<string, shared_ptr<IODevice>> table;
public:
void add(shared_ptr<IODevice> dev) {
table[dev->getName()] = dev;
cout << "[REG] registered: " << dev->getInfo() << "\n";
}
shared_ptr<IODevice> get(const string& n) {
auto it = table.find(n);
if (it == table.end()) throw runtime_error("Device not found: " + n);
return it->second;
}
void listAll() const {
cout << "\n=== Registered Devices (" << table.size() << ") ===\n";
for (auto& [k, v] : table)
cout << " " << (v->isOpen() ? "[OPEN] " : "[CLOSE] ")
<< v->getInfo() << "\n";
}
};
// ■■ main ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
int main() {
cout << "=== Device Registry Demo ===\n\n";
DeviceRegistry reg;
reg.add(make_shared<KeyboardDevice>("kbd0"));
reg.add(make_shared<DiskDevice>("sda", 2048));
reg.add(make_shared<NetworkDevice>("eth0","192.168.1.1",8080));
reg.get("kbd0")->open();
reg.get("sda" )->open();
reg.get("eth0")->open();
reg.listAll();
reg.get("kbd0")->read(5);
reg.get("sda" )->write("boot-sector-data");
reg.get("sda" )->read(0);
reg.get("eth0")->write("GET / HTTP/1.1");
reg.get("eth0")->read(512);
reg.get("kbd0")->close();
reg.get("sda" )->close();
reg.get("eth0")->close();
return 0;
}

