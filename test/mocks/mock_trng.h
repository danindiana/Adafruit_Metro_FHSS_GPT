#ifndef MOCK_TRNG_H
#define MOCK_TRNG_H

#include <stdint.h>
#include <stdlib.h>

// Mock TRNG for testing purposes
// In real hardware, this would use SAMD51 TRNG registers

#ifdef NATIVE_TEST

// Mock TRNG structure for native testing
class MockTRNG {
private:
    static uint32_t seedValue;
    static bool deterministic;
    static uint32_t* presetValues;
    static size_t presetIndex;
    static size_t presetCount;

public:
    static void init() {
        seedValue = 12345;
        deterministic = false;
        presetValues = nullptr;
        presetIndex = 0;
        presetCount = 0;
    }

    static void setSeed(uint32_t seed) {
        seedValue = seed;
        srand(seed);
    }

    static void setDeterministic(bool enable) {
        deterministic = enable;
    }

    // Set preset values for controlled testing
    static void setPresetValues(uint32_t* values, size_t count) {
        presetValues = values;
        presetCount = count;
        presetIndex = 0;
    }

    static uint32_t getData() {
        if (presetValues && presetIndex < presetCount) {
            return presetValues[presetIndex++];
        }

        if (deterministic) {
            // Use simple LCG for deterministic output
            seedValue = (1103515245 * seedValue + 12345) & 0x7fffffff;
            return seedValue;
        }

        // Pseudo-random for testing
        return ((uint32_t)rand() << 16) | ((uint32_t)rand() & 0xFFFF);
    }

    static void reset() {
        presetIndex = 0;
    }
};

#define TRNG_ENABLE() MockTRNG::init()
#define TRNG_GET_DATA() MockTRNG::getData()

#else

// Real hardware TRNG interface for Metro M4
typedef struct {
    volatile uint32_t CTRLA;
    volatile uint32_t EVCTRL;
    volatile uint32_t INTENCLR;
    volatile uint32_t INTENSET;
    volatile uint32_t INTFLAG;
    volatile uint32_t DATA;
} TRNG_Type;

typedef struct {
    volatile uint32_t APBCMASK;
} MCLK_Type;

extern TRNG_Type* TRNG;
extern MCLK_Type* MCLK;

#define TRNG_INTFLAG_MASK 0x01
#define TRNG_ENABLE() do { \
    MCLK->APBCMASK |= (1 << 0); \
    TRNG->CTRLA |= (1 << 1); \
} while(0)

#define TRNG_GET_DATA() (TRNG->DATA)

#endif // NATIVE_TEST

#endif // MOCK_TRNG_H
