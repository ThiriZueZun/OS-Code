#include <iostream>
#include <queue>
#include <unordered_set>
#include <vector>
using namespace std;

class PageReplacementFIFO {
private:
    int numFrames;
    queue<int> frameQueue;
    unordered_set<int> frameSet;
    int pageFaults;

public:
    PageReplacementFIFO(int frames) : numFrames(frames), pageFaults(0) {}

    // Implement page reference
    void referencePage(int page) {
        // If page not in memory → page fault
        if (frameSet.find(page) == frameSet.end()) {
            pageFaults++;

            // If frames are full, remove oldest page
            if (frameQueue.size() == numFrames) {
                int oldPage = frameQueue.front();
                frameQueue.pop();
                frameSet.erase(oldPage);
            }

            // Add new page
            frameQueue.push(page);
            frameSet.insert(page);
        }
    }

    // Simulate reference string
    void simulate(vector<int> referenceString) {
        cout << "Page Reference\tFrames\n";
        cout << "--------------------------\n";

        for (int page : referenceString) {
            referencePage(page);

            cout << page << "\t\t";
            displayFrames();
        }
    }

    // Display current frame contents
    void displayFrames() {
        queue<int> temp = frameQueue;
        while (!temp.empty()) {
            cout << temp.front() << " ";
            temp.pop();
        }
        cout << endl;
    }

    // Display results
    void displayResults() {
        cout << "\nTotal Page Faults: " << pageFaults << endl;
        cout << "Page Fault Rate: "
             << (double)pageFaults / (pageFaults + frameSet.size())
             << endl;
    }
};

int main() {
    int n;
    cout << "Enter number of page references: ";
    cin >> n;

    vector<int> referenceString(n);
    cout << "Enter reference string: ";
    for (int i = 0; i < n; i++) {
        cin >> referenceString[i];
    }

    PageReplacementFIFO fifo(3);
    fifo.simulate(referenceString);
    fifo.displayResults();

    return 0;
}
