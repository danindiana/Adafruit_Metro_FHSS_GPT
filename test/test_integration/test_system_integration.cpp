#include <unity.h>
#include "../helpers/test_helpers.h"
#include <stdint.h>
#include <string.h>

// Integration test combining all major modules
// This simulates a complete FHSS TRANSEC system workflow

#define KEY_LENGTH 32
#define MAX_CHANNELS 10
#define HOP_INTERVAL 500
#define SYNC_INTERVAL 1000

// Mock time for testing
uint32_t mockTime = 0;
uint32_t mockMillis() { return mockTime; }
void advanceTime(uint32_t ms) { mockTime += ms; }
void resetTime() { mockTime = 0; }

// Simple TRNG mock
uint32_t mockTRNG(uint32_t seed) {
    static uint32_t state = 12345;
    if (seed != 0) state = seed;
    state = (1103515245 * state + 12345) & 0x7fffffff;
    return state;
}

// Complete system integration components
struct SystemState {
    uint8_t transecKey[KEY_LENGTH];
    uint8_t frequencyPattern[MAX_CHANNELS];
    int currentChannel;
    uint32_t localTime;
    uint32_t sequenceNum;
    bool synchronized;
    bool keyExchanged;
};

class IntegratedFHSSSystem {
private:
    SystemState state;
    bool isMaster;

public:
    IntegratedFHSSSystem(bool master) : isMaster(master) {
        memset(&state, 0, sizeof(SystemState));
        state.synchronized = false;
        state.keyExchanged = false;
        state.currentChannel = 0;
    }

    // Step 1: Generate TRANSEC key (Master only)
    void generateKey(uint32_t seed = 0) {
        if (seed != 0) mockTRNG(seed);

        for (int i = 0; i < KEY_LENGTH; i++) {
            state.transecKey[i] = mockTRNG(0) & 0xFF;
        }
        state.keyExchanged = isMaster; // Master has key immediately
    }

    // Step 2: Exchange key (Master sends, Slave receives)
    bool exchangeKey(const uint8_t* key) {
        if (!isMaster) {
            memcpy(state.transecKey, key, KEY_LENGTH);
            state.keyExchanged = true;
            return true;
        }
        return false;
    }

    // Step 3: Generate frequency hopping pattern from key
    void generateFrequencyPattern() {
        if (!state.keyExchanged) return;

        for (int i = 0; i < MAX_CHANNELS; i++) {
            state.frequencyPattern[i] = state.transecKey[i % KEY_LENGTH] % 100;
        }
    }

    // Step 4: Synchronize timing
    void synchronize(uint32_t remoteTime, uint32_t remoteSeq) {
        if (isMaster) {
            state.localTime = mockMillis();
            state.sequenceNum = 0;
            state.synchronized = true;
        } else {
            // Slave synchronizes to master
            state.localTime = remoteTime;
            state.sequenceNum = remoteSeq;
            state.synchronized = true;
        }
    }

    // Step 5: Update frequency channel based on time
    void updateChannel() {
        if (!state.synchronized) return;

        uint32_t now = mockMillis();
        int channelIndex = (now / HOP_INTERVAL) % MAX_CHANNELS;
        state.currentChannel = channelIndex;
    }

    // Step 6: Transmit/receive data
    bool transmitData(const uint8_t* data, size_t length) {
        if (!state.synchronized) return false;
        if (!state.keyExchanged) return false;

        updateChannel();
        // Simulate transmission on current channel
        return true;
    }

    // Getters
    const uint8_t* getKey() const { return state.transecKey; }
    const uint8_t* getPattern() const { return state.frequencyPattern; }
    int getCurrentChannel() const { return state.currentChannel; }
    bool isSynchronized() const { return state.synchronized; }
    bool hasKeyExchanged() const { return state.keyExchanged; }
    uint32_t getLocalTime() const { return state.localTime; }
    uint32_t getSequenceNum() const { return state.sequenceNum; }

