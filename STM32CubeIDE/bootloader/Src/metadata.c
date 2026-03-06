/*
 * metadata.c
 *
 *  Created on: 05-Mar-2026
 *      Author: Yashwanth
 */


#include "metadata.h"
#include "flash_if.h"
#include "memory_map.h"


/* Initialize metadata */

void Metadata_Init(void)
{
    FirmwareMetadata_t meta = Metadata_Read();

    /* If flash is empty initialize metadata */

    if(meta.active_slot != SLOT_A && meta.active_slot != SLOT_B)
    {
        meta.active_slot = SLOT_A;
        meta.boot_attempts = MAX_BOOT_ATTEMPTS;

        meta.slotA.crc = 0;
        meta.slotA.size = 0;
        meta.slotA.version = 0;

        meta.slotB.crc = 0;
        meta.slotB.size = 0;
        meta.slotB.version = 0;

        Metadata_Write(&meta);
    }
}


/* Read metadata from flash */

FirmwareMetadata_t Metadata_Read(void)
{
    FirmwareMetadata_t *meta_ptr = (FirmwareMetadata_t*)METADATA_ADDR;
    return *meta_ptr;
}


/* Write metadata to flash */

void Metadata_Write(FirmwareMetadata_t *meta)
{
    FLASH_Erase(METADATA_ADDR, sizeof(FirmwareMetadata_t));

    FLASH_Write(METADATA_ADDR,
                (uint8_t*)meta,
                sizeof(FirmwareMetadata_t));
}


/* Active slot */

uint32_t Metadata_GetActiveSlot(void)
{
    FirmwareMetadata_t meta = Metadata_Read();
    return meta.active_slot;
}


void Metadata_SetActiveSlot(uint32_t slot)
{
    FirmwareMetadata_t meta = Metadata_Read();

    meta.active_slot = slot;

    Metadata_Write(&meta);
}


/* Boot attempts */

uint32_t Metadata_GetBootAttempts(void)
{
    FirmwareMetadata_t meta = Metadata_Read();
    return meta.boot_attempts;
}


void Metadata_DecrementBootAttempts(void)
{
    FirmwareMetadata_t meta = Metadata_Read();

    if(meta.boot_attempts > 0)
        meta.boot_attempts--;

    Metadata_Write(&meta);
}


void Metadata_ResetBootAttempts(void)
{
    FirmwareMetadata_t meta = Metadata_Read();

    meta.boot_attempts = MAX_BOOT_ATTEMPTS;

    Metadata_Write(&meta);
}


/* Slot A information */

void Metadata_SetSlotA(uint32_t crc, uint32_t size, uint32_t version)
{
    FirmwareMetadata_t meta = Metadata_Read();

    meta.slotA.crc = crc;
    meta.slotA.size = size;
    meta.slotA.version = version;

    Metadata_Write(&meta);
}


FirmwareInfo_t Metadata_GetSlotA(void)
{
    FirmwareMetadata_t meta = Metadata_Read();
    return meta.slotA;
}


/* Slot B information */

void Metadata_SetSlotB(uint32_t crc, uint32_t size, uint32_t version)
{
    FirmwareMetadata_t meta = Metadata_Read();

    meta.slotB.crc = crc;
    meta.slotB.size = size;
    meta.slotB.version = version;

    Metadata_Write(&meta);
}


FirmwareInfo_t Metadata_GetSlotB(void)
{
    FirmwareMetadata_t meta = Metadata_Read();
    return meta.slotB;
}
