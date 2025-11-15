#include <unity.h>
#include "../mocks/mock_trng.h"
#include "../helpers/test_helpers.h"

#define KEY_LENGTH 32
#define NUMBER_OF_FREQUENCIES 10

// Mock initialization for native testing
#ifdef NATIVE_TEST
uint32_t MockTRNG::seedValue = 12345;
bool MockTRNG::deterministic = false;
uint32_t* MockTRNG::presetValues = nullptr;
size_t MockTRNG::presetIndex = 0;
size_t MockTRNG::presetCount = 0;
#endif

// Function under test - from FHSS_TRANSEC_SAMD51_SPI.ino
uint8_t TRANSECKey[KEY_LENGTH];
uint8_t frequencyHoppingPattern[NUMBER_OF_FREQUENCIES];

uint32_t get_trng() {
#ifdef NATIVE_TEST
    return MockTRNG::getData();
#else
    while ((TRNG->INTFLAG.reg & TRNG_INTFLAG_MASK) == 0);
    return (TRNG->DATA.reg);
#endif
}

void generateTRANSECKey() {
    for (int i = 0; i < KEY_LENGTH; i++) {
        TRANSECKey[i] = get_trng() & 0xFF;
    }
}

void generateFrequencyPattern() {
    for (int i = 0; i < NUMBER_OF_FREQUENCIES; i++) {
        frequencyHoppingPattern[i] = TRANSECKey[i % KEY_LENGTH] % 100;
    }
}

// Test setup and teardown
void setUp(void) {
#ifdef NATIVE_TEST
    MockTRNG::init();
    memset(TRANSECKey, 0, KEY_LENGTH);
    memset(frequencyHoppingPattern, 0, NUMBER_OF_FREQUENCIES);
#endif
}

void tearDown(void) {
    // Clean up after each test
}

// ============================================================================
// TRNG Tests
// ============================================================================

void test_trng_generates_nonzero_values(void) {
    uint32_t value = get_trng();
    TEST_ASSERT_NOT_EQUAL(0, value);
}

void test_trng_generates_different_values(void) {
    uint32_t value1 = get_trng();
    uint32_t value2 = get_trng();
    uint32_t value3 = get_trng();

    // At least two values should be different
    bool allDifferent = (value1 != value2) || (value2 != value3) || (value1 != value3);
    TEST_ASSERT_TRUE(allDifferent);
}

void test_trng_output_distribution(void) {
    const int SAMPLE_SIZE = 256;
    uint8_t samples[SAMPLE_SIZE];

    for (int i = 0; i < SAMPLE_SIZE; i++) {
        samples[i] = get_trng() & 0xFF;
    }

    // Check entropy - should be reasonably high for random data
    double entropy = TestHelpers::calculateEntropy(samples, SAMPLE_SIZE);
    TEST_ASSERT_GREATER_THAN(6.0, entropy); // Good randomness > 6.0 bits
}

void test_trng_full_range_output(void) {
    const int SAMPLE_SIZE = 1000;
    bool hasLowBits = false;
    bool hasHighBits = false;

    for (int i = 0; i < SAMPLE_SIZE; i++) {
        uint32_t value = get_trng();
        if (value & 0x00FF) hasLowBits = true;
        if (value & 0xFF00) hasHighBits = true;
        if (hasLowBits && hasHighBits) break;
    }

    TEST_ASSERT_TRUE(hasLowBits);
    TEST_ASSERT_TRUE(hasHighBits);
}

// ============================================================================
// TRANSEC Key Generation Tests
// ============================================================================

void test_transec_key_length(void) {
    generateTRANSECKey();

    // Key should have correct length
    TEST_ASSERT_EQUAL(KEY_LENGTH, sizeof(TRANSECKey));
}

void test_transec_key_not_all_zeros(void) {
    generateTRANSECKey();

    bool allZeros = TestHelpers::isAllZeros(TRANSECKey, KEY_LENGTH);
    TEST_ASSERT_FALSE(allZeros);
}

void test_transec_key_not_all_same_value(void) {
    generateTRANSECKey();

    bool allSame = TestHelpers::isAllSameValue(TRANSECKey, KEY_LENGTH);
    TEST_ASSERT_FALSE(allSame);
}

