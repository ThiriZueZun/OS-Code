/*
* Program 4.1 —Encryption Pipeline: Caesar →XOR →RSA →MAC
* REAL SITUATION: Same principles used in OpenSSL, Windows CryptoAPI,
* and every TLS 1.3 handshake establishing HTTPS connections.
*/
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <functional>
#include <cstdint>
#include <algorithm>
// ── 1. Caesar Cipher (historic substitution) ─────────────────
class Caesar {
public:
 static std::string enc(std::string s, int k) {
 for (char& c : s) if (isalpha(c)) {
 char b = isupper(c) ? 'A' : 'a';
 c = (char)((c -b + k) % 26 + b);
 }
 return s;
 }
 static std::string dec(const std::string& s, int k) { return enc(s, 26-k); }
};
// ── 2. XOR Stream Cipher (symmetric, like RC4 concept) ───────
class XORCipher {
public:
 static std::vector<uint8_t> apply(const std::string& text,
 const std::string& key) {
 std::vector<uint8_t> out;
 for (size_t i = 0; i < text.size(); i++)
 out.push_back((uint8_t)(text[i] ^ key[i % key.size()]));
 return out;
 }
 static std::string recover(const std::vector<uint8_t>& ct,
 const std::string& key) {
 std::string out;
 for (size_t i = 0; i < ct.size(); i++)
 out += (char)(ct[i] ^ key[i % key.size()]);
 return out;
 }
 static void hex(const std::vector<uint8_t>& d) {
 for (uint8_t b : d)
 std::cout << std::hex << std::setw(2) << std::setfill('0')
 << (int)b << " ";
 std::cout << std::dec;
 }
};
// ── 3. RSA Simulation (educational small primes) ─────────────
class RSA {
 long long n, e, d;
 long long gcd(long long a, long long b) {
 return b ? gcd(b, a%b) : a; }
 long long modpow(long long b, long long x, long long m) {
 long long r=1; b%=m;
 while(x>0){ if(x&1) r=r*b%m; x>>=1; b=b*b%m; }
 return r; }
 long long modinv(long long a, long long m) {
 long long m0=m,x0=0,x1=1;
 while(a>1){ long long q=a/m,t=m;
 m=a%m; a=t; t=x0; x0=x1-q*x0; x1=t; }
 return x1<0?x1+m0:x1; }
public:
 RSA(long long p, long long q) {
 n = p*q;
 long long phi = (p-1)*(q-1);
 e = 3;
 while (gcd(e, phi) != 1) e += 2;
 d = modinv(e, phi);
 }
 long long encrypt(long long m) { return modpow(m, e, n); }
 long long decrypt(long long c) { return modpow(c, d, n); }
 void showKeys() {
 std::cout << " Public key (e,n): (" << e << ", " << n << ")\n"
 << " Private key (d,n): (" << d << ", " << n << ")\n";
 }
};
// ── 4. HMAC-style MAC (message authentication) ───────────────
class MAC {
public:
 static size_t sign(const std::string& msg, const std::string& key) {
 return std::hash<std::string>{}(key + msg + key);
 }
 static bool verify(const std::string& msg,
 const std::string& key, size_t tag) {
 return sign(msg, key) == tag;
 }
};
// ── 5. Diffie-Hellman Key Exchange Simulation ─────────────────
class DiffieHellman {
 long long p, g;
 long long modpow(long long b, long long e, long long m) {
 long long r=1; b%=m;
 while(e){ if(e&1) r=r*b%m; e>>=1; b=b*b%m; }
 return r; }
public:
 DiffieHellman(long long prime, long long gen) : p(prime), g(gen) {}
 long long publicKey(long long priv) { return modpow(g, priv, p); }
 long long sharedSecret(long long theirPublic, long long myPriv) {
 return modpow(theirPublic, myPriv, p);
 }
};

int main() {
 std::cout << "=== Cryptography as a Security Tool ===\n\n";
 // 1
 std::cout << "─── 1. Caesar Cipher ───\n";
 std::string pt = "AttackAtDawn";
 std::string ct = Caesar::enc(pt, 13);
 std::cout << " Plaintext: " << pt << "\n"
 << " ROT-13: " << ct << "\n"
 << " Decrypted: " << Caesar::dec(ct, 13) << "\n\n";
 // 2
 std::cout << "─── 2. XOR Stream Cipher ───\n";
 std::string msg = "SecretMessage!";
 std::string key = "K3yStr3am";
 auto cipher = XORCipher::apply(msg, key);
 std::cout << " Plaintext: " << msg << "\n"
 << " Ciphertext: "; XORCipher::hex(cipher);
 std::cout << "\n Recovered: " << XORCipher::recover(cipher, key) << "\n\n";
 // 3
 std::cout << "─── 3. RSA (p=61, q=53) ───\n";
 RSA rsa(61, 53);
 rsa.showKeys();
 long long m = 65, c = rsa.encrypt(m), r = rsa.decrypt(c);
 std::cout << " M=" << m << " C=" << c << " Decrypted=" << r << "\n\n";
 // 4
 std::cout << "─── 4. MAC (message authentication) ───\n";
 std::string data = "Pay Alice $500";
 std::string sk = "sharedKey42";
 size_t tag = MAC::sign(data, sk);
 std::cout << " Message: \"" << data << "\"\n"
 << " Tag: " << tag << "\n"
 << " Verify authentic: "
 << (MAC::verify(data, sk, tag) ? "VALID" : "INVALID") << "\n"
 << " Verify tampered: "
 << (MAC::verify("Pay Alice $5000", sk, tag) ? "VALID" : "INVALID") << "\n\n";
 // 5
 std::cout << "─── 5. Diffie-Hellman Key Exchange ───\n";
 DiffieHellman dh(23, 5); // small p,g for demo
 long long a_priv=6, b_priv=15;
 long long A = dh.publicKey(a_priv);
 long long B = dh.publicKey(b_priv);
 long long s1 = dh.sharedSecret(B, a_priv);
 long long s2 = dh.sharedSecret(A, b_priv);
 std::cout << " Alice public: " << A << " Bob public: " << B << "\n"
 << " Alice shared: " << s1 << "\n"
 << " Bob shared: " << s2 << "\n"
 << " Keys match: " << (s1==s2?"YES":"NO") << "\n";
 return 0;
}
