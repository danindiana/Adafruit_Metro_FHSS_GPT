#include <unity.h>
#include "../helpers/test_helpers.h"
#include <string.h>

// Mock AES implementation for testing
// In production, this would use a real crypto library like BearSSL or Crypto

#define AES_BLOCK_SIZE 16
#define AES_KEY_SIZE 16
#define DATA_SIZE 128

// Simple XOR-based mock encryption for testing (NOT SECURE - for testing only!)
class MockAES {
private:
    uint8_t key[AES_KEY_SIZE];
    uint8_t iv[AES_BLOCK_SIZE];

public:
    void setKey(const char* keyStr) {
        memset(key, 0, AES_KEY_SIZE);
        strncpy((char*)key, keyStr, AES_KEY_SIZE);
    }

    void generateIV(uint8_t* ivOut) {
        for (int i = 0; i < AES_BLOCK_SIZE; i++) {
            ivOut[i] = (uint8_t)(rand() & 0xFF);
        }
        memcpy(iv, ivOut, AES_BLOCK_SIZE);
    }

    void setIV(const uint8_t* ivIn) {
        memcpy(iv, ivIn, AES_BLOCK_SIZE);
    }

    int getCipherLength(size_t dataLength) {
        // Pad to block size
        return ((dataLength + AES_BLOCK_SIZE - 1) / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
    }

    void encrypt(const uint8_t* plaintext, size_t ptLen, uint8_t* ciphertext,
                 const uint8_t* keyData, size_t keyLen, const uint8_t* ivData) {
        // Mock encryption: XOR with key and IV (NOT SECURE!)
        size_t cipherLen = getCipherLength(ptLen);

        for (size_t i = 0; i < cipherLen; i++) {
            uint8_t pt = (i < ptLen) ? plaintext[i] : 0; // Padding
            ciphertext[i] = pt ^ keyData[i % keyLen] ^ ivData[i % AES_BLOCK_SIZE];
        }
    }

    void decrypt(const uint8_t* ciphertext, size_t ctLen, uint8_t* plaintext,
                 const uint8_t* keyData, size_t keyLen, const uint8_t* ivData) {
        // Mock decryption: XOR with key and IV (reverses encryption)
        for (size_t i = 0; i < ctLen; i++) {
            plaintext[i] = ciphertext[i] ^ keyData[i % keyLen] ^ ivData[i % AES_BLOCK_SIZE];
        }
    }
};

MockAES aes;
uint8_t testIV[AES_BLOCK_SIZE];

// ============================================================================
// Test Setup/Teardown
// ============================================================================

void setUp(void) {
    aes.setKey("TestKey12345678");
    memset(testIV, 0, AES_BLOCK_SIZE);
    srand(12345); // Deterministic for testing
}

void tearDown(void) {
    // Cleanup
}

// ============================================================================
// AES Encryption/Decryption Tests
// ============================================================================

void test_aes_encrypt_decrypt_roundtrip(void) {
    const char* plaintext = "This is a test message for encryption!";
    size_t ptLen = strlen(plaintext) + 1; // Include null terminator

    uint8_t key[AES_KEY_SIZE] = "MySecretKey12345";
    aes.generateIV(testIV);

    // Encrypt
    int cipherLen = aes.getCipherLength(ptLen);
    uint8_t ciphertext[256];
    aes.encrypt((const uint8_t*)plaintext, ptLen, ciphertext, key, AES_KEY_SIZE, testIV);

    // Decrypt
    uint8_t decrypted[256];
    memset(decrypted, 0, sizeof(decrypted));
    aes.decrypt(ciphertext, cipherLen, decrypted, key, AES_KEY_SIZE, testIV);

    // Verify roundtrip
    TEST_ASSERT_EQUAL_STRING(plaintext, (char*)decrypted);
}

void test_aes_ciphertext_differs_from_plaintext(void) {
    const char* plaintext = "Secret message";
    size_t ptLen = strlen(plaintext) + 1;

    uint8_t key[AES_KEY_SIZE] = "MySecretKey12345";
    aes.generateIV(testIV);

    uint8_t ciphertext[256];
    aes.encrypt((const uint8_t*)plaintext, ptLen, ciphertext, key, AES_KEY_SIZE, testIV);

    // Ciphertext should be different from plaintext
    TEST_ASSERT_FALSE(memcmp(plaintext, ciphertext, ptLen) == 0);
}

void test_aes_different_keys_produce_different_ciphertext(void) {
    const char* plaintext = "Test message";
    size_t ptLen = strlen(plaintext) + 1;

    uint8_t key1[AES_KEY_SIZE] = "Key1234567890123";
    uint8_t key2[AES_KEY_SIZE] = "DifferentKey1234";
    uint8_t iv[AES_BLOCK_SIZE] = {0}; // Same IV

    uint8_t cipher1[256], cipher2[256];

    aes.encrypt((const uint8_t*)plaintext, ptLen, cipher1, key1, AES_KEY_SIZE, iv);
    aes.encrypt((const uint8_t*)plaintext, ptLen, cipher2, key2, AES_KEY_SIZE, iv);

    // Different keys should produce different ciphertext
    TEST_ASSERT_FALSE(memcmp(cipher1, cipher2, aes.getCipherLength(ptLen)) == 0);
}

void test_aes_different_iv_produces_different_ciphertext(void) {
    const char* plaintext = "Test message";
    size_t ptLen = strlen(plaintext) + 1;

    uint8_t key[AES_KEY_SIZE] = "SameKey123456789";
    uint8_t iv1[AES_BLOCK_SIZE] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    uint8_t iv2[AES_BLOCK_SIZE] = {16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1};

    uint8_t cipher1[256], cipher2[256];

    aes.encrypt((const uint8_t*)plaintext, ptLen, cipher1, key, AES_KEY_SIZE, iv1);
    aes.encrypt((const uint8_t*)plaintext, ptLen, cipher2, key, AES_KEY_SIZE, iv2);

    // Different IVs should produce different ciphertext
    TEST_ASSERT_FALSE(memcmp(cipher1, cipher2, aes.getCipherLength(ptLen)) == 0);
}

void test_aes_empty_message(void) {
    const char* plaintext = "";
    size_t ptLen = 1; // Just null terminator

    uint8_t key[AES_KEY_SIZE] = "Key1234567890123";
    aes.generateIV(testIV);

    uint8_t ciphertext[256];
    uint8_t decrypted[256];

    aes.encrypt((const uint8_t*)plaintext, ptLen, ciphertext, key, AES_KEY_SIZE, testIV);
    aes.decrypt(ciphertext, aes.getCipherLength(ptLen), decrypted, key, AES_KEY_SIZE, testIV);

    TEST_ASSERT_EQUAL_STRING(plaintext, (char*)decrypted);
}

void test_aes_large_message(void) {
    char plaintext[512];
    memset(plaintext, 'A', sizeof(plaintext) - 1);
    plaintext[sizeof(plaintext) - 1] = '\0';
    size_t ptLen = strlen(plaintext) + 1;

    uint8_t key[AES_KEY_SIZE] = "LargeTestKey1234";
    aes.generateIV(testIV);

    uint8_t ciphertext[1024];
    uint8_t decrypted[1024];

    aes.encrypt((const uint8_t*)plaintext, ptLen, ciphertext, key, AES_KEY_SIZE, testIV);
    aes.decrypt(ciphertext, aes.getCipherLength(ptLen), decrypted, key, AES_KEY_SIZE, testIV);

    TEST_ASSERT_EQUAL_STRING(plaintext, (char*)decrypted);
}

void test_aes_wrong_key_produces_garbage(void) {
    const char* plaintext = "Secure message";
    size_t ptLen = strlen(plaintext) + 1;

    uint8_t correctKey[AES_KEY_SIZE] = "CorrectKey123456";
    uint8_t wrongKey[AES_KEY_SIZE] = "WrongKey12345678";
    aes.generateIV(testIV);

    uint8_t ciphertext[256];
    uint8_t decrypted[256];

    // Encrypt with correct key
    aes.encrypt((const uint8_t*)plaintext, ptLen, ciphertext, correctKey, AES_KEY_SIZE, testIV);

    // Decrypt with wrong key
    aes.decrypt(ciphertext, aes.getCipherLength(ptLen), decrypted, wrongKey, AES_KEY_SIZE, testIV);

    // Should not match original plaintext
    TEST_ASSERT_FALSE(strcmp(plaintext, (char*)decrypted) == 0);
}

void test_aes_padding_handling(void) {
    // Test various message lengths to ensure proper padding
    const char* messages[] = {
        "A",                    // 1 byte + null
        "Short",                // 5 bytes + null
        "ExactlyBlockSz!",      // 16 bytes + null (one block)
        "This is longer than one block!" // > 16 bytes
    };

    uint8_t key[AES_KEY_SIZE] = "PaddingTestKey12";
    aes.generateIV(testIV);

    for (size_t i = 0; i < sizeof(messages) / sizeof(messages[0]); i++) {
        size_t ptLen = strlen(messages[i]) + 1;
        uint8_t ciphertext[256];
        uint8_t decrypted[256];

        aes.encrypt((const uint8_t*)messages[i], ptLen, ciphertext, key, AES_KEY_SIZE, testIV);
        aes.decrypt(ciphertext, aes.getCipherLength(ptLen), decrypted, key, AES_KEY_SIZE, testIV);

        TEST_ASSERT_EQUAL_STRING(messages[i], (char*)decrypted);
    }
}

// ============================================================================
// IV Generation Tests
// ============================================================================

void test_iv_generation_produces_nonzero(void) {
    uint8_t iv[AES_BLOCK_SIZE];
    aes.generateIV(iv);

    bool allZeros = TestHelpers::isAllZeros(iv, AES_BLOCK_SIZE);
    TEST_ASSERT_FALSE(allZeros);
}

void test_iv_generation_produces_different_values(void) {
    uint8_t iv1[AES_BLOCK_SIZE];
    uint8_t iv2[AES_BLOCK_SIZE];

    aes.generateIV(iv1);
    aes.generateIV(iv2);

    // IVs should be different
    ASSERT_ARRAYS_NOT_EQUAL(iv1, iv2, AES_BLOCK_SIZE);
}

void test_iv_has_good_randomness(void) {
    uint8_t iv[AES_BLOCK_SIZE];
    aes.generateIV(iv);

    double entropy = TestHelpers::calculateEntropy(iv, AES_BLOCK_SIZE);
    // IV should have decent entropy
    TEST_ASSERT_GREATER_THAN(3.0, entropy);
}

// ============================================================================
// Security Property Tests
// ============================================================================

void test_encryption_is_deterministic_with_same_params(void) {
    const char* plaintext = "Deterministic test";
    size_t ptLen = strlen(plaintext) + 1;

    uint8_t key[AES_KEY_SIZE] = "DeterministicKey";
    uint8_t iv[AES_BLOCK_SIZE] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

    uint8_t cipher1[256], cipher2[256];

    // Encrypt twice with same parameters
    aes.encrypt((const uint8_t*)plaintext, ptLen, cipher1, key, AES_KEY_SIZE, iv);
    aes.encrypt((const uint8_t*)plaintext, ptLen, cipher2, key, AES_KEY_SIZE, iv);

    // Should produce identical ciphertext
    ASSERT_ARRAYS_EQUAL(cipher1, cipher2, aes.getCipherLength(ptLen));
}

void test_single_bit_change_affects_ciphertext(void) {
    char plaintext1[] = "Test message A";
    char plaintext2[] = "Test message B"; // Single char different
    size_t ptLen = strlen(plaintext1) + 1;

    uint8_t key[AES_KEY_SIZE] = "TestKey123456789";
    uint8_t iv[AES_BLOCK_SIZE] = {0};

    uint8_t cipher1[256], cipher2[256];

    aes.encrypt((const uint8_t*)plaintext1, ptLen, cipher1, key, AES_KEY_SIZE, iv);
    aes.encrypt((const uint8_t*)plaintext2, ptLen, cipher2, key, AES_KEY_SIZE, iv);

    // Single bit change should produce different ciphertext
    TEST_ASSERT_FALSE(memcmp(cipher1, cipher2, aes.getCipherLength(ptLen)) == 0);
}

// ============================================================================
// Test Runner
// ============================================================================

int main(int argc, char **argv) {
    UNITY_BEGIN();

    // Basic encryption/decryption tests
    RUN_TEST(test_aes_encrypt_decrypt_roundtrip);
    RUN_TEST(test_aes_ciphertext_differs_from_plaintext);
    RUN_TEST(test_aes_different_keys_produce_different_ciphertext);
    RUN_TEST(test_aes_different_iv_produces_different_ciphertext);
    RUN_TEST(test_aes_empty_message);
    RUN_TEST(test_aes_large_message);
    RUN_TEST(test_aes_wrong_key_produces_garbage);
    RUN_TEST(test_aes_padding_handling);

    // IV generation tests
    RUN_TEST(test_iv_generation_produces_nonzero);
    RUN_TEST(test_iv_generation_produces_different_values);
    RUN_TEST(test_iv_has_good_randomness);

    // Security property tests
    RUN_TEST(test_encryption_is_deterministic_with_same_params);
    RUN_TEST(test_single_bit_change_affects_ciphertext);

    return UNITY_END();
}
