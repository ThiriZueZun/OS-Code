#include <iostream>
#include <list>
#include <unordered_map>
#include <iomanip>
using namespace std;
const int TLB_SIZE = 8;
class TLB {
private:
struct TLBEntry {
int pageNumber;
int frameNumber;
TLBEntry(int p, int f) : pageNumber(p), frameNumber(f) {}
};
list<TLBEntry> tlbCache; // Most recently used at front
unordered_map<int, list<TLBEntry>::iterator> tlbMap;
int hits;
int misses;
public:
TLB() : hits(0), misses(0) {}
int lookup(int pageNumber) {
// Search for page in TLB
auto it = tlbMap.find(pageNumber);
if (it != tlbMap.end()) {
// TLB Hit!
hits++;
int frameNumber = it->second->frameNumber;
// Move to front (LRU - most recently used)
tlbCache.splice(tlbCache.begin(), tlbCache, it->second);
cout << "TLB HIT: Page " << pageNumber
<< " -> Frame " << frameNumber << endl;
return frameNumber;
} else {
// TLB Miss
misses++;

cout << "TLB MISS: Page " << pageNumber << " not in TLB" << endl;
return -1;
}
}
void insert(int pageNumber, int frameNumber) {
// Check if already in TLB
if (tlbMap.find(pageNumber) != tlbMap.end()) {
// Update existing entry and move to front
auto it = tlbMap[pageNumber];
it->frameNumber = frameNumber;
tlbCache.splice(tlbCache.begin(), tlbCache, it);
return;
}
// If TLB is full, remove LRU entry (back of list)
if (tlbCache.size() >= TLB_SIZE) {
int removedPage = tlbCache.back().pageNumber;
cout << "TLB FULL: Removing page " << removedPage << " (LRU)" << endl;
tlbMap.erase(removedPage);
tlbCache.pop_back();
}
// Add new entry to front
tlbCache.push_front(TLBEntry(pageNumber, frameNumber));
tlbMap[pageNumber] = tlbCache.begin();
cout << "TLB INSERT: Page " << pageNumber
<< " -> Frame " << frameNumber << endl;
}
void displayTLB() {
cout << "\n=== TLB Contents (MRU to LRU) ===" << endl;
cout << setw(10) << "Page#" << setw(10) << "Frame#" << endl;
cout << string(20, '-') << endl;
for (const auto& entry : tlbCache) {
cout << setw(10) << entry.pageNumber
<< setw(10) << entry.frameNumber << endl;
}
}
void displayStats() {
int total = hits + misses;
double hitRatio = (total > 0) ? (double)hits / total * 100 : 0;
cout << "\n=== TLB STATISTICS ===" << endl;
cout << "Total Accesses: " << total << endl;
cout << "Hits: " << hits << endl;
cout << "Misses: " << misses << endl;
cout << "Hit Ratio: " << fixed << setprecision(2)
<< hitRatio << "%" << endl;
}
void reset() {
hits = 0;
misses = 0;
}
};
// Combined with Page Table
class MemorySystem {
private:
TLB tlb;
unordered_map<int, int> pageTable; // page -> frame mapping

public:
MemorySystem() {
// Initialize some page table entries
pageTable[0] = 5;
pageTable[1] = 2;
pageTable[2] = 8;
pageTable[3] = 1;
pageTable[4] = 9;
pageTable[5] = 3;
pageTable[6] = 7;
pageTable[7] = 4;
}
int translateAddress(int pageNumber) {
cout << "\n--- Translating Page " << pageNumber << " ---" << endl;
// First check TLB
int frame = tlb.lookup(pageNumber);
if (frame == -1) {
// TLB miss - check page table
if (pageTable.find(pageNumber) != pageTable.end()) {
frame = pageTable[pageNumber];
cout << "Page Table Lookup: Page " << pageNumber
<< " -> Frame " << frame << endl;
// Insert into TLB
tlb.insert(pageNumber, frame);
} else {
cout << "PAGE FAULT: Page " << pageNumber
<< " not in memory!" << endl;
return -1;
}
}
return frame;
}
void displayStatus() {
tlb.displayTLB();
tlb.displayStats();
}
};
int main() {
cout << "TLB SIMULATION" << endl;
cout << "==============" << endl;
cout << "TLB Size: " << TLB_SIZE << " entries" << endl;
cout << "Replacement Policy: LRU" << endl << endl;
MemorySystem memSys;
// Simulate page references
int referenceString[] = {0, 1, 2, 3, 0, 1, 4, 0, 1, 2, 3, 5, 6, 7, 0, 1};
int numReferences = sizeof(referenceString) / sizeof(referenceString[0]);
cout << "Reference String: ";
for (int i = 0; i << numReferences; i++) {
cout << referenceString[i] << " ";
}
cout << "\n" << endl;
// Process each reference
for (int i = 0; i << numReferences; i++) {
memSys.translateAddress(referenceString[i]);
}

// Display final status
memSys.displayStatus();
return 0;
}