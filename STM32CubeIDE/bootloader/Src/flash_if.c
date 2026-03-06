/*
 * flash_if.c
 *
 *  Created on: 05-Mar-2026
 *      Author: Yashwanth
 */


#include "flash_if.h"
#include "memory_map.h"
#include "bootloader_config.h"


/* Get sector number from address */
static uint32_t GetSector(uint32_t Address)
{
    if(Address < 0x08004000) return FLASH_SECTOR_0;
    else if(Address < 0x08008000) return FLASH_SECTOR_1;
    else if(Address < 0x0800C000) return FLASH_SECTOR_2;
    else if(Address < 0x08010000) return FLASH_SECTOR_3;
    else if(Address < 0x08020000) return FLASH_SECTOR_4;
    else if(Address < 0x08040000) return FLASH_SECTOR_5;
    else if(Address < 0x08060000) return FLASH_SECTOR_6;
    else return FLASH_SECTOR_7;
}


/*******************************************************************************************/

/* Flash Erase */
void FLASH_Erase(uint32_t addr, uint32_t size)
{
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef eraseInit;
    uint32_t sectorError;

    uint32_t startSector = GetSector(addr);
    uint32_t endSector = GetSector(addr + size);

    eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    eraseInit.Sector = startSector;
    eraseInit.NbSectors = (endSector - startSector) + 1;

    HAL_FLASHEx_Erase(&eraseInit, &sectorError);

    HAL_FLASH_Lock();
}



/* Flash Write using 32-bit words */
uint8_t FLASH_Write(uint32_t addr, uint8_t *data, uint32_t len)
{
    HAL_FLASH_Unlock();

    uint32_t i = 0;

    while(i < len)
    {
        uint32_t word = 0xFFFFFFFF;

        /* Pack 4 bytes into a 32-bit word */
        for(uint8_t j = 0; j < 4 && i < len; j++, i++)
        {
            ((uint8_t*)&word)[j] = data[i];
        }

        /* Write word to flash */
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, word);

        /* Verify flash write */
        uint32_t written = *(uint32_t*)addr;

        if(written != word)
        {
            HAL_FLASH_Lock();
            return FLASH_WRITE_ERROR;
        }

        addr += 4;
    }

    HAL_FLASH_Lock();

    return FLASH_WRITE_OK;
}



/* Read 32-bit word from flash */
uint32_t FLASH_ReadWord(uint32_t addr)
{
    return *(volatile uint32_t*)addr;
}
