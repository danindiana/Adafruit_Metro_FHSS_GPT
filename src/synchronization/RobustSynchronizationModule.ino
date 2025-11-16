#include <SPI.h>

#define SYNC_PACKET_PIN 10
#define PACKET_HEADER 0xAA
#define MAX_RETRANSMISSIONS 3

const uint8_t channels[] = {1, 6, 11, 16, 21, 26};

struct SyncPacket {
  uint8_t header;
  uint32_t sequenceNumber;
  uint32_t timestamp;
  uint16_t crc;
};

unsigned long localTime;
unsigned long localSeq;
bool isMaster;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  pinMode(SYNC_PACKET_PIN, INPUT);

  isMaster = true; // Example: Set as master.

  localTime = millis();
  localSeq = 0;
}

void loop() {
  if (isMaster) {
    if (millis() - localTime > 1000) {
      localTime += 1000;
      localSeq++;
      sendSyncPacket();
    }
  } else {
    SyncPacket packet = receiveSyncPacket();
    if (packet.header == PACKET_HEADER) {
      if (checkCRC(packet)) {
        localTime = packet.timestamp + (millis() - packet.timestamp) / 2;
        localSeq = packet.sequenceNumber;
      } else {
        requestRetransmission();
      }
    }
  }

  uint8_t channelIndex = (localTime / 1000) % sizeof(channels);
  setChannel(channels[channelIndex]);
}

void sendSyncPacket(uint8_t retransmissions = 0) {
  if (retransmissions >= MAX_RETRANSMISSIONS) {
    return;
  }
  SyncPacket packet;
  packet.header = PACKET_HEADER;
  packet.sequenceNumber = localSeq;
  packet.timestamp = localTime;
  packet.crc = calculateCRC(packet);

  // Send packet logic here
  // ...

  // If ACK is not received, try retransmission
  if (!waitForAck()) {
    sendSyncPacket(retransmissions + 1);
  }
}

SyncPacket receiveSyncPacket() {
  // Receive sync packet logic here
  SyncPacket packet;
  packet.header = 0;
  return packet;
}

void setChannel(uint8_t channel) {
  // Logic to change to the specified channel
}

uint16_t calculateCRC(SyncPacket packet) {
  // Calculate and return CRC value
  uint16_t crc = 0xFFFF; // Example initialization value
  // Implement CRC calculation logic
  return crc;
}

bool checkCRC(SyncPacket packet) {
  return calculateCRC(packet) == packet.crc;
}

void requestRetransmission() {
  // Send a retransmission request
  // ...
}

bool waitForAck() {
  // Wait for an acknowledgment from the receiver
  // Return true if acknowledgment received, else return false
  return true; // placeholder
}
