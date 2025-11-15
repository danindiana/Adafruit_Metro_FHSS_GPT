#include <unity.h>
#include "../helpers/test_helpers.h"
#include <stdint.h>
#include <string.h>

// Packet structures and error handling
#define PACKET_SIZE 128
#define RETRANSMISSION_MAX_ATTEMPTS 3

enum PacketStatus {
    PACKET_OK,
    PACKET_CORRUPTED,
    PACKET_MISSING,
    PACKET_RETRANSMISSION_ATTEMPT_EXCEEDED
};

struct Packet {
    uint8_t header;
    uint8_t sequenceNum;
    uint8_t data[PACKET_SIZE - 4]; // Reserve space for header, seq, crc
    uint16_t crc;
};

class ErrorHandlingModule {
private:
    Packet lastReceivedPacket;
    PacketStatus lastPacketStatus;
    int retransmissionAttempts;
    uint8_t expectedSequence;
    bool initialized;

public:
    ErrorHandlingModule() {
        memset(&lastReceivedPacket, 0, sizeof(Packet));
        lastPacketStatus = PACKET_OK;
        retransmissionAttempts = 0;
        expectedSequence = 0;
        initialized = true;
    }

    uint16_t calculateCRC(const uint8_t* data, size_t length) const {
        return TestHelpers::calculateCRC16(data, length);
    }

    uint16_t calculatePacketCRC(const Packet& packet) const {
        size_t dataLength = sizeof(Packet) - sizeof(uint16_t);
        return calculateCRC((const uint8_t*)&packet, dataLength);
    }

    void createPacket(Packet& packet, uint8_t seq, const uint8_t* data, size_t dataLen) {
        packet.header = 0xAA;
        packet.sequenceNum = seq;

        size_t copyLen = dataLen < sizeof(packet.data) ? dataLen : sizeof(packet.data);
        memcpy(packet.data, data, copyLen);

        // Zero remaining space
        if (copyLen < sizeof(packet.data)) {
            memset(packet.data + copyLen, 0, sizeof(packet.data) - copyLen);
        }

        packet.crc = calculatePacketCRC(packet);
    }

    PacketStatus checkPacketIntegrity(const Packet& packet) {
        // Check header
        if (packet.header != 0xAA) {
            return PACKET_CORRUPTED;
        }

        // Check CRC
        uint16_t expectedCRC = calculatePacketCRC(packet);
        if (packet.crc != expectedCRC) {
            return PACKET_CORRUPTED;
        }

        // Check sequence number
        if (packet.sequenceNum != expectedSequence) {
            return PACKET_MISSING;
        }

        return PACKET_OK;
    }

    PacketStatus receivePacket(const Packet& packet) {
        lastPacketStatus = checkPacketIntegrity(packet);

        if (lastPacketStatus == PACKET_OK) {
            memcpy(&lastReceivedPacket, &packet, sizeof(Packet));
            expectedSequence++;
            retransmissionAttempts = 0;
        }

        return lastPacketStatus;
    }

    PacketStatus requestRetransmission() {
        retransmissionAttempts++;

        if (retransmissionAttempts > RETRANSMISSION_MAX_ATTEMPTS) {
            return PACKET_RETRANSMISSION_ATTEMPT_EXCEEDED;
        }

        return lastPacketStatus;
    }

    void processPacket(const Packet& packet) {
        memcpy(&lastReceivedPacket, &packet, sizeof(Packet));
    }

    const Packet& getLastPacket() const {
        return lastReceivedPacket;
    }

    int getRetransmissionCount() const {
        return retransmissionAttempts;
    }

    void resetRetransmissionCount() {
        retransmissionAttempts = 0;
    }

    void resetSequence() {
        expectedSequence = 0;
    }

    uint8_t getExpectedSequence() const {
        return expectedSequence;
    }
};

// ============================================================================
// Test Setup/Teardown
// ============================================================================

void setUp(void) {
    // Reset state
}

void tearDown(void) {
    // Cleanup
}

// ============================================================================
// CRC Calculation Tests
// ============================================================================

void test_crc_calculation_is_deterministic(void) {
    ErrorHandlingModule handler;
    const uint8_t testData[] = "Test data for CRC calculation";

    uint16_t crc1 = handler.calculateCRC(testData, sizeof(testData));
    uint16_t crc2 = handler.calculateCRC(testData, sizeof(testData));

    TEST_ASSERT_EQUAL_UINT16(crc1, crc2);
}

void test_crc_different_data_produces_different_crc(void) {
    ErrorHandlingModule handler;
    const uint8_t data1[] = "Data variant 1";
    const uint8_t data2[] = "Data variant 2";

    uint16_t crc1 = handler.calculateCRC(data1, sizeof(data1));
    uint16_t crc2 = handler.calculateCRC(data2, sizeof(data2));

    TEST_ASSERT_NOT_EQUAL(crc1, crc2);
}

