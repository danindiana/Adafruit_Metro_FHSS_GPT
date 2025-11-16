# Contributing to Adafruit Metro FHSS TRANSEC System

Thank you for your interest in contributing to this project! This document provides guidelines and information for contributors.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [How Can I Contribute?](#how-can-i-contribute)
- [Development Setup](#development-setup)
- [Coding Standards](#coding-standards)
- [Testing Guidelines](#testing-guidelines)
- [Submitting Changes](#submitting-changes)
- [Security Considerations](#security-considerations)

## Code of Conduct

This project aims to be welcoming and inclusive. When contributing:

- Be respectful and constructive in discussions
- Focus on technical merit
- Welcome newcomers and help them learn
- Acknowledge contributions from others

## How Can I Contribute?

### Reporting Bugs

Before submitting a bug report:
- Check existing issues to avoid duplicates
- Test with the latest version
- Gather relevant information (hardware setup, serial output, etc.)

When reporting bugs, include:
- Clear description of the problem
- Steps to reproduce
- Expected vs. actual behavior
- Hardware configuration
- Arduino IDE/PlatformIO version
- Serial monitor output (if applicable)

### Suggesting Enhancements

We welcome feature suggestions! Please:
- Check if the feature already exists or is planned
- Provide clear use cases
- Consider security implications
- Discuss implementation approach

### Contributing Code

We accept contributions in these areas:
- Bug fixes
- New features
- Performance improvements
- Documentation improvements
- Example sketches
- Test coverage expansion

## Development Setup

### Prerequisites

- **Hardware**: Adafruit Metro M4 (SAMD51)
- **Software**:
  - Arduino IDE 1.8.x or later, OR
  - PlatformIO (recommended)
  - Git

### Initial Setup

1. **Fork and clone the repository**
   ```bash
   git clone https://github.com/YOUR_USERNAME/Adafruit_Metro_FHSS_GPT.git
   cd Adafruit_Metro_FHSS_GPT
   ```

2. **Install dependencies** (PlatformIO will handle this automatically)
   - Adafruit Zero DMA Library
   - Crypto by rweather
   - ArduinoJson by bblanchon

3. **Build the project**
   ```bash
   # Using PlatformIO
   pio run

   # Or for specific environment
   pio run -e metro_m4
   ```

4. **Run tests**
   ```bash
   # Run unit tests (native)
   pio test -e native

   # Run hardware tests
   pio test -e metro_m4
   ```

### Project Structure

```
/
├── src/                      # Source code modules
│   ├── core/                 # Core FHSS/TRANSEC
│   ├── communication/        # RF communication
│   ├── synchronization/      # Sync modules
│   ├── inverse_multiplexing/ # Multiplexing
│   ├── security/             # Encryption/auth
│   └── interface/            # User interface
├── test/                     # Test suites
├── examples/                 # Example sketches
├── docs/                     # Documentation
└── platformio.ini            # Build configuration
```

## Coding Standards

### General Guidelines

1. **Code Style**
   - Use 2 spaces for indentation (not tabs)
   - Maximum line length: 100 characters
   - Use descriptive variable names
   - Add comments for complex logic

2. **Naming Conventions**
   - Functions: `camelCase` (e.g., `generateTransecKey()`)
   - Variables: `camelCase` (e.g., `currentFreqIndex`)
   - Constants: `UPPER_SNAKE_CASE` (e.g., `KEY_LENGTH`)
   - Classes: `PascalCase` (e.g., `FrequencyHopper`)

3. **File Organization**
   - One module per `.ino` file
   - Related functions grouped together
   - Header comments explaining purpose
   - Include guards for headers

### Example Code Style

```cpp
/*
 * Module Name
 *
 * Brief description of what this module does.
 */

#include <SPI.h>

// Constants
const int KEY_LENGTH = 32;
const int MAX_RETRIES = 3;

// Global variables
uint8_t transecKey[KEY_LENGTH];
bool keyInitialized = false;

/**
 * Initialize the TRANSEC key system
 *
 * @return true if initialization successful, false otherwise
 */
bool initializeTransecKey() {
  // Implementation here
  return true;
}

/**
 * Generate a new TRANSEC key using TRNG
 *
 * @param key Pointer to key buffer (must be KEY_LENGTH bytes)
 */
void generateTransecKey(uint8_t* key) {
  for (int i = 0; i < KEY_LENGTH; i++) {
    key[i] = getTRNG() & 0xFF;
  }
}
```

## Testing Guidelines

### Writing Tests

All new features and bug fixes should include tests. We use the Unity testing framework.

1. **Unit Tests** - Test individual functions in isolation
   ```cpp
   void test_key_generation(void) {
     uint8_t key[32];
     generateTransecKey(key);

     // Key should not be all zeros
     bool allZeros = true;
     for (int i = 0; i < 32; i++) {
       if (key[i] != 0) {
         allZeros = false;
         break;
       }
     }
     TEST_ASSERT_FALSE(allZeros);
   }
   ```

2. **Integration Tests** - Test module interactions
   - Located in `test/test_integration/`
   - Require hardware
   - Test real-world scenarios

3. **Test Organization**
   ```
   test/
   ├── test_crypto/          # Cryptography tests
   ├── test_key_exchange/    # Key exchange tests
   ├── test_sync/            # Synchronization tests
   ├── test_multiplexing/    # Multiplexing tests
   ├── test_error_handling/  # Error handling tests
   ├── test_integration/     # Integration tests
   ├── mocks/                # Mock objects
   └── helpers/              # Test utilities
   ```

### Running Tests

```bash
# All tests on native
pio test -e native

# Specific test suite
pio test -e native -f test_crypto

# Hardware tests
pio test -e metro_m4

# Integration tests
pio test -e metro_m4_integration
```

## Submitting Changes

### Pull Request Process

1. **Create a feature branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. **Make your changes**
   - Follow coding standards
   - Add tests
   - Update documentation

3. **Test your changes**
   ```bash
   pio test -e native
   pio run  # Verify compilation
   ```

4. **Commit your changes**
   ```bash
   git add .
   git commit -m "Brief description of changes

   Detailed explanation of what changed and why.
   Include any relevant issue numbers."
   ```

5. **Push to your fork**
   ```bash
   git push origin feature/your-feature-name
   ```

6. **Create Pull Request**
   - Go to GitHub and create a PR
   - Fill out the PR template
   - Link related issues
   - Request review

### Pull Request Guidelines

- **Title**: Clear, concise description (e.g., "Add AES encryption support")
- **Description**:
  - What changes were made
  - Why the changes were necessary
  - How to test the changes
  - Any breaking changes
- **Tests**: All tests must pass
- **Documentation**: Update relevant docs
- **Code Review**: Address reviewer feedback promptly

### Commit Message Format

```
Short summary (50 chars or less)

More detailed explanation if needed. Wrap at 72 characters.
Explain the problem this commit solves and why this approach
was chosen.

- Bullet points are okay
- Use present tense ("Add feature" not "Added feature")
- Reference issues: Fixes #123, Relates to #456
```

## Security Considerations

### Security-Critical Code

When working on security-related features:

1. **Never implement custom cryptography**
   - Use established libraries (Crypto, BearSSL)
   - Follow NIST/industry standards

2. **Key Management**
   - Never hardcode keys
   - Use secure random generation (TRNG)
   - Implement proper key derivation (HKDF)

3. **Input Validation**
   - Validate all external inputs
   - Check buffer boundaries
   - Sanitize user inputs

4. **Side-Channel Attacks**
   - Be aware of timing attacks
   - Use constant-time operations for crypto
   - Avoid data-dependent branches in security code

### Reporting Security Vulnerabilities

**Do NOT open public issues for security vulnerabilities.**

Instead:
1. Email the maintainer with details
2. Allow time for patch development
3. Coordinate disclosure timing

## Documentation

### Code Documentation

- Document all public functions
- Explain complex algorithms
- Include usage examples
- Note security considerations

### README Updates

When adding features:
- Update main README.md
- Add to appropriate section
- Update diagrams if needed
- Add to table of contents

### Example Contributions

When contributing examples:
- Include clear comments
- Provide hardware requirements
- Add to examples/README.md
- Test on actual hardware

## Questions?

If you have questions:
- Check existing documentation
- Search closed issues
- Open a discussion issue
- Be specific about what you need help with

## License

By contributing, you agree that your contributions will be licensed under the same license as the project (see LICENSE file).

---

Thank you for contributing to the Adafruit Metro FHSS TRANSEC System!
