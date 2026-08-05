/*
 * crypto_verify.h
 *
 *  Created on: 05-Mar-2026
 *      Author: Yashwanth
 */

#ifndef BOOTLOADER_INC_CRYPTO_VERIFY_H_
#define BOOTLOADER_INC_CRYPTO_VERIFY_H_

#include "utils.h"
#include "firmware_image.h"


/*
  Firmware image
      ↓
SHA-256 hash calculated
      ↓
Hash signed using private key (on PC/build server)
      ↓
Bootloader verifies signature using stored public key


ECDSA (Elliptic Curve Digital Signature Algorithm)


 */


/* Hash size for SHA256 */
#define SHA256_HASH_SIZE     32

/* ECDSA P256 signature size (r + s) */
#define SIGNATURE_SIZE       64


/* Verify firmware using digital signature */
uint8_t Crypto_VerifyFirmware(uint32_t firmware_addr, uint32_t firmware_size, uint8_t *signature);


/* Compute SHA256 hash — returns 0 on success, mbedTLS error code on failure */
int Crypto_SHA256(uint8_t *data, uint32_t length, uint8_t *hash_out);



#endif /* BOOTLOADER_INC_CRYPTO_VERIFY_H_ */
