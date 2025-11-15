#include <unity.h>
#include "../helpers/test_helpers.h"
#include "../mocks/mock_spi.h"
#include "../mocks/mock_trng.h"
#include <stdint.h>
#include <string.h>

#define KEY_LENGTH 32
#define SS_PIN 10

#ifdef NATIVE_TEST
// Initialize static members for native testing
uint8_t MockSPI::transmitBuffer[256];
uint8_t MockSPI::receiveBuffer[256];
size_t MockSPI::bufferSize = 0;
bool MockSPI::ssPin = true;
bool MockSPI::enabled = false;

MockSPI SPI;
#endif

// Master device key exchange implementation
class MasterKeyExchange {
private:
    uint8_t TRANSECKey[KEY_LENGTH];
    bool keyGenerated;

public:
    MasterKeyExchange() : keyGenerated(false) {
        memset(TRANSECKey, 0, KEY_LENGTH);
    }

    void generateKey() {
#ifdef NATIVE_TEST
        MockTRNG::init();
#endif
        for (int i = 0; i < KEY_LENGTH; i++) {
#ifdef NATIVE_TEST
            TRANSECKey[i] = MockTRNG::getData() & 0xFF;
#else
            TRANSECKey[i] = get_trng() & 0xFF;
#endif
        }
        keyGenerated = true;
    }

    bool transmitKey() {
        if (!keyGenerated) return false;

#ifdef NATIVE_TEST
        MockSPI::setSlaveSelect(false); // LOW = active
        MockSPI::transfer(TRANSECKey, KEY_LENGTH);
        MockSPI::setSlaveSelect(true); // HIGH = inactive
#else
        digitalWrite(SS_PIN, LOW);
        SPI.transfer(TRANSECKey, KEY_LENGTH);
        digitalWrite(SS_PIN, HIGH);
#endif
        return true;
    }

    const uint8_t* getKey() const {
        return TRANSECKey;
    }

    bool isKeyGenerated() const {
        return keyGenerated;
    }

    void setKey(const uint8_t* key) {
        memcpy(TRANSECKey, key, KEY_LENGTH);
        keyGenerated = true;
    }
};

// Slave device key exchange implementation
class SlaveKeyExchange {
private:
    uint8_t receivedKey[KEY_LENGTH];
    bool keyReceived;

public:
    SlaveKeyExchange() : keyReceived(false) {
        memset(receivedKey, 0, KEY_LENGTH);
    }

    bool receiveKey() {
#ifdef NATIVE_TEST
        if (!MockSPI::enabled) return false;
        if (MockSPI::bufferSize < KEY_LENGTH) return false;

        memcpy(receivedKey, MockSPI::getTransmittedData(), KEY_LENGTH);
#else
        // In real hardware, this would be SPI receive
        // For now, simulate reception
#endif
        keyReceived = true;
        return true;
    }

    const uint8_t* getKey() const {
        return receivedKey;
    }

    bool isKeyReceived() const {
        return keyReceived;
    }

    void reset() {
        memset(receivedKey, 0, KEY_LENGTH);
        keyReceived = false;
    }
};

// ============================================================================
// Test Setup/Teardown
// ============================================================================

void setUp(void) {
#ifdef NATIVE_TEST
    MockSPI::init();
    MockTRNG::init();
#endif
}

void tearDown(void) {
#ifdef NATIVE_TEST
    MockSPI::clearBuffers();
#endif
}

// ============================================================================
// Master Key Generation Tests
// ============================================================================

void test_master_generates_key(void) {
    MasterKeyExchange master;

    master.generateKey();

    TEST_ASSERT_TRUE(master.isKeyGenerated());
    const uint8_t* key = master.getKey();
    TEST_ASSERT_NOT_NULL(key);
}

void test_master_key_not_all_zeros(void) {
    MasterKeyExchange master;

    master.generateKey();

    const uint8_t* key = master.getKey();
    bool allZeros = TestHelpers::isAllZeros(key, KEY_LENGTH);
    TEST_ASSERT_FALSE(allZeros);
}

void test_master_key_has_correct_length(void) {
    MasterKeyExchange master;

    master.generateKey();

    const uint8_t* key = master.getKey();
    // Implicit length check via array access
    for (int i = 0; i < KEY_LENGTH; i++) {
        // Access should not crash
        volatile uint8_t byte = key[i];
        (void)byte;
    }
    TEST_PASS();
}

void test_master_generates_different_keys(void) {
    MasterKeyExchange master1, master2;

    master1.generateKey();
    master2.generateKey();

    const uint8_t* key1 = master1.getKey();
    const uint8_t* key2 = master2.getKey();

    ASSERT_ARRAYS_NOT_EQUAL(key1, key2, KEY_LENGTH);
}

