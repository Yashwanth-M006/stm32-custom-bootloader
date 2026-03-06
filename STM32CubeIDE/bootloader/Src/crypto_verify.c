/*
 * crypto_verify.c
 *
 *  Created on: 05-Mar-2026
 *      Author: Yashwanth
 */


#include "crypto_verify.h"
#include "firmware_image.h"


#include "mbedtls/sha256.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/ecp.h"
#include "mbedtls/bignum.h"


/* Public key stored in bootloader
   (Example values – replace with your generated key) */

static const uint8_t PUBLIC_KEY_X[32] =
{
    0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,
    0x90,0xAB,0xCD,0xEF,0x01,0x23,0x45,0x67,
    0x89,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0x10,
    0x21,0x32,0x43,0x54,0x65,0x76,0x87,0x98
};

static const uint8_t PUBLIC_KEY_Y[32] =
{
    0xA9,0xBA,0xCB,0xDC,0xED,0xFE,0x0F,0x1A,
    0x2B,0x3C,0x4D,0x5E,0x6F,0x70,0x81,0x92,
    0xA3,0xB4,0xC5,0xD6,0xE7,0xF8,0x09,0x1B,
    0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99
};


/* ------------------------------------------------ */
/* SHA256 calculation                               */
/* ------------------------------------------------ */

void Crypto_SHA256(uint8_t *data,
                   uint32_t length,
                   uint8_t *hash_out)
{
    mbedtls_sha256_context ctx;

    mbedtls_sha256_init(&ctx);

    mbedtls_sha256_starts_ret(&ctx, 0);

    mbedtls_sha256_update_ret(&ctx, data, length);

    mbedtls_sha256_finish_ret(&ctx, hash_out);

    mbedtls_sha256_free(&ctx);
}


/* ------------------------------------------------ */
/* Firmware signature verification                  */
/* ------------------------------------------------ */

uint8_t Crypto_VerifyFirmware(uint32_t firmware_addr,
                              uint32_t firmware_size,
                              uint8_t *signature)
{
    uint8_t hash[SHA256_HASH_SIZE];

    uint8_t *firmware = (uint8_t*)firmware_addr;

    /* Compute firmware hash */

    Crypto_SHA256(firmware,
                  firmware_size,
                  hash);

    mbedtls_ecdsa_context ctx;
    mbedtls_ecp_point Q;
    mbedtls_mpi r;
    mbedtls_mpi s;

    mbedtls_ecdsa_init(&ctx);
    mbedtls_ecp_point_init(&Q);
    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);


    /* Load P-256 curve */

    mbedtls_ecp_group_load(&ctx.grp,
                           MBEDTLS_ECP_DP_SECP256R1);


    /* Load public key */

    mbedtls_mpi_read_binary(&Q.X,
                            PUBLIC_KEY_X,
                            32);

    mbedtls_mpi_read_binary(&Q.Y,
                            PUBLIC_KEY_Y,
                            32);

    mbedtls_mpi_lset(&Q.Z, 1);

    ctx.Q = Q;


    /* Extract signature */

    mbedtls_mpi_read_binary(&r,
                            signature,
                            32);

    mbedtls_mpi_read_binary(&s,
                            signature + 32,
                            32);


    /* Verify signature */

    int result = mbedtls_ecdsa_verify(&ctx.grp,
                                      hash,
                                      SHA256_HASH_SIZE,
                                      &ctx.Q,
                                      &r,
                                      &s);


    mbedtls_ecdsa_free(&ctx);
    mbedtls_ecp_point_free(&Q);
    mbedtls_mpi_free(&r);
    mbedtls_mpi_free(&s);


    if(result == 0)
        return 1;   /* Signature valid */
    else
        return 0;   /* Signature invalid */
}
