#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <bitset>
using namespace std;
const int NUM_PAGES = 64;
const int NUM_FRAMES = 32;
const int PAGE_SIZE = 1024;
const int OFFSET_BITS = 10; // log2(1024) = 10 bits for offset
const int PAGE_BITS = 6; // log2(64) = 6 bits for page number
class PageTable {
private:
int pageTable[NUM_PAGES];
bool valid[NUM_PAGES];
public:
PageTable() {
srand(time(0));
bool usedFrames[NUM_FRAMES] = {false};
// Initialize page table with random frame numbers
for (int i = 0; i < NUM_PAGES; i++) {
// 75% of pages are valid
if (rand() % 100 < 75) {
int frame;
// Find an unused frame
do {
frame = rand() % NUM_FRAMES;
} while (usedFrames[frame]);
pageTable[i] = frame;
usedFrames[frame] = true;
valid[i] = true;
} else {
pageTable[i] = -1;
valid[i] = false;
}
}
}
int translateAddress(int logicalAddress) {
// Extract page number (upper bits)
int pageNumber = logicalAddress / PAGE_SIZE;

// Extract offset (lower bits)
int offset = logicalAddress % PAGE_SIZE;
// Validate page number
if (pageNumber < 0 || pageNumber >= NUM_PAGES) {
cout << "Error: Invalid page number " << pageNumber << endl;
return -1;
}
// Check if page is valid
if (!valid[pageNumber]) {
cout << "Page Fault: Page " << pageNumber << "is not in memory" << endl;
return -1;
}
// Get frame number from page table
int frameNumber = pageTable[pageNumber];
// Calculate physical address
int physicalAddress = (frameNumber * PAGE_SIZE) + offset;
// Display translation details
cout << "Logical Address: " << logicalAddress << endl;
cout << "Page Number: " << pageNumber << endl;
cout << "Offset: " << offset << endl;
cout << "Frame Number: " << frameNumber << endl;
cout << "Physical Address: " << physicalAddress << endl;
return physicalAddress;
}
void displayPageTable() {
cout << "\n=== PAGE TABLE ===" << endl;
cout << setw(10) << "Page#" << setw(10) << "Frame#" << setw(10) << "Valid" << endl;
cout << string(30, '-') << endl;
for (int i = 0; i < NUM_PAGES; i++) {
cout << setw(10) << i
<< setw(10) << (valid[i] ? to_string(pageTable[i]) : "N/A")
<< setw(10) << (valid[i] ? "Yes" : "No") << endl;
}
}
};
int main() {
cout << "ADDRESS TRANSLATION SIMULATOR" << endl;
cout << "=============================" << endl;
cout << "Page Size: " << PAGE_SIZE << " bytes" << endl;
cout << "Number of Pages: " << NUM_PAGES << endl;
cout << "Number of Frames: " << NUM_FRAMES << endl << endl;
PageTable pt;
// Test with various logical addresses
int testAddresses[] = {0, 1024, 2048, 5120, 10240, 65535};
for (int addr : testAddresses) {
cout << "\n--- Translation Test ---" << endl;
pt.translateAddress(addr);
}
// Display complete page table
pt.displayPageTable();
// Interactive mode
char choice;

cout << "\nEnter addresses interactively? (y/n): ";
cin >> choice;
if (choice == 'y' || choice == 'Y') {
int addr;
do {
cout << "\nEnter logical address (-1 to quit): ";
cin >> addr;
if (addr >= 0) {
pt.translateAddress(addr);
}
} while (addr >= 0);
}
return 0;
}