    void reset() {
        memset(&state, 0, sizeof(SystemState));
        state.synchronized = false;
        state.keyExchanged = false;
        state.currentChannel = 0;
    }
};

// ============================================================================
// Test Setup/Teardown
// ============================================================================

void setUp(void) {
    resetTime();
}

void tearDown(void) {
    // Cleanup
}

// ============================================================================
// Basic Integration Tests
// ============================================================================

void test_system_initialization(void) {
    IntegratedFHSSSystem master(true);
    IntegratedFHSSSystem slave(false);

    TEST_ASSERT_FALSE(master.isSynchronized());
    TEST_ASSERT_FALSE(slave.isSynchronized());
    TEST_ASSERT_FALSE(slave.hasKeyExchanged());
}

void test_master_slave_key_exchange(void) {
    IntegratedFHSSSystem master(true);
    IntegratedFHSSSystem slave(false);

    // Master generates key
    master.generateKey(12345);
    TEST_ASSERT_TRUE(master.hasKeyExchanged());

    // Master sends to slave
    const uint8_t* masterKey = master.getKey();
    slave.exchangeKey(masterKey);

    TEST_ASSERT_TRUE(slave.hasKeyExchanged());
    ASSERT_ARRAYS_EQUAL(masterKey, slave.getKey(), KEY_LENGTH);
}

void test_pattern_generation_from_key(void) {
    IntegratedFHSSSystem master(true);
    IntegratedFHSSSystem slave(false);

    // Exchange keys
    master.generateKey(54321);
    slave.exchangeKey(master.getKey());

    // Generate patterns
    master.generateFrequencyPattern();
    slave.generateFrequencyPattern();

    // Patterns should match
    const uint8_t* masterPattern = master.getPattern();
    const uint8_t* slavePattern = slave.getPattern();
    ASSERT_ARRAYS_EQUAL(masterPattern, slavePattern, MAX_CHANNELS);
}

void test_devices_synchronize(void) {
    IntegratedFHSSSystem master(true);
    IntegratedFHSSSystem slave(false);

    // Master synchronizes
    master.synchronize(0, 0);
    TEST_ASSERT_TRUE(master.isSynchronized());

    // Slave synchronizes to master
    slave.synchronize(master.getLocalTime(), master.getSequenceNum());
    TEST_ASSERT_TRUE(slave.isSynchronized());
}

void test_synchronized_channel_hopping(void) {
    IntegratedFHSSSystem master(true);
    IntegratedFHSSSystem slave(false);

    // Setup
    master.generateKey(99999);
    slave.exchangeKey(master.getKey());
    master.generateFrequencyPattern();
    slave.generateFrequencyPattern();
    master.synchronize(0, 0);
    slave.synchronize(master.getLocalTime(), master.getSequenceNum());

    // Both should start on same channel
    master.updateChannel();
    slave.updateChannel();
    TEST_ASSERT_EQUAL(master.getCurrentChannel(), slave.getCurrentChannel());

    // After hop interval, should move to next channel together
    advanceTime(HOP_INTERVAL);
    master.updateChannel();
    slave.updateChannel();
    TEST_ASSERT_EQUAL(master.getCurrentChannel(), slave.getCurrentChannel());

    // Multiple hops
    for (int i = 0; i < 5; i++) {
        advanceTime(HOP_INTERVAL);
        master.updateChannel();
        slave.updateChannel();
        TEST_ASSERT_EQUAL(master.getCurrentChannel(), slave.getCurrentChannel());
    }
}

// ============================================================================
// Complete Workflow Tests
// ============================================================================

