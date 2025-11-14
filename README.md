# Adafruit Metro FHSS TRANSEC System

A Frequency-Hopping Spread Spectrum (FHSS) TRANSEC key system for Adafruit Metro M4 boards using the SAMD51's True Random Number Generator (TRNG).

## Table of Contents

- [Overview](#overview)
- [System Architecture](#system-architecture)
- [Key Features](#key-features)
- [Module Descriptions](#module-descriptions)
- [System Diagrams](#system-diagrams)
  - [System Architecture Diagram](#system-architecture-diagram)
  - [Key Exchange Flow](#key-exchange-flow)
  - [Frequency Hopping Sequence](#frequency-hopping-sequence)
  - [Inverse Multiplexing Process](#inverse-multiplexing-process)
  - [Synchronization Flow](#synchronization-flow)
  - [Module Dependencies](#module-dependencies)
- [Getting Started](#getting-started)
- [Hardware Requirements](#hardware-requirements)
- [Security Considerations](#security-considerations)

## Overview

Military radios generate a frequency-hopping pattern under the control of a secret Transmission Security Key (TRANSEC) that the sender and receiver share in advance. This project implements a similar system using Adafruit Metro M4 boards with the SAMD51's TRNG to generate and share TRANSEC keys over SPI, I2C, and other interfaces.

### Key Concepts

1. **Key Generation**: Each board uses the SAMD51's TRNG to generate cryptographically secure random TRANSEC keys
2. **Key Exchange**: Secure exchange protocol between Master and Slave devices
3. **Frequency Hopping**: Pseudorandom frequency pattern generation seeded by the TRANSEC key
4. **Inverse Multiplexing**: High-rate data streams split across multiple frequency channels for improved throughput and reliability
5. **Synchronization**: Precise timing coordination between transmitter and receiver

## System Architecture

The system consists of multiple modular components working together to provide secure, reliable RF communication:

```mermaid
graph TB
    subgraph "Adafruit Metro M4 (SAMD51)"
        TRNG[TRNG<br/>True Random Number Generator]
        KeyGen[Key Generation Module]
        TRNG --> KeyGen
    end

    subgraph "Core Modules"
        KeyExch[Key Exchange Module<br/>Master/Slave]
        FreqHop[Frequency Hopping<br/>Pattern Generator]
        Sync[Synchronization Module]
        KeyGen --> KeyExch
        KeyExch --> FreqHop
        FreqHop --> Sync
    end

    subgraph "Data Processing"
        InvMux[Inverse Multiplexer<br/>Data Splitter]
        InvDemux[Inverse Demultiplexer<br/>Data Reassembler]
        ErrCorr[Error Correction Module]
        InvMux --> ErrCorr
        ErrCorr --> InvDemux
    end

    subgraph "Security Layer"
        Encrypt[Encryption Module]
        DevAuth[Device Authentication]
        KeyExch --> DevAuth
        DevAuth --> Encrypt
    end

    subgraph "Communication Layer"
        RF[RF Communication Module]
        ErrHandle[Error Handling Module]
        Sync --> RF
        InvDemux --> RF
        Encrypt --> RF
        RF --> ErrHandle
    end

    subgraph "Interface"
        UI[User Interface]
        SPI[SPI/I2C Interface]
        UI --> SPI
        SPI --> KeyExch
    end

    subgraph "ESP32 RF Module"
        ESP32[ESP32 Wireless Transceiver]
        RF <--> ESP32
    end

    style TRNG fill:#ff9999
    style KeyGen fill:#ff9999
    style Encrypt fill:#99ccff
    style DevAuth fill:#99ccff
    style RF fill:#99ff99
    style ESP32 fill:#99ff99
```

## Key Features

- **True Random Key Generation**: Utilizes SAMD51's hardware TRNG for cryptographically secure keys
- **Secure Key Exchange**: Master-Slave architecture for TRANSEC key distribution
- **Frequency Hopping**: Dynamic frequency switching based on shared TRANSEC keys
- **Inverse Multiplexing**: Data distribution across multiple channels for:
  - Increased throughput
  - Improved reliability and redundancy
  - Enhanced security through channel diversity
  - Better load balancing
- **Robust Synchronization**: Multiple synchronization strategies with error recovery
- **Error Handling**: ARQ (Automatic Repeat Request) and forward error correction
- **Device Authentication**: Secure authentication before communication
- **Encryption**: Additional data payload encryption for enhanced security

## Module Descriptions

### Core Modules

| Module | File | Description |
|--------|------|-------------|
| **Main System** | `FHSS_TRANSEC_SAMD51_SPI.ino` | Primary implementation with TRNG, key generation, and frequency hopping |
| **Frequency Hopping** | `FrequencyHoppingPatternGenerator.ino` | Generates pseudorandom frequency patterns from TRANSEC keys |
| **Master Key Exchange** | `Master_TRANSEC_Key_Exchange.ino` | Master device for secure key distribution via SPI |
| **Slave Key Exchange** | `Slave_TRANSEC_Key_Exchange.ino` | Slave device for receiving TRANSEC keys |

### Communication Modules

| Module | File | Description |
|--------|------|-------------|
| **RF Communication** | `RFCommunicationModule.ino` | Handles packet transmission/reception and frequency changes |
| **Error Handling** | `ErrorHandlingModule.ino` | ARQ implementation for lost packets and interference |

### Synchronization Modules

| Module | File | Description |
|--------|------|-------------|
| **Basic Sync** | `SynchronizationModule.ino` | Fundamental synchronization for frequency hopping |
| **Advanced Sync** | `AdvancedSynchronizationModule.ino` | Enhanced timing with packet timestamping |
| **Robust Sync** | `RobustSynchronizationModule.ino` | Delay compensation and advanced error handling |
| **Complete Sync** | `CompleteSynchronizationModule.ino` | Full-featured synchronization with ESP32 integration |

### Inverse Multiplexing Modules

| Module | File | Description |
|--------|------|-------------|
| **Dynamic InvMux** | `DynamicInverseMultiplexerModule.ino` | Splits high-rate data across multiple channels |
| **Dynamic InvDemux** | `DynamicInverseDemultiplexerModule.ino` | Reassembles data from multiple channels |
| **Synchronized InvDemux** | `SynchronizedInverseDemultiplexerModule.ino` | Time-synchronized data reassembly |
| **Error Correction InvDemux** | `ErrorCorrectionInverseDemultiplexerModule.ino` | Demultiplexing with forward error correction |
| **Basic InvMux** | `InverseMultiplexerModule.ino` | Simple inverse multiplexing implementation |

### Security Modules

| Module | File | Description |
|--------|------|-------------|
| **Encryption** | `EncryptionModule.ino` | Data payload encryption for secure transmission |
| **Device Authentication** | `DeviceAuthenticationModule.ino` | Authenticates devices before allowing communication |

### Interface Modules

| Module | File | Description |
|--------|------|-------------|
| **User Interface** | `UserInterface.ino` | Serial/physical interface for system control and monitoring |

## System Diagrams

### Key Exchange Flow

```mermaid
sequenceDiagram
    participant M as Master Device<br/>(Metro M4)
    participant S as Slave Device<br/>(Metro M4)

    Note over M: Initialize TRNG
    M->>M: Enable MCLK->APBCMASK.TRNG_
    M->>M: Enable TRNG->CTRLA.ENABLE

    Note over M: Generate TRANSEC Key
    loop For each byte (32 bytes)
        M->>M: TRANSECKey[i] = get_trng() & 0xFF
    end

    Note over M,S: Establish SPI Connection
    M->>M: SPI.begin()
    M->>M: pinMode(SS_PIN, OUTPUT)

    Note over M,S: Secure Key Exchange
    M->>M: digitalWrite(SS_PIN, LOW)
    M->>S: SPI.transfer(TRANSECKey, KEY_LENGTH)
    M->>M: digitalWrite(SS_PIN, HIGH)

    Note over S: Receive and Store Key
    S->>S: Store received TRANSEC key

    Note over M,S: Both devices now share<br/>identical TRANSEC key

    Note over M,S: Generate Frequency Patterns
    M->>M: Generate hopping pattern from key
    S->>S: Generate hopping pattern from key

    Note over M,S: Patterns are identical<br/>Ready for synchronized communication
```

### Frequency Hopping Sequence

```mermaid
stateDiagram-v2
    [*] --> Initialize
    Initialize --> GenerateKey: Power On

    GenerateKey --> CreatePattern: TRNG → TRANSEC Key
    CreatePattern --> Frequency0: Pattern[0...N]

    state "Frequency Hopping Loop" as Loop {
        Frequency0 --> Frequency1: HOP_INTERVAL elapsed
        Frequency1 --> Frequency2: HOP_INTERVAL elapsed
        Frequency2 --> FrequencyN: HOP_INTERVAL elapsed
        FrequencyN --> Frequency0: Wrap around
    }

    Loop --> Transmit: Channel Set
    Transmit --> Loop: Continue Hopping

    note right of GenerateKey
        Use SAMD51 TRNG
        32-byte key generation
    end note

    note right of CreatePattern
        PRNG seeded with TRANSEC key
        Generates N frequencies
    end note

    note right of Loop
        Default: 500ms interval
        Synchronized between
        Master and Slave
    end note
```

### Inverse Multiplexing Process

```mermaid
flowchart LR
    subgraph Transmitter
        A[High-Rate<br/>Data Stream] --> B{Data Splitter<br/>Inverse Multiplexer}
        B --> C1[Chunk 1]
        B --> C2[Chunk 2]
        B --> C3[Chunk 3]
        B --> C4[Chunk N]

        C1 --> D1[Channel 1<br/>Freq A]
        C2 --> D2[Channel 2<br/>Freq B]
        C3 --> D3[Channel 3<br/>Freq C]
        C4 --> D4[Channel N<br/>Freq X]
    end

    subgraph "RF Medium"
        D1 -.-> E1[Channel 1]
        D2 -.-> E2[Channel 2]
        D3 -.-> E3[Channel 3]
        D4 -.-> E4[Channel N]
    end

    subgraph Receiver
        E1 --> F1[Chunk 1]
        E2 --> F2[Chunk 2]
        E3 --> F3[Chunk 3]
        E4 --> F4[Chunk N]

        F1 --> G{Data Reassembler<br/>Inverse Demultiplexer}
        F2 --> G
        F3 --> G
        F4 --> G

        G --> H[Reconstructed<br/>High-Rate Stream]
    end

    style A fill:#ffe6e6
    style H fill:#e6ffe6
    style B fill:#fff0b3
    style G fill:#fff0b3
```

**Benefits of Inverse Multiplexing:**
1. **Increased Throughput**: Aggregate bandwidth across multiple channels
2. **Improved Reliability**: Redundancy across channels mitigates interference
3. **Enhanced Security**: Eavesdropper must intercept all channels
4. **Load Balancing**: Distributes traffic across available spectrum
5. **Adaptive Performance**: Adjust channel count based on conditions

### Synchronization Flow

```mermaid
sequenceDiagram
    participant M as Master
    participant R as Receiver

    Note over M: Master Mode
    M->>M: Initialize localTime = millis()
    M->>M: Initialize localSeq = 0

    loop Every 1000ms
        M->>M: localTime += 1000
        M->>M: localSeq++
        M->>M: Create SyncPacket<br/>(header, seq, timestamp, CRC)
        M->>R: Send SyncPacket via ESP32/SPI

        alt ACK received
            R->>M: ACK
        else No ACK (timeout)
            M->>M: Retransmission attempt
            alt Retry < MAX_RETRANSMISSIONS
                M->>R: Resend SyncPacket
            else Max retries exceeded
                M->>M: Log error, continue
            end
        end
    end

    Note over R: Receiver Mode
    loop Continuous
        R->>R: Listen for SyncPacket
        M->>R: SyncPacket received

        R->>R: Verify header == 0xAA

        alt CRC Valid
            R->>R: checkCRC(packet) == true
            R->>R: localTime = packet.timestamp<br/>+ (millis() - packet.timestamp)/2
            R->>R: localSeq = packet.sequenceNumber
            R->>M: Send ACK

            Note over R: Calculate channel<br/>channelIndex = (localTime/1000) % numChannels
            R->>R: setChannel(channels[channelIndex])
        else CRC Invalid
            R->>R: checkCRC(packet) == false
            R->>M: Request Retransmission
        end
    end

    Note over M,R: Both devices synchronized<br/>Hop frequencies in unison
```

### Module Dependencies

```mermaid
graph TD
    subgraph "Hardware Layer"
        H1[SAMD51 TRNG]
        H2[SPI Interface]
        H3[ESP32 RF Module]
    end

    subgraph "Cryptographic Layer"
        C1[TRNG Module]
        C2[TRANSEC Key Generation]
        C3[Device Authentication]
        C4[Encryption Module]

        H1 --> C1
        C1 --> C2
        C2 --> C3
        C3 --> C4
    end

    subgraph "Pattern Generation"
        P1[Frequency Hopping<br/>Pattern Generator]
        P2[PRNG Seeded<br/>by TRANSEC Key]

        C2 --> P1
        P1 --> P2
    end

    subgraph "Synchronization Layer"
        S1[Basic Sync Module]
        S2[Advanced Sync Module]
        S3[Robust Sync Module]
        S4[Complete Sync Module]

        P2 --> S1
        S1 --> S2
        S2 --> S3
        S3 --> S4
    end

    subgraph "Data Processing"
        D1[Inverse Multiplexer]
        D2[Error Correction Module]
        D3[Inverse Demultiplexer]

        D1 --> D2
        D2 --> D3
    end

    subgraph "Communication Layer"
        M1[RF Communication Module]
        M2[Error Handling Module]

        S4 --> M1
        D3 --> M1
        C4 --> M1
        M1 --> M2
        H3 --> M1
    end

    subgraph "Application Layer"
        A1[User Interface]
        A2[Key Exchange<br/>Master/Slave]

        A1 --> A2
        H2 --> A2
        A2 --> C2
    end

    style H1 fill:#ffcccc
    style H2 fill:#ffcccc
    style H3 fill:#ffcccc
    style C2 fill:#ccccff
    style C4 fill:#ccccff
    style M1 fill:#ccffcc
    style M2 fill:#ccffcc
```

## Getting Started

### Basic Setup

1. **Hardware Setup**
   - Connect two Adafruit Metro M4 boards via SPI
   - Ensure one board is designated as Master, the other as Slave
   - Connect ESP32 RF module to each board (if using wireless communication)

2. **Upload Firmware**
   - Upload `Master_TRANSEC_Key_Exchange.ino` to the master board
   - Upload `Slave_TRANSEC_Key_Exchange.ino` to the slave board

3. **Key Exchange**
   - Power on the master board first
   - Power on the slave board
   - Verify key exchange success via Serial Monitor

4. **Start Communication**
   - Both boards will automatically generate identical frequency hopping patterns
   - RF communication will begin with synchronized frequency hopping

### Example Usage Flow

```mermaid
flowchart TD
    Start([Power On System]) --> Init[Initialize Hardware]
    Init --> CheckRole{Master or Slave?}

    CheckRole -->|Master| GenKey[Generate TRANSEC Key<br/>using TRNG]
    CheckRole -->|Slave| WaitKey[Wait for Key<br/>via SPI]

    GenKey --> SendKey[Send Key to Slave<br/>via SPI]
    SendKey --> GenPattern1[Generate Freq Pattern]

    WaitKey --> RecvKey[Receive TRANSEC Key]
    RecvKey --> GenPattern2[Generate Freq Pattern]

    GenPattern1 --> SyncCheck{Synchronization<br/>Established?}
    GenPattern2 --> SyncCheck

    SyncCheck -->|No| SyncProc[Run Sync Protocol]
    SyncProc --> SyncCheck

    SyncCheck -->|Yes| CommMode{Communication Mode}

    CommMode -->|Simple| BasicComm[Basic FHSS Comm]
    CommMode -->|High Throughput| InvMuxComm[Inverse Multiplexing]

    BasicComm --> HopFreq[Hop Frequencies<br/>at HOP_INTERVAL]
    InvMuxComm --> SplitData[Split Data Across<br/>Multiple Channels]

    HopFreq --> Transmit[Transmit/Receive Data]
    SplitData --> Transmit

    Transmit --> Monitor{System Status}
    Monitor -->|Error Detected| ErrorHandle[Error Handling<br/>& Recovery]
    Monitor -->|Normal| Transmit

    ErrorHandle --> Transmit

    style Start fill:#e1f5e1
    style GenKey fill:#ffe1e1
    style SyncCheck fill:#fff4e1
    style CommMode fill:#e1f0ff
```

## Hardware Requirements

- **Microcontroller**: Adafruit Metro M4 (SAMD51) x2 minimum
- **RF Module**: ESP32 or compatible RF transceiver
- **Communication**: SPI or I2C interface between boards
- **Power**: Stable 5V supply for each board
- **Optional**: LEDs for status indication, buttons for mode selection

## Security Considerations

### Strengths

1. **Cryptographic Key Generation**: Hardware TRNG provides high-quality randomness
2. **Frequency Hopping**: Difficult to intercept without knowing the pattern
3. **Inverse Multiplexing**: Requires intercepting all channels simultaneously
4. **Device Authentication**: Prevents unauthorized devices from joining network
5. **Encryption**: Additional payload encryption layer

### Important Notes

⚠️ **This is an educational/experimental implementation**

- Not certified for military or commercial security applications
- Secure key exchange (`exchangeKeys()`) requires proper cryptographic protocols (e.g., Diffie-Hellman, RSA)
- Radio frequency usage must comply with local regulations (FCC, ETSI, etc.)
- Physical security of devices is critical - compromised hardware = compromised keys

### Recommendations for Production Use

1. Implement proper asymmetric key exchange (e.g., ECDH)
2. Use established cryptographic libraries (e.g., BearSSL, Crypto)
3. Add tamper detection and secure key storage
4. Implement certificate-based authentication
5. Regular security audits and penetration testing
6. Comply with relevant RF regulations and standards

## Additional Resources

- **ESP32 Integration**: See `ESP32_Firmware_Proposal.txt` for detailed ESP32 firmware specifications
- **Original Documentation**: See `README.txt` for the original project proposal and technical discussion

## License

Educational and experimental use. Ensure compliance with local RF regulations before deploying any wireless communication system.

## Contributing

Contributions are welcome! Please ensure any modifications maintain security best practices and include appropriate documentation.

---

**Note**: This system is designed for educational purposes to demonstrate FHSS concepts, TRANSEC key management, and secure communication principles. Always consult with security professionals and ensure regulatory compliance before deploying in any real-world scenario.
