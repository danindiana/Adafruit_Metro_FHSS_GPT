# Security Modules

This directory contains modules for encryption and device authentication.

## Modules

### EncryptionModule.ino
**Purpose:** Data payload encryption for secure transmission

**Features:**
- Symmetric encryption
- Key derivation from TRANSEC key
- Authenticated encryption (AEAD)
- Nonce management

**Key Functions:**
- `initEncryption(key)` - Initialize with encryption key
- `encrypt(plaintext, length)` - Encrypt data
- `decrypt(ciphertext, length)` - Decrypt data
- `generateNonce()` - Create unique nonce

**Encryption Flow:**
```
Plaintext --> [Encrypt with Key] --> Ciphertext
Ciphertext --> [Decrypt with Key] --> Plaintext
```

**Supported Algorithms:**
- **AES-128/256** - Industry standard (recommended)
- **ChaCha20** - Fast, constant-time
- **XOR** - Educational only (NOT secure)

**Usage Example:**
```cpp
#include "EncryptionModule.h"

// Initialize with 128-bit key
uint8_t key[16];
initEncryption(key);

// Encrypt message
uint8_t plaintext[] = "Secret message";
uint8_t ciphertext[32];
int len = encrypt(plaintext, sizeof(plaintext), ciphertext);

// Decrypt message
uint8_t decrypted[32];
decrypt(ciphertext, len, decrypted);
```

**Security Considerations:**
- Never reuse nonces with the same key
- Use authenticated encryption (AES-GCM, ChaCha20-Poly1305)
- Protect keys in memory
- Clear sensitive data after use

**Key Derivation:**
```cpp
// Derive encryption key from TRANSEC key
void deriveEncryptionKey(uint8_t* transecKey, uint8_t* encKey) {
  // Use HKDF or similar KDF
  hkdf_sha256(transecKey, 32, "encryption", encKey, 16);
}
```

---

### DeviceAuthenticationModule.ino
**Purpose:** Authenticate devices before allowing communication

**Features:**
- Challenge-response authentication
- Device identity verification
- Session key establishment
- Replay attack prevention

**Key Functions:**
- `authenticateDevice(deviceId)` - Verify device identity
- `generateChallenge()` - Create authentication challenge
- `verifyResponse(response)` - Check challenge response
- `establishSession()` - Set up authenticated session

**Authentication Flow:**
```
Device A                          Device B
--------                          --------
   |                                 |
   |------ Hello (ID_A) ------------>|
   |                                 |
   |<----- Challenge (Nonce) --------|
   |                                 |
   |-- Response (Sign(Nonce,Key)) -->|
   |                                 |
   |<----- Authenticated ------------|
   |                                 |
   |<======= Secure Session ========>|
```

**Challenge-Response:**
```cpp
// Authenticator (Device B)
uint32_t challenge = generateChallenge();
sendChallenge(challenge);

// Requestor (Device A)
uint32_t response = sign(challenge, sharedSecret);
sendResponse(response);

// Authenticator verifies
if (verify(response, challenge, sharedSecret)) {
  grantAccess();
}
```

**Security Features:**

1. **Mutual Authentication** - Both devices verify each other
2. **Replay Prevention** - Nonces prevent replay attacks
3. **Session Keys** - Temporary keys for each session
4. **Timeout Protection** - Challenges expire

**Authentication Methods:**

**Pre-Shared Key (PSK):**
```cpp
uint8_t sharedSecret[32];  // Established out-of-band
bool authenticated = pskAuthenticate(deviceId, sharedSecret);
```

**Certificate-Based:**
```cpp
Certificate deviceCert;
bool authenticated = verifyCertificate(deviceCert, caCert);
```

**TRANSEC Key-Based:**
```cpp
// Devices sharing TRANSEC key can authenticate
bool authenticated = transecAuthenticate(transecKey);
```

---

## Security Architecture

### Layered Security

```
┌─────────────────────────────────────┐
│     Application Data                │
└─────────────────────────────────────┘
              ↓
┌─────────────────────────────────────┐
│     Encryption Layer                │  ← EncryptionModule
│  (AES-GCM, ChaCha20-Poly1305)       │
└─────────────────────────────────────┘
              ↓
┌─────────────────────────────────────┐
│     Authentication Layer            │  ← DeviceAuthenticationModule
│  (Challenge-Response, Certificates) │
└─────────────────────────────────────┘
              ↓
┌─────────────────────────────────────┐
│     FHSS TRANSEC Layer              │  ← Core Modules
│  (Frequency Hopping, Key Exchange)  │
└─────────────────────────────────────┘
              ↓
┌─────────────────────────────────────┐
│     Physical Layer (RF)             │
└─────────────────────────────────────┘
```