void test_crc_single_bit_change_detected(void) {
    ErrorHandlingModule handler;
    uint8_t data1[] = "Test data";
    uint8_t data2[] = "Test data";
    data2[5] ^= 0x01; // Flip one bit

    uint16_t crc1 = handler.calculateCRC(data1, sizeof(data1));
    uint16_t crc2 = handler.calculateCRC(data2, sizeof(data2));

    TEST_ASSERT_NOT_EQUAL(crc1, crc2);
}

void test_crc_empty_data(void) {
    ErrorHandlingModule handler;
    const uint8_t emptyData[] = "";

    uint16_t crc = handler.calculateCRC(emptyData, 0);
    TEST_ASSERT_NOT_EQUAL(0, crc); // CRC should have initial value
}

// ============================================================================
// Packet Creation Tests
// ============================================================================

void test_packet_creation(void) {
    ErrorHandlingModule handler;
    Packet packet;
    const uint8_t testData[] = "Test packet data";

    handler.createPacket(packet, 0, testData, sizeof(testData));

    TEST_ASSERT_EQUAL_UINT8(0xAA, packet.header);
    TEST_ASSERT_EQUAL_UINT8(0, packet.sequenceNum);
    TEST_ASSERT_EQUAL_MEMORY(testData, packet.data, sizeof(testData));
    TEST_ASSERT_NOT_EQUAL(0, packet.crc);
}

void test_packet_crc_is_valid(void) {
    ErrorHandlingModule handler;
    Packet packet;
    const uint8_t testData[] = "Verify CRC";

    handler.createPacket(packet, 1, testData, sizeof(testData));

    uint16_t calculatedCRC = handler.calculatePacketCRC(packet);
    TEST_ASSERT_EQUAL_UINT16(calculatedCRC, packet.crc);
}

void test_packet_sequence_numbering(void) {
    ErrorHandlingModule handler;
    Packet packets[5];
    const uint8_t testData[] = "Data";

    for (uint8_t i = 0; i < 5; i++) {
        handler.createPacket(packets[i], i, testData, sizeof(testData));
        TEST_ASSERT_EQUAL_UINT8(i, packets[i].sequenceNum);
    }
}

// ============================================================================
// Packet Integrity Checking Tests
// ============================================================================

void test_valid_packet_accepted(void) {
    ErrorHandlingModule handler;
    Packet packet;
    const uint8_t testData[] = "Valid packet";

    handler.createPacket(packet, 0, testData, sizeof(testData));
    PacketStatus status = handler.checkPacketIntegrity(packet);

    TEST_ASSERT_EQUAL(PACKET_OK, status);
}

void test_corrupted_header_detected(void) {
    ErrorHandlingModule handler;
    Packet packet;
    const uint8_t testData[] = "Test data";

    handler.createPacket(packet, 0, testData, sizeof(testData));
    packet.header = 0xBB; // Corrupt header

    PacketStatus status = handler.checkPacketIntegrity(packet);
    TEST_ASSERT_EQUAL(PACKET_CORRUPTED, status);
}

void test_corrupted_crc_detected(void) {
    ErrorHandlingModule handler;
    Packet packet;
    const uint8_t testData[] = "Test data";

    handler.createPacket(packet, 0, testData, sizeof(testData));
    packet.crc ^= 0xFFFF; // Corrupt CRC

    PacketStatus status = handler.checkPacketIntegrity(packet);
    TEST_ASSERT_EQUAL(PACKET_CORRUPTED, status);
}

void test_corrupted_data_detected(void) {
    ErrorHandlingModule handler;
    Packet packet;
    const uint8_t testData[] = "Test data";

    handler.createPacket(packet, 0, testData, sizeof(testData));
    packet.data[5] ^= 0xFF; // Corrupt data

    PacketStatus status = handler.checkPacketIntegrity(packet);
    TEST_ASSERT_EQUAL(PACKET_CORRUPTED, status);
}

void test_out_of_sequence_packet_detected(void) {
    ErrorHandlingModule handler;
    Packet packet1, packet2;
    const uint8_t testData[] = "Data";

    // Receive packet 0
    handler.createPacket(packet1, 0, testData, sizeof(testData));
    handler.receivePacket(packet1);

    // Try to receive packet 2 (skipping 1)
    handler.createPacket(packet2, 2, testData, sizeof(testData));
    PacketStatus status = handler.checkPacketIntegrity(packet2);

    TEST_ASSERT_EQUAL(PACKET_MISSING, status);
}

// ============================================================================
// Packet Reception Tests
// ============================================================================

