/*
 * crc_verify.c
 *
 *  Created on: 05-Mar-2026
 *      Author: Yashwanth
 */


#include "crc_verify.h"
#include "bootloader_config.h"



/* Calculate CRC32 for a data buffer */

uint32_t CRC_Calculate(uint8_t *data, uint32_t length)
{
    uint32_t crc = CRC32_INIT_VALUE;

    for(uint32_t i = 0; i < length; i++)
    {
        crc ^= data[i];

        for(uint8_t j = 0; j < 8; j++)
        {
            if(crc & 1)
                crc = (crc >> 1) ^ CRC32_POLYNOMIAL;
            else
                crc >>= 1;
        }
    }

    return ~crc;
}


/* Calculate CRC32 directly from flash memory */

uint32_t CRC_CalculateFlash(uint32_t start_addr, uint32_t size)
{
    uint8_t *data = (uint8_t*)start_addr;

    uint32_t crc = CRC32_INIT_VALUE;

    for(uint32_t i = 0; i < size; i++)
    {
        crc ^= data[i];

        for(uint8_t j = 0; j < 8; j++)
        {
            if(crc & 1)
                crc = (crc >> 1) ^ CRC32_POLYNOMIAL;
            else
                crc >>= 1;
        }
    }

    return ~crc;
}


/* Verify firmware CRC */

uint8_t CRC_Verify(uint32_t start_addr, uint32_t size, uint32_t expected_crc)
{
    uint32_t calculated_crc;

    calculated_crc = CRC_CalculateFlash(start_addr, size);

    if(calculated_crc == expected_crc)
        return 1;
    else
        return 0;
}
