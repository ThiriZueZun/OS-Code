#include <iostream>
#include <cstring>

using namespace std;

#ifdef __linux__
#include <sys/mount.h>
#include <cerrno>
#endif

int main() {

    const char* source = "/dev/sdb1";
    const char* target = "/mnt/usb";
    const char* fstype = "vfat";
    const char* options = "utf8";

#ifdef __linux__
    unsigned long flags = MS_RDONLY;

    cout << "Attempting to mount " << source
         << " at " << target << "..." << endl;

    if (mount(source, target, fstype, flags, options) == -1) {
        cerr << "mount() failed: " << strerror(errno) << endl;
        cerr << "(This is expected without root or real device)" << endl;
        return 1;
    }

    cout << "Mount successful!" << endl;

#else

    // Windows / unsupported OS
    cout << "Mount operation simulation\n";
    cout << "Source      : " << source << endl;
    cout << "Target      : " << target << endl;
    cout << "Filesystem  : " << fstype << endl;
    cout << "Options     : " << options << endl;
    cout << "Mount successful! (simulation)" << endl;

#endif

    return 0;
}