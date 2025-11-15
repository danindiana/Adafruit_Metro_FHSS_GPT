#ifndef MOCK_SPI_H
#define MOCK_SPI_H

#include <stdint.h>
#include <string.h>

#ifdef NATIVE_TEST

// Mock SPI for testing key exchange and communication
class MockSPI {
private:
    static uint8_t transmitBuffer[256];
    static uint8_t receiveBuffer[256];
    static size_t bufferSize;
    static bool ssPin;
    static bool enabled;

public:
    static void init() {
        memset(transmitBuffer, 0, sizeof(transmitBuffer));
        memset(receiveBuffer, 0, sizeof(receiveBuffer));
        bufferSize = 0;
        ssPin = true; // HIGH (inactive)
        enabled = false;
    }

    static void begin() {
        enabled = true;
    }

    static void end() {
        enabled = false;
    }

    static uint8_t transfer(uint8_t data) {
        if (!enabled || ssPin) {
            return 0xFF; // No transmission if disabled or SS high
        }

        if (bufferSize < sizeof(transmitBuffer)) {
            transmitBuffer[bufferSize] = data;
            uint8_t received = receiveBuffer[bufferSize];
            bufferSize++;
            return received;
        }
        return 0xFF;
    }

    static void transfer(void* txBuf, size_t count) {
        if (!enabled || ssPin) return;

        uint8_t* tx = (uint8_t*)txBuf;
        for (size_t i = 0; i < count && bufferSize < sizeof(transmitBuffer); i++) {
            transmitBuffer[bufferSize] = tx[i];
            tx[i] = receiveBuffer[bufferSize];
            bufferSize++;
        }
    }

    static void setSlaveSelect(bool state) {
        ssPin = state;
        if (state) {
            // Transaction complete, could trigger callbacks
        }
    }

    // Test helper functions
    static void setReceiveData(const uint8_t* data, size_t size) {
        memcpy(receiveBuffer, data, size < sizeof(receiveBuffer) ? size : sizeof(receiveBuffer));
    }

    static const uint8_t* getTransmittedData() {
        return transmitBuffer;
    }

    static size_t getTransmittedSize() {
        return bufferSize;
    }

    static void clearBuffers() {
        memset(transmitBuffer, 0, sizeof(transmitBuffer));
        memset(receiveBuffer, 0, sizeof(receiveBuffer));
        bufferSize = 0;
    }
};

// Global mock SPI instance
extern MockSPI SPI;

#define digitalWrite(pin, val) MockSPI::setSlaveSelect(val)

#endif // NATIVE_TEST

#endif // MOCK_SPI_H