### Defense in Depth

Multiple security layers provide redundancy:

1. **Physical Security** - Frequency hopping (hard to intercept)
2. **Access Control** - Device authentication (prevent unauthorized)
3. **Confidentiality** - Encryption (protect data content)
4. **Integrity** - MAC/AEAD (detect tampering)

---

## Cryptographic Best Practices

### Key Management

**DO:**
- Generate keys using TRNG
- Use proper key derivation (HKDF)
- Store keys securely
- Rotate keys regularly
- Clear keys from memory after use

**DON'T:**
- Hardcode keys in source
- Reuse keys across contexts
- Share keys insecurely
- Use weak key generation

### Algorithm Selection

**Recommended:**
- **Encryption:** AES-128-GCM, ChaCha20-Poly1305
- **Hashing:** SHA-256, SHA-3
- **MAC:** HMAC-SHA256, Poly1305
- **KDF:** HKDF-SHA256

**Avoid:**
- Custom cryptography
- Deprecated algorithms (DES, MD5, SHA1)
- Unauthenticated encryption (ECB mode)

### Implementation Security

**Constant-Time Operations:**
```cpp
// DON'T: Variable-time comparison
bool compare(uint8_t* a, uint8_t* b, int len) {
  for (int i = 0; i < len; i++) {
    if (a[i] != b[i]) return false;  // Early exit leaks timing
  }
  return true;
}

// DO: Constant-time comparison
bool compare_ct(uint8_t* a, uint8_t* b, int len) {
  uint8_t diff = 0;
  for (int i = 0; i < len; i++) {
    diff |= (a[i] ^ b[i]);  // Always check all bytes
  }
  return (diff == 0);
}
```

**Memory Safety:**
```cpp
// Clear sensitive data
void clearSensitiveData(uint8_t* data, int len) {
  volatile uint8_t* p = data;
  while (len--) {
    *p++ = 0;
  }
}
```

---

## Security Limitations

### Known Limitations

This is an **educational/experimental** implementation:

1. **Not Certified** - No formal security audit
2. **Side-Channel Attacks** - Not hardened against timing/power analysis
3. **Physical Security** - No tamper protection
4. **Key Storage** - Keys in RAM (not secure element)

### Production Requirements

For production use, implement:

1. **Hardware Security**
   - Secure element for key storage
   - Tamper detection
   - Side-channel resistance

2. **Cryptographic Libraries**
   - Use audited libraries (BearSSL, mbedTLS)
   - Hardware acceleration where available
   - Certified implementations

3. **Key Management**
   - Proper key lifecycle
   - Certificate infrastructure
   - Key revocation mechanism

4. **Compliance**
   - FIPS 140-2/3 for cryptography
   - Common Criteria evaluation
   - Industry-specific standards

---

## Testing Security

### Unit Tests

Test encryption/decryption:
```cpp
void test_encryption() {
  uint8_t plaintext[] = "test message";
  uint8_t ciphertext[32];
  uint8_t decrypted[32];

  encrypt(plaintext, 12, ciphertext);
  decrypt(ciphertext, 12, decrypted);

  assert(memcmp(plaintext, decrypted, 12) == 0);
}
```

Test authentication:
```cpp
void test_authentication() {
  uint32_t challenge = generateChallenge();
  uint32_t response = generateResponse(challenge, key);

  assert(verifyResponse(response, challenge, key) == true);
  assert(verifyResponse(response + 1, challenge, key) == false);
}
```

### Security Testing

**Test for:**
- Replay attacks (reused nonces/challenges)
- Timing attacks (constant-time operations)
- Invalid inputs (malformed packets)
- Key reuse (different contexts)

---

## Compliance and Regulations

### Cryptographic Export

Some jurisdictions regulate cryptographic software export:
- United States: EAR, ITAR
- European Union: Dual-use regulations
- Other countries: Various restrictions

**Responsibility:** Users must comply with applicable laws.

### RF Regulations

Wireless transmission requires compliance:
- **United States:** FCC Part 15, Part 97
- **Europe:** ETSI standards
- **Other regions:** Local regulations

**Ensure compliance before transmitting.**

---

## See Also

- [Core Modules](../core/) - TRANSEC key generation
- [Communication Modules](../communication/) - Encrypted transmission
- [CONTRIBUTING.md](../../CONTRIBUTING.md) - Security contribution guidelines
- [Main README](../../README.md) - Security considerations
