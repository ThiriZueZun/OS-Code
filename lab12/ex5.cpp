#include <iostream>
#include <dirent.h>   // for directory functions
#include <sys/stat.h> // for file size
#include <string>

using namespace std;

int main() {
    string dirPath = ".";   // current directory

    cout << "Directory listing for: " << dirPath << endl;
    cout << "--------------------------------------------------" << endl;

    DIR *dir;
    struct dirent *entry;

    dir = opendir(dirPath.c_str());

    if (dir == NULL) {
        cout << "Cannot open directory!" << endl;
        return 1;
    }

    while ((entry = readdir(dir)) != NULL) {

        string name = entry->d_name;
        struct stat fileStat;

        stat(name.c_str(), &fileStat);

        if (S_ISREG(fileStat.st_mode)) {
            cout << "[FILE] " << name
                 << " (" << fileStat.st_size << " bytes)" << endl;
        }
        else if (S_ISDIR(fileStat.st_mode)) {
            cout << "[DIR]  " << name << "/" << endl;
        }
        else {
            cout << "[OTHER] " << name << endl;
        }
    }

    closedir(dir);

    return 0;
}