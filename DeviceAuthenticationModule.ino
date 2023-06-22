#include <SHA256.h>

#define SECRET_KEY "ThisIsASecretKey" // The secret key shared between sender and receiver

SHA256 sha256;

void setup() {
    // Begin serial communication
    Serial.begin(9600);
}

void loop() {
    // Example message to be authenticated
    char message[] = "This is the message to be authenticated";

    // Generate HMAC
    byte hmac[SHA256::HASH_SIZE];
    generateHMAC(SECRET_KEY, message, hmac);

    // Now, hmac contains the HMAC. Send this along with your message.

    // In the receiver end, the receiver should compute the HMAC using the same secret key,
    // and compare it with the received HMAC. If they match, the message is authentic.

    // Let's simulate that here:
    byte recomputedHmac[SHA256::HASH_SIZE];
    generateHMAC(SECRET_KEY, message, recomputedHmac);

    if (memcmp(hmac, recomputedHmac, SHA256::HASH_SIZE) == 0) {
        Serial.println("Message is authentic");
    } else {
        Serial.println("Message is NOT authentic");
    }

    // Pause for a while before the next authentication
    delay(5000);
}

void generateHMAC(const char *key, const char *message, byte *hmac) {
    // Generate the HMAC
    sha256.initHmac((const byte *)key, strlen(key));
    sha256.print(message);
    sha256.resultHmac(hmac);
}
