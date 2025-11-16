# Inverse Multiplexing Modules

This directory contains modules for splitting and reassembling data streams across multiple channels.

## What is Inverse Multiplexing?

**Inverse Multiplexing** splits a single high-rate data stream across multiple lower-rate channels.

```
High-Rate Stream --> [Split] --> Multiple Channels --> [Reassemble] --> High-Rate Stream
```

### Benefits

1. **Increased Throughput** - Aggregate bandwidth of multiple channels
2. **Improved Reliability** - Redundancy across channels
3. **Enhanced Security** - Eavesdropper must intercept all channels
4. **Load Balancing** - Distribute traffic across spectrum
5. **Adaptive Performance** - Adjust channel count based on conditions

---

## Modules

### InverseMultiplexerModule.ino
**Purpose:** Basic inverse multiplexing implementation

**Level:** Basic
**Features:**
- Simple round-robin data distribution
- Fixed number of channels
- Basic packet sequencing

**Key Functions:**
- `splitData(data, length)` - Divide data into chunks
- `distributeChunks()` - Send chunks to channels
- `assignChannel(chunk)` - Allocate chunk to channel

**Data Flow:**
```
Input: [ABCDEFGHIJKLMNOP]
       ↓
Split: [ABCD] [EFGH] [IJKL] [MNOP]
       ↓       ↓       ↓       ↓
      Ch1     Ch2     Ch3     Ch4
```

**Use When:**
- Learning inverse multiplexing concepts
- Fixed-channel applications
- Simple bandwidth aggregation

---

### DynamicInverseMultiplexerModule.ino
**Purpose:** Dynamic channel allocation on transmit side

**Level:** Intermediate
**Features:**
- Dynamic channel count
- Channel quality assessment
- Adaptive chunk sizing
- Priority-based distribution

**Key Functions:**
- `addChannel(frequency)` - Add new channel
- `removeChannel(channelId)` - Remove channel
- `assessChannelQuality()` - Measure channel performance
- `optimizeDistribution()` - Adjust allocation strategy

**Advanced Capabilities:**
- Adjust to varying conditions
- Skip degraded channels
- Optimize for throughput or latency
- Variable packet sizes

**Use When:**
- Varying channel quality
- Dynamic bandwidth requirements
- Optimization needed

---

### DynamicInverseDemultiplexerModule.ino
**Purpose:** Dynamic reassembly on receive side

**Level:** Intermediate
**Features:**
- Out-of-order packet handling
- Buffer management
- Timeout handling
- Packet reordering

**Key Functions:**
- `receiveChunk(channel, data)` - Accept chunk from channel
- `reorderPackets()` - Sort chunks by sequence
- `reassembleStream()` - Reconstruct original data
- `handleMissingChunks()` - Deal with lost packets

**Packet Buffer:**
```
Received: [4] [1] [3] [2]
Reorder:  [1] [2] [3] [4]
Output:   [ABCDEFGHIJKLMNOP]
```

**Use When:**
- Variable channel delays
- Packet loss expected
- Network jitter present

---

### SynchronizedInverseDemultiplexerModule.ino
**Purpose:** Time-synchronized data reassembly

**Level:** Advanced
**Features:**
- All DynamicInverseDemultiplexer features, plus:
- Time-based synchronization
- Jitter compensation
- Precise ordering
- Latency optimization

**Key Functions:**
- `synchronizeChannels()` - Align channel timing
- `timestampChunk()` - Add precise timestamps
- `compensateJitter()` - Reduce delay variation
- `orderByTimestamp()` - Time-based reordering

**Synchronization:**
```
Ch1: [1](t=100) [4](t=400) [7](t=700)
Ch2: [2](t=150) [5](t=450) [8](t=750)
Ch3: [3](t=200) [6](t=500) [9](t=800)
       ↓
Output by timestamp: 1,2,3,4,5,6,7,8,9
```

**Use When:**
- Strict timing requirements
- Real-time applications
- Minimizing latency critical

---

### ErrorCorrectionInverseDemultiplexerModule.ino
**Purpose:** Demultiplexing with forward error correction

**Level:** Advanced
**Features:**
- All SynchronizedInverseDemultiplexer features, plus:
- Forward error correction (FEC)
- Redundant data transmission
- Erasure coding
- Recovery without retransmission

**Key Functions:**
- `addRedundancy(data)` - Add FEC parity
- `recoverLostChunks()` - Reconstruct from FEC
- `calculateParity()` - Generate error correction
- `verifyIntegrity()` - Check data correctness