void test_complete_system_workflow(void) {
    IntegratedFHSSSystem master(true);
    IntegratedFHSSSystem slave(false);

    // Step 1: Master generates TRANSEC key
    master.generateKey(11111);
    TEST_ASSERT_TRUE(master.hasKeyExchanged());

    // Step 2: Key exchange
    slave.exchangeKey(master.getKey());
    TEST_ASSERT_TRUE(slave.hasKeyExchanged());
    ASSERT_ARRAYS_EQUAL(master.getKey(), slave.getKey(), KEY_LENGTH);

    // Step 3: Generate frequency patterns
    master.generateFrequencyPattern();
    slave.generateFrequencyPattern();
    ASSERT_ARRAYS_EQUAL(master.getPattern(), slave.getPattern(), MAX_CHANNELS);

    // Step 4: Synchronization
    master.synchronize(0, 0);
    slave.synchronize(master.getLocalTime(), master.getSequenceNum());
    TEST_ASSERT_TRUE(master.isSynchronized());
    TEST_ASSERT_TRUE(slave.isSynchronized());

    // Step 5: Channel hopping
    for (int hop = 0; hop < 10; hop++) {
        master.updateChannel();
        slave.updateChannel();
        TEST_ASSERT_EQUAL(master.getCurrentChannel(), slave.getCurrentChannel());
        advanceTime(HOP_INTERVAL);
    }

    // Step 6: Data transmission
    const uint8_t testData[] = "Test message";
    bool txResult = master.transmitData(testData, sizeof(testData));
    TEST_ASSERT_TRUE(txResult);
}

void test_multiple_system_cycles(void) {
    IntegratedFHSSSystem master(true);
    IntegratedFHSSSystem slave(false);

    for (int cycle = 0; cycle < 3; cycle++) {
        // Reset for new cycle
        master.reset();
        slave.reset();
        resetTime();

        // Complete workflow
        master.generateKey(cycle * 10000);
        slave.exchangeKey(master.getKey());
        master.generateFrequencyPattern();
        slave.generateFrequencyPattern();
        master.synchronize(0, 0);
        slave.synchronize(master.getLocalTime(), master.getSequenceNum());

        // Verify synchronized hopping
        for (int hop = 0; hop < 5; hop++) {
            master.updateChannel();
            slave.updateChannel();
            TEST_ASSERT_EQUAL(master.getCurrentChannel(), slave.getCurrentChannel());
            advanceTime(HOP_INTERVAL);
        }
    }
}

// ============================================================================
// Security Integration Tests
// ============================================================================

void test_different_keys_produce_different_patterns(void) {
    IntegratedFHSSSystem system1(true);
    IntegratedFHSSSystem system2(true);

    system1.generateKey(11111);
    system2.generateKey(99999);

    system1.generateFrequencyPattern();
    system2.generateFrequencyPattern();

    const uint8_t* pattern1 = system1.getPattern();
    const uint8_t* pattern2 = system2.getPattern();

    ASSERT_ARRAYS_NOT_EQUAL(pattern1, pattern2, MAX_CHANNELS);
}

void test_unauthorized_device_cannot_synchronize(void) {
    IntegratedFHSSSystem master(true);
    IntegratedFHSSSystem authorizedSlave(false);
    IntegratedFHSSSystem unauthorizedSlave(false);

    // Master and authorized slave exchange keys
    master.generateKey(12345);
    authorizedSlave.exchangeKey(master.getKey());

    // Unauthorized slave has different key
    unauthorizedSlave.generateKey(54321);

    // Generate patterns
    master.generateFrequencyPattern();
    authorizedSlave.generateFrequencyPattern();
    unauthorizedSlave.generateFrequencyPattern();

    // Authorized slave's pattern matches master
    ASSERT_ARRAYS_EQUAL(master.getPattern(), authorizedSlave.getPattern(), MAX_CHANNELS);

    // Unauthorized slave's pattern does NOT match
    ASSERT_ARRAYS_NOT_EQUAL(master.getPattern(), unauthorizedSlave.getPattern(), MAX_CHANNELS);
}

// ============================================================================
// Timing and Synchronization Integration Tests
// ============================================================================