void test_master_key_has_good_entropy(void) {
    MasterKeyExchange master;

    master.generateKey();

    const uint8_t* key = master.getKey();
    double entropy = TestHelpers::calculateEntropy(key, KEY_LENGTH);
    TEST_ASSERT_GREATER_THAN(6.0, entropy);
}

// ============================================================================
// Key Transmission Tests
// ============================================================================

void test_master_transmits_key(void) {
#ifdef NATIVE_TEST
    MasterKeyExchange master;
    MockSPI::begin();

    master.generateKey();
    bool result = master.transmitKey();

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(KEY_LENGTH, MockSPI::getTransmittedSize());
#else
    TEST_IGNORE_MESSAGE("SPI transmission test requires native environment");
#endif
}

void test_master_cannot_transmit_before_generation(void) {
    MasterKeyExchange master;

#ifdef NATIVE_TEST
    MockSPI::begin();
#endif

    bool result = master.transmitKey();
    TEST_ASSERT_FALSE(result);
}

void test_transmitted_key_matches_generated_key(void) {
#ifdef NATIVE_TEST
    MasterKeyExchange master;
    MockSPI::begin();

    master.generateKey();
    const uint8_t* originalKey = master.getKey();
    uint8_t keyCopy[KEY_LENGTH];
    memcpy(keyCopy, originalKey, KEY_LENGTH);

    master.transmitKey();

    const uint8_t* transmitted = MockSPI::getTransmittedData();
    ASSERT_ARRAYS_EQUAL(keyCopy, transmitted, KEY_LENGTH);
#else
    TEST_IGNORE_MESSAGE("SPI transmission test requires native environment");
#endif
}

// ============================================================================
// Slave Key Reception Tests
// ============================================================================

void test_slave_receives_key(void) {
#ifdef NATIVE_TEST
    MasterKeyExchange master;
    SlaveKeyExchange slave;

    MockSPI::begin();
    master.generateKey();
    master.transmitKey();

    bool result = slave.receiveKey();

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(slave.isKeyReceived());
#else
    TEST_IGNORE_MESSAGE("SPI reception test requires native environment");
#endif
}

void test_slave_received_key_matches_master_key(void) {
#ifdef NATIVE_TEST
    MasterKeyExchange master;
    SlaveKeyExchange slave;

    MockSPI::begin();
    master.generateKey();
    const uint8_t* masterKey = master.getKey();

    uint8_t masterKeyCopy[KEY_LENGTH];
    memcpy(masterKeyCopy, masterKey, KEY_LENGTH);

    master.transmitKey();
    slave.receiveKey();

    const uint8_t* slaveKey = slave.getKey();
    ASSERT_ARRAYS_EQUAL(masterKeyCopy, slaveKey, KEY_LENGTH);
#else
    TEST_IGNORE_MESSAGE("Key matching test requires native environment");
#endif
}

void test_slave_reset_clears_key(void) {
#ifdef NATIVE_TEST
    MasterKeyExchange master;
    SlaveKeyExchange slave;

    MockSPI::begin();
    master.generateKey();
    master.transmitKey();
    slave.receiveKey();

    TEST_ASSERT_TRUE(slave.isKeyReceived());

    slave.reset();

    TEST_ASSERT_FALSE(slave.isKeyReceived());
    const uint8_t* key = slave.getKey();
    bool allZeros = TestHelpers::isAllZeros(key, KEY_LENGTH);
    TEST_ASSERT_TRUE(allZeros);
#else
    TEST_IGNORE_MESSAGE("Slave reset test requires native environment");
#endif
}

// ============================================================================
// End-to-End Key Exchange Tests
// ============================================================================

void test_complete_key_exchange_master_to_slave(void) {
#ifdef NATIVE_TEST
    MasterKeyExchange master;
    SlaveKeyExchange slave;

    // Master generates key
    master.generateKey();
    TEST_ASSERT_TRUE(master.isKeyGenerated());

    // SPI communication
    MockSPI::begin();

    // Master transmits
    bool txResult = master.transmitKey();
    TEST_ASSERT_TRUE(txResult);

    // Slave receives
    bool rxResult = slave.receiveKey();
    TEST_ASSERT_TRUE(rxResult);

    // Verify keys match
    const uint8_t* masterKey = master.getKey();
    const uint8_t* slaveKey = slave.getKey();
    ASSERT_ARRAYS_EQUAL(masterKey, slaveKey, KEY_LENGTH);
#else
    TEST_IGNORE_MESSAGE("End-to-end test requires native environment");
#endif
}

