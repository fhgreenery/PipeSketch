# TreeSketch

TreeSketch is a memory-efficient sketch for network traffic measurement, written in C++. It compresses a high-speed packet stream into a small, fixed amount of memory and answers per-flow and whole-stream queries from that summary:

- **Flow size estimation**: how many bytes a given flow (5-tuple) has sent
- **Heavy hitter detection**: which flows exceed a share of total traffic
- **Heavy changer detection**: which flows changed the most between two time windows
- **Cardinality estimation**: how many distinct flows are in the stream
- **Flow size distribution and entropy**: estimated with an EM algorithm over the counters

The repository also contains baseline sketches (Count-Min, CU, MV-Sketch and Elastic Sketch) and the benchmark programs used to compare them against TreeSketch on real packet traces such as CAIDA.

## Motivation

Network traffic grows faster than the memory and CPU budget available for measuring it, so exact per-flow counting is not practical on a switch or a busy server. Sketches trade a small, bounded error for a large saving in memory, but a plain sketch suffers from hash collisions, and it wastes memory because most flows are small while a few flows carry most of the bytes.

TreeSketch addresses this in three ways:

1. **Skew-aware layered storage.** Real traffic is heavily skewed, so flows are stored in different layers according to their size. Small flows stay in compact 16-bit counters, and only the flows that outgrow them move on to larger structures, which raises memory utilisation.
2. **Heavy part with probabilistic replacement.** Large flows are kept in a key-value heavy table. A newly arriving large flow can evict a resident one with a probability that decays with the resident flow's score, and the evicted flow is re-hashed into a second-level sketch instead of being lost. This reduces the error caused by hash collisions.
3. **SIMD hashing.** The hash computations for a packet can be run in parallel with SIMD instructions to increase packet-processing throughput.

## How it works

`TreeSketch` (`treesketch.hpp`, `treesketch.cpp`) has three parts:

| Part | Structure | Counter | Holds |
|---|---|---|---|
| Light | 4 rows × `lgwidth` | 16-bit | every flow while it is small |
| Regular | 3 rows × `rgwidth` | 32-bit | large flows that are not in the heavy table |
| Heavy | 1 row × `hvwidth` | 13-byte key, 32-bit size, score | the largest flows, with their keys |

- **Insert.** A packet first goes to the light part, which is updated with conservative update (only the counters equal to the current minimum are increased). Once a flow's light estimate passes about 64,000 bytes, which is close to the 16-bit limit, further packets go to the heavy part. On a collision in the heavy table, the resident flow is replaced with probability `1 / 1.03^count`, where `count` grows with its score. A replaced flow is moved into the regular part.
- **Query.** Small flows are answered from the light part. Large flows are answered as the light part's minimum plus either the heavy table entry or, if the flow is not in the heavy table, the regular part's minimum.
- **Heavy hitters** are reported from the keys held in the heavy table.
- **Cardinality** is estimated by linear counting over the zero counters of the light part.
- **Flow size distribution** is recovered from the light counters with EM (`EMFSD.h`), and entropy is derived from it.

Memory is split between the three parts by a ratio set at the top of each benchmark program, for example 86% light, 9% regular and 5% heavy.

## Repository layout

```
treesketch.hpp / .cpp   TreeSketch data structure
flowsize.cpp            flow size accuracy (AAE, ARE) and throughput vs. CM and CU
heavyhitter.cpp         heavy hitter precision, recall and error vs. CM-Heap and MV-Sketch
heavy_changer.hpp       heavy changer detection over two consecutive windows
main_changer.cpp        heavy changer benchmark driver
cardinality.cpp         cardinality estimation vs. linear counting
adaptor.hpp / .cpp      reads a .pcap file and extracts 5-tuple flow keys
EMFSD.h                 EM algorithm for flow size distribution
hash.c / hash.h         MurmurHash and other hash functions
CM/, CU/                Count-Min and CU sketch baselines
MVSketch/, MVSketch2/   MV-Sketch baseline (including a SIMD version and P4 code)
elastic/                Elastic Sketch baseline
iptraces.txt            list of .pcap trace files to run on, one path per line
```

## Build and run

Requirements: a C++14 compiler (g++ or clang++) and libpcap.

```bash
# Ubuntu / Debian
sudo apt-get install build-essential libpcap-dev

# macOS
brew install libpcap
```

Each benchmark is a single translation unit, so it can be built directly:

```bash
g++ -std=c++14 -O2 -I. flowsize.cpp    -o flowsize    -lpcap
g++ -std=c++14 -O2 -I. heavyhitter.cpp -o heavyhitter -lpcap
g++ -std=c++14 -O2 -I. cardinality.cpp -o cardinality -lpcap
```

List the packet traces to use in `iptraces.txt`, one `.pcap` path per line (for example `traces/caida201601.pcap`), then run a benchmark from the repository root:

```bash
./flowsize
```

The memory budget (in KB) and the split between the light, regular and heavy parts are set at the top of each `main()`. Each program prints its accuracy metrics (such as AAE, ARE, precision and recall) and the throughput for TreeSketch and the baselines it is compared with.

## Using TreeSketch in your own code

```cpp
#include "treesketch.cpp"

TreeSketch sketch(lgwidth, rgwidth, hvwidth);

sketch.insert(key, pkt_size);          // key: 13-byte 5-tuple
uint32_t size = sketch.query(key);     // estimated flow size in bytes

myvector heavy;
sketch.getHeavyHitter(threshold, heavy);
uint32_t flows = sketch.getCardinality();
```

## 中文简介

网络数据爆发式增长，现有的网络测量方法在内存资源和计算资源两方面受到了限制，从而导致测量结果不准确。对此，首先采用 sketch 结构将海量数据一一压缩到狭小的内存空间中。但是 sketch 中存在大量哈希冲突，所以提出了 heavy cuckoo 算法通过二次散列的方式来提高准确率。接着，利用网络流量的偏态分布对数据流进行分类存储，提高了内存利用率。最后，使用 SIMD 指令集来并行执行 sketch 中的哈希运算，大大加快了数据包的处理速度。
