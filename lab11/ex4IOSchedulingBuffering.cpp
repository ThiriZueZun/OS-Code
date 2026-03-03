#include <iostream>
#include <queue>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <map>
#include <mutex>
#include <thread>

using namespace std;

//============================================
// I/O Request Structure
//============================================
struct IORequest {
    enum class Type { READ, WRITE };
    enum class Priority { HIGH, MEDIUM, LOW };

    int requestId;
    Type type;
    Priority priority;
    size_t blockNumber;
    size_t dataSize;
    vector<uint8_t> data;

    chrono::time_point<chrono::steady_clock> arrivalTime;

    IORequest(int id, Type t, Priority p, size_t block, size_t size)
        : requestId(id), type(t), priority(p),
          blockNumber(block), dataSize(size) {
        arrivalTime = chrono::steady_clock::now();
        data.resize(size, 0);
    }

    string getTypeStr() const {
        return type == Type::READ ? "READ" : "WRITE";
    }

    string getPriorityStr() const {
        switch (priority) {
            case Priority::HIGH: return "HIGH";
            case Priority::MEDIUM: return "MEDIUM";
            case Priority::LOW: return "LOW";
        }
        return "UNKNOWN";
    }

    void display() const {
        cout << "Request[" << requestId << "] "
             << getTypeStr()
             << " Block:" << blockNumber
             << " Size:" << dataSize
             << " Priority:" << getPriorityStr()
             << endl;
    }
};

//============================================
// FCFS Scheduler
//============================================
class FCFSScheduler {
private:
    queue<IORequest> requestQueue;
    int processedCount;

public:
    FCFSScheduler() : processedCount(0) {}

    void addRequest(const IORequest& req) {
        requestQueue.push(req);
        cout << "[FCFS] Added: ";
        req.display();
    }

    bool processNext() {
        if (requestQueue.empty()) return false;

        IORequest req = requestQueue.front();
        requestQueue.pop();
        processedCount++;

        cout << "[FCFS] Processing: ";
        req.display();

        this_thread::sleep_for(chrono::milliseconds(100));
        return true;
    }

    void processAll() {
        cout << "\n[FCFS] Processing all requests...\n";
        while (processNext()) {}
        cout << "[FCFS] Total processed: "
             << processedCount << endl;
    }
};

//============================================
// Priority Scheduler
//============================================
class PriorityScheduler {
private:
    queue<IORequest> highPriority;
    queue<IORequest> mediumPriority;
    queue<IORequest> lowPriority;
    int processedCount;

public:
    PriorityScheduler() : processedCount(0) {}

    void addRequest(const IORequest& req) {
        switch (req.priority) {
            case IORequest::Priority::HIGH:
                highPriority.push(req);
                break;
            case IORequest::Priority::MEDIUM:
                mediumPriority.push(req);
                break;
            case IORequest::Priority::LOW:
                lowPriority.push(req);
                break;
        }

        cout << "[PRIORITY] Added: ";
        req.display();
    }

    bool processNext() {
        if (!highPriority.empty()) {
            cout << "[PRIORITY] Processing HIGH: ";
            highPriority.front().display();
            highPriority.pop();
        }
        else if (!mediumPriority.empty()) {
            cout << "[PRIORITY] Processing MEDIUM: ";
            mediumPriority.front().display();
            mediumPriority.pop();
        }
        else if (!lowPriority.empty()) {
            cout << "[PRIORITY] Processing LOW: ";
            lowPriority.front().display();
            lowPriority.pop();
        }
        else {
            return false;
        }

        processedCount++;
        this_thread::sleep_for(chrono::milliseconds(100));
        return true;
    }

    void processAll() {
        cout << "\n[PRIORITY] Processing all requests...\n";
        while (processNext()) {}
        cout << "[PRIORITY] Total processed: "
             << processedCount << endl;
    }
};

//============================================
// Circular Buffer
//============================================
class CircularBuffer {
private:
    vector<uint8_t> buffer;
    int head, tail;
    int capacity;
    int count;
    mutex bufMutex;

public:
    CircularBuffer(int size)
        : capacity(size), head(0), tail(0), count(0) {
        buffer.resize(size);
    }

    bool write(uint8_t data) {
        lock_guard<mutex> lock(bufMutex);
        if (isFull()) {
            cout << "[CircBuf] Buffer full! Dropping data.\n";
            return false;
        }

        buffer[tail] = data;
        tail = (tail + 1) % capacity;
        count++;
        return true;
    }

