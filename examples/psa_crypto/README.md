# Example Applications for PSA Crypto
This is a showcase for the use of the PSA Crypto for cryptographic operations.
It shows the following operations:
- AES 128 CBC
- HMAC SHA256
- ECDSA with a P256 curve

When building this application multiple times with different backends, in is best to remove the `bin` directory in between builds.

## Available backends
Several backends can be configured using build time options, depending on the target platform.

### Hardware
If a target platform has a cryptographic hardware accelerator, this application will automatically be built with the hardware backend.
Currently only the CryptoCell accelerator on the NRF52840dk is supported by PSA Crypto.

### Software
When building for `native` and boards without hardware accelerators, this application builds the following software backends:
- AES 128 CBC: RIOT Cipher Module
- HMAC SHA256: RIOT Hash Module
- ECDSA: Micro-ECC Package

On platforms with an accelerator, the use of software backends can be enforced by building with `SOFTWARE=1`.

### Secure Elements
When a secure element is available and correctly configured, this application can be built with the `SECURE_ELEMENT=1` option to perform all operations on an SE.
The option `MULTIPLE_SE=1` can be chosen, if there are two SEs present.

Currently a maximum of two ATECC608A devices with the configuration found in `examples/atecc608a_configure_and_lock` is supported by PSA Crypto and this application.

## Constraints
This application will not compile for boards with very small memory (e.g. the arduino-mega2560 with only 8kB RAM).
It may still compile with secure elements as backends.
