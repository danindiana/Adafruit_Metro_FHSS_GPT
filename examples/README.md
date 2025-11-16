# FHSS TRANSEC Examples

This directory contains example sketches demonstrating different aspects of the FHSS TRANSEC system.

## Available Examples

### 1. BasicFHSS

**Difficulty:** Beginner
**Hardware:** 1x Metro M4, ESP32 RF module

A minimal example demonstrating basic frequency hopping using the SAMD51's TRNG. Perfect for understanding the fundamentals of FHSS.

**What it demonstrates:**
- TRNG initialization
- TRANSEC key generation
- Frequency pattern generation
- Basic frequency hopping

**To run:**
1. Open `BasicFHSS/BasicFHSS.ino` in Arduino IDE
2. Upload to your Metro M4
3. Open Serial Monitor at 115200 baud
4. Observe the frequency hopping pattern

---

### 2. MasterSlaveKeyExchange

**Difficulty:** Intermediate
**Hardware:** 2x Metro M4 boards, SPI wiring

Demonstrates secure key exchange between two Metro M4 boards using SPI communication.

**What it demonstrates:**
- Master/Slave architecture
- TRANSEC key generation on Master
- Secure key transmission via SPI
- Key reception and verification

**Hardware Setup:**
```
Master          Slave
------          -----
MOSI (11)  -->  MOSI (11)
MISO (12)  <--  MISO (12)
SCK  (13)  -->  SCK  (13)
SS   (10)  -->  SS   (10)
GND        ---  GND
```

**To run:**
1. Wire the two boards as shown above
2. Upload the sketch to both boards
3. Open Serial Monitor for first board, type 'M' to select Master mode
4. Open Serial Monitor for second board, type 'S' to select Slave mode
5. Press ENTER on Master to initiate key exchange
6. Verify both boards show identical keys

---

### 3. SecureTransmission

**Difficulty:** Advanced
**Hardware:** 1x Metro M4, ESP32 RF module

A complete example combining multiple system components for secure, reliable communication.

**What it demonstrates:**
- TRANSEC key generation
- Encryption key derivation
- Frequency hopping
- Data encryption (XOR-based, for demonstration)
- Error detection (CRC16 + checksum)
- Automatic retry on transmission failure

**To run:**
1. Open `SecureTransmission/SecureTransmission.ino` in Arduino IDE
2. Upload to your Metro M4
3. Open Serial Monitor at 115200 baud
4. Observe the complete secure transmission process

**Note:** This example uses simplified encryption (XOR) for demonstration. For production use, implement proper AES or ChaCha20 encryption.

---

## Learning Path

If you're new to this project, we recommend following this learning path:

1. **Start with BasicFHSS** - Understand the fundamentals
2. **Try MasterSlaveKeyExchange** - Learn about key distribution
3. **Move to SecureTransmission** - See how everything works together
4. **Explore the source modules** - Deep dive into `src/` for production-ready code

## Modifying Examples

These examples are designed to be educational and are simplified for clarity. For production use:

- Replace XOR encryption with AES-128 or ChaCha20
- Implement proper key derivation (HKDF)
- Add device authentication
- Use hardware-accelerated cryptography where available
- Implement proper SPI slave mode for Metro M4
- Add comprehensive error handling

## Need Help?

- Check the main [README.md](../README.md) for system architecture
- Review module documentation in `src/` subdirectories
- See [CONTRIBUTING.md](../CONTRIBUTING.md) for development guidelines

## Contributing Examples

Have a useful example? We'd love to include it! Please see [CONTRIBUTING.md](../CONTRIBUTING.md) for guidelines on submitting examples.
