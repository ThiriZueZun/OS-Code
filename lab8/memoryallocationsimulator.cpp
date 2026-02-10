#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
using namespace std;
struct MemoryBlock {
int startAddress;
int size;
bool isFree;
int processID;
MemoryBlock(int start, int s, bool free = true, int pid = -1)
: startAddress(start), size(s), isFree(free), processID(pid) {}
};
class MemoryManager {
private:
vector<MemoryBlock> blocks;
const int TOTAL_MEMORY = 1048576; // 1MB
void mergeAdjacentFreeBlocks() {
for (size_t i = 0; i < blocks.size() - 1; ) {
if (blocks[i].isFree && blocks[i + 1].isFree) {
// Merge blocks
blocks[i].size += blocks[i + 1].size;
blocks.erase(blocks.begin() + i + 1);
} else {
i++;
}
}
}
public:
MemoryManager() {
// Initialize with one large free block
blocks.push_back(MemoryBlock(0, TOTAL_MEMORY, true));
}
bool allocateFirstFit(int processID, int size) {
cout << "\n--- First-Fit Allocation ---" << endl;
cout << "Process " << processID << " requests " << size << " bytes" << endl;
// Find first block that fits
for (size_t i = 0; i < blocks.size(); i++) {
if (blocks[i].isFree && blocks[i].size >= size) {
// Found suitable block
cout << "Found free block at address " << blocks[i].startAddress
<< " with size " << blocks[i].size << endl;
// Allocate from this block
if (blocks[i].size > size) {
// Split the block

int newBlockStart = blocks[i].startAddress + size;
int newBlockSize = blocks[i].size - size;
blocks[i].size = size;
blocks[i].isFree = false;
blocks[i].processID = processID;
// Insert free block after allocated block
blocks.insert(blocks.begin() + i + 1,
MemoryBlock(newBlockStart, newBlockSize, true));
} else {
// Use entire block
blocks[i].isFree = false;
blocks[i].processID = processID;
}
cout << "Allocated successfully at address "
<< blocks[i].startAddress << endl;
return true;
}
}
cout << "Allocation FAILED: No suitable block found" << endl;
return false;
}
bool allocateBestFit(int processID, int size) {
cout << "\n--- Best-Fit Allocation ---" << endl;
cout << "Process " << processID << " requests " << size << " bytes" << endl;
int bestIndex = -1;
int bestSize = TOTAL_MEMORY + 1;
// Find smallest block that fits
for (size_t i = 0; i < blocks.size(); i++) {
if (blocks[i].isFree && blocks[i].size >= size) {
if (blocks[i].size < bestSize) {
bestSize = blocks[i].size;
bestIndex = i;
}
}
}
if (bestIndex == -1) {
cout << "Allocation FAILED: No suitable block found" << endl;
return false;
}
cout << "Found best-fit block at address " << blocks[bestIndex].startAddress
<< " with size " << blocks[bestIndex].size << endl;
// Allocate from best-fit block
if (blocks[bestIndex].size > size) {
// Split the block
int newBlockStart = blocks[bestIndex].startAddress + size;
int newBlockSize = blocks[bestIndex].size - size;
blocks[bestIndex].size = size;
blocks[bestIndex].isFree = false;
blocks[bestIndex].processID = processID;
blocks.insert(blocks.begin() + bestIndex + 1,
MemoryBlock(newBlockStart, newBlockSize, true));
} else {
blocks[bestIndex].isFree = false;
blocks[bestIndex].processID = processID;

}
cout << "Allocated successfully at address "
<< blocks[bestIndex].startAddress << endl;
return true;
}
bool allocateWorstFit(int processID, int size) {
cout << "\n--- Worst-Fit Allocation ---" << endl;
cout << "Process " << processID << " requests " << size << " bytes" << endl;
int worstIndex = -1;
int worstSize = -1;
// Find largest block
for (size_t i = 0; i < blocks.size(); i++) {
if (blocks[i].isFree && blocks[i].size >= size) {
if (blocks[i].size > worstSize) {
worstSize = blocks[i].size;
worstIndex = i;
}
}
}
if (worstIndex == -1) {
cout << "Allocation FAILED: No suitable block found" << endl;
return false;
}
cout << "Found worst-fit block at address " << blocks[worstIndex].startAddress
<< " with size " << blocks[worstIndex].size << endl;
// Allocate from worst-fit block
if (blocks[worstIndex].size > size) {
// Split the block
int newBlockStart = blocks[worstIndex].startAddress + size;
int newBlockSize = blocks[worstIndex].size - size;
blocks[worstIndex].size = size;
blocks[worstIndex].isFree = false;
blocks[worstIndex].processID = processID;
blocks.insert(blocks.begin() + worstIndex + 1,
MemoryBlock(newBlockStart, newBlockSize, true));
} else {
blocks[worstIndex].isFree = false;
blocks[worstIndex].processID = processID;
}
cout << "Allocated successfully at address "
<< blocks[worstIndex].startAddress << endl;
return true;
}
void deallocate(int processID) {
cout << "\n--- Deallocation ---" << endl;
cout << "Freeing memory for Process " << processID << endl;
bool found = false;
for (size_t i = 0; i < blocks.size(); i++) {
if (!blocks[i].isFree && blocks[i].processID == processID) {
cout << "Freed block at address " << blocks[i].startAddress
<< " with size " << blocks[i].size << endl;
blocks[i].isFree = true;
blocks[i].processID = -1;
found = true;

}
}
if (!found) {
cout << "Process " << processID << " not found in memory" << endl;
return;
}
// Merge adjacent free blocks
mergeAdjacentFreeBlocks();
cout << "Adjacent free blocks merged" << endl;
}
void displayMemory() {
cout << "\n=== MEMORY MAP ===" << endl;
cout << setw(12) << "Start Addr" << setw(10) << "Size"
<< setw(10) << "Status" << setw(12) << "Process ID" << endl;
cout << string(44, '-') << endl;
for (const auto& block : blocks) {
cout << setw(12) << block.startAddress
<< setw(10) << block.size
<< setw(10) << (block.isFree ? "FREE" : "USED")
<< setw(12) << (block.isFree ? "-" : to_string(block.processID))
<< endl;
}
}
void calculateFragmentation() {
int totalFreeSpace = 0;
int largestFreeBlock = 0;
int numFreeBlocks = 0;
int totalAllocatedSpace = 0;
int totalRequestedSpace = 0; // Would need to track this separately
for (const auto& block : blocks) {
if (block.isFree) {
totalFreeSpace += block.size;
numFreeBlocks++;
if (block.size > largestFreeBlock) {
largestFreeBlock = block.size;
}
} else {
totalAllocatedSpace += block.size;
}
}
cout << "\n=== FRAGMENTATION ANALYSIS ===" << endl;
cout << "Total Free Space: " << totalFreeSpace << " bytes" << endl;
cout << "Largest Free Block: " << largestFreeBlock << " bytes" << endl;
cout << "Number of Free Blocks: " << numFreeBlocks << endl;
cout << "Total Allocated Space: " << totalAllocatedSpace << " bytes" << endl;
// External Fragmentation: free space that cannot be used
int externalFrag = totalFreeSpace - largestFreeBlock;
cout << "External Fragmentation: " << externalFrag << " bytes" << endl;
double fragPercent = (totalFreeSpace > 0) ?
(double)externalFrag / totalFreeSpace * 100 : 0;
cout << "Fragmentation Percentage: " << fixed << setprecision(2)
<< fragPercent << "%" << endl;
}
};
int main() {
cout << "MEMORY ALLOCATION SIMULATOR" << endl;

cout << "===========================" << endl;
cout << "Total Memory: 1 MB (1048576 bytes)" << endl;
MemoryManager mm;
// Test First-Fit
cout << "\n\n========== TESTING FIRST-FIT ==========" << endl;
mm.allocateFirstFit(1, 200000);
mm.allocateFirstFit(2, 150000);
mm.allocateFirstFit(3, 300000);
mm.displayMemory();
mm.deallocate(2);
mm.displayMemory();
mm.allocateFirstFit(4, 100000);
mm.displayMemory();
mm.calculateFragmentation();
// Reset for Best-Fit test
cout << "\n\n========== TESTING BEST-FIT ==========" << endl;
MemoryManager mm2;
mm2.allocateBestFit(1, 200000);
mm2.allocateBestFit(2, 150000);
mm2.allocateBestFit(3, 300000);
mm2.displayMemory();
mm2.deallocate(2);
mm2.allocateBestFit(4, 100000);
mm2.displayMemory();
mm2.calculateFragmentation();
// Reset for Worst-Fit test
cout << "\n\n========== TESTING WORST-FIT ==========" << endl;
MemoryManager mm3;
mm3.allocateWorstFit(1, 200000);
mm3.allocateWorstFit(2, 150000);
mm3.allocateWorstFit(3, 300000);
mm3.displayMemory();
mm3.deallocate(2);
mm3.allocateWorstFit(4, 100000);
mm3.displayMemory();
mm3.calculateFragmentation();
return 0;
}