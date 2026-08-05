/*
 * memory_map.h
 *
 *  Created on: 05-Mar-2026
 *      Author: Yashwanth
 */

#ifndef BOOTLOADER_INC_MEMORY_MAP_H_
#define BOOTLOADER_INC_MEMORY_MAP_H_


/*
 * STM32F407VGTx Flash Sector Layout (1 MB total):
 *
 *  Sector 0 : 0x08000000 - 0x08003FFF  16 KB  ─┐
 *  Sector 1 : 0x08004000 - 0x08007FFF  16 KB   │ Bootloader (64 KB, S0-S3)
 *  Sector 2 : 0x08008000 - 0x0800BFFF  16 KB   │
 *  Sector 3 : 0x0800C000 - 0x0800FFFF  16 KB  ─┘
 *  Sector 4 : 0x08010000 - 0x0801FFFF  64 KB  ─┐
 *  Sector 5 : 0x08020000 - 0x0803FFFF 128 KB   │ Slot A (448 KB, S4-S7)
 *  Sector 6 : 0x08040000 - 0x0805FFFF 128 KB   │
 *  Sector 7 : 0x08060000 - 0x0807FFFF 128 KB  ─┘
 *  Sector 8 : 0x08080000 - 0x0809FFFF 128 KB  ─┐
 *  Sector 9 : 0x080A0000 - 0x080BFFFF 128 KB   │ Slot B (384 KB, S8-S10)
 *  Sector 10: 0x080C0000 - 0x080DFFFF 128 KB  ─┘
 *  Sector 11: 0x080E0000 - 0x080FFFFF 128 KB  ── Metadata (S11)
 */


/* Bootloader — Sectors 0–3 */

#define BOOTLOADER_START_ADDR   0x08000000U
#define BOOTLOADER_SIZE         (64U * 1024U)


/* Firmware Slot A — Sectors 4–7 */

#define SLOT_A_START_ADDR       0x08010000U
#define SLOT_A_SIZE             (448U * 1024U)


/* Firmware Slot B — Sectors 8–10 */

#define SLOT_B_START_ADDR       0x08080000U
#define SLOT_B_SIZE             (384U * 1024U)


/* Metadata — Sector 11 (isolated, never overlaps with slots) */

#define METADATA_ADDR           0x080E0000U
#define METADATA_SIZE           (128U * 1024U)



#endif /* BOOTLOADER_INC_MEMORY_MAP_H_ */
