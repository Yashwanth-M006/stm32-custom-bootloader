/*
 * memory_map.h
 *
 *  Created on: 05-Mar-2026
 *      Author: Yashwanth
 */

#ifndef BOOTLOADER_INC_MEMORY_MAP_H_
#define BOOTLOADER_INC_MEMORY_MAP_H_


/* Boot-loader */

#define BOOTLOADER_START_ADDR   0x08000000
#define BOOTLOADER_SIZE         (64 * 1024)


/* Firmware Slots */

#define SLOT_A_START_ADDR       0x08010000
#define SLOT_A_SIZE             (384 * 1024)

#define SLOT_B_START_ADDR       0x08070000
#define SLOT_B_SIZE             (384 * 1024)


/* Meta-data */

#define METADATA_ADDR           0x080D0000
#define METADATA_SIZE           (16 * 1024)



#endif /* BOOTLOADER_INC_MEMORY_MAP_H_ */
