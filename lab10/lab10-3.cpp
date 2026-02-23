#include <iostream>
#include <queue>
using namespace std;
struct NVMRequest {
int lba; // Logical Block Address
string type; // "read" or "write"
};
void processNVM(queue<NVMRequest>& a_q) {
while (!a_q.empty()) {
NVMRequest r = a_q.front(); a_q.pop();
// NVM has near-uniform access time — no seek needed
cout << r.type << " LBA " << r.lba
<< " -> latency: ~0.1ms" << endl;
}
}
int main() {
queue<NVMRequest> q;
q.push({100, "read"});
q.push({5, "write"});
q.push({999, "read"});
processNVM(q);
return 0;
}
