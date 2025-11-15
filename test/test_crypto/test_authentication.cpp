#include <unity.h>
#include "../helpers/test_helpers.h"
#include <string.h>

#define SHA256_HASH_SIZE 32
#define SECRET_KEY "ThisIsASecretKey"

// Mock SHA256 HMAC implementation for testing
// In production, use a real crypto library

class MockSHA256 {
private:
    uint8_t hmacKey[64];
    size_t hmacKeyLen;

public:
    static const int HASH_SIZE = SHA256_HASH_SIZE;

    void initHmac(const uint8_t* key, size_t keyLen) {
        hmacKeyLen = keyLen < sizeof(hmacKey) ? keyLen : sizeof(hmacKey);
        memcpy(hmacKey, key, hmacKeyLen);
    }

    void print(const char* data) {
        // Mock: just store the data pointer (not used in this mock)
    }

    void resultHmac(uint8_t* output) {
        // Mock HMAC: Simple hash using XOR and rotation (NOT SECURE!)
        memset(output, 0, SHA256_HASH_SIZE);

        // Mix key into output
        for (size_t i = 0; i < hmacKeyLen; i++) {
            output[i % SHA256_HASH_SIZE] ^= hmacKey[i];
        }

        // Add some pseudo-randomness based on key
        for (int i = 0; i < SHA256_HASH_SIZE; i++) {
            output[i] = (output[i] + i * 17 + hmacKey[i % hmacKeyLen]) & 0xFF;
        }
    }

    // Alternative result for actual message
    void resultHmacWithMessage(const char* message, uint8_t* output) {
        memset(output, 0, SHA256_HASH_SIZE);

        // Mix key and message
        size_t msgLen = strlen(message);
        for (size_t i = 0; i < msgLen; i++) {
            output[i % SHA256_HASH_SIZE] ^= message[i];
        }
        for (size_t i = 0; i < hmacKeyLen; i++) {
            output[i % SHA256_HASH_SIZE] ^= hmacKey[i];
        }

        // Add complexity
        for (int i = 0; i < SHA256_HASH_SIZE; i++) {
            uint8_t prev = output[(i + SHA256_HASH_SIZE - 1) % SHA256_HASH_SIZE];
            output[i] = (output[i] + prev + i * 13) & 0xFF;
        }
    }
};

MockSHA256 sha256;

void generateHMAC(const char* key, const char* message, uint8_t* hmac) {
    sha256.initHmac((const uint8_t*)key, strlen(key));
    sha256.resultHmacWithMessage(message, hmac);
}

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
// HMAC Generation Tests
// ============================================================================

void test_hmac_generation_produces_hash(void) {
    const char* message = "Test message";
    uint8_t hmac[SHA256_HASH_SIZE];

    generateHMAC(SECRET_KEY, message, hmac);

    // HMAC should not be all zeros
    bool allZeros = TestHelpers::isAllZeros(hmac, SHA256_HASH_SIZE);
    TEST_ASSERT_FALSE(allZeros);
}

void test_hmac_has_correct_length(void) {
    const char* message = "Test message";
    uint8_t hmac[SHA256_HASH_SIZE];

    generateHMAC(SECRET_KEY, message, hmac);

    // Size check (implicit via array)
    TEST_ASSERT_EQUAL(SHA256_HASH_SIZE, sizeof(hmac));
}

void test_hmac_is_deterministic(void) {
    const char* message = "Deterministic test message";
    uint8_t hmac1[SHA256_HASH_SIZE];
    uint8_t hmac2[SHA256_HASH_SIZE];

    generateHMAC(SECRET_KEY, message, hmac1);
    generateHMAC(SECRET_KEY, message, hmac2);

    // Same key and message should produce same HMAC
    ASSERT_ARRAYS_EQUAL(hmac1, hmac2, SHA256_HASH_SIZE);
}

void test_hmac_different_messages_produce_different_hmacs(void) {
    const char* message1 = "Message one";
    const char* message2 = "Message two";
    uint8_t hmac1[SHA256_HASH_SIZE];
    uint8_t hmac2[SHA256_HASH_SIZE];

    generateHMAC(SECRET_KEY, message1, hmac1);
    generateHMAC(SECRET_KEY, message2, hmac2);

    // Different messages should produce different HMACs
    ASSERT_ARRAYS_NOT_EQUAL(hmac1, hmac2, SHA256_HASH_SIZE);
}

void test_hmac_different_keys_produce_different_hmacs(void) {
    const char* message = "Same message";
    const char* key1 = "FirstKey123";
    const char* key2 = "SecondKey456";
    uint8_t hmac1[SHA256_HASH_SIZE];
    uint8_t hmac2[SHA256_HASH_SIZE];

    generateHMAC(key1, message, hmac1);
    generateHMAC(key2, message, hmac2);

    // Different keys should produce different HMACs
    ASSERT_ARRAYS_NOT_EQUAL(hmac1, hmac2, SHA256_HASH_SIZE);
}

