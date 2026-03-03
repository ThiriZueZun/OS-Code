#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <stdexcept>

using namespace std;

// =======================================
// 1. Memory Simulation
// =======================================
class Memory {
private:
    vector<uint8_t> data;
    size_t size;

public:
    Memory(size_t size) : size(size), data(size, 0) {}

    void write(size_t address, uint8_t value) {
        if (address >= size) {
            throw out_of_range("Memory write out of bounds!");
        }
        data[address] = value;
    }

    uint8_t read(size_t address) {
        if (address >= size) {
            throw out_of_range("Memory read out of bounds!");
        }
        return data[address];
    }

    void display(size_t start, size_t length) {
        if (start + length > size) {
            throw out_of_range("Memory display out of bounds!");
        }

        for (size_t i = 0; i < length; i++) {
            cout << (int)data[start + i] << " ";
        }
        cout << endl;
    }
};

// =======================================
// 2. Programmed I/O (CPU copies every byte)
// =======================================
class ProgrammedIO {
private:
    int cpuCyclesUsed;

public:
    ProgrammedIO() : cpuCyclesUsed(0) {}

    long long transfer(Memory& mem, size_t destAddr,
                       vector<uint8_t>& source, size_t size) {

        auto start = chrono::high_resolution_clock::now();

        cout << "[PIO] CPU transferring " << size << " bytes..." << endl;

        for (size_t i = 0; i < size; i++) {
            mem.write(destAddr + i, source[i]);
            cpuCyclesUsed++;  // CPU used for each byte
        }

        auto end = chrono::high_resolution_clock::now();
        auto duration =
            chrono::duration_cast<chrono::microseconds>(end - start).count();

        cout << "[PIO] Transfer complete! CPU cycles used: "
             << cpuCyclesUsed << endl;

        return duration;
    }
};

// =======================================
// 3. DMA Controller Simulation
// =======================================
class DMAController {
private:
    size_t sourceAddress;
    size_t destAddress;
    size_t transferCount;
    int cpuCyclesUsed;
    bool busy;

public:
    DMAController()
        : sourceAddress(0), destAddress(0),
          transferCount(0), cpuCyclesUsed(0), busy(false) {}

    void setup(size_t src, size_t dest, size_t count) {
        sourceAddress = src;
        destAddress = dest;
        transferCount = count;
        cpuCyclesUsed = 1; // CPU used once for setup
        busy = true;
    }

    long long transfer(Memory& mem, vector<uint8_t>& source) {

        auto start = chrono::high_resolution_clock::now();

        cout << "[DMA] Transferring " << transferCount
             << " bytes using DMA..." << endl;

        // DMA copies block without CPU per-byte involvement
        for (size_t i = 0; i < transferCount; i++) {
            mem.write(destAddress + i, source[sourceAddress + i]);
        }

        cpuCyclesUsed++; // interrupt notification

        busy = false;

        auto end = chrono::high_resolution_clock::now();
        auto duration =
            chrono::duration_cast<chrono::microseconds>(end - start).count();

        cout << "[DMA] Transfer complete! CPU cycles used: "
             << cpuCyclesUsed << endl;
        cout << "[DMA] CPU interrupt sent to notify completion" << endl;

        return duration;
    }
};

// =======================================
// 4. MAIN
// =======================================
int main() {

    cout << "=== DMA vs Programmed I/O Comparison ===" << endl;

    const size_t DATA_SIZE = 1024; // 1KB

    Memory memory(4096);
    vector<uint8_t> sourceData(DATA_SIZE);

    for (size_t i = 0; i < DATA_SIZE; i++) {
        sourceData[i] = static_cast<uint8_t>(i % 256);
    }

    // ==========================
    // Programmed I/O Test
    // ==========================
    cout << "\n=== Programmed I/O ===" << endl;
    ProgrammedIO pio;

    long long pioTime =
        pio.transfer(memory, 0, sourceData, DATA_SIZE);

    // ==========================
    // DMA Test
    // ==========================
    cout << "\n=== DMA Transfer ===" << endl;

    Memory memory2(4096);
    DMAController dma;

    dma.setup(0, 0, DATA_SIZE);

    long long dmaTime =
        dma.transfer(memory2, sourceData);

    // ==========================
    // Performance Comparison
    // ==========================
    cout << "\n=== Performance Comparison ===" << endl;
    cout << "PIO Time: " << pioTime << " microseconds" << endl;
    cout << "DMA Time: " << dmaTime << " microseconds" << endl;

    if (dmaTime < pioTime)
        cout << "DMA is more efficient (less CPU involvement)" << endl;
    else
        cout << "PIO appears faster in this run (depends on system)" << endl;

    return 0;
}
