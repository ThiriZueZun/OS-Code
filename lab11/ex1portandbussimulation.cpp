#include <iostream>
#include <bitset>
#include <thread>
#include <chrono>
#include <queue>
#include <functional>
#include <mutex>
#include <string>

using namespace std;

// =============================
// 1. Status Register
// =============================
struct StatusRegister {
    bool busy;
    bool error;
    bool ready;
    bool transferComplete;

    StatusRegister() {
        busy = false;
        error = false;
        ready = true;  // เริ่มต้นพร้อมใช้งาน
        transferComplete = false;
    }
};

// =============================
// 2. IOPort Class
// =============================
class IOPort {
public:
    uint8_t dataRegister;
    StatusRegister status;
    uint8_t controlRegister;

    mutex mtx;

    IOPort() {
        dataRegister = 0;
        controlRegister = 0;
    }

    // Write Data
    void writeData(uint8_t data) {
        lock_guard<mutex> lock(mtx);

        if (status.busy) {
            cout << "ERROR: Device is busy!\n";
            status.error = true;
            return;
        }

        status.busy = true;
        status.ready = false;
        status.transferComplete = false;

        dataRegister = data;

        cout << "Writing data: " << (int)data << endl;

        // จำลองการทำงาน 2 วินาที
        this_thread::sleep_for(chrono::seconds(2));

        status.busy = false;
        status.ready = true;
        status.transferComplete = true;

        cout << "Transfer Complete!\n";
    }

    // Read Data
    uint8_t readData() {
        lock_guard<mutex> lock(mtx);
        return dataRegister;
    }

    // Poll Status
    bool pollStatus() {
        lock_guard<mutex> lock(mtx);
        return status.ready;
    }
};

// =============================
// 3. Device Controller
// =============================
class DeviceController {
private:
    IOPort port;
    string deviceName;
    queue<function<void()>> interruptQueue;
    mutex mtx;

public:
    DeviceController(string name) {
        deviceName = name;
    }

    // Polling I/O
    void pollingIO(uint8_t data) {
        cout << "[POLLING] Waiting for device ready...\n";

        while (!port.pollStatus()) {
            cout << "Device busy... waiting...\n";
            this_thread::sleep_for(chrono::milliseconds(500));
        }

        port.writeData(data);
        cout << "[POLLING] Data sent successfully!\n";
    }

    // Interrupt-driven I/O
    void interruptDrivenIO(uint8_t data, function<void()> callback) {
        cout << "[INTERRUPT] Sending data and registering handler...\n";

        thread t([=]() {
            port.writeData(data);

            lock_guard<mutex> lock(mtx);
            interruptQueue.push(callback);
        });

        t.detach(); // ไม่ block
    }

    void processInterrupts() {
        lock_guard<mutex> lock(mtx);

        while (!interruptQueue.empty()) {
            auto handler = interruptQueue.front();
            interruptQueue.pop();
            handler();  // เรียก callback
        }
    }
};

// =============================
// 4. MAIN
// =============================
int main() {

    cout << "=== I/O Hardware Simulation ===\n";

    DeviceController keyboard("Keyboard");
    DeviceController disk("Disk Drive");

    // ==========================
    // Polling Test
    // ==========================
    cout << "\n--- Polling I/O Test ---\n";
    keyboard.pollingIO(65);   // ASCII 'A'

    // ==========================
    // Interrupt Test
    // ==========================
    cout << "\n--- Interrupt-driven I/O Test ---\n";

    disk.interruptDrivenIO(100, []() {
        cout << "[INTERRUPT HANDLER] Disk transfer finished!\n";
    });

    // รอให้อุปกรณ์ทำงานเสร็จ
    this_thread::sleep_for(chrono::seconds(3));

    disk.processInterrupts();

    return 0;
}
