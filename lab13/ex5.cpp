/*
* Program 5.1 —PAM-style Authentication: hashed passwords + TOTP + lockout
* REAL SITUATION: Models /etc/shadow salted hashing, Google Authenticator
* TOTP, and pam_tally2 / faillock lockout on Linux systems.
*/
#include <iostream>
#include <string>
#include <map>
#include <random>
#include <sstream>
#include <iomanip>
#include <functional>
#include <ctime>
// ── Salted password hasher (/etc/shadow concept) ─────────────
class ShadowHash {
public:
 static std::string salt(int len = 8) {
 static const std::string C =
 "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
 std::random_device rd;
 std::mt19937 g(rd());
 std::uniform_int_distribution<> d(0, (int)C.size()-1);
 std::string s;
 for (int i=0;i<len;i++) s+=C[d(g)];
 return s;
 }
 // Returns "$6$SALT$HASH" —mirrors SHA-512 crypt format
 static std::string hash(const std::string& pw, const std::string& s) {
 size_t h = std::hash<std::string>{}(s + pw + s);
 std::ostringstream o;
 o << "$6$" << s << "$" << std::hex << h;
 return o.str();
 }
 static bool verify(const std::string& pw, const std::string& stored) {
 // parse: $6$<salt>$<hash>
 size_t p1 = stored.find('$', 3); // end of "$6$"
 size_t p2 = stored.find('$', p1+1);
 std::string s = stored.substr(3, p1-3);
 return hash(pw, s) == stored;
 }
 };
// ── TOTP (RFC 6238, Google Authenticator concept) ────────────
class TOTP {
 std::string secret_;
 static const int STEP = 30;
public:
 explicit TOTP(const std::string& s) : secret_(s) {}
 int token() const {
 auto step = (long long)time(nullptr) / STEP;
 size_t h = std::hash<std::string>{}(secret_) ^ (size_t)step;
 return (int)(h % 1000000);
 }
 bool valid(int t) const { return t == token(); }
};
// ── User store
//────────────────────────────────────────────────
struct Account {
 std::string name, pwHash, totpSecret;
 int fails = 0;
 bool locked = false;
 time_t lockTs = 0;
 bool mfa = false;
};
// ── Authentication system (pam_unix + pam_google_authenticator) ──
class AuthSystem {
 std::map<std::string, Account> db_;
 const int MAX_FAIL = 3;
 const int LOCK_SECS = 30;
 bool unlockIfExpired(Account& a) {
 if (!a.locked) return false;
 if (time(nullptr) - a.lockTs >= LOCK_SECS) {
    a.locked=false; a.fails=0;
 std::cout << " [UNLOCK] Account unlocked after timeout.\n";
 return false;
 }
 return true;
 }
 void fail(Account& a) {
 if (++a.fails >= MAX_FAIL) {
 a.locked=true; a.lockTs=time(nullptr);
 std::cout << " [LOCKOUT] Account locked ("
 << LOCK_SECS << "s) —pam_faillock\n";
 } else {
 std::cout << " [FAIL] " << a.fails << "/" << MAX_FAIL << "\n";
 }
 }
public:
 void addUser(const std::string& name, const std::string& pw, bool mfa=false) {
 Account a;
 a.name = name;
 std::string s = ShadowHash::salt();
 a.pwHash = ShadowHash::hash(pw, s);
 a.mfa = mfa;
 a.totpSecret = mfa ? "TOTP_" + name : "";
 db_[name] = a;
 std::cout << "[ADDUSER] " << name << " shadow: "
 << a.pwHash.substr(0,30) << "...\n";
 }
 bool login(const std::string& name, const std::string& pw, int otp=-1) {
 std::cout << "\n[LOGIN] " << name << "\n";
 auto it = db_.find(name);
 if (it == db_.end()) {
 std::cout << " [DENY] Unknown user.\n"; return false;
 }
 Account& a = it->second;
 if (unlockIfExpired(a)) {
 std::cout << " [DENY] Account locked.\n"; return false; }
 if (a.locked) {
 std::cout << " [DENY] Account locked.\n"; return false; }
 if (!ShadowHash::verify(pw, a.pwHash)) {
 fail(a);
 std::cout << " [DENY] Bad password.\n";
 return false;
 }
 if (a.mfa) {
 TOTP totp(a.totpSecret);
 if (!totp.valid(otp)) {
 fail(a);
 std::cout << " [DENY] Bad OTP token.\n";
 return false;
 }
 std::cout << " [MFA OK] TOTP verified.\n";
 }
 a.fails = 0;
 std::cout << " [GRANT] Access granted to " << name << "\n";
 return true;
 }
};
int main() {
 std::cout << "=== PAM-style Authentication System ===\n\n";
 AuthSystem auth;
 auth.addUser("alice", "Pa$$w0rd!", false);
 auth.addUser("bob", "Linux2024", true);
 std::cout << "\n─── Scenario 1: alice normal login ───";
 auth.login("alice", "Pa$$w0rd!");
 std::cout << "\n─── Scenario 2: brute-force →lockout ───";
 auth.login("alice", "wrong1");
 auth.login("alice", "wrong2");
 auth.login("alice", "wrong3"); // triggers lockout
 auth.login("alice", "Pa$$w0rd!"); // still locked
 std::cout << "\n─── Scenario 3: bob MFA login ───";
 TOTP totp("TOTP_bob");
 int t = totp.token();
 std::cout << "\n [DEVICE] Current token: "
 << std::setw(6) << std::setfill('0') << t << "\n";
 auth.login("bob", "Linux2024", t); // correct OTP
 auth.login("bob", "Linux2024", 999999); // wrong OTP
 return 0;
}
