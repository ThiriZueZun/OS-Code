/*
* Program 2.1 —Buffer Overflow + Trojan Horse + Logic Bomb
* REAL SITUATION:
* Buffer overflow —basis of Morris Worm (1988)
* Logic bomb —UBS PaineWebber 2002 (Roger Duronio case)
* Trojan —malicious code hidden inside legitimate utility
*/
#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <ctime>
// ── Section A: Buffer Overflow ───────────────────────────────
class VulnerableInput {
public:
 // UNSAFE —strcpy has no bounds check
 void process_unsafe(const char* input) {
 char buffer[16];
 bool privileged = false; // sits adjacent on the stack
 strcpy(buffer, input); // DANGER: no length check
 std::cout << "[UNSAFE] buffer = \"" << buffer << "\"\n";
 std::cout << "[UNSAFE] privileged = " << privileged << "\n";
 if (privileged)
 std::cout << "[UNSAFE] *** PRIVILEGE ESCALATION via overflow! ***\n";
 }
 // SAFE —bounds-checked alternative
 void process_safe(const std::string& input) {
 const size_t MAX = 15;
 if (input.size() > MAX) {
 std::cerr << "[SAFE] Rejected: input too long ("
 << input.size() << " > " << MAX << ")\n";
 return;
 }
 char buffer[16];
 memcpy(buffer, input.c_str(), input.size() + 1);
 std::cout << "[SAFE] buffer = \"" << buffer << "\"\n";
 }
};
// ── Section B: Trojan Horse
//───────────────────────────────────
//Poses as a helpful string-sort utility
std::vector<std::string> trojanSort(std::vector<std::string>& data,
 const std::string& callerUser) {
 std::sort(data.begin(), data.end()); // legitimate work
 // Hidden payload: exfiltrate caller identity
 std::ofstream log("/tmp/.exfil_log", std::ios::app);
 log << "[TROJAN] User '" << callerUser
 << "' called sort at t=" << time(nullptr) << "\n";
 std::cout << "[TROJAN] Hidden payload: logged caller to /tmp/.exfil_log\n";
 return data;
}
// ── Section C: Logic Bomb
//────────────────────────────────────
class LogicBomb {
private:
 int triggerMonth, triggerDay;
public:
 LogicBomb(int m, int d) : triggerMonth(m), triggerDay(d) {}
 void runDailyBackup() {
 time_t now = time(nullptr);
 struct tm* t = localtime(&now);
 int month = t->tm_mon + 1;
 int day = t->tm_mday;
 std::cout << "[BACKUP] Running scheduled backup (" << day
 << "/" << month << ")...\n";
 if (month == triggerMonth && day == triggerDay) {
 // Triggered —would destroy data in a real attack
 std::cout << "[BOMB] *** LOGIC BOMB TRIGGERED ***\n";
 std::cout << "[BOMB] Simulating: shred -u /data/*, "
 "drop databases...\n";
 } else {
 std::cout << "[BACKUP] Backup completed normally. Bomb dormant.\n";
 }
 }
};
int main() {
 std::cout << "=== Program Threats Demo ===\n\n";
 // A: Buffer Overflow
 std::cout << "─── A: Buffer Overflow ───\n";
 VulnerableInput vi;
 std::cout << "[Test 1] Normal input:\n";
 vi.process_unsafe("hello");
 std::cout << "[Test 2] Overflow input (16 'A' + 0x01):\n";
 vi.process_unsafe("AAAAAAAAAAAAAAAA\x01"); // corrupts `privileged`
 std::cout << "[Test 3] Safe version with long input:\n";
 vi.process_safe(std::string(100, 'X'));
 // B: Trojan Horse
 std::cout << "\n─── B: Trojan Horse ───\n";
 std::vector<std::string> files = {"report.pdf", "accounts.xls", "backup.tar"};
 std::cout << "[USER] Calling sort utility...\n";
 trojanSort(files, "alice");
 std::cout << "[USER] Sorted result: ";
 for (auto& s : files) std::cout << s << " ";
 std::cout << "\n";
 // C: Logic Bomb (triggers today for demo)
 std::cout << "\n─── C: Logic Bomb ───\n";
 time_t now = time(nullptr);
 struct tm* t = localtime(&now);
 LogicBomb bomb(t->tm_mon + 1, t->tm_mday); // trigger = today
 bomb.runDailyBackup();
 return 0;
}
