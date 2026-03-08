#include <iostream>
#include <dirent.h>
#include <sys/stat.h>
#include <string>

using namespace std;

void traverseDirectory(string dirPath, int depth = 0) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(dirPath.c_str());

    if (dir == NULL) {
        cout << string(depth * 2, ' ') << "[Cannot open directory]" << endl;
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        string name = entry->d_name;

        // Skip "." and ".."
        if (name == "." || name == "..")
            continue;

        string fullPath = dirPath + "/" + name;
        struct stat fileStat;

        stat(fullPath.c_str(), &fileStat);

        string indent(depth * 2, ' ');

        if (S_ISDIR(fileStat.st_mode)) {
            cout << indent << "[DIR]  " << name << "/" << endl;

            // Recursive call
            traverseDirectory(fullPath, depth + 1);
        }
        else if (S_ISREG(fileStat.st_mode)) {
            cout << indent << "[FILE] " << name
                 << " (" << fileStat.st_size << " B)" << endl;
        }
        else {
            cout << indent << "[OTHER] " << name << endl;
        }
    }

    closedir(dir);
}

int main() {
    traverseDirectory(".");
    return 0;
}