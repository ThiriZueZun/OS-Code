/*
* Program 3.1 —Port Scanner + IDS + Worm Simulation
* REAL SITUATION:
* Attacker perspective: nmap-style reconnaissance
* Defender perspective: Snort/Suricata IDS pattern detection
* Worm behavior: exponential self-replication across subnets
*/
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <iomanip>
#include <ctime>
// ── Port Scanner (attacker view) ─────────────────────────────
struct PortInfo { std::string service; bool vulnerable; std::string cve; };
class PortScanner {
 std::map<int, PortInfo> portDB = {
 {21, {"FTP", true, "CVE-2010-4221"}},
 {22, {"SSH", false, ""} },
 {23, {"Telnet", true, "CVE-2011-4862"}},
 {80, {"HTTP", false, ""} },
 {443, {"HTTPS", false, ""} },
 {445, {"SMB", true, "CVE-2017-0144"}}, // EternalBlue
 {3306,{"MySQL", true, "CVE-2012-2122"}},
 {4444,{"Backdoor/mshell",true, "N/A"} }
 };
public:
 void scan(const std::string& target, int lo, int hi) {
 std::cout << "[SCAN] Target: " << target
 << " Range: " << lo << "-" << hi << "\n\n";
 std::cout << std::left
 << std::setw(6) << "PORT"
 << std::setw(10) << "STATE"
 << std::setw(18) << "SERVICE"
 << "CVE\n"
 << std::string(56, '-') << "\n";
 for (int p = lo; p <= hi; p++) {
 auto it = portDB.find(p);
 if (it != portDB.end()) {
 std::cout << std::setw(6) << p
 << std::setw(10) << "OPEN"
 << std::setw(18) << it->second.service;
 if (it->second.vulnerable)
 std::cout << "[VULN] " << it->second.cve;
 else
 std::cout << "OK";
 std::cout << "\n";
 }
 }
 }
};
// ── IDS (Snort-style defender view) ──────────────────────────
class IDS {
 struct Attempt { std::string ip; int port; time_t ts; };
 std::vector<Attempt> log_;
 const int THRESHOLD = 5;
 const int WINDOW = 3; // seconds
public:
 void observe(const std::string& ip, int port) {
 log_.push_back({ip, port, time(nullptr)});
 time_t now = time(nullptr);
 int count = 0;
 for (auto& e : log_)
 if (e.ip == ip && (now - e.ts) <= WINDOW) count++;
 if (count == THRESHOLD + 1) { // alert once
 std::cout << "\n[IDS ALERT] Port scan from " << ip
 << " (" << count << " probes in "
 << WINDOW << "s)\n";
 std::cout << "[IDS ACTION] Simulating: "
 "iptables -I INPUT -s " << ip << " -j DROP\n\n";
 }
 }
 void printLog() {
 std::cout << "\n[IDS LOG]\n";
 for (auto& e : log_)
 std::cout << " " << e.ip << " -> :" << e.port
 << " @ t+" << (e.ts - log_[0].ts) << "s\n";
 }
};
// ── Worm Propagation
//─────────────────────────────────────────
class Worm {
 std::vector<std::string> infected_;
 const int RATE = 3; //new hosts per infected host per iteration
public:
 void spread(const std::string& seed, int rounds) {
 infected_.push_back(seed);
 std::cout << "[WORM] Seed host: " << seed << "\n";
 for (int r = 0; r < rounds; r++) {
 size_t prev = infected_.size();
 size_t lim = prev; // only existing hosts propagate
 for (size_t i = 0; i < lim; i++) {
 for (int k = 0; k < RATE; k++) {
 std::string h = "10." + std::to_string(r+1) + "."
 + std::to_string((int)i) + "."
 + std::to_string(k+2);
 if (std::find(infected_.begin(),
 infected_.end(), h) == infected_.end())
 infected_.push_back(h);
 }
 }
 std::cout << "[WORM] Round " << r+1
 << ": +" << (infected_.size()-prev)
 << " new hosts | total=" << infected_.size() << "\n";
 }
 }
};
int main() {
 std::cout << "=== System & Network Threats Demo ===\n\n";
 // Port scan
 std::cout << "─── Attacker: Reconnaissance ───\n";
 PortScanner ps;
 ps.scan("192.168.1.10", 20, 4450);
 // IDS
 std::cout << "\n─── Defender: IDS Monitoring ───\n";
 IDS ids;
 std::string atk = "203.0.113.99";
 for (int p : {21, 22, 23, 80, 443, 445, 3306, 8080})
 ids.observe(atk, p);
 ids.printLog();
 // Worm
 std::cout << "\n─── Worm Propagation ───\n";
 Worm worm;
 worm.spread("192.168.1.1", 4);
 return 0;
}