    bool read(uint8_t& data) {
        lock_guard<mutex> lock(bufMutex);
        if (isEmpty()) return false;

        data = buffer[head];
        head = (head + 1) % capacity;
        count--;
        return true;
    }

    bool isFull() { return count == capacity; }
    bool isEmpty() { return count == 0; }

    void displayStatus() {
        cout << "CircularBuffer: ["
             << count << "/" << capacity << "] "
             << (isEmpty() ? "EMPTY" :
                isFull() ? "FULL" : "OK")
             << endl;
    }
};

//============================================
// I/O Cache (LRU)
//============================================
class IOCache {
private:
    struct CacheEntry {
        vector<uint8_t> data;
        int accessCount;
        bool dirty;
        chrono::time_point<chrono::steady_clock> lastAccess;

        CacheEntry(const vector<uint8_t>& d)
            : data(d), accessCount(1), dirty(false) {
            lastAccess = chrono::steady_clock::now();
        }
    };

    map<size_t, CacheEntry> cache;
    size_t maxEntries;
    int hits, misses;

public:
    IOCache(size_t maxSize)
        : maxEntries(maxSize), hits(0), misses(0) {}

    bool lookup(size_t blockNum, vector<uint8_t>& data) {
        auto it = cache.find(blockNum);
        if (it != cache.end()) {
            hits++;
            it->second.accessCount++;
            it->second.lastAccess = chrono::steady_clock::now();
            data = it->second.data;
            cout << "[Cache] HIT block " << blockNum << endl;
            return true;
        }

        misses++;
        cout << "[Cache] MISS block " << blockNum << endl;
        return false;
    }

    void insert(size_t blockNum, const vector<uint8_t>& data) {
        if (cache.size() >= maxEntries)
            evict();

        cache.emplace(blockNum, CacheEntry(data));
        cout << "[Cache] Inserted block "
             << blockNum << endl;
    }

    void evict() {
        auto oldest = cache.begin();
        for (auto it = cache.begin(); it != cache.end(); ++it) {
            if (it->second.lastAccess <
                oldest->second.lastAccess)
                oldest = it;
        }

        cout << "[Cache] Evicting block "
             << oldest->first << " (LRU)\n";
        cache.erase(oldest);
    }

    double getHitRate() {
        int total = hits + misses;
        return total > 0 ?
               (double)hits / total * 100.0 : 0.0;
    }

    void displayStats() {
        cout << "\n=== Cache Statistics ===\n";
        cout << "Hits: " << hits << endl;
        cout << "Misses: " << misses << endl;
        cout << "Hit Rate: "
             << getHitRate() << "%\n";
        cout << "Cached Blocks: "
             << cache.size() << "/"
             << maxEntries << endl;
    }
};

//============================================
// MAIN
//============================================
int main() {
    cout << "=== Kernel I/O Subsystem Simulation ===\n";

    vector<IORequest> requests = {
        {1, IORequest::Type::READ, IORequest::Priority::LOW, 5, 512},
        {2, IORequest::Type::WRITE, IORequest::Priority::HIGH, 1, 512},
        {3, IORequest::Type::READ, IORequest::Priority::MEDIUM, 3, 1024},
        {4, IORequest::Type::WRITE, IORequest::Priority::HIGH, 2, 256},
        {5, IORequest::Type::READ, IORequest::Priority::LOW, 8, 512},
        {6, IORequest::Type::READ, IORequest::Priority::MEDIUM, 4, 512},
    };

    cout << "\n--- FCFS Scheduler ---\n";
    FCFSScheduler fcfs;
    for (auto& r : requests) fcfs.addRequest(r);
    fcfs.processAll();

    cout << "\n--- Priority Scheduler ---\n";
    PriorityScheduler prio;
    for (auto& r : requests) prio.addRequest(r);
    prio.processAll();

    cout << "\n--- Circular Buffer ---\n";
    CircularBuffer buf(5);
    for (int i = 0; i < 7; i++)
        buf.write(i * 10);
    buf.displayStatus();

    uint8_t val;
    while (buf.read(val))
        cout << "Read: " << (int)val << endl;
    buf.displayStatus();

    cout << "\n--- I/O Cache ---\n";
    IOCache cache(3);
    vector<uint8_t> block(512, 0xAA);
    vector<uint8_t> result;

    cache.lookup(1, result);
    cache.insert(1, block);
    cache.lookup(1, result);

    cache.lookup(2, result);
    cache.insert(2, block);

    cache.lookup(3, result);
    cache.insert(3, block);

    cache.lookup(4, result);
    cache.insert(4, block);

    cache.displayStats();

    return 0;
}
