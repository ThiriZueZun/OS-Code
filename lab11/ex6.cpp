#include <iostream>
#include <string>
#include <queue>
#include <memory>
#include <vector>
#include <algorithm>
using namespace std;
// ■■ Message ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
struct StreamMessage {
enum class Type { DATA, IOCTL, PROTO } type;
string payload;
explicit StreamMessage(Type t, string p)
: type(t), payload(move(p)) {}
string typeStr() const {
switch(type){
case Type::DATA: return "M_DATA";
case Type::IOCTL: return "M_IOCTL";
case Type::PROTO: return "M_PROTO";
} return "?";
}
};
// ■■ Abstract Module ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
class StreamModule {
protected:
string name_;
public:
queue<StreamMessage> readQ, writeQ; // upstream / downstream
explicit StreamModule(string n) : name_(move(n)) {}
virtual ~StreamModule() = default;
const string& name() const { return name_; }
// Process one message going downstream (write path)
virtual void processDownstream(StreamMessage& msg) {
writeQ.push(msg); // default: pass through
}
// Process one message going upstream (read path)
virtual void processUpstream(StreamMessage& msg) {
readQ.push(msg); // default: pass through
}
};
// ■■ Driver Module (bottom of stack) ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
class DriverModule : public StreamModule {
public:
DriverModule() : StreamModule("DriverEnd") {}
void processDownstream(StreamMessage& msg) override {
cout << "[DRIVER] << downstream " << msg.typeStr()
<< ": '" << msg.payload << "'\n";
// simulate device response — put reply in readQ
StreamMessage reply(StreamMessage::Type::DATA,
"ACK:" + msg.payload);
readQ.push(reply);
}
void processUpstream(StreamMessage& msg) override {
cout << "[DRIVER] >> upstream: '" << msg.payload << "'\n";
readQ.push(msg);
}
};
// ■■ Encryption Module (ROT13) ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
class EncryptionModule : public StreamModule {
static string rot13(const string& s) {
string r=s;
for(char& c:r){
if(c>='a'&&c<='z') c=(c-'a'+13)%26+'a';
else if(c>='A'&&c<='Z') c=(c-'A'+13)%26+'A';
}
return r;
}
public:
EncryptionModule() : StreamModule("Encrypt(ROT13)") {}
void processDownstream(StreamMessage& msg) override {
if(msg.type==StreamMessage::Type::DATA){
string orig=msg.payload;
msg.payload=rot13(msg.payload);
cout<<"[ENCRYPT] encrypt '"<<orig<<"' -> '"<<msg.payload<<"'\n";
}
writeQ.push(msg);
}
void processUpstream(StreamMessage& msg) override {
if(msg.type==StreamMessage::Type::DATA){
string enc=msg.payload;
msg.payload=rot13(msg.payload);
cout<<"[ENCRYPT] decrypt '"<<enc<<"' -> '"<<msg.payload<<"'\n";
}
readQ.push(msg);
}
};
// ■■ Logging Module (transparent) ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
class LoggingModule : public StreamModule {
public:
LoggingModule() : StreamModule("Logger") {}
void processDownstream(StreamMessage& msg) override {
cout<<"[LOG] DS " << msg.typeStr() <<": '"<<msg.payload<<"'\n";
writeQ.push(msg);
}
void processUpstream(StreamMessage& msg) override {
cout<<"[LOG] US " << msg.typeStr() <<": '"<<msg.payload<<"'\n";
readQ.push(msg);
}
};
// ■■ Stream Head ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
class StreamHead {
vector<shared_ptr<StreamModule>> stack; // index 0 = top (user-side)
shared_ptr<DriverModule> driver;
public:
explicit StreamHead() {
driver = make_shared<DriverModule>();
}
void iPush(shared_ptr<StreamModule> mod) {
stack.insert(stack.begin(), mod);
cout << "[HEAD] I_PUSH: " << mod->name() << "\n";
printStack();
}
void iPop() {
if(stack.empty()){ cout<<"[HEAD] I_POP: stack empty\n"; return; }
cout<<"[HEAD] I_POP: "<<stack.front()->name()<<"\n";
stack.erase(stack.begin());
printStack();
}
void write(const string& data) {
cout << "\n[HEAD] write: '" << data << "'\n";
StreamMessage msg(StreamMessage::Type::DATA, data);
// pass downstream through stack then to driver
for(auto& mod : stack) {
mod->processDownstream(msg);
if(!mod->writeQ.empty()){ msg=mod->writeQ.front(); mod->writeQ.pop(); }
}
driver->processDownstream(msg);
// now propagate reply upstream
if(!driver->readQ.empty()){
auto reply = driver->readQ.front(); driver->readQ.pop();
for(int i=(int)stack.size()-1; i>=0; --i){
stack[i]->processUpstream(reply);
if(!stack[i]->readQ.empty()){ reply=stack[i]->readQ.front(); stack[i]->readQ.pop(); }
}
cout<<"[HEAD] upstream result: '"<<reply.payload<<"'\n";
}
}
void printStack() const {
cout << " Stack (top->bottom): [HEAD]";
for(auto& m:stack) cout<<" -> "<<m->name();
cout<<" -> [DRIVER]\n";
}
};
// ■■ main ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
int main() {
cout << "=== STREAMS Framework Demo ===\n\n";
StreamHead head;
// Push modules (order matters!)
head.iPush(make_shared<LoggingModule>());
head.iPush(make_shared<EncryptionModule>());
// Write data — flows: HEAD -> Encrypt -> Logger -> Driver
head.write("Hello STREAMS");
head.write("SecretData");
// Pop encryption module at runtime
cout << "\n--- I_POP: removing encryption ---\n";
head.iPop();
head.write("PlainText");
return 0;
}