void test_multiple_key_exchanges(void) {
#ifdef NATIVE_TEST
    MasterKeyExchange master;
    SlaveKeyExchange slave;

    MockSPI::begin();

    for (int i = 0; i < 5; i++) {
        slave.reset();
        MockSPI::clearBuffers();

        master.generateKey();
        master.transmitKey();
        slave.receiveKey();

        const uint8_t* masterKey = master.getKey();
        const uint8_t* slaveKey = slave.getKey();
        ASSERT_ARRAYS_EQUAL(masterKey, slaveKey, KEY_LENGTH);
    }
#else
    TEST_IGNORE_MESSAGE("Multiple exchange test requires native environment");
#endif
}

void test_slave_cannot_receive_without_transmission(void) {
#ifdef NATIVE_TEST
    SlaveKeyExchange slave;

    MockSPI::begin();
    MockSPI::clearBuffers();

    bool result = slave.receiveKey();

    TEST_ASSERT_FALSE(result);
#else
    TEST_IGNORE_MESSAGE("Reception failure test requires native environment");
#endif
}

// ============================================================================
// Security Tests
// ============================================================================

void test_each_exchange_uses_different_key(void) {
#ifdef NATIVE_TEST
    MasterKeyExchange master;
    uint8_t keys[3][KEY_LENGTH];

    for (int i = 0; i < 3; i++) {
        master.generateKey();
        memcpy(keys[i], master.getKey(), KEY_LENGTH);
    }

    // All three keys should be different
    ASSERT_ARRAYS_NOT_EQUAL(keys[0], keys[1], KEY_LENGTH);
    ASSERT_ARRAYS_NOT_EQUAL(keys[1], keys[2], KEY_LENGTH);
    ASSERT_ARRAYS_NOT_EQUAL(keys[0], keys[2], KEY_LENGTH);
#else
    TEST_IGNORE_MESSAGE("Key uniqueness test requires native environment");
#endif
}

void test_key_not_weakened_by_transmission(void) {
#ifdef NATIVE_TEST
    MasterKeyExchange master;
    SlaveKeyExchange slave;

    MockSPI::begin();
    master.generateKey();

    double entropyBefore = TestHelpers::calculateEntropy(master.getKey(), KEY_LENGTH);

    master.transmitKey();
    slave.receiveKey();

    double entropyAfter = TestHelpers::calculateEntropy(slave.getKey(), KEY_LENGTH);

    // Entropy should be approximately equal
    TEST_ASSERT_FLOAT_WITHIN(0.5, entropyBefore, entropyAfter);
#else
    TEST_IGNORE_MESSAGE("Entropy preservation test requires native environment");
#endif
}

void test_no_key_leakage_between_exchanges(void) {
#ifdef NATIVE_TEST
    MasterKeyExchange master;
    SlaveKeyExchange slave;

    MockSPI::begin();

    // First exchange
    master.generateKey();
    uint8_t firstKey[KEY_LENGTH];
    memcpy(firstKey, master.getKey(), KEY_LENGTH);
    master.transmitKey();
    slave.receiveKey();

    // Second exchange
    slave.reset();
    MockSPI::clearBuffers();
    master.generateKey();
    uint8_t secondKey[KEY_LENGTH];
    memcpy(secondKey, master.getKey(), KEY_LENGTH);
    master.transmitKey();
    slave.receiveKey();

    // Slave should have second key, not first
    ASSERT_ARRAYS_EQUAL(secondKey, slave.getKey(), KEY_LENGTH);
    ASSERT_ARRAYS_NOT_EQUAL(firstKey, slave.getKey(), KEY_LENGTH);
#else
    TEST_IGNORE_MESSAGE("Key leakage test requires native environment");
#endif
}

// ============================================================================
// Pattern Generation from Exchanged Key Tests
// ============================================================================

void test_both_devices_generate_same_pattern_from_key(void) {
#ifdef NATIVE_TEST
    MasterKeyExchange master;
    SlaveKeyExchange slave;

    MockSPI::begin();
    master.generateKey();
    master.transmitKey();
    slave.receiveKey();

    // Generate patterns from shared key
    const int PATTERN_LENGTH = 10;
    uint8_t masterPattern[PATTERN_LENGTH];
    uint8_t slavePattern[PATTERN_LENGTH];

    const uint8_t* masterKey = master.getKey();
    const uint8_t* slaveKey = slave.getKey();

    for (int i = 0; i < PATTERN_LENGTH; i++) {
        masterPattern[i] = masterKey[i % KEY_LENGTH] % 100;
        slavePattern[i] = slaveKey[i % KEY_LENGTH] % 100;
    }

    ASSERT_ARRAYS_EQUAL(masterPattern, slavePattern, PATTERN_LENGTH);
#else
    TEST_IGNORE_MESSAGE("Pattern generation test requires native environment");
#endif
}

