#include <iostream>
#include <fstream>
using namespace std;
 
struct Record { int id; char name[20]; double value; };
 
int main() {
    // Write 5 records
    fstream file("direct.bin", ios::binary | ios::out | ios::in | ios::trunc);
    Record records[] = {
        {1,"Alpha",1.1}, {2,"Beta",2.2}, {3,"Gamma",3.3},
        {4,"Delta",4.4}, {5,"Epsilon",5.5}
    };
    for (auto& r : records)
        file.write(reinterpret_cast<char*>(&r), sizeof(Record));
 
    // Direct access: jump to record index 2 (3rd record)
    int targetIndex = 2;
    streampos offset = targetIndex * sizeof(Record);
    file.seekp(offset, ios::beg);  // seekp for write position
 
    Record updated = {3, "Gamma_UPDATED", 99.9};
    file.write(reinterpret_cast<char*>(&updated), sizeof(Record));
 
    // Verify by reading all records
    file.seekg(0, ios::beg);
    Record r;
    while (file.read(reinterpret_cast<char*>(&r), sizeof(Record)))
        cout << "ID:" << r.id << " Name:" << r.name << " Val:" << r.value << endl;
    file.close();
    return 0;
}