void test_transec_key_uniqueness(void) {
    uint8_t key1[KEY_LENGTH];
    uint8_t key2[KEY_LENGTH];

    // Generate first key
    generateTRANSECKey();
    memcpy(key1, TRANSECKey, KEY_LENGTH);

    // Generate second key
    generateTRANSECKey();
    memcpy(key2, TRANSECKey, KEY_LENGTH);

    // Keys should be different (very high probability)
    ASSERT_ARRAYS_NOT_EQUAL(key1, key2, KEY_LENGTH);
}

void test_transec_key_has_good_entropy(void) {
    generateTRANSECKey();

    double entropy = TestHelpers::calculateEntropy(TRANSECKey, KEY_LENGTH);
    TEST_ASSERT_GREATER_THAN(6.0, entropy);
}

void test_transec_key_no_hardcoded_values(void) {
    // Common weak keys to check against
    const uint8_t weakKeys[][KEY_LENGTH] = {
        {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11,
         0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11,
         0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11,
         0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11}
    };

    generateTRANSECKey();

    for (size_t i = 0; i < sizeof(weakKeys) / sizeof(weakKeys[0]); i++) {
        TEST_ASSERT_FALSE(memcmp(TRANSECKey, weakKeys[i], KEY_LENGTH) == 0);
    }
}

// ============================================================================
// Frequency Pattern Generation Tests
// ============================================================================

void test_frequency_pattern_determinism(void) {
#ifdef NATIVE_TEST
    // Set deterministic TRNG
    MockTRNG::setDeterministic(true);
    MockTRNG::setSeed(12345);

    // Generate first key and pattern
    generateTRANSECKey();
    uint8_t key1[KEY_LENGTH];
    memcpy(key1, TRANSECKey, KEY_LENGTH);
    generateFrequencyPattern();
    uint8_t pattern1[NUMBER_OF_FREQUENCIES];
    memcpy(pattern1, frequencyHoppingPattern, NUMBER_OF_FREQUENCIES);

    // Reset and generate second key and pattern with same seed
    MockTRNG::setSeed(12345);
    generateTRANSECKey();
    uint8_t key2[KEY_LENGTH];
    memcpy(key2, TRANSECKey, KEY_LENGTH);
    generateFrequencyPattern();
    uint8_t pattern2[NUMBER_OF_FREQUENCIES];
    memcpy(pattern2, frequencyHoppingPattern, NUMBER_OF_FREQUENCIES);

    // Same seed should produce same key and pattern
    ASSERT_ARRAYS_EQUAL(key1, key2, KEY_LENGTH);
    ASSERT_ARRAYS_EQUAL(pattern1, pattern2, NUMBER_OF_FREQUENCIES);

    MockTRNG::setDeterministic(false);
#else
    TEST_IGNORE_MESSAGE("Determinism test only runs in native environment");
#endif
}

void test_frequency_pattern_generation(void) {
    generateTRANSECKey();
    generateFrequencyPattern();

    // All frequencies should be in valid range (0-99)
    for (int i = 0; i < NUMBER_OF_FREQUENCIES; i++) {
        ASSERT_IN_RANGE(frequencyHoppingPattern[i], 0, 99);
    }
}

void test_frequency_pattern_not_all_same(void) {
    generateTRANSECKey();
    generateFrequencyPattern();

    bool allSame = TestHelpers::isAllSameValue(frequencyHoppingPattern, NUMBER_OF_FREQUENCIES);
    TEST_ASSERT_FALSE(allSame);
}

void test_frequency_pattern_reproducible_from_key(void) {
    // Generate key
    generateTRANSECKey();
    uint8_t savedKey[KEY_LENGTH];
    memcpy(savedKey, TRANSECKey, KEY_LENGTH);

    // Generate pattern 1
    generateFrequencyPattern();
    uint8_t pattern1[NUMBER_OF_FREQUENCIES];
    memcpy(pattern1, frequencyHoppingPattern, NUMBER_OF_FREQUENCIES);

    // Restore key and regenerate pattern
    memcpy(TRANSECKey, savedKey, KEY_LENGTH);
    generateFrequencyPattern();
    uint8_t pattern2[NUMBER_OF_FREQUENCIES];
    memcpy(pattern2, frequencyHoppingPattern, NUMBER_OF_FREQUENCIES);

    // Same key should produce same pattern
    ASSERT_ARRAYS_EQUAL(pattern1, pattern2, NUMBER_OF_FREQUENCIES);
}

