# Interface Module

This directory contains the user interface module for system control and monitoring.

## Module

### UserInterface.ino
**Purpose:** Serial and physical interface for system control and monitoring

**Features:**
- Serial command interface
- Status display
- Configuration management
- Diagnostics and debugging
- User feedback (LEDs, buzzer)

---

## Serial Interface

### Command Structure

Commands are sent via Serial Monitor at **115200 baud**.

```
COMMAND [PARAMETER] [VALUE]
```

### Available Commands

#### System Commands

```
HELP                  - Display available commands
STATUS                - Show system status
RESET                 - Reset system to defaults
VERSION               - Display firmware version
```

#### Key Management

```
GENKEY                - Generate new TRANSEC key
SHOWKEY               - Display current key (first 8 bytes)
SENDKEY               - Transmit key to slaves (Master only)
CLEARKEY              - Clear current key
```

#### Frequency Hopping

```
SHOWFREQ              - Display frequency pattern
HOP                   - Manual frequency hop
SETHOP <ms>           - Set hop interval (milliseconds)
```

#### Communication

```
SEND <message>        - Transmit message
RECEIVE               - Enter receive mode
STATS                 - Show communication statistics
```

#### Diagnostics

```
TEST                  - Run self-test
TRNG                  - Test TRNG output
RSSI                  - Show signal strength
CHANNELS              - Display channel quality
```

### Example Session

```
> HELP
Available commands: HELP, STATUS, GENKEY, SHOWKEY, ...

> STATUS
System Status:
- Mode: Master
- Key: Set
- Frequency: 902.5 MHz
- Hop Interval: 500 ms
- Packets Sent: 142
- Packets Received: 138
- Success Rate: 97.2%

> GENKEY
Generating TRANSEC key...
Key generated: A3 F2 8B 1C ...

> SEND Hello World
Transmitting: "Hello World"
[ACK] Message sent successfully

> STATS
Communication Statistics:
- Total Packets: 280
- Successful: 272
- Failed: 8
- Success Rate: 97.1%
- Average RSSI: -65 dBm
```

---

## Physical Interface

### LED Indicators

**Power LED (Green)**
- **ON:** System powered
- **OFF:** No power

**Status LED (Blue)**
- **Solid:** System ready
- **Slow Blink:** Normal operation
- **Fast Blink:** Transmitting/Receiving
- **OFF:** Error state

**Error LED (Red)**
- **OFF:** No errors
- **Slow Blink:** Warning (degraded performance)
- **Fast Blink:** Error (sync lost, high packet loss)
- **Solid:** Critical error (system halted)

**Sync LED (Yellow)**
- **OFF:** Not synchronized
- **Blinking:** Synchronizing
- **Solid:** Synchronized

### Button Interface

**Mode Button**
- **Short Press:** Cycle display modes
- **Long Press (3s):** Enter configuration mode

**Action Button**
- **Short Press:** Manual frequency hop
- **Long Press (3s):** Generate new TRANSEC key
- **Double Press:** Send sync packet

**Reset Button**
- **Press:** Soft reset
- **Long Press (10s):** Factory reset

---

## Display Modes

### Mode 1: Status Display

```
FHSS TRANSEC v1.0
Mode: Master
Freq: 902.5 MHz
RSSI: -65 dBm
```

### Mode 2: Statistics

```
Packets: 142/138
Success: 97.2%
Sync: OK
Errors: 0
```

### Mode 3: Channel Quality

```
Ch1: ████████░░ 80%
Ch2: ██████████ 100%
Ch3: ██████░░░░ 60%
Ch4: ████████░░ 85%
```

---

## Configuration

### Serial Configuration

```cpp
const int SERIAL_BAUD = 115200;
const int COMMAND_TIMEOUT_MS = 5000;
const int MAX_COMMAND_LENGTH = 64;
```

### LED Pin Assignments

```cpp
const int LED_POWER = 2;
const int LED_STATUS = 3;
const int LED_ERROR = 4;
const int LED_SYNC = 5;
```

### Button Pin Assignments

```cpp
const int BTN_MODE = 6;
const int BTN_ACTION = 7;
const int BTN_RESET = 8;

// Button timing
const int DEBOUNCE_MS = 50;
const int LONG_PRESS_MS = 3000;
```

---

## Implementation Example

### Command Parser

```cpp
void processCommand(String cmd) {
  cmd.trim();
  cmd.toUpperCase();

  if (cmd == "HELP") {
    printHelp();
  }
  else if (cmd == "STATUS") {
    printStatus();
  }
  else if (cmd == "GENKEY") {
    generateNewKey();
  }
  else if (cmd.startsWith("SEND ")) {
    String message = cmd.substring(5);
    sendMessage(message);
  }
  else {
    Serial.println("Unknown command. Type HELP for commands.");
  }
}
```

