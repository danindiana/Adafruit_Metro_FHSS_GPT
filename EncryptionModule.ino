#include <AESLib.h>

AESLib aesLib;

#define AES_KEY "ThisIsAESKey128bit" // Replace this with the key agreed upon between sender and receiver

#define DATA_SIZE 128 // Change according to your data size requirements
#define BLOCK_SIZE 16 // AES block size is 16 bytes

byte iv[BLOCK_SIZE]; // Initialization vector - random for each encryption and sent with the cipher

void setup() {
    // Begin serial communication
    Serial.begin(9600);
    // Initialize AES key
    aesLib.gen_iv(iv);
}

void loop() {
    // Example plaintext to be encrypted
    char data[DATA_SIZE] = "This is the plain text message that needs to be encrypted!";

    // Encrypt data
    int cipherLength = aesLib.get_cipher_length(sizeof(data));
    char cipher[cipherLength];
    aesLib.encrypt((byte *)data, sizeof(data), cipher, AES_KEY, sizeof(AES_KEY), iv);

    // Send encrypted data (cipher) using your communication protocol

    // For testing purposes, let's decrypt the data back to plaintext
    char decryptedData[DATA_SIZE];
    aesLib.decrypt((byte *)cipher, cipherLength, decryptedData, AES_KEY, sizeof(AES_KEY), iv);

    // Print decrypted data to serial
    Serial.println(decryptedData);

    // Implement your communication logic to send/receive encrypted data

    // Pause for a while before next encryption/decryption
    delay(5000);
}
