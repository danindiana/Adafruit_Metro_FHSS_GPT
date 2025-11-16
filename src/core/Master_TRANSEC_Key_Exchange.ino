// Master.ino
#include <SPI.h>

#define KEY_LENGTH 32
#define SS_PIN 10

uint8_t TRANSECKey[KEY_LENGTH];

void setup() {
  Serial.begin(115200);
  while (!Serial); // wait for serial port to connect
  
  // Initialize TRNG
  MCLK->APBCMASK.bit.TRNG_ = 1;  // enable clock
  TRNG->CTRLA.bit.ENABLE = 1;    // enable the TRNG
  
  // Generate TRANSEC key
  generateTRANSECKey();

  // Setup SPI
  SPI.begin();
  pinMode(SS_PIN, OUTPUT);

  // Send TRANSEC key to the slave
  digitalWrite(SS_PIN, LOW);
  SPI.transfer(TRANSECKey, KEY_LENGTH);
  digitalWrite(SS_PIN, HIGH);
  
  Serial.println("TRANSEC key sent.");
}

void loop() {
}

void generateTRANSECKey() {
  for (int i = 0; i < KEY_LENGTH; i++) {
    TRANSECKey[i] = get_trng() & 0xFF;
  }
}

uint32_t get_trng() {
  while ((TRNG->INTFLAG.reg & TRNG_INTFLAG_MASK) == 0);
  return (TRNG->DATA.reg);
}