### LED Control

```cpp
void updateStatusLED() {
  static unsigned long lastBlink = 0;
  static bool ledState = false;

  if (systemError) {
    digitalWrite(LED_ERROR, HIGH);
    return;
  }

  if (millis() - lastBlink > BLINK_INTERVAL) {
    ledState = !ledState;
    digitalWrite(LED_STATUS, ledState);
    lastBlink = millis();
  }
}
```

### Button Handling

```cpp
void handleButton() {
  static unsigned long pressStart = 0;
  static bool wasPressed = false;

  bool pressed = (digitalRead(BTN_ACTION) == LOW);

  if (pressed && !wasPressed) {
    // Button just pressed
    pressStart = millis();
  }
  else if (!pressed && wasPressed) {
    // Button just released
    unsigned long pressDuration = millis() - pressStart;

    if (pressDuration > LONG_PRESS_MS) {
      handleLongPress();
    } else {
      handleShortPress();
    }
  }

  wasPressed = pressed;
}
```

---

## Status Information

### System Status

Display current system state:
- Operating mode (Master/Slave)
- TRANSEC key status
- Current frequency
- Synchronization status
- Error conditions

### Communication Statistics

Track performance metrics:
- Packets sent/received
- Success/failure rate
- Average RSSI
- Retry count
- Error count

### Diagnostics

Provide debugging information:
- Memory usage
- TRNG health
- RF module status
- Sync quality
- Channel conditions

---

## Error Reporting

### Error Codes

```
E001 - TRNG Initialization Failed
E002 - RF Module Not Responding
E003 - Key Exchange Failed
E004 - Synchronization Lost
E005 - High Packet Loss (>20%)
E006 - Memory Allocation Failed
E007 - Invalid Configuration
E008 - Hardware Malfunction
```

### Error Handling

```cpp
void reportError(int errorCode, String message) {
  Serial.print("ERROR E");
  if (errorCode < 100) Serial.print("0");
  if (errorCode < 10) Serial.print("0");
  Serial.print(errorCode);
  Serial.print(": ");
  Serial.println(message);

  // Visual indication
  blinkErrorLED(errorCode);

  // Log to persistent storage if available
  logError(errorCode, message);
}
```

---

## Advanced Features

### Scripting Support

Execute command sequences:

```
> SCRIPT
BEGIN
  GENKEY
  WAIT 100
  SHOWKEY
  SENDKEY
END
Script executed successfully
```

### Data Logging

Log events to SD card or serial:

```cpp
void logEvent(String event) {
  String timestamp = getTimestamp();
  String logEntry = timestamp + " - " + event;

  // Serial logging
  Serial.println(logEntry);

  // SD card logging (if available)
  if (sdAvailable) {
    logFile.println(logEntry);
  }
}
```

### Remote Configuration

Configure via RF commands (authenticated):

```
Remote Command: SET HOP_INTERVAL 1000
Authenticating...
Configuration updated: HOP_INTERVAL = 1000 ms
```

---

## Customization

### Adding New Commands

```cpp
// 1. Add command string
const String CMD_MYCOMMAND = "MYCOMMAND";

// 2. Add to command processor
void processCommand(String cmd) {
  // ... existing commands ...

  else if (cmd == CMD_MYCOMMAND) {
    handleMyCommand();
  }
}

// 3. Implement handler
void handleMyCommand() {
  Serial.println("My command executed!");
  // Your implementation here
}

// 4. Add to help text
void printHelp() {
  Serial.println("MYCOMMAND - Does something useful");
}
```

### Custom LED Patterns

```cpp
void blinkPattern(int pattern) {
  switch (pattern) {
    case PATTERN_SUCCESS:
      // Quick double blink
      blink(LED_STATUS, 100, 2);
      break;

    case PATTERN_ERROR:
      // SOS pattern
      sos(LED_ERROR);
      break;

    case PATTERN_SYNC:
      // Fade in/out
      fade(LED_SYNC, 1000);
      break;
  }
}
```

---

## Accessibility

### Serial Output Formatting

- **Clear labels:** All output clearly labeled
- **Consistent format:** Standardized output structure
- **Progress indicators:** Show long operations
- **Error messages:** Descriptive error explanations

### Visual Feedback

- **Color coding:** Different LEDs for different states
- **Blink patterns:** Distinguishable patterns
- **Brightness levels:** Variable brightness for severity

---

## See Also

- [Core Modules](../core/) - System components
- [Examples](../../examples/) - Usage examples
- [CONTRIBUTING.md](../../CONTRIBUTING.md) - Adding UI features
- [Main README](../../README.md) - System overview
