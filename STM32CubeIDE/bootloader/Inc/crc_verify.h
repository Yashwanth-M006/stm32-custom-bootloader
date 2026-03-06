/*
 * crc_verify.h
 *
 *  Created on: 05-Mar-2026
 *      Author: Yashwanth
 */

#ifndef BOOTLOADER_INC_CRC_VERIFY_H_
#define BOOTLOADER_INC_CRC_VERIFY_H_


#include "utils.h"

/* CRC32 Polynomial used by Ethernet, ZIP etc */

#define CRC32_POLYNOMIAL 0xEDB88320
#define CRC32_INIT_VALUE 0xFFFFFFFF


/* Calculate CRC32 for data buffer */

uint32_t CRC_Calculate(uint8_t *data, uint32_t length);


/* Calculate CRC32 directly from flash memory */

uint32_t CRC_CalculateFlash(uint32_t start_addr, uint32_t size);


/* Verify firmware CRC */

uint8_t CRC_Verify(uint32_t start_addr, uint32_t size, uint32_t expected_crc);



#endif /* BOOTLOADER_INC_CRC_VERIFY_H_ */