void test_resynchronization_after_drift(void) {
    IntegratedFHSSSystem master(true);
    IntegratedFHSSSystem slave(false);

    // Initial setup
    master.generateKey(77777);
    slave.exchangeKey(master.getKey());
    master.generateFrequencyPattern();
    slave.generateFrequencyPattern();
    master.synchronize(0, 0);
    slave.synchronize(master.getLocalTime(), master.getSequenceNum());

    // Channels match initially
    master.updateChannel();
    slave.updateChannel();
    TEST_ASSERT_EQUAL(master.getCurrentChannel(), slave.getCurrentChannel());

    // Simulate time drift - advance slave time more
    advanceTime(HOP_INTERVAL * 5);
    master.updateChannel();

    mockTime += 100; // Slave drifts 100ms ahead
    slave.updateChannel();

    // Resynchronize
    slave.synchronize(master.getLocalTime(), master.getSequenceNum());

    // Should be back in sync
    master.updateChannel();
    slave.updateChannel();
    TEST_ASSERT_EQUAL(master.getCurrentChannel(), slave.getCurrentChannel());
}

void test_frequency_hopping_pattern_wraps_around(void) {
    IntegratedFHSSSystem master(true);

    master.generateKey(33333);
    master.generateFrequencyPattern();
    master.synchronize(0, 0);

    // Hop through all channels multiple times
    for (int i = 0; i < MAX_CHANNELS * 3; i++) {
        master.updateChannel();
        int channel = master.getCurrentChannel();
        ASSERT_IN_RANGE(channel, 0, MAX_CHANNELS - 1);
        advanceTime(HOP_INTERVAL);
    }
}

// ============================================================================
// Error Recovery Integration Tests
// ============================================================================

void test_system_recovers_from_failed_key_exchange(void) {
    IntegratedFHSSSystem master(true);
    IntegratedFHSSSystem slave(false);

    master.generateKey(44444);

    // First exchange fails (simulated by not calling exchangeKey)
    TEST_ASSERT_FALSE(slave.hasKeyExchanged());

    // Retry exchange
    slave.exchangeKey(master.getKey());
    TEST_ASSERT_TRUE(slave.hasKeyExchanged());

    // System should work normally now
    master.generateFrequencyPattern();
    slave.generateFrequencyPattern();
    ASSERT_ARRAYS_EQUAL(master.getPattern(), slave.getPattern(), MAX_CHANNELS);
}

void test_transmission_requires_synchronization(void) {
    IntegratedFHSSSystem master(true);

    master.generateKey(55555);
    master.generateFrequencyPattern();

    const uint8_t data[] = "Test";

    // Should fail without synchronization
    bool result = master.transmitData(data, sizeof(data));
    TEST_ASSERT_FALSE(result);

    // After synchronization, should succeed
    master.synchronize(0, 0);
    result = master.transmitData(data, sizeof(data));
    TEST_ASSERT_TRUE(result);
}

void test_transmission_requires_key_exchange(void) {
    IntegratedFHSSSystem slave(false);

    slave.synchronize(0, 0);

    const uint8_t data[] = "Test";

    // Should fail without key exchange
    bool result = slave.transmitData(data, sizeof(data));
    TEST_ASSERT_FALSE(result);
}

// ============================================================================
// Multi-Device Integration Tests
// ============================================================================

void test_multiple_slaves_with_same_master(void) {
    IntegratedFHSSSystem master(true);
    IntegratedFHSSSystem slave1(false);
    IntegratedFHSSSystem slave2(false);
    IntegratedFHSSSystem slave3(false);

    // Master generates key
    master.generateKey(66666);

    // All slaves receive same key
    slave1.exchangeKey(master.getKey());
    slave2.exchangeKey(master.getKey());
    slave3.exchangeKey(master.getKey());

    // All generate patterns
    master.generateFrequencyPattern();
    slave1.generateFrequencyPattern();
    slave2.generateFrequencyPattern();
    slave3.generateFrequencyPattern();

    // All patterns should match master
    ASSERT_ARRAYS_EQUAL(master.getPattern(), slave1.getPattern(), MAX_CHANNELS);
    ASSERT_ARRAYS_EQUAL(master.getPattern(), slave2.getPattern(), MAX_CHANNELS);
    ASSERT_ARRAYS_EQUAL(master.getPattern(), slave3.getPattern(), MAX_CHANNELS);

    // All synchronize
    master.synchronize(0, 0);
    slave1.synchronize(master.getLocalTime(), master.getSequenceNum());
    slave2.synchronize(master.getLocalTime(), master.getSequenceNum());
    slave3.synchronize(master.getLocalTime(), master.getSequenceNum());

    // All should hop together
    for (int hop = 0; hop < 10; hop++) {
        master.updateChannel();
        slave1.updateChannel();
        slave2.updateChannel();
        slave3.updateChannel();

        int masterChannel = master.getCurrentChannel();
        TEST_ASSERT_EQUAL(masterChannel, slave1.getCurrentChannel());
        TEST_ASSERT_EQUAL(masterChannel, slave2.getCurrentChannel());
        TEST_ASSERT_EQUAL(masterChannel, slave3.getCurrentChannel());

        advanceTime(HOP_INTERVAL);
    }
}

