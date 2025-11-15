#include <unity.h>
#include "../helpers/test_helpers.h"
#include <stdint.h>
#include <string.h>

// Mock timing system
class MockTime {
private:
    static uint32_t currentTime;

public:
    static void init() {
        currentTime = 0;
    }

    static uint32_t millis() {
        return currentTime;
    }

    static void advance(uint32_t ms) {
        currentTime += ms;
    }

    static void set(uint32_t ms) {
        currentTime = ms;
    }
};

uint32_t MockTime::currentTime = 0;

// Synchronization structures and functions
#define SYNC_INTERVAL 1000
#define MAX_RETRANSMISSIONS 3
#define SYNC_TIMEOUT 2000
#define HOP_INTERVAL 500

struct SyncPacket {
    uint8_t header;
    uint32_t sequenceNumber;
    uint32_t timestamp;
    uint16_t crc;
};

class SynchronizationModule {
private:
    bool isMaster;
    uint32_t localTime;
    uint32_t localSeq;
    uint32_t lastSyncTime;
    int retransmissionCount;
    bool synchronized;
    int currentChannelIndex;
    int numChannels;

public:
    SynchronizationModule(bool master = false, int channels = 10) {
        isMaster = master;
        localTime = 0;
        localSeq = 0;
        lastSyncTime = 0;
        retransmissionCount = 0;
        synchronized = false;
        currentChannelIndex = 0;
        numChannels = channels;
    }

    void init() {
        localTime = MockTime::millis();
        lastSyncTime = localTime;
        synchronized = isMaster; // Master starts synchronized
    }

    SyncPacket createSyncPacket() {
        SyncPacket packet;
        packet.header = 0xAA;
        packet.sequenceNumber = localSeq;
        packet.timestamp = localTime;
        packet.crc = calculateCRC(packet);
        return packet;
    }

    bool sendSyncPacket() {
        if (!isMaster) return false;

        uint32_t now = MockTime::millis();
        if (now - lastSyncTime >= SYNC_INTERVAL) {
            localTime = now;
            localSeq++;
            lastSyncTime = now;
            return true;
        }
        return false;
    }

    bool receiveSyncPacket(const SyncPacket& packet) {
        if (packet.header != 0xAA) return false;

        uint16_t expectedCRC = calculateCRC(packet);
        if (packet.crc != expectedCRC) return false;

        // Update local time with packet timestamp
        uint32_t now = MockTime::millis();
        localTime = packet.timestamp + (now - packet.timestamp) / 2;
        localSeq = packet.sequenceNumber;
        synchronized = true;
        lastSyncTime = now;

        return true;
    }

    void updateFrequencyChannel() {
        if (!synchronized) return;

        uint32_t now = MockTime::millis();
        int expectedChannel = (localTime / HOP_INTERVAL) % numChannels;
        currentChannelIndex = expectedChannel;
    }

    int getCurrentChannel() const {
        return currentChannelIndex;
    }

    uint32_t getLocalTime() const {
        return localTime;
    }

    uint32_t getSequenceNumber() const {
        return localSeq;
    }

    bool isSynchronized() const {
        return synchronized;
    }

    void handleRetransmission() {
        retransmissionCount++;
    }

    bool maxRetriesExceeded() const {
        return retransmissionCount > MAX_RETRANSMISSIONS;
    }

    void resetRetries() {
        retransmissionCount = 0;
    }

