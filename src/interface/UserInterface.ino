#define TRANSEC_KEY_MAX_LENGTH 32

char transecKey[TRANSEC_KEY_MAX_LENGTH];
bool isFHSSActive = false;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("FHSS TRANSEC Key System Control");
  Serial.println("Commands:");
  Serial.println("  start - Start FHSS");
  Serial.println("  stop - Stop FHSS");
  Serial.println("  setkey <key> - Set TRANSEC Key");
  Serial.println("  status - Display System Status");
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "start") {
      isFHSSActive = true;
      Serial.println("FHSS started.");
      // Start the FHSS system here
    } else if (command == "stop") {
      isFHSSActive = false;
      Serial.println("FHSS stopped.");
      // Stop the FHSS system here
    } else if (command.startsWith("setkey ")) {
      String key = command.substring(7);
      setTransecKey(key);
      Serial.println("TRANSEC Key set to: " + key);
      // Set the TRANSEC key here
    } else if (command == "status") {
      displayStatus();
    } else {
      Serial.println("Unknown command.");
    }
  }
}

void setTransecKey(String key) {
  // Ensure the key length does not exceed the maximum length
  if (key.length() > TRANSEC_KEY_MAX_LENGTH - 1) {
    key = key.substring(0, TRANSEC_KEY_MAX_LENGTH - 1);
  }

  // Copy the key to the transecKey buffer
  key.toCharArray(transecKey, TRANSEC_KEY_MAX_LENGTH);
}

void displayStatus() {
  Serial.println("System Status:");
  Serial.println("  FHSS Active: " + String(isFHSSActive ? "Yes" : "No"));
  Serial.println("  TRANSEC Key: " + String(transecKey));
  // Add more system status information here as needed
}
