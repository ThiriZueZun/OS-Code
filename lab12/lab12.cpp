#include <iostream>
#include <fstream>
#include <unistd.h>   // access()
#include <sys/stat.h>
#include <cerrno>
#include <cstring>
using namespace std;
 
void checkPermissions(const char* path) {
    cout << "Checking permissions for: " << path << endl;
 
    // access() checks real UID/GID permissions
    cout << "  Read    (R_OK): "
         << (access(path, R_OK) == 0 ? "ALLOWED" : "DENIED") << endl;
    cout << "  Write   (W_OK): "
         << (access(path, W_OK) == 0 ? "ALLOWED" : "DENIED") << endl;
    cout << "  Execute (X_OK): "
         << (access(path, X_OK) == 0 ? "ALLOWED" : "DENIED") << endl;
    cout << "  Exists  (F_OK): "
         << (access(path, F_OK) == 0 ? "YES" : "NO") << endl;
}
 
void safeOpen(const char* path) {
    if (access(path, R_OK) != 0) {
        cerr << "Permission denied: cannot read '" << path << "'" << endl;
        cerr << "Reason: " << strerror(errno) << endl;
        return;
    }
    ifstream f(path);
    cout << "File opened successfully." << endl;
}
 
int main() {
    // Create test file with write-only permissions
    ofstream f("test_perms.txt");
    f << "test content";
    f.close();
 
    chmod("test_perms.txt", S_IWUSR);  // write-only: no read
    checkPermissions("test_perms.txt");
    cout << endl;
    safeOpen("test_perms.txt");   // Should be denied
 
    chmod("test_perms.txt", S_IRUSR | S_IWUSR);  // restore
    return 0;
}
// access() vs open(): access() checks permissions WITHOUT opening;
// useful for pre-checks (though TOCTOU race conditions can occur)