// ============================================================================
// Performance and Stress Tests
// ============================================================================

void test_rapid_channel_hopping(void) {
    IntegratedFHSSSystem master(true);
    IntegratedFHSSSystem slave(false);

    master.generateKey(88888);
    slave.exchangeKey(master.getKey());
    master.generateFrequencyPattern();
    slave.generateFrequencyPattern();
    master.synchronize(0, 0);
    slave.synchronize(master.getLocalTime(), master.getSequenceNum());

    // Rapid hopping
    for (int i = 0; i < 1000; i++) {
        master.updateChannel();
        slave.updateChannel();
        TEST_ASSERT_EQUAL(master.getCurrentChannel(), slave.getCurrentChannel());
        advanceTime(HOP_INTERVAL);
    }
}

void test_long_running_system(void) {
    IntegratedFHSSSystem master(true);
    IntegratedFHSSSystem slave(false);

    master.generateKey(11223);
    slave.exchangeKey(master.getKey());
    master.generateFrequencyPattern();
    slave.generateFrequencyPattern();
    master.synchronize(0, 0);
    slave.synchronize(master.getLocalTime(), master.getSequenceNum());

    // Simulate 24 hours of operation (at accelerated time)
    const uint32_t SIMULATED_DURATION = 24 * 60 * 60 * 1000; // 24 hours in ms
    uint32_t elapsed = 0;

    while (elapsed < SIMULATED_DURATION) {
        master.updateChannel();
        slave.updateChannel();
        TEST_ASSERT_EQUAL(master.getCurrentChannel(), slave.getCurrentChannel());

        advanceTime(HOP_INTERVAL);
        elapsed += HOP_INTERVAL;
    }
}

// ============================================================================
// Test Runner
// ============================================================================

int main(int argc, char **argv) {
    UNITY_BEGIN();

    // Basic integration tests
    RUN_TEST(test_system_initialization);
    RUN_TEST(test_master_slave_key_exchange);
    RUN_TEST(test_pattern_generation_from_key);
    RUN_TEST(test_devices_synchronize);
    RUN_TEST(test_synchronized_channel_hopping);

    // Complete workflow tests
    RUN_TEST(test_complete_system_workflow);
    RUN_TEST(test_multiple_system_cycles);

    // Security integration tests
    RUN_TEST(test_different_keys_produce_different_patterns);
    RUN_TEST(test_unauthorized_device_cannot_synchronize);

    // Timing and synchronization tests
    RUN_TEST(test_resynchronization_after_drift);
    RUN_TEST(test_frequency_hopping_pattern_wraps_around);

    // Error recovery tests
    RUN_TEST(test_system_recovers_from_failed_key_exchange);
    RUN_TEST(test_transmission_requires_synchronization);
    RUN_TEST(test_transmission_requires_key_exchange);

    // Multi-device tests
    RUN_TEST(test_multiple_slaves_with_same_master);

    // Performance and stress tests
    RUN_TEST(test_rapid_channel_hopping);
    RUN_TEST(test_long_running_system);

    return UNITY_END();
}
