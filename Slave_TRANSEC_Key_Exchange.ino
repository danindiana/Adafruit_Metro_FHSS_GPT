// Slave.ino
#include <SPI.h>

#define KEY_LENGTH 32
#define SS_PIN 10

uint8_t TRANSECKey[KEY_LENGTH];

void setup() {
  Serial.begin(115200);
  while (!Serial); // wait for serial port to connect
  
  // Setup SPI
  SPI.begin();
  pinMode(SS_PIN, INPUT);

  // Wait for TRANSEC key from the master
  while (digitalRead(SS_PIN) == HIGH);
  SPI.transfer(TRANSECKey, KEY_LENGTH);
  while (digitalRead(SS_PIN) == LOW);
  
  Serial.println("TRANSEC key received:");
  for (int i = 0; i < KEY_LENGTH; i++) {
    Serial.print(TRANSECKey[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void loop() {
}