void test_packet_reception_success(void) {
    ErrorHandlingModule handler;
    Packet packet;
    const uint8_t testData[] = "Reception test";

    handler.createPacket(packet, 0, testData, sizeof(testData));
    PacketStatus status = handler.receivePacket(packet);

    TEST_ASSERT_EQUAL(PACKET_OK, status);
    TEST_ASSERT_EQUAL_MEMORY(testData, handler.getLastPacket().data, sizeof(testData));
}

void test_sequential_packet_reception(void) {
    ErrorHandlingModule handler;
    const uint8_t testData[] = "Sequence";

    for (uint8_t i = 0; i < 10; i++) {
        Packet packet;
        handler.createPacket(packet, i, testData, sizeof(testData));
        PacketStatus status = handler.receivePacket(packet);
        TEST_ASSERT_EQUAL(PACKET_OK, status);
    }

    TEST_ASSERT_EQUAL(10, handler.getExpectedSequence());
}

void test_packet_reception_resets_retry_count(void) {
    ErrorHandlingModule handler;
    Packet packet;
    const uint8_t testData[] = "Data";

    // Simulate some retries
    for (int i = 0; i < 2; i++) {
        handler.requestRetransmission();
    }
    TEST_ASSERT_EQUAL(2, handler.getRetransmissionCount());

    // Successful reception should reset
    handler.createPacket(packet, 0, testData, sizeof(testData));
    handler.receivePacket(packet);

    TEST_ASSERT_EQUAL(0, handler.getRetransmissionCount());
}

// ============================================================================
// Retransmission Tests
// ============================================================================

void test_retransmission_request(void) {
    ErrorHandlingModule handler;

    PacketStatus status = handler.requestRetransmission();
    TEST_ASSERT_NOT_EQUAL(PACKET_RETRANSMISSION_ATTEMPT_EXCEEDED, status);
    TEST_ASSERT_EQUAL(1, handler.getRetransmissionCount());
}

void test_max_retransmissions_detected(void) {
    ErrorHandlingModule handler;
    PacketStatus status;

    for (int i = 0; i < RETRANSMISSION_MAX_ATTEMPTS; i++) {
        status = handler.requestRetransmission();
        TEST_ASSERT_NOT_EQUAL(PACKET_RETRANSMISSION_ATTEMPT_EXCEEDED, status);
    }

    // One more should exceed
    status = handler.requestRetransmission();
    TEST_ASSERT_EQUAL(PACKET_RETRANSMISSION_ATTEMPT_EXCEEDED, status);
}

void test_retransmission_count_increments(void) {
    ErrorHandlingModule handler;

    for (int i = 1; i <= RETRANSMISSION_MAX_ATTEMPTS; i++) {
        handler.requestRetransmission();
        TEST_ASSERT_EQUAL(i, handler.getRetransmissionCount());
    }
}

void test_retransmission_count_reset(void) {
    ErrorHandlingModule handler;

    handler.requestRetransmission();
    handler.requestRetransmission();
    TEST_ASSERT_EQUAL(2, handler.getRetransmissionCount());

    handler.resetRetransmissionCount();
    TEST_ASSERT_EQUAL(0, handler.getRetransmissionCount());
}

// ============================================================================
// Error Recovery Tests
// ============================================================================

void test_recovery_from_corrupted_packet(void) {
    ErrorHandlingModule handler;
    Packet packet1, packet2;
    const uint8_t testData[] = "Data";

    // Send valid packet 0
    handler.createPacket(packet1, 0, testData, sizeof(testData));
    PacketStatus status1 = handler.receivePacket(packet1);
    TEST_ASSERT_EQUAL(PACKET_OK, status1);

    // Send corrupted packet 1
    handler.createPacket(packet2, 1, testData, sizeof(testData));
    packet2.crc ^= 0xFFFF; // Corrupt
    PacketStatus status2 = handler.receivePacket(packet2);
    TEST_ASSERT_EQUAL(PACKET_CORRUPTED, status2);

    // Resend valid packet 1
    handler.createPacket(packet2, 1, testData, sizeof(testData));
    PacketStatus status3 = handler.receivePacket(packet2);
    TEST_ASSERT_EQUAL(PACKET_OK, status3);
}

void test_recovery_from_missing_packet(void) {
    ErrorHandlingModule handler;
    Packet packet;
    const uint8_t testData[] = "Data";

    // Receive packet 0
    handler.createPacket(packet, 0, testData, sizeof(testData));
    handler.receivePacket(packet);

    // Try packet 2 (missing 1)
    handler.createPacket(packet, 2, testData, sizeof(testData));
    PacketStatus status = handler.checkPacketIntegrity(packet);
    TEST_ASSERT_EQUAL(PACKET_MISSING, status);

    // Request retransmission
    handler.requestRetransmission();

    // Receive correct packet 1
    handler.createPacket(packet, 1, testData, sizeof(testData));
    handler.receivePacket(packet);

    TEST_ASSERT_EQUAL(2, handler.getExpectedSequence());
}

