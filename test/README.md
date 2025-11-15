# Test Suite for Adafruit Metro FHSS TRANSEC

## Quick Start

```bash
# Install PlatformIO
pip install platformio

# Run all tests
pio test -e native

# Run specific test category
pio test -e native -f test_crypto
pio test -e native -f test_sync
pio test -e native -f test_error_handling
pio test -e native -f test_key_exchange
pio test -e native -f test_multiplexing

# Run integration tests
pio test -e native -f test_integration
```

## Test Categories

| Directory | Purpose | Test Count |
|-----------|---------|------------|
| `test_crypto/` | Cryptographic functions (TRNG, encryption, auth) | 46 |
| `test_sync/` | Synchronization and timing | 25 |
| `test_error_handling/` | Packet integrity and error recovery | 28 |
| `test_key_exchange/` | Master/slave key exchange | 21 |
| `test_multiplexing/` | Inverse multiplexing/demultiplexing | 24 |
| `test_integration/` | End-to-end system tests | 20 |
| **TOTAL** | | **164** |

## Test Files

### Cryptographic Tests
- `test_crypto/test_trng.cpp` - TRNG and key generation
- `test_crypto/test_encryption.cpp` - AES encryption/decryption
- `test_crypto/test_authentication.cpp` - HMAC and device auth

### Synchronization Tests
- `test_sync/test_synchronization.cpp` - Timing sync and freq hopping

### Error Handling Tests
- `test_error_handling/test_packet_integrity.cpp` - CRC, ARQ, recovery

### Key Exchange Tests
- `test_key_exchange/test_key_exchange.cpp` - TRANSEC key distribution

### Multiplexing Tests
- `test_multiplexing/test_inverse_multiplexer.cpp` - Channel allocation, data splitting

### Integration Tests
- `test_integration/test_system_integration.cpp` - Complete system workflows

## Support Files

### Mocks
- `mocks/mock_trng.h` - Mock True Random Number Generator
- `mocks/mock_spi.h` - Mock SPI communication

### Helpers
- `helpers/test_helpers.h` - Common test utilities and macros

## Expected Output

Successful test run:
```
Testing...
-----------------------
test/test_crypto/test_trng.cpp:*: test_trng_generates_nonzero_values [PASSED]
test/test_crypto/test_trng.cpp:*: test_trng_generates_different_values [PASSED]
...
-----------------------
164 Tests 0 Failures 0 Ignored
OK
```

## Documentation

See `../TEST_DOCUMENTATION.md` for comprehensive documentation including:
- Detailed test descriptions
- Test execution environments
- Debugging guidelines
- Adding new tests
- CI/CD integration

## Coverage Areas

The test suite validates:

✅ **Cryptographic Security**
- TRNG randomness and entropy
- TRANSEC key generation
- AES encryption/decryption
- HMAC authentication
- Weak key detection

✅ **Synchronization**
- Master/slave time sync
- Frequency hopping coordination
- Clock drift compensation
- Sync packet validation

✅ **Error Handling**
- CRC calculation and validation
- Packet corruption detection
- Retransmission logic (ARQ)
- Sequence number management

✅ **Key Exchange**
- SPI key transmission
- Master/slave roles
- Key integrity verification
- Pattern generation from keys

✅ **Inverse Multiplexing**
- Channel allocation/release
- Data chunking
- Out-of-order reassembly
- Chunk integrity (CRC)

✅ **System Integration**
- End-to-end workflows
- Multi-device networks
- Security enforcement
- Long-term stability

## Hardware Testing

To run tests on actual Metro M4 hardware:

```bash
# Connect Metro M4 via USB
pio test -e metro_m4

# Run integration tests on hardware
pio test -e metro_m4_integration -f test_integration
```

## Troubleshooting

**Tests not found:**
```bash
# Rebuild test environment
pio test -e native --clean
```

**Compilation errors:**
```bash
# Check PlatformIO installation
pio --version

# Update platforms
pio platform update
```

**Import errors:**
```bash
# Ensure relative paths are correct
# Mocks: #include "../mocks/mock_trng.h"
# Helpers: #include "../helpers/test_helpers.h"
```

## Contributing

When adding new tests:

1. Follow existing test structure
2. Include setUp() and tearDown() functions
3. Use descriptive test names (test_feature_behavior)
4. Add assertions with clear failure messages
5. Update test count in this README
6. Document in TEST_DOCUMENTATION.md

## License

Educational and experimental use. See project root for license details.
