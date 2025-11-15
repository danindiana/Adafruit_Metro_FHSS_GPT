#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <unity.h>
#include <stdint.h>
#include <string.h>

// Helper macros for common test assertions
#define ASSERT_ARRAYS_EQUAL(expected, actual, size) \
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, actual, size)

#define ASSERT_ARRAYS_NOT_EQUAL(array1, array2, size) \
    TEST_ASSERT_FALSE(memcmp(array1, array2, size) == 0)

#define ASSERT_IN_RANGE(value, min, max) \
    do { \
        TEST_ASSERT_GREATER_OR_EQUAL(min, value); \
        TEST_ASSERT_LESS_OR_EQUAL(max, value); \
    } while(0)

// Statistical test helpers
namespace TestHelpers {

    // Calculate basic entropy/randomness
    inline double calculateEntropy(const uint8_t* data, size_t length) {
        uint32_t counts[256] = {0};
        for (size_t i = 0; i < length; i++) {
            counts[data[i]]++;
        }

        double entropy = 0.0;
        for (int i = 0; i < 256; i++) {
            if (counts[i] > 0) {
                double p = (double)counts[i] / length;
                entropy -= p * log2(p);
            }
        }
        return entropy;
    }

    // Check if data appears random (basic statistical test)
    inline bool isRandomDistribution(const uint8_t* data, size_t length, double threshold = 7.0) {
        return calculateEntropy(data, length) >= threshold;
    }

    // Check for all zeros (weak key check)
    inline bool isAllZeros(const uint8_t* data, size_t length) {
        for (size_t i = 0; i < length; i++) {
            if (data[i] != 0) return false;
        }
        return true;
    }

    // Check for all same value (weak key check)
    inline bool isAllSameValue(const uint8_t* data, size_t length) {
        if (length == 0) return false;
        uint8_t first = data[0];
        for (size_t i = 1; i < length; i++) {
            if (data[i] != first) return false;
        }
        return true;
    }

    // Calculate simple CRC16 for testing
    inline uint16_t calculateCRC16(const uint8_t* data, size_t length) {
        uint16_t crc = 0xFFFF;
        for (size_t i = 0; i < length; i++) {
            crc ^= data[i];
            for (int j = 0; j < 8; j++) {
                if (crc & 0x0001) {
                    crc = (crc >> 1) ^ 0xA001;
                } else {
                    crc >>= 1;
                }
            }
        }
        return crc;
    }

    // Calculate CRC32 for testing
    inline uint32_t calculateCRC32(const uint8_t* data, size_t length) {
        uint32_t crc = 0xFFFFFFFF;
        for (size_t i = 0; i < length; i++) {
            crc ^= data[i];
            for (int j = 0; j < 8; j++) {
                crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
            }
        }
        return ~crc;
    }

    // Measure timing consistency (mock implementation)
    inline uint32_t getMockMillis() {
        static uint32_t mockTime = 0;
        return mockTime;
    }

    inline void advanceMockMillis(uint32_t ms) {
        static uint32_t mockTime = 0;
        mockTime += ms;
    }

    inline void resetMockMillis() {
        static uint32_t mockTime = 0;
        mockTime = 0;
    }

    // Print array in hex for debugging
    inline void printHexArray(const char* label, const uint8_t* data, size_t length) {
        printf("%s: ", label);
        for (size_t i = 0; i < length; i++) {
            printf("%02X ", data[i]);
            if ((i + 1) % 16 == 0) printf("\n   ");
        }
        printf("\n");
    }
}

#endif // TEST_HELPERS_H
