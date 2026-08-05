/*
 * metadata.h
 *
 *  Created on: 05-Mar-2026
 *      Author: Yashwanth
 */

#ifndef BOOTLOADER_INC_METADATA_H_
#define BOOTLOADER_INC_METADATA_H_


#include "utils.h"
#include "memory_map.h"
#include "bootloader_config.h"   /* MAX_BOOT_ATTEMPTS, single source */

/*
 * Flash layout of FirmwareMetadata_t at METADATA_ADDR (0x080E0000, Sector 11):
 *
 * Offset   Field
 * -------  ------------
 * +0x00    magic            (must == METADATA_MAGIC to be considered valid)
 * +0x04    active_slot
 * +0x08    boot_attempts
 * +0x0C    slotA.crc
 * +0x10    slotA.size
 * +0x14    slotA.version
 * +0x18    slotB.crc
 * +0x1C    slotB.size
 * +0x20    slotB.version
 *
 * Total struct size = 36 bytes
 */


/* Magic value that identifies a valid, completely-written metadata block.
 * On a blank / corrupted flash the magic will not match and Metadata_Init()
 * will re-initialize the struct to safe defaults. */

#define METADATA_MAGIC  0xB007AB1EU


/* Slot identifiers */

#define SLOT_A  0U
#define SLOT_B  1U


/* Firmware information for a single slot */

typedef struct
{
    uint32_t crc;
    uint32_t size;
    uint32_t version;

} FirmwareInfo_t;


/* Metadata block stored in flash (Sector 11) */

typedef struct
{
    uint32_t magic;           /* METADATA_MAGIC — validity sentinel    */
    uint32_t active_slot;     /* SLOT_A or SLOT_B                      */
    uint32_t boot_attempts;   /* Counts down; rollback at 0            */

    FirmwareInfo_t slotA;
    FirmwareInfo_t slotB;

} FirmwareMetadata_t;


/* Initialization */

void Metadata_Init(void);


/* Read / Write */

FirmwareMetadata_t Metadata_Read(void);
void Metadata_Write(FirmwareMetadata_t *meta);


/* Active slot */

uint32_t Metadata_GetActiveSlot(void);
void Metadata_SetActiveSlot(uint32_t slot);


/* Boot attempts */

uint32_t Metadata_GetBootAttempts(void);
void Metadata_DecrementBootAttempts(void);
void Metadata_ResetBootAttempts(void);


/* Slot A information */

void Metadata_SetSlotA(uint32_t crc, uint32_t size, uint32_t version);
FirmwareInfo_t Metadata_GetSlotA(void);


/* Slot B information */

void Metadata_SetSlotB(uint32_t crc, uint32_t size, uint32_t version);
FirmwareInfo_t Metadata_GetSlotB(void);


#endif /* BOOTLOADER_INC_METADATA_H_ */