**FEC Example:**
```
Original: [A] [B] [C] [D]
Add FEC:  [A] [B] [C] [D] [P1] [P2]

If [B] lost:
Recover [B] from [A] [C] [D] [P1] [P2]
```

**FEC Schemes:**
- Reed-Solomon codes
- Hamming codes
- Low-Density Parity Check (LDPC)

**Use When:**
- High packet loss expected
- Retransmission not feasible
- One-way communication
- Maximum reliability needed

---

## Configuration Guide

### Choosing Number of Channels

**Factors to consider:**
- Available bandwidth
- Channel quality
- Processing capacity
- Latency requirements

**Example:**
```cpp
const int NUM_CHANNELS = 4;  // Use 4 parallel channels

// For 1 Mbps per channel:
// Total throughput = 4 channels × 1 Mbps = 4 Mbps
```

### Chunk Size Optimization

**Small chunks (64-256 bytes):**
- Lower latency
- Better for real-time
- More overhead

**Large chunks (512-1024 bytes):**
- Higher throughput
- Less overhead
- Higher latency

```cpp
const int CHUNK_SIZE = 256;  // Balance of latency and throughput
```

### Buffer Management

**Considerations:**
- Available RAM
- Maximum out-of-order delay
- Packet loss tolerance

```cpp
const int BUFFER_SIZE = 10;  // Hold up to 10 out-of-order packets
```

---

## Module Selection Guide

| Use Case | Recommended Module |
|----------|-------------------|
| Learning/Testing | InverseMultiplexerModule |
| Fixed channels | InverseMultiplexerModule |
| Variable quality | DynamicInverseMultiplexerModule |
| Packet reordering | DynamicInverseDemultiplexerModule |
| Real-time data | SynchronizedInverseDemultiplexerModule |
| High packet loss | ErrorCorrectionInverseDemultiplexerModule |
| One-way comms | ErrorCorrectionInverseDemultiplexerModule |

---

## Performance Characteristics

### Throughput Calculation

```
Theoretical Throughput = N × Channel_Rate

Where:
N = Number of channels
Channel_Rate = Data rate per channel

Actual Throughput ≈ N × Channel_Rate × (1 - Overhead)

Overhead includes:
- Protocol headers
- FEC redundancy
- Retransmissions
```

### Latency Considerations

```
Total Latency = Max(Channel_Delays) + Processing_Time + Reordering_Time

Minimum with N channels: 1/N of single-channel latency
Maximum: Limited by slowest channel
```

---

## Example Usage

### Basic Setup

```cpp
// Transmitter
InverseMultiplexer mux;
mux.init(4);  // 4 channels

uint8_t data[1024];
mux.splitData(data, 1024);
mux.transmit();

// Receiver
InverseDemultiplexer demux;
demux.init(4);  // 4 channels

while (!demux.complete()) {
  demux.receiveChunk();
}
uint8_t* result = demux.reassemble();
```

### With Error Correction

```cpp
// Transmitter
ErrorCorrectionDemux demux;
demux.setRedundancy(0.25);  // 25% FEC overhead

uint8_t data[1024];
demux.addRedundancy(data, 1024);
demux.transmit();

// Receiver
ErrorCorrectionDemux demux;
while (!demux.complete()) {
  demux.receiveChunk();
}

if (demux.canRecover()) {
  uint8_t* result = demux.recoverAndReassemble();
}
```

---

## Troubleshooting

### Poor Throughput

**Symptoms:** Lower than expected data rate

**Causes:**
- Channel interference
- Excessive overhead
- Processing bottleneck

**Solutions:**
1. Optimize chunk size
2. Reduce FEC redundancy
3. Use faster channels

### Frequent Timeouts

**Symptoms:** Chunks not arriving

**Causes:**
- Packet loss
- Synchronization issues
- Channel quality degradation

**Solutions:**
1. Increase timeout values
2. Add error correction
3. Reduce channel count

### High Memory Usage

**Symptoms:** Running out of RAM

**Causes:**
- Large buffers
- Too many channels
- Excessive buffering

**Solutions:**
1. Reduce buffer size
2. Decrease chunk size
3. Process data incrementally

---

## See Also

- [Communication Modules](../communication/) - RF transmission
- [Synchronization Modules](../synchronization/) - Timing coordination
- [Main README](../../README.md) - System overview
- [Examples](../../examples/) - Usage examples
