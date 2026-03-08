#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
using namespace std;
 
struct MountEntry {
    string device, mountPoint, fsType, options;
};
 
int main() {
    ifstream mountFile("/proc/mounts");  // Linux mount table
    if (!mountFile.is_open()) {
        cerr << "Cannot open /proc/mounts" << endl;
        return 1;
    }
    cout << left << setw(20) << "DEVICE"
         << setw(25) << "MOUNT POINT"
         << setw(12) << "TYPE"
         << "OPTIONS" << endl;
    cout << string(75, '=') << endl;
 
    string line;
    while (getline(mountFile, line)) {
        istringstream iss(line);
        MountEntry e;
        int dumpFreq, passNo;
        iss >> e.device >> e.mountPoint >> e.fsType >> e.options
            >> dumpFreq >> passNo;
        cout << left << setw(20) << e.device
             << setw(25) << e.mountPoint
             << setw(12) << e.fsType
             << e.options << endl;
    }
    mountFile.close();
    return 0;
}