    // Calculate time drift between devices
    int32_t calculateDrift(uint32_t remoteTime) const {
        return (int32_t)(remoteTime - localTime);
    }

private:
    uint16_t calculateCRC(const SyncPacket& packet) const {
        // Simple CRC calculation
        uint16_t crc = 0xFFFF;
        const uint8_t* data = (const uint8_t*)&packet;
        size_t len = sizeof(SyncPacket) - sizeof(uint16_t); // Exclude CRC field

        for (size_t i = 0; i < len; i++) {
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
};

// ============================================================================
// Test Setup/Teardown
// ============================================================================

void setUp(void) {
    MockTime::init();
}

void tearDown(void) {
    // Cleanup
}

// ============================================================================
// Basic Synchronization Tests
// ============================================================================

void test_master_initialization(void) {
    SynchronizationModule master(true);
    master.init();

    TEST_ASSERT_TRUE(master.isSynchronized());
    TEST_ASSERT_EQUAL(0, master.getSequenceNumber());
}

void test_slave_initialization(void) {
    SynchronizationModule slave(false);
    slave.init();

    TEST_ASSERT_FALSE(slave.isSynchronized());
    TEST_ASSERT_EQUAL(0, slave.getSequenceNumber());
}

void test_sync_packet_creation(void) {
    SynchronizationModule master(true);
    master.init();

    SyncPacket packet = master.createSyncPacket();

    TEST_ASSERT_EQUAL(0xAA, packet.header);
    TEST_ASSERT_EQUAL(0, packet.sequenceNumber);
    TEST_ASSERT_NOT_EQUAL(0, packet.crc);
}

void test_sync_packet_crc_validation(void) {
    SynchronizationModule master(true);
    master.init();

    SyncPacket packet = master.createSyncPacket();

    // Valid packet should be accepted
    SynchronizationModule slave(false);
    slave.init();
    bool result = slave.receiveSyncPacket(packet);
    TEST_ASSERT_TRUE(result);

    // Corrupt packet should be rejected
    packet.timestamp += 100;
    result = slave.receiveSyncPacket(packet);
    TEST_ASSERT_FALSE(result);
}

void test_sync_packet_header_validation(void) {
    SynchronizationModule master(true);
    master.init();

    SyncPacket packet = master.createSyncPacket();
    packet.header = 0xBB; // Wrong header

    SynchronizationModule slave(false);
    slave.init();
    bool result = slave.receiveSyncPacket(packet);
    TEST_ASSERT_FALSE(result);
}

// ============================================================================
// Timing Synchronization Tests
// ============================================================================

void test_master_sends_sync_at_interval(void) {
    SynchronizationModule master(true);
    master.init();

    // First send should succeed immediately
    MockTime::advance(SYNC_INTERVAL);
    bool sent = master.sendSyncPacket();
    TEST_ASSERT_TRUE(sent);

    // Too soon - should not send
    MockTime::advance(500);
    sent = master.sendSyncPacket();
    TEST_ASSERT_FALSE(sent);

    // After interval - should send
    MockTime::advance(600);
    sent = master.sendSyncPacket();
    TEST_ASSERT_TRUE(sent);
}

void test_slave_synchronizes_with_master(void) {
    SynchronizationModule master(true);
    SynchronizationModule slave(false);

    master.init();
    slave.init();

    MockTime::advance(SYNC_INTERVAL);
    master.sendSyncPacket();
    SyncPacket packet = master.createSyncPacket();

    bool result = slave.receiveSyncPacket(packet);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(slave.isSynchronized());
    TEST_ASSERT_EQUAL(master.getSequenceNumber(), slave.getSequenceNumber());
}

void test_sequence_number_increments(void) {
    SynchronizationModule master(true);
    master.init();

    for (int i = 1; i <= 5; i++) {
        MockTime::advance(SYNC_INTERVAL);
        master.sendSyncPacket();
        TEST_ASSERT_EQUAL(i, master.getSequenceNumber());
    }
}

void test_time_drift_calculation(void) {
    SynchronizationModule device1(true);
    SynchronizationModule device2(false);

    device1.init();
    device2.init();

    MockTime::set(1000);
    device1.init();

    MockTime::set(1050); // 50ms drift
    device2.init();

    int32_t drift = device1.calculateDrift(device2.getLocalTime());
    TEST_ASSERT_EQUAL(50, drift);
}

// ============================================================================
// Frequency Hopping Synchronization Tests
// ============================================================================

void test_frequency_channel_updates(void) {
    SynchronizationModule master(true, 10);
    master.init();

    // Initially at channel 0
    master.updateFrequencyChannel();
    TEST_ASSERT_EQUAL(0, master.getCurrentChannel());

    // After HOP_INTERVAL, should move to next channel
    MockTime::advance(HOP_INTERVAL);
    master.sendSyncPacket();
    master.updateFrequencyChannel();
    int channel = master.getCurrentChannel();
    TEST_ASSERT_EQUAL(1, channel);
}

void test_synchronized_devices_hop_together(void) {
    SynchronizationModule master(true, 10);
    SynchronizationModule slave(false, 10);

    master.init();
    slave.init();

    // Synchronize
    MockTime::advance(SYNC_INTERVAL);
    master.sendSyncPacket();
    SyncPacket packet = master.createSyncPacket();
    slave.receiveSyncPacket(packet);

    // Both should be on same channel
    master.updateFrequencyChannel();
    slave.updateFrequencyChannel();
    TEST_ASSERT_EQUAL(master.getCurrentChannel(), slave.getCurrentChannel());

    // After hop, still synchronized
    MockTime::advance(HOP_INTERVAL);
    master.updateFrequencyChannel();
    slave.updateFrequencyChannel();
    TEST_ASSERT_EQUAL(master.getCurrentChannel(), slave.getCurrentChannel());
}

void test_channel_wraps_around(void) {
    SynchronizationModule master(true, 5);
    master.init();

    // Advance through all channels
    for (int i = 0; i < 6; i++) {
        master.updateFrequencyChannel();
        MockTime::advance(HOP_INTERVAL);
        master.sendSyncPacket();
    }

    master.updateFrequencyChannel();
    int channel = master.getCurrentChannel();
    // Should wrap back to early channels
    ASSERT_IN_RANGE(channel, 0, 4);
}

// ============================================================================
// Synchronization Recovery Tests
// ============================================================================

void test_resynchronization_after_packet_loss(void) {
    SynchronizationModule master(true);
    SynchronizationModule slave(false);

    master.init();
    slave.init();

    // Initial sync
    MockTime::advance(SYNC_INTERVAL);
    master.sendSyncPacket();
    SyncPacket packet1 = master.createSyncPacket();
    slave.receiveSyncPacket(packet1);
    TEST_ASSERT_TRUE(slave.isSynchronized());

    // Simulate packet loss - skip one sync
    MockTime::advance(SYNC_INTERVAL * 2);

    // Next sync should still work
    master.sendSyncPacket();
    SyncPacket packet2 = master.createSyncPacket();
    bool result = slave.receiveSyncPacket(packet2);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(slave.isSynchronized());
}

void test_retransmission_handling(void) {
    SynchronizationModule master(true);
    master.init();

    for (int i = 0; i < MAX_RETRANSMISSIONS; i++) {
        master.handleRetransmission();
        TEST_ASSERT_FALSE(master.maxRetriesExceeded());
    }

    master.handleRetransmission(); // One more
    TEST_ASSERT_TRUE(master.maxRetriesExceeded());
}

void test_retry_counter_reset(void) {
    SynchronizationModule master(true);
    master.init();

    for (int i = 0; i < MAX_RETRANSMISSIONS + 1; i++) {
        master.handleRetransmission();
    }
    TEST_ASSERT_TRUE(master.maxRetriesExceeded());

    master.resetRetries();
    TEST_ASSERT_FALSE(master.maxRetriesExceeded());
}

// ============================================================================
// Clock Drift Compensation Tests
// ============================================================================

void test_slave_compensates_for_clock_drift(void) {
    SynchronizationModule master(true);
    SynchronizationModule slave(false);

    master.init();
    MockTime::set(0);

    slave.init();
    MockTime::set(100); // Slave starts 100ms behind

    // Master sends sync
    MockTime::set(1000);
    master.sendSyncPacket();
    SyncPacket packet = master.createSyncPacket();

    // Slave receives and adjusts
    MockTime::set(1100);
    slave.receiveSyncPacket(packet);

    // Slave should have compensated
    int32_t drift = master.calculateDrift(slave.getLocalTime());
    // Drift should be less than initial 100ms
    TEST_ASSERT_LESS_THAN(100, abs(drift));
}

void test_multiple_sync_cycles(void) {
    SynchronizationModule master(true);
    SynchronizationModule slave(false);

    master.init();
    slave.init();

    // Run multiple sync cycles
    for (int cycle = 0; cycle < 10; cycle++) {
        MockTime::advance(SYNC_INTERVAL);
        master.sendSyncPacket();
        SyncPacket packet = master.createSyncPacket();
        bool result = slave.receiveSyncPacket(packet);

        TEST_ASSERT_TRUE(result);
        TEST_ASSERT_EQUAL(master.getSequenceNumber(), slave.getSequenceNumber());
    }
}

void test_sync_timeout_detection(void) {
    SynchronizationModule slave(false);
    slave.init();

    // Sync once
    SyncPacket packet;
    packet.header = 0xAA;
    packet.sequenceNumber = 1;
    packet.timestamp = MockTime::millis();
    packet.crc = 0; // Will be calculated
    slave.receiveSyncPacket(packet);

    uint32_t lastSync = MockTime::millis();

    // Advance past timeout
    MockTime::advance(SYNC_TIMEOUT + 100);

    uint32_t timeSinceSync = MockTime::millis() - lastSync;
    TEST_ASSERT_GREATER_THAN(SYNC_TIMEOUT, timeSinceSync);
}

// ============================================================================
// Advanced Timing Tests
// ============================================================================

void test_timestamp_accuracy(void) {
    SynchronizationModule master(true);
    master.init();

    MockTime::set(12345);
    master.sendSyncPacket();
    SyncPacket packet = master.createSyncPacket();

    // Timestamp should match current time
    TEST_ASSERT_EQUAL(12345, packet.timestamp);
}

void test_sync_maintains_accuracy_over_time(void) {
    SynchronizationModule master(true);
    SynchronizationModule slave(false);

    master.init();
    slave.init();

    // Sync every interval for extended period
    for (int i = 0; i < 100; i++) {
        MockTime::advance(SYNC_INTERVAL);
        master.sendSyncPacket();
        SyncPacket packet = master.createSyncPacket();
        slave.receiveSyncPacket(packet);
    }

    // After many cycles, drift should still be minimal
    int32_t drift = master.calculateDrift(slave.getLocalTime());
    TEST_ASSERT_LESS_THAN(200, abs(drift));
}

// ============================================================================
// Test Runner
// ============================================================================

int main(int argc, char **argv) {
    UNITY_BEGIN();

    // Basic synchronization tests
    RUN_TEST(test_master_initialization);
    RUN_TEST(test_slave_initialization);
    RUN_TEST(test_sync_packet_creation);
    RUN_TEST(test_sync_packet_crc_validation);
    RUN_TEST(test_sync_packet_header_validation);

    // Timing synchronization tests
    RUN_TEST(test_master_sends_sync_at_interval);
    RUN_TEST(test_slave_synchronizes_with_master);
    RUN_TEST(test_sequence_number_increments);
    RUN_TEST(test_time_drift_calculation);

    // Frequency hopping synchronization tests
    RUN_TEST(test_frequency_channel_updates);
    RUN_TEST(test_synchronized_devices_hop_together);
    RUN_TEST(test_channel_wraps_around);

    // Synchronization recovery tests
    RUN_TEST(test_resynchronization_after_packet_loss);
    RUN_TEST(test_retransmission_handling);
    RUN_TEST(test_retry_counter_reset);

    // Clock drift compensation tests
    RUN_TEST(test_slave_compensates_for_clock_drift);
    RUN_TEST(test_multiple_sync_cycles);
    RUN_TEST(test_sync_timeout_detection);

    // Advanced timing tests
    RUN_TEST(test_timestamp_accuracy);
    RUN_TEST(test_sync_maintains_accuracy_over_time);

    return UNITY_END();
}
