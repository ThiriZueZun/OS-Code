#include <iostream>
#include <map>
#include <string>
#include <functional>
#include <vector>
#include <optional>
#include <iomanip>
#include <cstdint>
using namespace std;
// ■■ Type aliases ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
using MajorNum = int;
using MinorNum = int;
using InodeNum = size_t;
using BlockNum = size_t;
struct DeviceID { MajorNum major; MinorNum minor; };
struct InodeEntry {
DeviceID dev;
InodeNum inode;
size_t sizeBytes;
vector<BlockNum> blocks; // logical→physical mapping
};
// ■■ VFS Layer ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
class VFSLayer {
map<string, InodeEntry> table;
public:
void mount(const string& path, InodeEntry e) { table[path] = e; }
optional<InodeEntry> lookup(const string& path) const {
cout << "[VFS] lookup '" << path << "'\n";
auto it = table.find(path);
if (it == table.end()) {
cout << "[VFS] ENOENT\n"; return nullopt;
}
cout << "[VFS] -> inode=" << it->second.inode
<< " dev=(" << it->second.dev.major << ","
<< it->second.dev.minor << ")\n";
return it->second;
}
};
// ■■ File System Layer ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
class FSLayer {
public:
// translate logical block offset to physical block number
BlockNum logicalToPhysical(const InodeEntry& inode, size_t logBlk) {
if (logBlk >= inode.blocks.size())
throw out_of_range("block beyond EOF");
BlockNum phys = inode.blocks[logBlk];
cout << "[FS] logical blk " << logBlk
<< " -> physical blk " << phys << "\n";
return phys;
}
};
// ■■ Driver Dispatch Table ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
using DriverReadFn = function<vector<uint8_t>(MinorNum, BlockNum)>;
class DriverDispatchTable {
map<MajorNum, DriverReadFn> table;
public:
void registerDriver(MajorNum maj, DriverReadFn fn) {
table[maj] = fn;
cout << "[DDT] registered driver for major=" << maj << "\n";
}
vector<uint8_t> dispatch(MajorNum maj, MinorNum min, BlockNum blk) {
cout << "[DDT] dispatch major=" << maj
<< " minor=" << min << " blk=" << blk << "\n";
return table.at(maj)(min, blk);
}
};
// ■■ Page Cache ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
class PageCache {
map<BlockNum, vector<uint8_t>> cache;
public:
bool has(BlockNum b) const { return cache.count(b); }
const vector<uint8_t>& get(BlockNum b) { return cache.at(b); }
void insert(BlockNum b, const vector<uint8_t>& d) { cache[b]=d; }
};
// ■■ I/O Pipeline ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
class IOPipeline {
VFSLayer& vfs;
FSLayer& fs;
DriverDispatchTable& ddt;
PageCache pcache;
public:
IOPipeline(VFSLayer& v,FSLayer& f,DriverDispatchTable& d)
: vfs(v), fs(f), ddt(d) {}
vector<uint8_t> read(const string& path, size_t logBlk) {
cout << "\n" << string(56,'=') << "\n";
cout << " USER read(\"" << path << "\", blk=" << logBlk << ")\n";
cout << string(56,'=') << "\n";
// Stage 1 – VFS
auto inodeOpt = vfs.lookup(path);
if (!inodeOpt) throw runtime_error("file not found");
auto& inode = *inodeOpt;
// Stage 2 – FS: logical → physical block
BlockNum physBlk = fs.logicalToPhysical(inode, logBlk);
// Stage 3 – Page cache check
if (pcache.has(physBlk)) {
cout << "[PCACHE] HIT blk=" << physBlk << " — no disk I/O!\n";
return pcache.get(physBlk);
}
cout << "[PCACHE] MISS blk=" << physBlk << "\n";
// Stage 4 – Driver dispatch
auto data = ddt.dispatch(inode.dev.major, inode.dev.minor, physBlk);
// Stage 5 – ISR completion: insert into page cache
cout << "[ISR] DMA done — inserting blk=" << physBlk << " into page cache\n";
pcache.insert(physBlk, data);
// Stage 6 – copy to user buffer
cout << "[SYS] copy " << data.size() << " bytes to user buffer\n";
cout << "[SYS] read() returns " << data.size() << "\n";
return data;
}
};
// ■■ main ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
int main() {
cout << "=== I/O Request Pipeline Demo ===\n";
VFSLayer vfs;
FSLayer fs;
DriverDispatchTable ddt;
// Register a simulated SCSI disk driver (major=8)
ddt.registerDriver(8, [](MinorNum minor, BlockNum blk) -> vector<uint8_t> {
cout << "[DRIVER] SCSI disk: seek, DMA read blk=" << blk << "\n";
return vector<uint8_t>(512, static_cast<uint8_t>(blk & 0xFF));
});
// Mount a file at VFS
vfs.mount("/home/os/notes.txt", {
{8, 0}, // major=8 (SCSI), minor=0 (/dev/sda)
42, // inode 42
1536, // 3 blocks * 512 bytes
{100, 101, 200} // logical 0->phys 100, 1->101, 2->200
});
IOPipeline pipeline(vfs, fs, ddt);
// First read — cache miss → full pipeline
auto d1 = pipeline.read("/home/os/notes.txt", 0);
cout << " data[0]=0x" << hex << (int)d1[0] << dec << "\n";
// Second read of same block — cache hit
auto d2 = pipeline.read("/home/os/notes.txt", 0);
cout << " data[0]=0x" << hex << (int)d2[0] << dec << "\n";
// Read a different block
pipeline.read("/home/os/notes.txt", 2);
return 0;
}