void test_different_keys_produce_different_patterns(void) {
#ifdef NATIVE_TEST
    MockTRNG::setDeterministic(true);

    // Generate first key and pattern
    MockTRNG::setSeed(11111);
    generateTRANSECKey();
    generateFrequencyPattern();
    uint8_t pattern1[NUMBER_OF_FREQUENCIES];
    memcpy(pattern1, frequencyHoppingPattern, NUMBER_OF_FREQUENCIES);

    // Generate second key and pattern with different seed
    MockTRNG::setSeed(99999);
    generateTRANSECKey();
    generateFrequencyPattern();
    uint8_t pattern2[NUMBER_OF_FREQUENCIES];
    memcpy(pattern2, frequencyHoppingPattern, NUMBER_OF_FREQUENCIES);

    // Different keys should produce different patterns
    ASSERT_ARRAYS_NOT_EQUAL(pattern1, pattern2, NUMBER_OF_FREQUENCIES);

    MockTRNG::setDeterministic(false);
#else
    TEST_IGNORE_MESSAGE("Pattern difference test only runs in native environment");
#endif
}

// ============================================================================
// Security Tests
// ============================================================================

void test_key_strength_minimum_hamming_weight(void) {
    generateTRANSECKey();

    // Count number of set bits (Hamming weight)
    int setBits = 0;
    for (int i = 0; i < KEY_LENGTH; i++) {
        uint8_t byte = TRANSECKey[i];
        while (byte) {
            setBits += byte & 1;
            byte >>= 1;
        }
    }

    // A good random key should have roughly 50% bits set
    // For 256 bits, expect around 128 ± 40 bits set
    ASSERT_IN_RANGE(setBits, 88, 168);
}

void test_key_no_obvious_patterns(void) {
    generateTRANSECKey();

    // Check for ascending sequence
    bool ascending = true;
    for (int i = 1; i < KEY_LENGTH; i++) {
        if (TRANSECKey[i] != TRANSECKey[i-1] + 1) {
            ascending = false;
            break;
        }
    }
    TEST_ASSERT_FALSE(ascending);

    // Check for repeating pattern
    bool repeating = true;
    for (int i = 2; i < KEY_LENGTH; i++) {
        if (TRANSECKey[i] != TRANSECKey[i % 2]) {
            repeating = false;
            break;
        }
    }
    TEST_ASSERT_FALSE(repeating);
}

// ============================================================================
// Test Runner
// ============================================================================

int main(int argc, char **argv) {
    UNITY_BEGIN();

    // TRNG Tests
    RUN_TEST(test_trng_generates_nonzero_values);
    RUN_TEST(test_trng_generates_different_values);
    RUN_TEST(test_trng_output_distribution);
    RUN_TEST(test_trng_full_range_output);

    // TRANSEC Key Generation Tests
    RUN_TEST(test_transec_key_length);
    RUN_TEST(test_transec_key_not_all_zeros);
    RUN_TEST(test_transec_key_not_all_same_value);
    RUN_TEST(test_transec_key_uniqueness);
    RUN_TEST(test_transec_key_has_good_entropy);
    RUN_TEST(test_transec_key_no_hardcoded_values);

    // Frequency Pattern Generation Tests
    RUN_TEST(test_frequency_pattern_determinism);
    RUN_TEST(test_frequency_pattern_generation);
    RUN_TEST(test_frequency_pattern_not_all_same);
    RUN_TEST(test_frequency_pattern_reproducible_from_key);
    RUN_TEST(test_different_keys_produce_different_patterns);

    // Security Tests
    RUN_TEST(test_key_strength_minimum_hamming_weight);
    RUN_TEST(test_key_no_obvious_patterns);

    return UNITY_END();
}
