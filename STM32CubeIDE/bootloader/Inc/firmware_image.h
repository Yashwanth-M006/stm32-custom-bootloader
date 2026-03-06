/*
 * firmware_image.h
 *
 *  Created on: 05-Mar-2026
 *      Author: Yashwanth
 */

#ifndef BOOTLOADER_INC_FIRMWARE_IMAGE_H_
#define BOOTLOADER_INC_FIRMWARE_IMAGE_H_

#include "utils.h"
#include "bootloader_config.h"



/* Magic number used to identify valid firmware */

#define FW_MAGIC_NUMBER 0xDEADBEEF


/* Signature length */

#define FW_SIGNATURE_SIZE 64


/* Firmware header stored at beginning of each slot */

typedef struct
{
    uint32_t magic_number;
    uint32_t version;
    uint32_t image_size;
    uint32_t crc;

    uint8_t signature[FW_SIGNATURE_SIZE];

} FirmwareHeader_t;


/* Firmware validation results */

typedef enum
{
    FW_STATUS_INVALID = 0,
    FW_STATUS_VALID

} FirmwareStatus_t;


/* Read firmware header from flash */

FirmwareHeader_t* Firmware_ReadHeader(uint32_t slot_addr);


/* Validate magic number */

uint8_t Firmware_CheckMagic(FirmwareHeader_t *header);


/* Verify firmware CRC */

uint8_t Firmware_CheckCRC(uint32_t slot_addr, FirmwareHeader_t *header);


/* Verify firmware digital signature */

uint8_t Firmware_CheckSignature(uint32_t slot_addr, FirmwareHeader_t *header);


/* Full firmware validation */

FirmwareStatus_t Firmware_Validate(uint32_t slot_addr);



#endif /* BOOTLOADER_INC_FIRMWARE_IMAGE_H_ */
