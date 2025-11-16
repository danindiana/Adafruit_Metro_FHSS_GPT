/*
 * Basic FHSS Example
 *
 * This example demonstrates basic Frequency-Hopping Spread Spectrum (FHSS)
 * communication using the SAMD51's TRNG to generate a frequency hopping pattern.
 *
 * Hardware Required:
 * - Adafruit Metro M4 (SAMD51)
 * - ESP32 or compatible RF module
 *
 * This is a minimal example to get started with FHSS communication.
 */

#include <SPI.h>

// SAMD51 TRNG registers
#define TRNG_CTRLA (*(volatile uint8_t*)0x42002800)
#define TRNG_DATA  (*(volatile uint32_t*)0x42002820)

// Configuration
const int NUM_FREQUENCIES = 50;
const int HOP_INTERVAL_MS = 500;
const float BASE_FREQ = 902.0;  // MHz
const float FREQ_STEP = 0.5;    // MHz

// TRANSEC Key
uint8_t transecKey[32];
uint32_t frequencies[NUM_FREQUENCIES];
int currentFreqIndex = 0;
unsigned long lastHopTime = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("Basic FHSS Example");
  Serial.println("==================");

  // Initialize TRNG
  initTRNG();

  // Generate TRANSEC key
  generateTransecKey();

  // Generate frequency hopping pattern
  generateFrequencyPattern();

  // Display the pattern
  displayFrequencyPattern();

  Serial.println("\nStarting frequency hopping...");
}

void loop() {
  unsigned long currentTime = millis();

  // Check if it's time to hop
  if (currentTime - lastHopTime >= HOP_INTERVAL_MS) {
    hopToNextFrequency();
    lastHopTime = currentTime;
  }

  // Simulate transmission
  transmitData();

  delay(100);
}

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

void generateTransecKey() {
  Serial.println("\nGenerating TRANSEC key using hardware TRNG...");

  for (int i = 0; i < 32; i++) {
    transecKey[i] = getTRNG() & 0xFF;
  }

  Serial.print("TRANSEC Key (first 8 bytes): ");
  for (int i = 0; i < 8; i++) {
    if (transecKey[i] < 0x10) Serial.print("0");
    Serial.print(transecKey[i], HEX);
    Serial.print(" ");
  }
  Serial.println("...");
}

void generateFrequencyPattern() {
  Serial.println("\nGenerating frequency hopping pattern...");

  // Simple PRNG seeded with TRANSEC key
  uint32_t seed = 0;
  for (int i = 0; i < 4; i++) {
    seed |= ((uint32_t)transecKey[i] << (i * 8));
  }

  randomSeed(seed);

  // Generate frequency list (in units of 0.1 MHz for integer math)
  for (int i = 0; i < NUM_FREQUENCIES; i++) {
    // Generate frequency offset: 0-50 channels
    int channel = random(0, 50);
    frequencies[i] = (uint32_t)((BASE_FREQ + channel * FREQ_STEP) * 10);
  }

  Serial.println("Frequency pattern generated");
}

void displayFrequencyPattern() {
  Serial.println("\nFrequency Hopping Pattern:");
  Serial.println("---------------------------");
  for (int i = 0; i < min(10, NUM_FREQUENCIES); i++) {
    Serial.print("Channel ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(frequencies[i] / 10.0, 1);
    Serial.println(" MHz");
  }
  if (NUM_FREQUENCIES > 10) {
    Serial.println("... (showing first 10 frequencies)");
  }
}

void hopToNextFrequency() {
  currentFreqIndex = (currentFreqIndex + 1) % NUM_FREQUENCIES;

  float freq = frequencies[currentFreqIndex] / 10.0;

  Serial.print("Hopping to channel ");
  Serial.print(currentFreqIndex);
  Serial.print(": ");
  Serial.print(freq, 1);
  Serial.println(" MHz");

  // In a real implementation, you would set the RF module frequency here
  // setRFFrequency(freq);
}

void transmitData() {
  // Simulate data transmission
  // In a real implementation, you would use your RF module here

  static int packetCount = 0;

  if (packetCount % 5 == 0) {  // Print every 5th packet
    Serial.print("  Transmitting packet #");
    Serial.print(packetCount);
    Serial.print(" on ");
    Serial.print(frequencies[currentFreqIndex] / 10.0, 1);
    Serial.println(" MHz");
  }

  packetCount++;
}
