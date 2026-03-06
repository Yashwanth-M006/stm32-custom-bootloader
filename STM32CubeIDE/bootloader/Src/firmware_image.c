/*
 * firmware_image.c
 *
 *  Created on: 05-Mar-2026
 *      Author: Yashwanth
 */


#include "firmware_image.h"
#include "crc_verify.h"
#include "crypto_verify.h"



/* ------------------------------------------------ */
/* Read firmware header                             */
/* ------------------------------------------------ */

FirmwareHeader_t* Firmware_ReadHeader(uint32_t slot_addr)
{
    return (FirmwareHeader_t*)slot_addr;
}



/* ------------------------------------------------ */
/* Check magic number                               */
/* ------------------------------------------------ */

uint8_t Firmware_CheckMagic(FirmwareHeader_t *header)
{
    if(header->magic_number == FW_MAGIC_NUMBER)
        return 1;
    else
        return 0;
}



/* ------------------------------------------------ */
/* Verify CRC                                       */
/* ------------------------------------------------ */

uint8_t Firmware_CheckCRC(uint32_t slot_addr, FirmwareHeader_t *header)
{
    uint32_t firmware_start;

    firmware_start = slot_addr + sizeof(FirmwareHeader_t);

    return CRC_Verify(firmware_start, header->image_size, header->crc);
}



/* ------------------------------------------------ */
/* Verify digital signature                         */
/* ------------------------------------------------ */

uint8_t Firmware_CheckSignature(uint32_t slot_addr, FirmwareHeader_t *header)
{
    uint32_t firmware_start;

    firmware_start = slot_addr + sizeof(FirmwareHeader_t);

    return Crypto_VerifyFirmware(firmware_start, header->image_size, header->signature);
}



/* ------------------------------------------------ */
/* Full firmware validation                         */
/* ------------------------------------------------ */

FirmwareStatus_t Firmware_Validate(uint32_t slot_addr)
{
    FirmwareHeader_t *header;

    header = Firmware_ReadHeader(slot_addr);


    /* Check magic number */

    if(!Firmware_CheckMagic(header))
        return FW_STATUS_INVALID;


    /* Verify CRC */

    if(!Firmware_CheckCRC(slot_addr, header))
        return FW_STATUS_INVALID;


    /* Verify signature */

    if(!Firmware_CheckSignature(slot_addr, header))
        return FW_STATUS_INVALID;


    return FW_STATUS_VALID;
}
