/*
* Program 7.1 — Stateful Firewall + DMZ Topology Simulator
* REAL SITUATION: Models iptables conntrack (stateful inspection),
* application-layer rules (like iptables -m string --string),
* and a 3-zone network (Internet | DMZ | LAN) topology.
*/

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <sstream>
#include <algorithm>

using namespace std;

// ── Packet representation ─────────────────────────────────────
struct Packet {
    string srcIP, dstIP;
    int srcPort, dstPort;
    string proto;      // TCP / UDP / ICMP
    string direction;  // INPUT / OUTPUT / FORWARD
    string payload;
    bool isSyn = false;
};

// ── Connection tracking (iptables conntrack) ──────────────────
struct ConnKey {
    string srcIP; 
    int srcPort;
    string dstIP; 
    int dstPort;
    string proto;

    bool operator<(const ConnKey& o) const {
        ostringstream a, b;
        a << srcIP << srcPort << dstIP << dstPort << proto;
        b << o.srcIP << o.srcPort << o.dstIP << o.dstPort << o.proto;
        return a.str() < b.str();
    }
};

enum ConnState { SYN_SENT, ESTABLISHED, CLOSED };

map<ConnKey, ConnState> conntrack;

// ── Firewall rule ─────────────────────────────────────────────
struct Rule {
    string chain;     // INPUT / OUTPUT / FORWARD
    string srcNet;    // CIDR or "ANY"
    string dstNet;
    int dstPort;      // -1 = any
    string proto;     // TCP/UDP/ICMP/ANY
    string target;    // ACCEPT / DROP / REJECT / LOG
    string comment;
    bool stateful;
};

// ── Stateful Firewall Engine ──────────────────────────────────
class Firewall {

    vector<Rule> rules_;
    int accepted_ = 0;
    int dropped_ = 0;

    bool matchNet(const string& ip, const string& net) {

        if(net == "ANY") return true;

        string prefix = net.substr(0, net.find('/'));

        if(prefix.find('.') == string::npos)
            return ip.find(prefix) == 0;

        return ip.substr(0, prefix.size()) == prefix;
    }

    // Check conntrack for ESTABLISHED
    bool isEstablished(const Packet& p) {

        ConnKey k{p.srcIP,p.srcPort,p.dstIP,p.dstPort,p.proto};
        ConnKey r{p.dstIP,p.dstPort,p.srcIP,p.srcPort,p.proto};

        return (conntrack.count(k) && conntrack[k]==ESTABLISHED) ||
               (conntrack.count(r) && conntrack[r]==ESTABLISHED);
    }

    void trackConn(const Packet& p) {

        ConnKey k{p.srcIP,p.srcPort,p.dstIP,p.dstPort,p.proto};

        if(p.isSyn)
            conntrack[k] = SYN_SENT;
        else if(conntrack.count(k) && conntrack[k]==SYN_SENT)
            conntrack[k] = ESTABLISHED;
    }

public:

    void addRule(const Rule& r) {
        rules_.push_back(r);
    }

    string process(const Packet& p) {

        // Allow established connections
        if(isEstablished(p)) {
            trackConn(p);
            accepted_++;
            return "ACCEPT (ESTABLISHED)";
        }

        for(auto& r : rules_) {

            bool matchProto = (r.proto=="ANY" || r.proto==p.proto);
            bool matchSrc   = matchNet(p.srcIP, r.srcNet);
            bool matchDst   = (r.dstPort==-1 || r.dstPort==p.dstPort);
            bool matchDir   = (r.chain==p.direction || r.chain=="BOTH");

            if(matchProto && matchSrc && matchDst && matchDir) {

                // Deep Packet Inspection
                if(!p.payload.empty() &&
                   p.payload.find("DROP_KEYWORD") != string::npos) {
                    dropped_++;
                    return "DROP (DPI: banned content)";
                }

                trackConn(p);

                if(r.target=="ACCEPT" || r.target=="LOG") {
                    accepted_++;
                    return r.target + " [" + r.comment + "]";
                }

                dropped_++;
                return r.target + " [" + r.comment + "]";
            }
        }

        dropped_++;
        return "DROP (default policy)";
    }

    void stats() {
        cout << "\n[FW STATS] Accepted=" << accepted_
             << " Dropped=" << dropped_ << "\n";
    }
};

// ── Zone helper ───────────────────────────────────────────────
string zoneOf(const string& ip) {

    if(ip.find("192.168.") == 0) return "LAN";
    if(ip.find("172.16.")  == 0) return "DMZ";

    return "INTERNET";
}

// ── Simulation driver ─────────────────────────────────────────
int main() {

    Firewall fw;

    // Firewall rules (similar to iptables)

    fw.addRule({"FORWARD","ANY","ANY",80,"TCP","ACCEPT","Allow HTTP to DMZ",true});
    fw.addRule({"FORWARD","ANY","ANY",443,"TCP","ACCEPT","Allow HTTPS",true});
    fw.addRule({"FORWARD","ANY","ANY",22,"TCP","DROP","Block SSH from Internet",false});
    fw.addRule({"INPUT","ANY","ANY",-1,"ICMP","ACCEPT","Allow ping",false});
    fw.addRule({"BOTH","ANY","ANY",-1,"ANY","LOG","Log traffic",false});

    vector<Packet> traffic = {

        {"8.8.8.8","172.16.0.10",40000,80,"TCP","FORWARD","GET /index",true},

        {"172.16.0.10","8.8.8.8",80,40000,"TCP","FORWARD","HTTP RESPONSE",false},

        {"5.5.5.5","192.168.1.5",50000,22,"TCP","FORWARD","SSH attempt",true},

        {"192.168.1.10","8.8.4.4",35000,443,"TCP","OUTPUT","TLS handshake",true},

        {"1.2.3.4","172.16.0.10",41000,80,"TCP","FORWARD","DROP_KEYWORD attack",true}

    };

    cout << "=== Firewall Simulation ===\n\n";

    for(auto& p : traffic) {

        cout << "Packet "
             << zoneOf(p.srcIP) << " -> "
             << zoneOf(p.dstIP)
             << "  (" << p.proto << ":" << p.dstPort << ")  ";

        cout << fw.process(p) << endl;
    }

    fw.stats();

    return 0;
}
