#include <iostream>
#include <queue>
#include <map>
#include <vector>
#include <mutex>
#include <string>
#include <chrono>
#include <algorithm>
using namespace std;
using Clock = chrono::steady_clock;
// ■■■■■■■■■■■■■■■■■■■ I/O REQUEST ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
struct IORequest {
enum class Priority { HIGH, MEDIUM, LOW };
int id;
string op; // "READ" or "WRITE"
size_t block;
Priority priority;
void print() const {
string ps = priority==Priority::HIGH?"HIGH":
priority==Priority::MEDIUM?"MED":"LOW";
cout<<" [IOReq #"<<id<<"] "<<op<<" blk="<<block<<" pri="<<ps<<"\n";
}
};
// ■■■■■■■■■■■■■■■■■■■ FCFS SCHEDULER ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
class FCFSScheduler {
queue<IORequest> q;
int done = 0;
public:
void submit(IORequest r) { q.push(r); }
bool dispatch() {
if (q.empty()) return false;
cout << "[FCFS] dispatch:"; q.front().print(); q.pop(); ++done;
return true;
}
void runAll() { cout<<"\n[FCFS] Processing...\n"; while(dispatch());
cout<<"[FCFS] done="<<done<<"\n"; }
};
// ■■■■■■■■■■■■■■■■■■■ PRIORITY SCHEDULER ■■■■■■■■■■■■■■■■■■■■■■■■■
class PriorityScheduler {
queue<IORequest> high, med, low;
int done = 0;
public:
void submit(IORequest r) {
switch(r.priority) {
case IORequest::Priority::HIGH: high.push(r); break;
case IORequest::Priority::MEDIUM: med.push(r); break;
case IORequest::Priority::LOW: low.push(r); break;
}
}
bool dispatch() {
auto pop = [&](queue<IORequest>& q, const string& tag) {
cout<<"[PRIO/"<<tag<<"] dispatch:"; q.front().print(); q.pop(); ++done;
};
if (!high.empty()) { pop(high,"HIGH"); return true; }
if (!med.empty()) { pop(med, "MED" ); return true; }
if (!low.empty()) { pop(low, "LOW" ); return true; }
return false;
}
void runAll() { cout<<"\n[PRIO] Processing...\n"; while(dispatch());
cout<<"[PRIO] done="<<done<<"\n"; }
};
// ■■■■■■■■■■■■■■■■■■■ CIRCULAR BUFFER ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
template<typename T>
class CircularBuffer {
vector<T> buf;
int head=0, tail=0, count_=0;
int cap;
mutex mtx;
public:
explicit CircularBuffer(int c) : buf(c), cap(c) {}
bool write(const T& val) {
lock_guard<mutex> lk(mtx);
if (count_==cap) { cout<<"[CBUF] FULL — drop\n"; return false; }
buf[tail]=(val); tail=(tail+1)%cap; ++count_;
cout<<"[CBUF] wrote, count="<<count_<<"/"<<cap<<"\n";
return true;
}
bool read(T& val) {
lock_guard<mutex> lk(mtx);
if (!count_) return false;
val=buf[head]; head=(head+1)%cap; --count_;
cout<<"[CBUF] read, count="<<count_<<"/"<<cap<<"\n";
return true;
}
int size() const { return count_; }
bool isEmpty() const { return count_==0; }
bool isFull() const { return count_==cap; }
};
// ■■■■■■■■■■■■■■■■■■■ LRU CACHE ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
class LRUCache {
struct Entry {
vector<uint8_t> data;
bool dirty = false;
chrono::time_point<Clock> lastUsed;
};
map<size_t,Entry> cache;
size_t cap;
int hits=0, misses=0;
void evict() {
auto oldest = min_element(cache.begin(), cache.end(),
[](auto& a,auto& b){ return a.second.lastUsed < b.second.lastUsed; });
cout<<"[CACHE] evict blk="<<oldest->first
<<(oldest->second.dirty ? " (flush dirty)" : "")<<"\n";
cache.erase(oldest);
}
public:
explicit LRUCache(size_t c) : cap(c) {}
bool lookup(size_t blk, vector<uint8_t>& out) {
if (auto it=cache.find(blk); it!=cache.end()) {
it->second.lastUsed = Clock::now(); out = it->second.data;
cout<<"[CACHE] HIT blk="<<blk<<"\n"; ++hits; return true;
}
cout<<"[CACHE] MISS blk="<<blk<<"\n"; ++misses; return false;
}
void insert(size_t blk, const vector<uint8_t>& data, bool dirty=false) {
if (cache.size()>=cap) evict();
cache[blk] = {data, dirty, Clock::now()};
cout<<"[CACHE] insert blk="<<blk<<"\n";
}
void markDirty(size_t blk) {
if (cache.count(blk)) cache[blk].dirty=true;
}
void stats() const {
int total=hits+misses;
cout<<"[CACHE] hits="<<hits<<" misses="<<misses
<<" ratio="<<(total?100*hits/total:0)<<"%\n";
}
};
// ■■ main ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
int main() {
cout << "=== Kernel I/O Subsystem Demo ===\n";
// ■■ I/O Scheduling
using P = IORequest::Priority;
vector<IORequest> reqs = {
{1,"READ",5,P::LOW}, {2,"WRITE",1,P::HIGH},
{3,"READ",3,P::MEDIUM},{4,"WRITE",2,P::HIGH},
{5,"READ",8,P::LOW}, {6,"READ",4,P::MEDIUM},
};
FCFSScheduler fcfs;
PriorityScheduler prio;
for (auto& r : reqs) { fcfs.submit(r); prio.submit(r); }
fcfs.runAll();
prio.runAll();
// ■■ Circular Buffer
cout << "\n=== Circular Buffer (cap=4) ===\n";
CircularBuffer<int> cb(4);
for (int i=1; i<=6; i++) cb.write(i*10);
int v;
while (cb.read(v)) cout<<" val="<<v<<"\n";
// ■■ LRU Cache
cout << "\n=== LRU Cache (cap=3) ===\n";
LRUCache cache(3);
vector<uint8_t> blk(512, 0xAA);
vector<uint8_t> tmp;
cache.lookup(1,tmp); cache.insert(1,blk);
cache.lookup(2,tmp); cache.insert(2,blk);
cache.lookup(3,tmp); cache.insert(3,blk);
cache.lookup(1,tmp); // HIT
cache.lookup(4,tmp); cache.insert(4,blk); // evicts LRU
cache.stats();
return 0;
}