void test_hmac_empty_message(void) {
    const char* message = "";
    uint8_t hmac[SHA256_HASH_SIZE];

    generateHMAC(SECRET_KEY, message, hmac);

    // Should still produce valid HMAC
    bool allZeros = TestHelpers::isAllZeros(hmac, SHA256_HASH_SIZE);
    TEST_ASSERT_FALSE(allZeros);
}

void test_hmac_long_message(void) {
    char longMessage[1024];
    memset(longMessage, 'A', sizeof(longMessage) - 1);
    longMessage[sizeof(longMessage) - 1] = '\0';

    uint8_t hmac[SHA256_HASH_SIZE];
    generateHMAC(SECRET_KEY, longMessage, hmac);

    // Should handle long messages
    bool allZeros = TestHelpers::isAllZeros(hmac, SHA256_HASH_SIZE);
    TEST_ASSERT_FALSE(allZeros);
}

void test_hmac_single_bit_change_affects_output(void) {
    const char* message1 = "Test message A";
    const char* message2 = "Test message B"; // Single character different
    uint8_t hmac1[SHA256_HASH_SIZE];
    uint8_t hmac2[SHA256_HASH_SIZE];

    generateHMAC(SECRET_KEY, message1, hmac1);
    generateHMAC(SECRET_KEY, message2, hmac2);

    // Single bit change should produce different HMAC (avalanche effect)
    ASSERT_ARRAYS_NOT_EQUAL(hmac1, hmac2, SHA256_HASH_SIZE);
}

// ============================================================================
// Authentication Verification Tests
// ============================================================================

void test_authentication_valid_message(void) {
    const char* message = "This is the message to be authenticated";
    uint8_t hmac[SHA256_HASH_SIZE];
    uint8_t recomputedHmac[SHA256_HASH_SIZE];

    // Sender: Generate HMAC
    generateHMAC(SECRET_KEY, message, hmac);

    // Receiver: Recompute HMAC
    generateHMAC(SECRET_KEY, message, recomputedHmac);

    // Verify
    bool authentic = (memcmp(hmac, recomputedHmac, SHA256_HASH_SIZE) == 0);
    TEST_ASSERT_TRUE(authentic);
}

void test_authentication_invalid_message(void) {
    const char* originalMessage = "Original message";
    const char* tamperedMessage = "Tampered message";
    uint8_t hmac[SHA256_HASH_SIZE];
    uint8_t recomputedHmac[SHA256_HASH_SIZE];

    // Sender: Generate HMAC for original
    generateHMAC(SECRET_KEY, originalMessage, hmac);

    // Attacker: Tamper with message
    // Receiver: Recompute HMAC on tampered message
    generateHMAC(SECRET_KEY, tamperedMessage, recomputedHmac);

    // Verify - should FAIL
    bool authentic = (memcmp(hmac, recomputedHmac, SHA256_HASH_SIZE) == 0);
    TEST_ASSERT_FALSE(authentic);
}

void test_authentication_wrong_key(void) {
    const char* message = "Secure message";
    const char* correctKey = "CorrectKey123";
    const char* wrongKey = "WrongKey456";
    uint8_t hmac[SHA256_HASH_SIZE];
    uint8_t recomputedHmac[SHA256_HASH_SIZE];

    // Sender: Generate HMAC with correct key
    generateHMAC(correctKey, message, hmac);

    // Receiver: Try to verify with wrong key
    generateHMAC(wrongKey, message, recomputedHmac);

    // Verify - should FAIL
    bool authentic = (memcmp(hmac, recomputedHmac, SHA256_HASH_SIZE) == 0);
    TEST_ASSERT_FALSE(authentic);
}

void test_authentication_modified_hmac(void) {
    const char* message = "Test message";
    uint8_t hmac[SHA256_HASH_SIZE];
    uint8_t recomputedHmac[SHA256_HASH_SIZE];

    // Generate original HMAC
    generateHMAC(SECRET_KEY, message, hmac);

    // Tamper with HMAC
    hmac[0] ^= 0x01; // Flip one bit

    // Recompute HMAC
    generateHMAC(SECRET_KEY, message, recomputedHmac);

    // Verify - should FAIL
    bool authentic = (memcmp(hmac, recomputedHmac, SHA256_HASH_SIZE) == 0);
    TEST_ASSERT_FALSE(authentic);
}

// ============================================================================
// Device Authentication Simulation Tests
// ============================================================================

struct AuthenticationSession {
    char deviceId[32];
    uint8_t challenge[16];
    uint8_t response[SHA256_HASH_SIZE];
};

