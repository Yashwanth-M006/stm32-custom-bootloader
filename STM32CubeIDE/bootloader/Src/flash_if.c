/*
 * flash_if.c
 *
 *  Created on: 05-Mar-2026
 *      Author: Yashwanth
 */


#include "flash_if.h"
#include "memory_map.h"
#include "bootloader_config.h"


/*
 * Map a flash address to its sector number.
 *
 * STM32F407VGTx sector boundaries (1 MB / 12 sectors):
 *   S0-S3  : 16 KB each  (0x08000000 – 0x0800FFFF)
 *   S4     : 64 KB        (0x08010000 – 0x0801FFFF)
 *   S5-S11 : 128 KB each  (0x08020000 – 0x080FFFFF)
 */
static uint32_t GetSector(uint32_t Address)
{
    if     (Address < 0x08004000U) return FLASH_SECTOR_0;
    else if(Address < 0x08008000U) return FLASH_SECTOR_1;
    else if(Address < 0x0800C000U) return FLASH_SECTOR_2;
    else if(Address < 0x08010000U) return FLASH_SECTOR_3;
    else if(Address < 0x08020000U) return FLASH_SECTOR_4;
    else if(Address < 0x08040000U) return FLASH_SECTOR_5;
    else if(Address < 0x08060000U) return FLASH_SECTOR_6;
    else if(Address < 0x08080000U) return FLASH_SECTOR_7;
    else if(Address < 0x080A0000U) return FLASH_SECTOR_8;
    else if(Address < 0x080C0000U) return FLASH_SECTOR_9;
    else if(Address < 0x080E0000U) return FLASH_SECTOR_10;
    else                            return FLASH_SECTOR_11;
}


/*******************************************************************************************/

/*
 * Erase all sectors that cover the address range [addr, addr + size).
 *
 * Returns FLASH_OK on success, FLASH_ERROR if the HAL erase fails or if
 * any sector reports an error (sectorError != 0xFFFFFFFF).
 */
uint8_t FLASH_Erase(uint32_t addr, uint32_t size)
{
    FLASH_EraseInitTypeDef eraseInit;
    uint32_t sectorError;
    HAL_StatusTypeDef status;

    /* Use addr + size - 1 to find the last byte's sector safely */
    uint32_t startSector = GetSector(addr);
    uint32_t endSector   = GetSector(addr + size - 1U);

    eraseInit.TypeErase    = FLASH_TYPEERASE_SECTORS;
    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    eraseInit.Sector       = startSector;
    eraseInit.NbSectors    = (endSector - startSector) + 1U;

    HAL_FLASH_Unlock();

    status = HAL_FLASHEx_Erase(&eraseInit, &sectorError);

    HAL_FLASH_Lock();

    if(status != HAL_OK || sectorError != 0xFFFFFFFFU)
    {
        return FLASH_ERROR;
    }

    return FLASH_OK;
}



/*
 * Write len bytes from data[] to flash starting at addr.
 * Flash must be erased before writing.
 * Writes in 32-bit words; pads the final word with 0xFF if len is not a
 * multiple of 4.
 */
uint8_t FLASH_Write(uint32_t addr, uint8_t *data, uint32_t len)
{
    uint32_t i = 0;

    HAL_FLASH_Unlock();

    while(i < len)
    {
        uint32_t word = 0xFFFFFFFFU;

        /* Pack up to 4 bytes into a 32-bit word (little-endian) */
        for(uint8_t j = 0; j < 4U && i < len; j++, i++)
        {
            ((uint8_t*)&word)[j] = data[i];
        }

        if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, word) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return FLASH_ERROR;
        }

        /* Read-back verify */
        if(*(volatile uint32_t*)addr != word)
        {
            HAL_FLASH_Lock();
            return FLASH_ERROR;
        }

        addr += 4U;
    }

    HAL_FLASH_Lock();

    return FLASH_OK;
}



/*
 * Read a single 32-bit word directly from flash.
 */
uint32_t FLASH_ReadWord(uint32_t addr)
{
    return *(volatile uint32_t*)addr;
}
