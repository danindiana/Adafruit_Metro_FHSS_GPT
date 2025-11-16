# Repository Badges

This document explains the badges displayed in the README and what they indicate.

## Status Badges

### CI (Continuous Integration)
[![CI](https://github.com/danindiana/Adafruit_Metro_FHSS_GPT/workflows/CI/badge.svg)](https://github.com/danindiana/Adafruit_Metro_FHSS_GPT/actions)

**What it shows:** Current status of the automated testing workflow

**Statuses:**
- ![passing](https://img.shields.io/badge/build-passing-brightgreen) - All tests passed
- ![failing](https://img.shields.io/badge/build-failing-red) - One or more tests failed
- ![pending](https://img.shields.io/badge/build-pending-yellow) - Tests are running

**What's tested:**
- Unit tests (native environment)
- Build verification for Metro M4
- Code quality checks
- Documentation validation
- Examples compilation
- Security scanning

**Click the badge to:** View detailed test results and logs

---

## Technology Badges

### PlatformIO
[![PlatformIO](https://img.shields.io/badge/PlatformIO-Compatible-orange.svg)](https://platformio.org/)

**What it shows:** This project uses PlatformIO for building and testing

**Benefits:**
- Unified build system
- Automated dependency management
- Multi-platform testing
- IDE integration

**Getting started:** See [Development Setup](CONTRIBUTING.md#development-setup)

---

### License
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

**What it shows:** Project is licensed under MIT License

**This means you can:**
- ✓ Use commercially
- ✓ Modify
- ✓ Distribute
- ✓ Use privately

**With conditions:**
- Include copyright notice
- Include license text

**Important:** See [LICENSE](LICENSE) for educational use disclaimers and regulatory compliance notes

---

### Hardware
[![Hardware](https://img.shields.io/badge/Hardware-Adafruit%20Metro%20M4-blue.svg)](https://www.adafruit.com/product/3382)

**What it shows:** Target hardware platform

**Specifications:**
- **Board:** Adafruit Metro M4 Express
- **MCU:** ATSAMD51J19A
- **Clock:** 120 MHz ARM Cortex-M4F
- **RAM:** 192 KB
- **Flash:** 512 KB
- **Special:** Hardware TRNG

**Purchase:** [Adafruit Store](https://www.adafruit.com/product/3382)

---

### Platform
[![Platform](https://img.shields.io/badge/Platform-SAMD51-green.svg)](https://www.microchip.com/en-us/product/ATSAMD51J19A)

**What it shows:** Microcontroller family

**SAMD51 Features Used:**
- True Random Number Generator (TRNG)
- Hardware cryptography support
- SPI/I2C peripherals
- High-performance ARM Cortex-M4F

**Datasheet:** [Microchip ATSAMD51](https://www.microchip.com/en-us/product/ATSAMD51J19A)

---

### Framework
[![Framework](https://img.shields.io/badge/Framework-Arduino-00979D.svg)](https://www.arduino.cc/)

**What it shows:** Built using Arduino framework

**Why Arduino:**
- Familiar API for embedded developers
- Large ecosystem of libraries
- Cross-platform development
- Easy prototyping

**Compatibility:** Arduino IDE 1.8.x+ or PlatformIO

---

### Language
[![Language](https://img.shields.io/badge/Language-C%2B%2B-blue.svg)](https://isocpp.org/)

**What it shows:** Primary programming language

**C++ Features Used:**
- Object-oriented design
- Templates (for generic code)
- RAII (Resource Acquisition Is Initialization)
- Modern C++11/14 features

**Standard:** C++11 minimum

---

## Community Badges

### PRs Welcome
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](CONTRIBUTING.md)

**What it shows:** Contributions are encouraged

**How to contribute:**
1. Read [CONTRIBUTING.md](CONTRIBUTING.md)
2. Fork the repository
3. Create feature branch
4. Make changes with tests
5. Submit pull request

**We welcome:**
- Bug fixes
- New features
- Documentation improvements
- Examples
- Test coverage

---

### Documentation
[![Documentation](https://img.shields.io/badge/docs-comprehensive-brightgreen.svg)](docs/)

**What it shows:** Project has extensive documentation

**Documentation includes:**
- Main README with architecture diagrams
- Module-specific READMEs in each src/ subdirectory
- API documentation
- Usage examples
- Contributing guidelines

**Browse docs:**
- [Main Documentation](README.md)
- [Core Modules](src/core/README.md)
- [Communication](src/communication/README.md)
- [Synchronization](src/synchronization/README.md)
- [Inverse Multiplexing](src/inverse_multiplexing/README.md)
- [Security](src/security/README.md)
- [Interface](src/interface/README.md)

---

### Examples
[![Examples](https://img.shields.io/badge/examples-3-blue.svg)](examples/)

**What it shows:** Number of working examples included

**Available examples:**
1. **BasicFHSS** - Introduction to frequency hopping
2. **MasterSlaveKeyExchange** - Key distribution between devices
3. **SecureTransmission** - Complete secure communication system

**Each example includes:**
- Full source code
- Hardware requirements
- Wiring diagrams (where applicable)
- Detailed comments
- Usage instructions

**Browse examples:** [examples/](examples/)

---

## Dynamic Badges (Future)

These badges could be added as the project grows:

### Code Coverage
```markdown
[![codecov](https://codecov.io/gh/danindiana/Adafruit_Metro_FHSS_GPT/branch/main/graph/badge.svg)](https://codecov.io/gh/danindiana/Adafruit_Metro_FHSS_GPT)
```
Shows percentage of code covered by tests

### Release Version
```markdown
[![GitHub release](https://img.shields.io/github/release/danindiana/Adafruit_Metro_FHSS_GPT.svg)](https://github.com/danindiana/Adafruit_Metro_FHSS_GPT/releases)
```
Shows latest release version

### Downloads
```markdown
[![GitHub downloads](https://img.shields.io/github/downloads/danindiana/Adafruit_Metro_FHSS_GPT/total.svg)](https://github.com/danindiana/Adafruit_Metro_FHSS_GPT/releases)
```
Shows total download count

### Contributors
```markdown
[![GitHub contributors](https://img.shields.io/github/contributors/danindiana/Adafruit_Metro_FHSS_GPT.svg)](https://github.com/danindiana/Adafruit_Metro_FHSS_GPT/graphs/contributors)
```
Shows number of contributors

### Issues
```markdown
[![GitHub issues](https://img.shields.io/github/issues/danindiana/Adafruit_Metro_FHSS_GPT.svg)](https://github.com/danindiana/Adafruit_Metro_FHSS_GPT/issues)
```
Shows open issues count

### Stars
```markdown
[![GitHub stars](https://img.shields.io/github/stars/danindiana/Adafruit_Metro_FHSS_GPT.svg)](https://github.com/danindiana/Adafruit_Metro_FHSS_GPT/stargazers)
```
Shows repository stars

---

## Custom Badges

You can create custom badges using [shields.io](https://shields.io/):

```markdown
![Custom](https://img.shields.io/badge/your-badge-color.svg)
```

**Examples:**
- `![Status](https://img.shields.io/badge/status-experimental-orange.svg)`
- `![Security](https://img.shields.io/badge/security-educational-yellow.svg)`
- `![RF](https://img.shields.io/badge/RF-902--928MHz-blue.svg)`

---

## Badge Best Practices

1. **Keep it relevant** - Only show badges that add value
2. **Maintain accuracy** - Update badges when project changes
3. **Link to details** - Make badges clickable for more info
4. **Limit quantity** - Too many badges = visual clutter
5. **Order logically** - Status first, then tech, then community

---

## Questions?

If you have questions about any badge or want to suggest new ones, please open an issue!