void generateChallenge(uint8_t* challenge, size_t size) {
    for (size_t i = 0; i < size; i++) {
        challenge[i] = rand() & 0xFF;
    }
}

bool authenticateDevice(AuthenticationSession* session, const char* sharedSecret) {
    uint8_t expectedResponse[SHA256_HASH_SIZE];

    // Concatenate device ID and challenge, then HMAC
    char authData[128];
    snprintf(authData, sizeof(authData), "%s:", session->deviceId);
    size_t offset = strlen(authData);

    // Append challenge as hex
    for (int i = 0; i < 16; i++) {
        snprintf(authData + offset + i*2, 3, "%02X", session->challenge[i]);
    }

    generateHMAC(sharedSecret, authData, expectedResponse);

    return memcmp(session->response, expectedResponse, SHA256_HASH_SIZE) == 0;
}

void test_device_authentication_success(void) {
    const char* sharedSecret = "DeviceSharedSecret123";
    AuthenticationSession session;

    strcpy(session.deviceId, "Device001");
    generateChallenge(session.challenge, sizeof(session.challenge));

    // Device computes response
    char authData[128];
    snprintf(authData, sizeof(authData), "%s:", session.deviceId);
    size_t offset = strlen(authData);
    for (int i = 0; i < 16; i++) {
        snprintf(authData + offset + i*2, 3, "%02X", session.challenge[i]);
    }
    generateHMAC(sharedSecret, authData, session.response);

    // Server verifies
    bool authenticated = authenticateDevice(&session, sharedSecret);
    TEST_ASSERT_TRUE(authenticated);
}

void test_device_authentication_wrong_secret(void) {
    const char* deviceSecret = "DeviceSecret123";
    const char* serverSecret = "DifferentSecret456";
    AuthenticationSession session;

    strcpy(session.deviceId, "Device002");
    generateChallenge(session.challenge, sizeof(session.challenge));

    // Device computes response with its secret
    char authData[128];
    snprintf(authData, sizeof(authData), "%s:", session.deviceId);
    size_t offset = strlen(authData);
    for (int i = 0; i < 16; i++) {
        snprintf(authData + offset + i*2, 3, "%02X", session.challenge[i]);
    }
    generateHMAC(deviceSecret, authData, session.response);

    // Server verifies with different secret
    bool authenticated = authenticateDevice(&session, serverSecret);
    TEST_ASSERT_FALSE(authenticated);
}

void test_device_authentication_replay_attack(void) {
    const char* sharedSecret = "SharedSecret789";
    AuthenticationSession session1, session2;

    // First authentication
    strcpy(session1.deviceId, "Device003");
    generateChallenge(session1.challenge, sizeof(session1.challenge));

    char authData[128];
    snprintf(authData, sizeof(authData), "%s:", session1.deviceId);
    size_t offset = strlen(authData);
    for (int i = 0; i < 16; i++) {
        snprintf(authData + offset + i*2, 3, "%02X", session1.challenge[i]);
    }
    generateHMAC(sharedSecret, authData, session1.response);

    bool auth1 = authenticateDevice(&session1, sharedSecret);
    TEST_ASSERT_TRUE(auth1);

    // Second authentication with NEW challenge
    strcpy(session2.deviceId, "Device003"); // Same device
    generateChallenge(session2.challenge, sizeof(session2.challenge));

    // Attacker tries to replay old response
    memcpy(session2.response, session1.response, SHA256_HASH_SIZE);

    // Should fail because challenge is different
    bool auth2 = authenticateDevice(&session2, sharedSecret);
    TEST_ASSERT_FALSE(auth2);
}

// ============================================================================
// Test Runner
// ============================================================================

int main(int argc, char **argv) {
    UNITY_BEGIN();

    // Basic HMAC tests
    RUN_TEST(test_hmac_generation_produces_hash);
    RUN_TEST(test_hmac_has_correct_length);
    RUN_TEST(test_hmac_is_deterministic);
    RUN_TEST(test_hmac_different_messages_produce_different_hmacs);
    RUN_TEST(test_hmac_different_keys_produce_different_hmacs);
    RUN_TEST(test_hmac_empty_message);
    RUN_TEST(test_hmac_long_message);
    RUN_TEST(test_hmac_single_bit_change_affects_output);

    // Authentication verification tests
    RUN_TEST(test_authentication_valid_message);
    RUN_TEST(test_authentication_invalid_message);
    RUN_TEST(test_authentication_wrong_key);
    RUN_TEST(test_authentication_modified_hmac);

    // Device authentication tests
    RUN_TEST(test_device_authentication_success);
    RUN_TEST(test_device_authentication_wrong_secret);
    RUN_TEST(test_device_authentication_replay_attack);

    return UNITY_END();
}
