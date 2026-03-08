#include <iostream>
#include <sys/mount.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

using namespace std;

int main() {

    const char* source = "/dev/sdb1";   // device
    const char* target = "/mnt/usb";    // mount directory
    const char* fstype = "vfat";        // filesystem type
    unsigned long flags = MS_RDONLY;   // read-only
    const char* options = "utf8";       // filesystem options

    cout << "Attempting to mount " << source
         << " at " << target << "..." << endl;

    int result = mount(source, target, fstype, flags, options);

    if (result == -1) {
        cerr << "mount() failed: " << strerror(errno) << endl;
        cerr << "This usually happens if you are not root or the device does not exist." << endl;
        return 1;
    }

    cout << "Mount successful!" << endl;

    return 0;
}