// ============================================================================
// Stress Tests
// ============================================================================

void test_many_consecutive_packets(void) {
    ErrorHandlingModule handler;
    const uint8_t testData[] = "Stress test";
    const int NUM_PACKETS = 256;

    for (int i = 0; i < NUM_PACKETS; i++) {
        Packet packet;
        handler.createPacket(packet, i % 256, testData, sizeof(testData));
        PacketStatus status = handler.receivePacket(packet);
        TEST_ASSERT_EQUAL(PACKET_OK, status);
    }
}

void test_alternating_valid_corrupt_packets(void) {
    ErrorHandlingModule handler;
    const uint8_t testData[] = "Data";
    int validCount = 0;
    int corruptCount = 0;

    for (int i = 0; i < 20; i++) {
        Packet packet;
        handler.createPacket(packet, i, testData, sizeof(testData));

        if (i % 2 == 1) {
            packet.crc ^= 0x0001; // Corrupt odd packets
        }

        PacketStatus status = handler.receivePacket(packet);
        if (status == PACKET_OK) {
            validCount++;
        } else {
            corruptCount++;
        }
    }

    TEST_ASSERT_EQUAL(10, validCount);
    TEST_ASSERT_EQUAL(10, corruptCount);
}

// ============================================================================
// Boundary Tests
// ============================================================================

void test_maximum_packet_size(void) {
    ErrorHandlingModule handler;
    Packet packet;
    uint8_t maxData[PACKET_SIZE];
    memset(maxData, 0xAA, sizeof(maxData));

    handler.createPacket(packet, 0, maxData, sizeof(maxData));
    PacketStatus status = handler.checkPacketIntegrity(packet);

    TEST_ASSERT_EQUAL(PACKET_OK, status);
}

void test_minimum_packet_size(void) {
    ErrorHandlingModule handler;
    Packet packet;
    const uint8_t minData[] = "";

    handler.createPacket(packet, 0, minData, 0);
    PacketStatus status = handler.checkPacketIntegrity(packet);

    TEST_ASSERT_EQUAL(PACKET_OK, status);
}

void test_sequence_number_rollover(void) {
    ErrorHandlingModule handler;
    const uint8_t testData[] = "Rollover";

    // Set to near rollover
    handler.resetSequence();
    for (int i = 0; i < 256; i++) {
        Packet packet;
        handler.createPacket(packet, i % 256, testData, sizeof(testData));
        handler.receivePacket(packet);
    }

    // Should have rolled over
    TEST_ASSERT_EQUAL(0, handler.getExpectedSequence());
}

// ============================================================================
// Test Runner
// ============================================================================

int main(int argc, char **argv) {
    UNITY_BEGIN();

    // CRC calculation tests
    RUN_TEST(test_crc_calculation_is_deterministic);
    RUN_TEST(test_crc_different_data_produces_different_crc);
    RUN_TEST(test_crc_single_bit_change_detected);
    RUN_TEST(test_crc_empty_data);

    // Packet creation tests
    RUN_TEST(test_packet_creation);
    RUN_TEST(test_packet_crc_is_valid);
    RUN_TEST(test_packet_sequence_numbering);

    // Packet integrity checking tests
    RUN_TEST(test_valid_packet_accepted);
    RUN_TEST(test_corrupted_header_detected);
    RUN_TEST(test_corrupted_crc_detected);
    RUN_TEST(test_corrupted_data_detected);
    RUN_TEST(test_out_of_sequence_packet_detected);

    // Packet reception tests
    RUN_TEST(test_packet_reception_success);
    RUN_TEST(test_sequential_packet_reception);
    RUN_TEST(test_packet_reception_resets_retry_count);

    // Retransmission tests
    RUN_TEST(test_retransmission_request);
    RUN_TEST(test_max_retransmissions_detected);
    RUN_TEST(test_retransmission_count_increments);
    RUN_TEST(test_retransmission_count_reset);

    // Error recovery tests
    RUN_TEST(test_recovery_from_corrupted_packet);
    RUN_TEST(test_recovery_from_missing_packet);

    // Stress tests
    RUN_TEST(test_many_consecutive_packets);
    RUN_TEST(test_alternating_valid_corrupt_packets);

    // Boundary tests
    RUN_TEST(test_maximum_packet_size);
    RUN_TEST(test_minimum_packet_size);
    RUN_TEST(test_sequence_number_rollover);

    return UNITY_END();
}
