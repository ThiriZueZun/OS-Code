#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdio>

using namespace std;

// Fix for Windows compilers that don't define these
#ifndef S_IRGRP
#define S_IRGRP 00040
#endif
#ifndef S_IWGRP
#define S_IWGRP 00020
#endif
#ifndef S_IXGRP
#define S_IXGRP 00010
#endif

#ifndef S_IROTH
#define S_IROTH 00004
#endif
#ifndef S_IWOTH
#define S_IWOTH 00002
#endif
#ifndef S_IXOTH
#define S_IXOTH 00001
#endif

string permString(mode_t mode) {
    string perm = "---------";

    // Owner
    if (mode & S_IRUSR) perm[0] = 'r';
    if (mode & S_IWUSR) perm[1] = 'w';
    if (mode & S_IXUSR) perm[2] = 'x';

    // Group
    if (mode & S_IRGRP) perm[3] = 'r';
    if (mode & S_IWGRP) perm[4] = 'w';
    if (mode & S_IXGRP) perm[5] = 'x';

    // Others
    if (mode & S_IROTH) perm[6] = 'r';
    if (mode & S_IWOTH) perm[7] = 'w';
    if (mode & S_IXOTH) perm[8] = 'x';

    return perm;
}

int main() {

    const char* fname = "protected.txt";

    // (a) Create file
    ofstream f(fname);
    f << "Sensitive data";
    f.close();

    // (b) Set permissions
    if (chmod(fname, S_IRUSR | S_IWUSR) == -1) {
        perror("chmod");
        return 1;
    }

    // (c) Read permissions
    struct stat info;

    if (stat(fname, &info) == -1) {
        perror("stat");
        return 1;
    }

    cout << "Permissions for '" << fname << "': "
         << permString(info.st_mode & 0777) << endl;

    cout << "Octal: " << oct << (info.st_mode & 0777) << dec << endl;

    return 0;
}