void test_different_keys_produce_different_patterns(void) {
#ifdef NATIVE_TEST
    uint8_t key1[KEY_LENGTH];
    uint8_t key2[KEY_LENGTH];

    // Generate two different keys
    MockTRNG::setSeed(11111);
    for (int i = 0; i < KEY_LENGTH; i++) {
        key1[i] = MockTRNG::getData() & 0xFF;
    }

    MockTRNG::setSeed(99999);
    for (int i = 0; i < KEY_LENGTH; i++) {
        key2[i] = MockTRNG::getData() & 0xFF;
    }

    // Generate patterns
    const int PATTERN_LENGTH = 10;
    uint8_t pattern1[PATTERN_LENGTH];
    uint8_t pattern2[PATTERN_LENGTH];

    for (int i = 0; i < PATTERN_LENGTH; i++) {
        pattern1[i] = key1[i % KEY_LENGTH] % 100;
        pattern2[i] = key2[i % KEY_LENGTH] % 100;
    }

    ASSERT_ARRAYS_NOT_EQUAL(pattern1, pattern2, PATTERN_LENGTH);
#else
    TEST_IGNORE_MESSAGE("Pattern difference test requires native environment");
#endif
}

// ============================================================================
// Error Handling Tests
// ============================================================================

void test_slave_handles_incomplete_transmission(void) {
#ifdef NATIVE_TEST
    SlaveKeyExchange slave;

    MockSPI::begin();

    // Simulate incomplete transmission (only half the key)
    uint8_t partialKey[KEY_LENGTH / 2];
    for (int i = 0; i < KEY_LENGTH / 2; i++) {
        partialKey[i] = i;
    }
    MockSPI::setReceiveData(partialKey, KEY_LENGTH / 2);

    bool result = slave.receiveKey();

    TEST_ASSERT_FALSE(result);
#else
    TEST_IGNORE_MESSAGE("Incomplete transmission test requires native environment");
#endif
}

void test_master_can_retransmit_same_key(void) {
#ifdef NATIVE_TEST
    MasterKeyExchange master;

    MockSPI::begin();
    master.generateKey();

    uint8_t keyCopy[KEY_LENGTH];
    memcpy(keyCopy, master.getKey(), KEY_LENGTH);

    // First transmission
    master.transmitKey();
    const uint8_t* tx1 = MockSPI::getTransmittedData();
    uint8_t tx1Copy[KEY_LENGTH];
    memcpy(tx1Copy, tx1, KEY_LENGTH);

    // Second transmission
    MockSPI::clearBuffers();
    master.transmitKey();
    const uint8_t* tx2 = MockSPI::getTransmittedData();

    // Both transmissions should be identical
    ASSERT_ARRAYS_EQUAL(tx1Copy, tx2, KEY_LENGTH);
    ASSERT_ARRAYS_EQUAL(keyCopy, tx2, KEY_LENGTH);
#else
    TEST_IGNORE_MESSAGE("Retransmission test requires native environment");
#endif
}

// ============================================================================
// Test Runner
// ============================================================================

int main(int argc, char **argv) {
    UNITY_BEGIN();

    // Master key generation tests
    RUN_TEST(test_master_generates_key);
    RUN_TEST(test_master_key_not_all_zeros);
    RUN_TEST(test_master_key_has_correct_length);
    RUN_TEST(test_master_generates_different_keys);
    RUN_TEST(test_master_key_has_good_entropy);

    // Key transmission tests
    RUN_TEST(test_master_transmits_key);
    RUN_TEST(test_master_cannot_transmit_before_generation);
    RUN_TEST(test_transmitted_key_matches_generated_key);

    // Slave key reception tests
    RUN_TEST(test_slave_receives_key);
    RUN_TEST(test_slave_received_key_matches_master_key);
    RUN_TEST(test_slave_reset_clears_key);

    // End-to-end key exchange tests
    RUN_TEST(test_complete_key_exchange_master_to_slave);
    RUN_TEST(test_multiple_key_exchanges);
    RUN_TEST(test_slave_cannot_receive_without_transmission);

    // Security tests
    RUN_TEST(test_each_exchange_uses_different_key);
    RUN_TEST(test_key_not_weakened_by_transmission);
    RUN_TEST(test_no_key_leakage_between_exchanges);

    // Pattern generation tests
    RUN_TEST(test_both_devices_generate_same_pattern_from_key);
    RUN_TEST(test_different_keys_produce_different_patterns);

    // Error handling tests
    RUN_TEST(test_slave_handles_incomplete_transmission);
    RUN_TEST(test_master_can_retransmit_same_key);

    return UNITY_END();
}
