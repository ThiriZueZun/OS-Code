#include <iostream>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <iomanip>

using namespace std;

// Fix missing permission macros on Windows
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

string getPerms(mode_t mode) {

    string perm = "---------";

    if (mode & S_IRUSR) perm[0] = 'r';
    if (mode & S_IWUSR) perm[1] = 'w';
    if (mode & S_IXUSR) perm[2] = 'x';

    if (mode & S_IRGRP) perm[3] = 'r';
    if (mode & S_IWGRP) perm[4] = 'w';
    if (mode & S_IXGRP) perm[5] = 'x';

    if (mode & S_IROTH) perm[6] = 'r';
    if (mode & S_IWOTH) perm[7] = 'w';
    if (mode & S_IXOTH) perm[8] = 'x';

    return perm;
}

void listDir(const string& path) {

    DIR *dir;
    struct dirent *entry;

    dir = opendir(path.c_str());

    cout << left << setw(12) << "PERM"
         << setw(10) << "SIZE(B)"
         << "NAME" << endl;

    cout << "----------------------------------------" << endl;

    while ((entry = readdir(dir)) != NULL) {

        struct stat info;
        stat(entry->d_name, &info);

        cout << setw(12) << getPerms(info.st_mode)
             << setw(10) << info.st_size
             << entry->d_name
             << endl;
    }

    closedir(dir);
}

int main() {

    ofstream("file_a.txt") << "Hello World";
    ofstream("file_b.txt") << "OSC Chapter 13";

    cout << "=== Directory Listing ===" << endl;

    listDir(".");

    return 0;
}