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

/*
Address        Field
--------------------------------
0x080D0000     active_slot
0x080D0004     boot_attempts

0x080D0008     slotA_crc
0x080D000C     slotA_size
0x080D0010     slotA_version

0x080D0014     slotB_crc
0x080D0018     slotB_size
0x080D001C     slotB_version


total size  = 28 bytes
*/




/* Slot identifiers */

#define SLOT_A 0
#define SLOT_B 1

/* Maximum boot attempts before rollback */

#define MAX_BOOT_ATTEMPTS 3


/* Firmware information for a slot */

typedef struct
{
    uint32_t crc;
    uint32_t size;
    uint32_t version;

} FirmwareInfo_t;


/* Metadata stored in flash */

typedef struct
{
    uint32_t active_slot;
    uint32_t boot_attempts;

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
