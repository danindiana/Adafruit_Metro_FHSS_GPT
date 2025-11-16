# Communication Modules

This directory contains modules for RF communication and error handling.

## Modules

### RFCommunicationModule.ino
**Purpose:** Handle packet transmission, reception, and frequency management

**Features:**
- Packet transmission over RF
- Packet reception and decoding
- Dynamic frequency changes
- RF module interface abstraction
- Power management

**Key Functions:**
- `transmitPacket(data, length)` - Send data packet
- `receivePacket(buffer)` - Receive and decode packet
- `setFrequency(freq)` - Change RF frequency
- `getRSSI()` - Get received signal strength

**Packet Structure:**
```
[Header][Sequence][Payload][CRC][Footer]
```

**Usage Example:**
```cpp
// Initialize RF module
initRFModule();

// Set initial frequency
setFrequency(902.5);

// Transmit data
uint8_t data[] = "Hello";
transmitPacket(data, 5);

// Receive data
uint8_t buffer[64];
int len = receivePacket(buffer);
```

---

### ErrorHandlingModule.ino
**Purpose:** ARQ (Automatic Repeat Request) implementation for reliable communication

**Features:**
- Automatic retransmission on failure
- Packet acknowledgment (ACK/NAK)
- Timeout detection
- Error statistics
- Configurable retry limits

**Key Functions:**
- `sendWithRetry(data, length)` - Send with automatic retry
- `waitForAck(timeout)` - Wait for acknowledgment
- `sendAck()` - Send acknowledgment
- `handlePacketError()` - Process transmission errors

**Error Handling Flow:**
```
1. Transmit packet
2. Start timeout timer
3. Wait for ACK
   - If ACK received: Success
   - If NAK received: Retransmit
   - If timeout: Retransmit
4. Retry up to MAX_RETRIES
5. Report failure if max retries exceeded
```

**Configuration:**
```cpp
const int MAX_RETRIES = 3;        // Maximum retry attempts
const int ACK_TIMEOUT_MS = 1000;  // ACK wait time
const int RETRY_DELAY_MS = 100;   // Delay between retries
```

**Usage Example:**
```cpp
// Send data with automatic retry
uint8_t data[] = "Critical message";
bool success = sendWithRetry(data, sizeof(data));

if (success) {
  Serial.println("Message delivered");
} else {
  Serial.println("Transmission failed after retries");
}
```

**Error Statistics:**
- Track success rate
- Monitor retry counts
- Identify problematic frequencies
- Adjust parameters based on conditions

---

## Integration

The communication modules work together:

1. **RFCommunicationModule** handles low-level RF operations
2. **ErrorHandlingModule** provides reliability layer
3. Frequency hopping coordinated with synchronization modules
4. Encryption applied before transmission (see [Security Modules](../security/))

## RF Module Compatibility

Tested with:
- ESP32 with built-in WiFi/BLE
- LoRa modules (RFM95W, etc.)
- Generic 900 MHz transceivers

Requires:
- SPI or UART interface
- Frequency setting capability
- Transmit/receive functions

## Performance Tuning

**For High Reliability:**
- Increase `MAX_RETRIES`
- Increase `ACK_TIMEOUT_MS`
- Use forward error correction

**For Low Latency:**
- Decrease `ACK_TIMEOUT_MS`
- Reduce `RETRY_DELAY_MS`
- Optimize packet size

**For Noisy Environments:**
- Increase transmit power
- Use robust modulation
- Implement FEC (Forward Error Correction)

## See Also

- [Synchronization Modules](../synchronization/) - Timing coordination
- [Error Correction](../inverse_multiplexing/) - FEC implementation
- [Main README](../../README.md) - System overview
