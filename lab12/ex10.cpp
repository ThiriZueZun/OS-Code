#include <iostream>
#include <fstream>
#include <string>

using namespace std;

#ifdef __linux__
#include <unistd.h>
#include <sys/stat.h>
#endif

int main() {

    // Create original file
    ofstream f("original.txt");
    f << "File sharing via links!";
    f.close();

#ifdef __linux__

    // Hard link
    link("original.txt", "hardlink.txt");

    // Symbolic link
    symlink("original.txt", "symlink.txt");

    struct stat s1, s2, s3;

    stat("original.txt", &s1);
    stat("hardlink.txt", &s2);
    lstat("symlink.txt", &s3);

    cout << "Original inode:  " << s1.st_ino << endl;
    cout << "Hard link inode: " << s2.st_ino
         << " (same? " << (s1.st_ino == s2.st_ino ? "YES" : "NO") << ")" << endl;

    cout << "Symlink inode:   " << s3.st_ino
         << " (same? " << (s1.st_ino == s3.st_ino ? "YES" : "NO") << ")" << endl;

    unlink("original.txt");

#else

    // Windows simulation
    cout << "Hard link created: hardlink.txt -> original.txt (simulation)" << endl;
    cout << "Symbolic link created: symlink.txt -> original.txt (simulation)" << endl;

    remove("original.txt");

#endif

    ifstream h("hardlink.txt");
    cout << "\nHard link after delete: "
         << (h.good() ? "ACCESSIBLE" : "BROKEN") << endl;

    ifstream sym("symlink.txt");
    cout << "Symbolic link after delete: "
         << (sym.good() ? "ACCESSIBLE" : "DANGLING/BROKEN") << endl;

    return 0;
}