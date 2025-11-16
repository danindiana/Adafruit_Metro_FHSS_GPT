#include <SPI.h>

#define TRANSEC_KEY_LENGTH 32  // length of TRANSEC key in bytes
#define HOPPING_PATTERN_LENGTH 100 // number of frequency hops in the pattern

uint8_t TRANSEC_KEY[TRANSEC_KEY_LENGTH];
uint32_t frequencyHoppingPattern[HOPPING_PATTERN_LENGTH];

// SAMD51 TRNG Initialization
void initTRNG() {
  MCLK->APBCMASK.bit.TRNG_ = 1;  // enable clock  (APBC clock is already enabled)
  TRNG->CTRLA.bit.ENABLE = 1;    // enable the TRNG
}

// Get a random number using TRNG
uint32_t getTRNG() {
  while ((TRNG->INTFLAG.reg & TRNG_INTFLAG_MASK) == 0);  // Busy loop waiting for next RN to be created
  return (TRNG->DATA.reg);   // return the new number.
}

void setup() {
  Serial.begin(9600);
  while (!Serial);  // Wait for Serial Monitor to open
  initTRNG();  // Initialize the TRNG
  
  // Assume TRANSEC_KEY is already shared and stored in both devices.
  // Here we will just simulate that by generating a random TRANSEC key
  for (int i = 0; i < TRANSEC_KEY_LENGTH; i++) {
    TRANSEC_KEY[i] = (uint8_t)getTRNG();
  }
  
  // Generate frequency hopping pattern
  for (int i = 0; i < HOPPING_PATTERN_LENGTH; i++) {
    // This example assumes frequency range from 2400MHz to 2500MHz for simplicity
    frequencyHoppingPattern[i] = 2400000000 + (getTRNG() % 100000000);
  }

  // Display the frequency hopping pattern
  for (int i = 0; i < HOPPING_PATTERN_LENGTH; i++) {
    Serial.print("Hop ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(frequencyHoppingPattern[i]);
    Serial.println(" Hz");
  }
}

void loop() {
  // Nothing to do here in this example.
}
