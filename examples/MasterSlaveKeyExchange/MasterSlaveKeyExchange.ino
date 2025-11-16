/*
 * Master-Slave TRANSEC Key Exchange Example
 *
 * This example demonstrates secure key exchange between two Metro M4 boards.
 * Upload this sketch to both boards, then use the Serial Monitor to select
 * Master or Slave mode.
 *
 * Hardware Required:
 * - 2x Adafruit Metro M4 (SAMD51)
 * - SPI connection between boards:
 *   - Master MOSI (11) -> Slave MOSI (11)
 *   - Master MISO (12) -> Slave MISO (12)
 *   - Master SCK (13) -> Slave SCK (13)
 *   - Master SS (10) -> Slave SS (10)
 *   - Common GND
 *
 * Instructions:
 * 1. Upload this sketch to both boards
 * 2. Open Serial Monitor for first board, type 'M' for Master
 * 3. Open Serial Monitor for second board, type 'S' for Slave
 * 4. Master will generate and send key to Slave
 */

#include <SPI.h>

// SAMD51 TRNG registers
#define TRNG_CTRLA (*(volatile uint8_t*)0x42002800)
#define TRNG_DATA  (*(volatile uint32_t*)0x42002820)

// Configuration
#define SS_PIN 10
#define KEY_LENGTH 32

// Device mode
enum DeviceMode {
  MODE_NONE,
  MODE_MASTER,
  MODE_SLAVE
};

DeviceMode deviceMode = MODE_NONE;
uint8_t transecKey[KEY_LENGTH];
bool keyReceived = false;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("Master-Slave TRANSEC Key Exchange");
  Serial.println("==================================");
  Serial.println();
  Serial.println("Select mode:");
  Serial.println("  M - Master (generates and sends key)");
  Serial.println("  S - Slave (receives key)");
  Serial.println();
  Serial.print("Enter mode: ");

  // Wait for mode selection
  while (deviceMode == MODE_NONE) {
    if (Serial.available()) {
      char mode = Serial.read();
      if (mode == 'M' || mode == 'm') {
        deviceMode = MODE_MASTER;
        Serial.println("MASTER");
        setupMaster();
      } else if (mode == 'S' || mode == 's') {
        deviceMode = MODE_SLAVE;
        Serial.println("SLAVE");
        setupSlave();
      }
    }
  }
}

void loop() {
  if (deviceMode == MODE_MASTER) {
    loopMaster();
  } else if (deviceMode == MODE_SLAVE) {
    loopSlave();
  }
}

// ============================================================================
// MASTER MODE
// ============================================================================

void setupMaster() {
  Serial.println("\n--- MASTER MODE ---");

  // Initialize TRNG
  initTRNG();

  // Initialize SPI as master
  pinMode(SS_PIN, OUTPUT);
  digitalWrite(SS_PIN, HIGH);
  SPI.begin();

  Serial.println("Master initialized");
  Serial.println("\nPress ENTER to generate and send TRANSEC key...");
}

void loopMaster() {
  static bool keySent = false;

  if (!keySent && Serial.available()) {
    Serial.read();  // Clear the input

    // Generate TRANSEC key
    Serial.println("\nGenerating TRANSEC key...");
    generateTransecKey();
    displayKey();

    // Send key to slave
    Serial.println("\nSending key to slave...");
    sendKeyToSlave();

    keySent = true;
    Serial.println("\nKey exchange complete!");
    Serial.println("Both Master and Slave now share the same TRANSEC key.");
  }
}

void generateTransecKey() {
  for (int i = 0; i < KEY_LENGTH; i++) {
    transecKey[i] = getTRNG() & 0xFF;
  }
}

void sendKeyToSlave() {
  digitalWrite(SS_PIN, LOW);
  delay(10);  // Give slave time to prepare

  for (int i = 0; i < KEY_LENGTH; i++) {
    SPI.transfer(transecKey[i]);
    delay(1);  // Small delay between bytes
  }

  digitalWrite(SS_PIN, HIGH);
  Serial.println("Key transmitted via SPI");
}

// ============================================================================
// SLAVE MODE
// ============================================================================

void setupSlave() {
  Serial.println("\n--- SLAVE MODE ---");

  // Initialize SPI as slave
  pinMode(SS_PIN, INPUT);
  pinMode(MISO, OUTPUT);
  SPI.begin();

  // Note: For true SPI slave mode, you would need to configure
  // SERCOM in slave mode. This is a simplified example.

  Serial.println("Slave initialized");
  Serial.println("Waiting for key from master...");
}

void loopSlave() {
  if (!keyReceived) {
    // Check if SS pin is LOW (master is sending)
    if (digitalRead(SS_PIN) == LOW) {
      receiveKeyFromMaster();
      keyReceived = true;

      Serial.println("\nKey received from master!");
      displayKey();
      Serial.println("\nKey exchange complete!");
      Serial.println("Both Master and Slave now share the same TRANSEC key.");
    }
  }
}

void receiveKeyFromMaster() {
  delay(10);  // Synchronize with master

  for (int i = 0; i < KEY_LENGTH; i++) {
    // In a real implementation, this would receive via SPI slave
    // This is simplified for demonstration
    while (digitalRead(SS_PIN) == LOW) {
      if (SPI.available()) {
        transecKey[i] = SPI.transfer(0xFF);
        break;
      }
    }
  }
}

// ============================================================================
// SHARED FUNCTIONS
// ============================================================================

void initTRNG() {
  // Enable TRNG in MCLK
  MCLK->APBCMASK.bit.TRNG_ = 1;

  // Enable TRNG
  TRNG_CTRLA |= 0x02;  // Enable bit

  Serial.println("TRNG initialized");
}

uint32_t getTRNG() {
  while (!(TRNG_CTRLA & 0x01));  // Wait for data ready
  return TRNG_DATA;
}

void displayKey() {
  Serial.println("\nTRANSEC Key:");
  Serial.println("-------------");

  // Display in rows of 8 bytes
  for (int i = 0; i < KEY_LENGTH; i++) {
    if (transecKey[i] < 0x10) Serial.print("0");
    Serial.print(transecKey[i], HEX);
    Serial.print(" ");

    if ((i + 1) % 8 == 0) {
      Serial.println();
    }
  }
  Serial.println();
}
