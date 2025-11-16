# Core Modules

This directory contains the fundamental modules for FHSS TRANSEC operation.

## Modules

### FHSS_TRANSEC_SAMD51_SPI.ino
**Purpose:** Main FHSS implementation with TRNG and key generation

**Features:**
- SAMD51 TRNG initialization and access
- TRANSEC key generation
- Basic frequency hopping implementation
- SPI interface support

**Key Functions:**
- `initTRNG()` - Initialize the True Random Number Generator
- `getTRNG()` - Get random data from TRNG
- `generateTransecKey()` - Generate cryptographic TRANSEC key

---

### FrequencyHoppingPatternGenerator.ino
**Purpose:** Generate pseudorandom frequency patterns from TRANSEC keys

**Features:**
- PRNG seeded with TRANSEC key
- Frequency pattern generation
- Channel selection algorithm
- Pattern reproducibility (same key = same pattern)

**Key Functions:**
- `generatePattern(key)` - Create frequency hopping sequence
- `getNextFrequency()` - Get next frequency in pattern
- `resetPattern()` - Restart pattern from beginning

---

### Master_TRANSEC_Key_Exchange.ino
**Purpose:** Master device for secure TRANSEC key distribution

**Features:**
- Key generation using TRNG
- SPI master mode for key transmission
- Key exchange protocol (master side)

**Key Functions:**
- `generateKey()` - Create new TRANSEC key
- `sendKeyToSlave()` - Transmit key via SPI
- `verifyKeyExchange()` - Confirm successful transfer

**Usage:**
Upload to the designated master device. The master generates the TRANSEC key and distributes it to slave devices.

---

### Slave_TRANSEC_Key_Exchange.ino
**Purpose:** Slave device for receiving TRANSEC keys

**Features:**
- SPI slave mode for key reception
- Key storage and validation
- Key exchange protocol (slave side)

**Key Functions:**
- `receiveKeyFromMaster()` - Receive key via SPI
- `storeKey()` - Save received TRANSEC key
- `validateKey()` - Verify key integrity

**Usage:**
Upload to slave devices that will receive the TRANSEC key from the master.

---

## Integration

These core modules form the foundation of the FHSS system:

1. **Master device** generates TRANSEC key using TRNG
2. **Master** sends key to **Slave** devices via SPI
3. Both devices use **FrequencyHoppingPatternGenerator** to create identical patterns
4. Communication begins with synchronized frequency hopping

## Security Notes

- TRNG provides cryptographically secure random numbers
- TRANSEC key should be 32 bytes (256 bits) minimum
- Key exchange over SPI requires physical security
- For production, implement proper key exchange protocol (e.g., Diffie-Hellman)

## See Also

- [Main README](../../README.md) - System architecture
- [Examples](../../examples/) - Usage examples
- [Security Modules](../security/) - Encryption and authentication
