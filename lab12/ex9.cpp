#include <iostream>
#include <fstream>
#include <string>

using namespace std;

// WRITER (simulated exclusive lock)
void writer(const char* filename, const char* data) {
    ofstream file(filename, ios::app);  // append mode

    if (!file) {
        cout << "Error opening file\n";
        return;
    }

    file << data << endl;
    file.close();

    cout << "Writer: wrote and released lock (simulated)" << endl;
}

// READER
void reader(const char* filename) {
    ifstream file(filename);

    if (!file) {
        cout << "Error opening file\n";
        return;
    }

    cout << "Reader output:" << endl;

    string line;
    while (getline(file, line)) {
        cout << line << endl;
    }

    file.close();
}

int main() {

    writer("shared.txt", "Process A data");
    writer("shared.txt", "Process B data");

    reader("shared.txt");

    return 0;
}