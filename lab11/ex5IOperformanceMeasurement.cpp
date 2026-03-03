#include <iostream>
#include <vector>
#include <chrono>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>

using namespace std;

//============================================
// Performance Metrics
//============================================
struct IOMetrics {
    string testName;
    double throughput;
    double latency;
    double iops;
    long long totalBytes;
    int totalOperations;
    double duration;

    void display() {
        cout << "\n=== " << testName << " ===\n";
        cout << fixed << setprecision(2);
        cout << "Throughput: " << throughput /1024 /1024 << " MB/s\n";
        cout << "Latency: " << latency << " ms\n";
        cout << "IOPS: " << iops << " ops/sec\n";
        cout << "Total Data: " << totalBytes /1024 << " KB\n";
        cout << "Operations: " << totalOperations << endl;
        cout << "Duration: " << duration << " sec\n";
    }
};

//============================================
// Simulated Disk
//============================================
class SimulatedDisk {
private:
    vector<vector<uint8_t>> sectors;
    size_t sectorSize;
    size_t numSectors;
    int currentHead;

public:
    SimulatedDisk(size_t sectorSz, size_t numSect)
        : sectorSize(sectorSz), numSectors(numSect), currentHead(0) {
        sectors.resize(numSectors, vector<uint8_t>(sectorSize, 0));
    }

    double sequentialWrite(size_t numOps) {
        auto start = chrono::high_resolution_clock::now();
        for (size_t i = 0; i < numOps && i < numSectors; i++) {
            currentHead = i;
            sectors[i] = vector<uint8_t>(sectorSize, i % 256);
        }
        auto end = chrono::high_resolution_clock::now();
        return chrono::duration<double>(end - start).count();
    }

    double sequentialRead(size_t numOps) {
        auto start = chrono::high_resolution_clock::now();
        for (size_t i = 0; i < numOps && i < numSectors; i++) {
            currentHead = i;
            volatile uint8_t dummy = sectors[i][0];
            (void)dummy;
        }
        auto end = chrono::high_resolution_clock::now();
        return chrono::duration<double>(end - start).count();
    }

    double randomWrite(size_t numOps) {
        auto start = chrono::high_resolution_clock::now();
        srand(42);
        for (size_t i = 0; i < numOps; i++) {
            size_t target = rand() % numSectors;
            currentHead = target;
            sectors[target] = vector<uint8_t>(sectorSize, i % 256);
        }
        auto end = chrono::high_resolution_clock::now();
        return chrono::duration<double>(end - start).count();
    }

    double randomRead(size_t numOps) {
        auto start = chrono::high_resolution_clock::now();
        srand(42);
        for (size_t i = 0; i < numOps; i++) {
            size_t target = rand() % numSectors;
            currentHead = target;
            volatile uint8_t dummy = sectors[target][0];
            (void)dummy;
        }
        auto end = chrono::high_resolution_clock::now();
        return chrono::duration<double>(end - start).count();
    }

    size_t getSectorSize() { return sectorSize; }
};

//============================================
// Performance Report
//============================================
class PerformanceReport {
private:
    vector<IOMetrics> results;

public:
    void addResult(IOMetrics m) {
        results.push_back(m);
    }

    void generateReport() {
        cout << "\n================ I/O PERFORMANCE REPORT ================\n";
        cout << left << setw(20) << "Test"
             << right << setw(12) << "MB/s"
             << setw(15) << "Latency(ms)"
             << setw(12) << "IOPS\n";
        cout << "--------------------------------------------------------\n";

        for (auto& m : results) {
            cout << left << setw(20) << m.testName
                 << right << fixed << setprecision(2)
                 << setw(12) << m.throughput /1024 /1024
                 << setw(15) << m.latency
                 << setw(12) << (int)m.iops << endl;
        }
    }

    void analyzeBestWorst() {
        if (results.empty()) return;

        auto best = max_element(results.begin(), results.end(),
            [](const IOMetrics& a, const IOMetrics& b) {
                return a.throughput < b.throughput;
            });

        auto worst = min_element(results.begin(), results.end(),
            [](const IOMetrics& a, const IOMetrics& b) {
                return a.throughput < b.throughput;
            });

        cout << "\nBest Throughput: " << best->testName
             << " (" << best->throughput /1024 /1024 << " MB/s)\n";
        cout << "Worst Throughput: " << worst->testName
             << " (" << worst->throughput /1024 /1024 << " MB/s)\n";
    }
};

//============================================
// MAIN
//============================================
int main() {
    cout << "=== I/O Performance Measurement ===\n";

    PerformanceReport report;
    SimulatedDisk disk(512, 10000);
    const size_t NUM_OPS = 5000;

    // Sequential Write
    double t1 = disk.sequentialWrite(NUM_OPS);
    IOMetrics m1;
    m1.testName = "Seq Write";
    m1.totalBytes = NUM_OPS * disk.getSectorSize();
    m1.totalOperations = NUM_OPS;
    m1.duration = t1;
    m1.throughput = m1.totalBytes / t1;
    m1.latency = (t1 / NUM_OPS) * 1000;
    m1.iops = NUM_OPS / t1;
    report.addResult(m1);

    // Sequential Read
    double t2 = disk.sequentialRead(NUM_OPS);
    IOMetrics m2;
    m2.testName = "Seq Read";
    m2.totalBytes = NUM_OPS * disk.getSectorSize();
    m2.totalOperations = NUM_OPS;
    m2.duration = t2;
    m2.throughput = m2.totalBytes / t2;
    m2.latency = (t2 / NUM_OPS) * 1000;
    m2.iops = NUM_OPS / t2;
    report.addResult(m2);

    // Random Write
    double t3 = disk.randomWrite(NUM_OPS);
    IOMetrics m3;
    m3.testName = "Rand Write";
    m3.totalBytes = NUM_OPS * disk.getSectorSize();
    m3.totalOperations = NUM_OPS;
    m3.duration = t3;
    m3.throughput = m3.totalBytes / t3;
    m3.latency = (t3 / NUM_OPS) * 1000;
    m3.iops = NUM_OPS / t3;
    report.addResult(m3);

    // Random Read
    double t4 = disk.randomRead(NUM_OPS);
    IOMetrics m4;
    m4.testName = "Rand Read";
    m4.totalBytes = NUM_OPS * disk.getSectorSize();
    m4.totalOperations = NUM_OPS;
    m4.duration = t4;
    m4.throughput = m4.totalBytes / t4;
    m4.latency = (t4 / NUM_OPS) * 1000;
    m4.iops = NUM_OPS / t4;
    report.addResult(m4);

    // Show results
    m1.display();
    m2.display();
    m3.display();
    m4.display();

    report.generateReport();
    report.analyzeBestWorst();

    return 0;
}
