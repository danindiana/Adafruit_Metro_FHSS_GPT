# Synchronization Modules

This directory contains modules for timing synchronization between transmitter and receiver.

## Module Evolution

The synchronization modules are organized by complexity and features:

```
SynchronizationModule.ino
    ↓
AdvancedSynchronizationModule.ino
    ↓
RobustSynchronizationModule.ino
    ↓
CompleteSynchronizationModule.ino
```

---

## Modules

### SynchronizationModule.ino
**Purpose:** Basic synchronization for frequency hopping

**Level:** Basic
**Features:**
- Simple time synchronization
- Periodic sync packets
- Basic clock drift compensation

**Key Functions:**
- `synchronize()` - Perform synchronization
- `sendSyncPacket()` - Transmit sync information
- `receiveSyncPacket()` - Process sync information

**Use When:**
- Learning synchronization concepts
- Simple FHSS applications
- Controlled environments
- Short communication sessions

---

### AdvancedSynchronizationModule.ino
**Purpose:** Enhanced timing with packet timestamping

**Level:** Intermediate
**Features:**
- All basic features, plus:
- Packet timestamping
- Drift prediction
- Improved accuracy
- Bidirectional sync

**Key Functions:**
- `timestampPacket()` - Add precise timestamp
- `calculateDrift()` - Measure clock drift
- `adjustClock()` - Compensate for drift

**Improvements over Basic:**
- Better long-term accuracy
- Handles clock drift
- More precise timing

**Use When:**
- Extended communication sessions
- Higher precision required
- Moderate interference expected

---

### RobustSynchronizationModule.ino
**Purpose:** Delay compensation and advanced error handling

**Level:** Advanced
**Features:**
- All advanced features, plus:
- Network delay measurement
- Adaptive synchronization intervals
- Outlier detection and filtering
- Quality metrics

**Key Functions:**
- `measureDelay()` - Calculate round-trip time
- `filterOutliers()` - Remove bad measurements
- `adaptiveSyncInterval()` - Adjust sync frequency
- `getSyncQuality()` - Evaluate sync accuracy

**Advanced Capabilities:**
- Handles variable latency
- Robust to packet loss
- Self-adjusting parameters
- Quality monitoring

**Use When:**
- Unreliable communication channels
- Variable network conditions
- Mission-critical synchronization
- High interference environments

---

### CompleteSynchronizationModule.ino
**Purpose:** Full-featured synchronization with ESP32 integration

**Level:** Production-Ready
**Features:**
- All robust features, plus:
- ESP32 RF module integration
- Master/slave architecture
- Automatic recovery
- Statistics and diagnostics
- Power-efficient operation

**Key Functions:**
- `initMasterMode()` - Configure as sync master
- `initSlaveMode()` - Configure as sync slave
- `monitorSyncHealth()` - Track sync status
- `recoverFromDesync()` - Automatic resynchronization

**Complete System:**
```cpp
// Master setup
initMasterMode();
startSyncBroadcast();  // Periodic sync packets

// Slave setup
initSlaveMode();
waitForSync();         // Lock to master
```

**Use When:**
- Production deployments
- Full system integration
- Maximum reliability needed
- Long-duration operation

---

## Synchronization Theory

### Why Synchronization Matters

For FHSS to work, transmitter and receiver must:
1. Switch frequencies at the **exact same time**
2. Use the **same frequency at the same time**
3. Maintain synchronization despite clock drift

### Clock Drift

All microcontrollers have clock drift:
- Typical drift: 50-100 ppm (parts per million)
- After 1 minute: ~3-6 ms drift
- After 1 hour: ~180-360 ms drift

**Without compensation:** Devices desynchronize within minutes.

### Synchronization Strategies

1. **Periodic Sync Packets**
   - Master sends sync packets regularly
   - Slaves adjust their clocks
   - Simple but effective

2. **Timestamp-Based Sync**
   - Each packet includes precise timestamp
   - Receiver calculates offset
   - More accurate

3. **Drift Compensation**
   - Measure and predict drift
   - Adjust clock frequency
   - Reduces sync packet frequency

4. **Quality Monitoring**
   - Track synchronization accuracy
   - Detect degradation
   - Trigger resync when needed

---

## Module Selection Guide

| Requirement | Recommended Module |
|-------------|-------------------|
| Learning/Testing | SynchronizationModule |
| Short sessions (<5 min) | SynchronizationModule |
| Medium sessions (5-30 min) | AdvancedSynchronizationModule |
| Long sessions (>30 min) | RobustSynchronizationModule |
| Production deployment | CompleteSynchronizationModule |
| Noisy environment | RobustSynchronizationModule+ |
| Low power requirements | CompleteSynchronizationModule |
| Maximum reliability | CompleteSynchronizationModule |

---

## Configuration Parameters

### Sync Interval
```cpp
const unsigned long SYNC_INTERVAL_MS = 1000;  // Send sync every 1 second
```
- **Shorter interval:** Better sync, more overhead
- **Longer interval:** Less overhead, drift accumulates

### Sync Timeout
```cpp
const unsigned long SYNC_TIMEOUT_MS = 5000;  // Consider desync after 5 seconds
```
- How long without sync before triggering recovery

### Drift Tolerance
```cpp
const int MAX_DRIFT_MS = 50;  // Maximum acceptable drift
```
- Threshold for resynchronization

---

## Performance Metrics

Monitor these to evaluate sync quality:

1. **Sync Accuracy** - Clock offset between devices
2. **Packet Loss Rate** - Percentage of lost sync packets
3. **Resync Events** - How often recovery triggered
4. **Drift Rate** - ppm of clock drift

---

## Troubleshooting

### Devices Won't Sync

**Symptoms:** Slave never locks to master

**Causes:**
- No RF connection
- Different TRANSEC keys
- Incompatible sync protocols

**Solutions:**
1. Verify RF communication
2. Confirm identical keys
3. Check module versions match

### Frequent Resync

**Symptoms:** Constant desynchronization

**Causes:**
- Excessive clock drift
- High packet loss
- Sync interval too long

**Solutions:**
1. Decrease sync interval
2. Improve RF signal quality
3. Use more robust module

### Gradual Desync

**Symptoms:** Slow drift over time

**Causes:**
- Clock drift not compensated
- Sync packets delayed

**Solutions:**
1. Enable drift compensation
2. Use AdvancedSynchronization or higher
3. Reduce communication load

---

## See Also

- [Core Modules](../core/) - TRANSEC key and frequency generation
- [Communication Modules](../communication/) - Packet transmission
- [Main README](../../README.md) - System architecture
