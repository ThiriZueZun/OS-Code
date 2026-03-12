/*
* Program 6.1 —RBAC + MAC (Bell-LaPadula) + Stack Canary
* REAL SITUATION:
* RBAC —Active Directory / Linux /etc/group with sudo rules
* MAC —SELinux mandatory labels (like httpd_t cannot read shadow_t)
* Stack canary —GCC -fstack-protector inserts __stack_chk_guard
*/
#include <iostream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <iomanip>
#include <cstring>
#include <cstdint>
// ── Permission bitmask
//────────────────────────────────────────
enum Perm : uint8_t {
 P_NONE=0, P_R=1, P_W=2, P_X=4, P_D=8, P_A=16
};
std::string ps(uint8_t p) {
 std::string s;
 s+=(p&P_R)?"r":"-"; s+=(p&P_W)?"w":"-"; s+=(p&P_X)?"x":"-";
 s+=(p&P_D)?"d":"-"; s+=(p&P_A)?"a":"-";
 return s;
}
// ── Security labels (SELinux-style clearance) ─────────────────
int rank(const std::string& l) {
 if(l=="top_secret") return 3;
 if(l=="secret") return 2;
 if(l=="confidential")return 1;
 return 0;
}
// ── Resource and Subject types ────────────────────────────────
struct Resource {
 std::string owner, label;
 std::map<std::string,uint8_t> acl; // role →perms
};
struct Subject {
 std::set<std::string> roles;
 std::string clearance;
 bool disabled=false;
};
// ── Access Control Engine
//─────────────────────────────────────
class ACE {
 std::map<std::string,Subject> S_;
 std::map<std::string,Resource> R_;
 // Role hierarchy: admin inherits manager inherits analyst inherits user
 static std::set<std::string> expand(const std::set<std::string>& base) {
 static const std::map<std::string,std::vector<std::string>> H={
 {"admin", {"manager","analyst","user"}},
 {"manager",{"analyst","user"}},
 {"analyst",{"user"}},{"user",{}}
 };
 std::set<std::string> out=base;
 for(auto& r:base) if(H.count(r)) for(auto& i:H.at(r)) out.insert(i);
 return out;
 }
public:
 void addSubject(const std::string& n,
 std::vector<std::string> roles,
 const std::string& clr="unclassified") {
 S_[n]={{roles.begin(),roles.end()},clr};
 }
 void addResource(const std::string& n, const std::string& owner,
 const std::string& label,
 std::map<std::string,uint8_t> acl) {
 R_[n]={owner,label,acl};
 }
 bool check(const std::string& subj,
 const std::string& res, uint8_t req) {
 std::cout << "[CHECK] " << subj << " [" << ps(req) << "] '"
 << res << "' →";
 if(!S_.count(subj)||!R_.count(res)){
 std::cout<<"DENY(not found)\n"; return false;}
 auto& s=S_[subj]; auto& r=R_[res];
 if(s.disabled){std::cout<<"DENY(disabled)\n";return false;}
 // MAC: Bell-LaPadula
 if((req&P_R)&&rank(s.clearance)<rank(r.label)){
 std::cout<<"DENY(MAC:no-read-up)\n";return false;}
 if((req&P_W)&&rank(s.clearance)>rank(r.label)){
 std::cout<<"DENY(MAC:no-write-down)\n";return false;}
 // DAC: RBAC
 uint8_t eff = r.owner==subj ? (P_R|P_W|P_X|P_D) : P_NONE;
 for(auto& role:expand(s.roles))
 if(r.acl.count(role)) eff|=r.acl.at(role);
 if((eff&req)==req){
 std::cout<<"ALLOW(eff="<<ps(eff)<<")\n";return true;}
 std::cout<<"DENY(eff="<<ps(eff)<<")\n";return false;
 }
};
// ── Stack Canary Demo (mimics GCC __stack_chk_guard) ──────────
class CanaryGuard {
 static constexpr uint64_t CANARY = 0xDEADBEEFCAFEBABEULL;
public:
 void safeProcess(const char* input, size_t maxInput) {
 volatile uint64_t guard1 = CANARY;
 char buf[32];
 volatile uint64_t guard2 = CANARY;
 size_t len = strnlen(input, maxInput);
 if(len >= sizeof(buf)){
 std::cout << "[CANARY] Overlong input rejected ("
 << len << " bytes)\n";
 return;
 }
 memcpy(buf, input, len); buf[len]='\0';
 if(guard1!=CANARY || guard2!=CANARY){
 std::cout << "[CANARY] *** STACK CORRUPTION DETECTED ***\n";
 std::abort();
 }
 std::cout << "[CANARY] Safe: \"" << buf << "\"\n";
 }
};
// ── Audit Logger
//──────────────────────────────────────────────
class AuditLog {
 std::vector<std::string> entries_;
public:
 void write(const std::string& entry) {
 time_t t = time(nullptr);
 char ts[20]; strftime(ts,sizeof(ts),"%Y-%m-%d %H:%M:%S",localtime(&t));
 entries_.push_back(std::string(ts) + " | " + entry);
 }
 void dump() {
 std::cout << "\n[AUDIT LOG] (" << entries_.size() << " entries)\n";
 for(auto& e:entries_) std::cout<<" "<<e<<"\n";
 }
};
int main() {
 std::cout << "=== Security Defenses: RBAC + MAC + Canary ===\n\n";
 // ── RBAC + MAC ──
 ACE ace;
 ace.addSubject("root", {"admin"}, "top_secret");
 ace.addSubject("alice", {"analyst"}, "secret");
 ace.addSubject("bob", {"user"}, "unclassified");
 ace.addResource("/etc/shadow","root","secret",
 {{"admin",P_R|P_W},{"user",P_NONE}});
 ace.addResource("/var/log/auth","root","confidential",
    {{"analyst",P_R},{"user",P_NONE}});
 ace.addResource("/home/bob/notes","bob","unclassified",
 {{"user",P_R|P_W}});
 std::cout << "─── Access Matrix ───\n";
 ace.check("root", "/etc/shadow", P_R|P_W);
 ace.check("alice", "/etc/shadow", P_R); // clearance ok, role no
 ace.check("alice", "/var/log/auth", P_R); // analyst + confidential ok
 ace.check("bob", "/var/log/auth", P_R); //no clearance
 ace.check("bob", "/home/bob/notes",P_R|P_W); //owner
 ace.check("alice", "/home/bob/notes",P_W); // write-down violation
 // ── Stack Canary ──
 std::cout << "\n─── Stack Canary Guard ───\n";
 CanaryGuard g;
 g.safeProcess("HelloWorld", 256);
 g.safeProcess(std::string(200,'A').c_str(), 256); // rejected by length check
 // ── Audit Trail ──
 AuditLog log;
 log.write("root ALLOW /etc/shadow READ");
 log.write("alice DENY /etc/shadow READ (role)");
 log.write("bob DENY /var/log/auth READ (clearance)");
 log.dump();
 return 0;
}
