#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstring>
using namespace std;
using hrc = chrono::high_resolution_clock;
// ■■ Metrics ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
struct Metrics {
string testName;
double throughputMBps;
double latencyMs;
double iops;
long totalOps;
long totalBytes;
void print() const {
cout << left << setw(28) << testName
<< right << fixed << setprecision(2)
<< setw(10) << throughputMBps << " MB/s"
<< setw(10) << latencyMs << " ms"
<< setw(12) << (long)iops << " IOPS\n";
}
};
// ■■ Simulated Disk ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
class SimDisk {
static const size_t SECT = 512;
vector<vector<uint8_t>> sectors;
size_t total, head = 0;
double seekMsPerTrack; // ms per 100 sectors
public:
SimDisk(size_t n, double seekMs=0.1)
: sectors(n, vector<uint8_t>(SECT,0)),
total(n), seekMsPerTrack(seekMs) {}
// Returns duration in seconds
double sequentialRead(size_t count) {
auto t0 = hrc::now();
for (size_t i=0; i<count && i<total; ++i) {
head = i;
volatile uint8_t x = sectors[i][0]; (void)x;
}
auto t1 = hrc::now();
return chrono::duration<double>(t1-t0).count();
}
double randomRead(size_t count) {
auto t0 = hrc::now();
srand(42);
for (size_t i=0; i<count; ++i) {
size_t tgt = rand() % total;
// simulate seek time (proportional to distance)
double seekSec = abs((long)tgt-(long)head)
* seekMsPerTrack / 100.0 / 1000.0;
// busy-wait for very small times in simulation
auto deadline = hrc::now()
+ chrono::duration<double>(seekSec*0.0001); // scaled
while(hrc::now() < deadline);
head = tgt;
volatile uint8_t x = sectors[tgt][0]; (void)x;
}
auto t1 = hrc::now();
return chrono::duration<double>(t1-t0).count();
}
size_t getSectSize() const { return SECT; }
};
// ■■ Buffer Benchmark ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
class BufferBench {
public:
static Metrics run(size_t bufSize, size_t totalBytes) {
vector<uint8_t> src(totalBytes), dst(totalBytes);
iota(src.begin(), src.end(), 0);
auto t0 = hrc::now();
long ops = 0;
for (size_t off=0; off<totalBytes; off+=bufSize) {
size_t n = min(bufSize, totalBytes-off);
memcpy(dst.data()+off, src.data()+off, n);
++ops;
}
double dur = chrono::duration<double>(hrc::now()-t0).count();
Metrics m;
m.testName = "Buffer " + to_string(bufSize/1024>0?bufSize/1024:bufSize)
+ (bufSize>=1024?" KB":" B");
m.totalOps = ops;
m.totalBytes = totalBytes;
m.throughputMBps= totalBytes / dur / (1024*1024);
m.latencyMs = dur / ops * 1000.0;
m.iops = ops / dur;
return m;
}
};
// ■■ I/O Scheduler ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
class IOScheduler {
public:
static long fcfs(vector<int> reqs, int start) {
long dist=0; int cur=start;
for (int r : reqs) { dist+=abs(r-cur); cur=r; }
return dist;
}
static long sstf(vector<int> reqs, int start) {
long dist=0; int cur=start;
vector<int> pending=reqs;
while (!pending.empty()) {
auto it = min_element(pending.begin(), pending.end(),
[&](int a,int b){ return abs(a-cur)<abs(b-cur); });
dist += abs(*it-cur); cur=*it;
pending.erase(it);
}
return dist;
}
};
// ■■ Performance Report ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
void printReport(const vector<Metrics>& results) {
cout << "\n" << string(64,'=') << "\n";
cout << " I/O PERFORMANCE REPORT\n";
cout << string(64,'=') << "\n";
cout << left << setw(28) << " Test"
<< right << setw(15) << "Throughput"
<< setw(13) << "Latency"
<< setw(13) << "IOPS" << "\n";
cout << string(64,'-') << "\n";
for (auto& m : results) m.print();
cout << string(64,'=') << "\n";
auto best = max_element(results.begin(),results.end(),
[](auto& a,auto& b){ return a.throughputMBps<b.throughputMBps; });
auto worst = min_element(results.begin(),results.end(),
[](auto& a,auto& b){ return a.throughputMBps<b.throughputMBps; });
cout << " Best: " << best->testName << " (" << fixed<<setprecision(1)
<< best->throughputMBps << " MB/s)\n";
cout << " Worst: " << worst->testName << " (" << fixed<<setprecision(1)
<< worst->throughputMBps << " MB/s)\n";
}
// ■■ main ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
int main() {
vector<Metrics> results;
const size_t TOTAL = 4 * 1024 * 1024; // 4 MB
const size_t OPS = 2000;
// Disk sequential vs random
SimDisk disk(10000);
{
double dur = disk.sequentialRead(OPS);
Metrics m; m.testName="Seq Read (disk)";
m.totalOps=OPS; m.totalBytes=OPS*512;
m.throughputMBps=m.totalBytes/dur/(1024*1024);
m.latencyMs=dur/OPS*1000; m.iops=OPS/dur;
results.push_back(m);
}
{
double dur = disk.randomRead(OPS);
Metrics m; m.testName="Rand Read (disk)";
m.totalOps=OPS; m.totalBytes=OPS*512;
m.throughputMBps=m.totalBytes/dur/(1024*1024);
m.latencyMs=dur/OPS*1000; m.iops=OPS/dur;
results.push_back(m);
}
// Buffer size sweep
for (size_t bs : {64UL,512UL,4096UL,65536UL,1048576UL})
results.push_back(BufferBench::run(bs, TOTAL));
printReport(results);
// Scheduling comparison
vector<int> requests = {98, 183, 37, 122, 14, 124, 65, 67};
int startHead = 53;
long fcfsDist = IOScheduler::fcfs(requests, startHead);
long sstfDist = IOScheduler::sstf(requests, startHead);
cout << "\n=== Disk Scheduling (head=" << startHead << ") ===\n";
cout << " Requests: 98 183 37 122 14 124 65 67\n";
cout << " FCFS total seek: " << fcfsDist << " tracks\n";
cout << " SSTF total seek: " << sstfDist << " tracks\n";
cout << " SSTF improvement: "
<< fixed<<setprecision(1) << 100.0*(fcfsDist-sstfDist)/fcfsDist
<< "% fewer tracks\n";
return 0;
}
