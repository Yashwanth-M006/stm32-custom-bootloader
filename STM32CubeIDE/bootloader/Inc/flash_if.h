/*
 * flash_if.h
 *
 *  Created on: 05-Mar-2026
 *      Author: Yashwanth
 */

#ifndef BOOTLOADER_INC_FLASH_IF_H_
#define BOOTLOADER_INC_FLASH_IF_H_

#include "utils.h"
#include "memory_map.h"


/* Return codes */

#define FLASH_OK     0U
#define FLASH_ERROR  1U


/* Erase sectors covering [addr, addr+size) */

uint8_t FLASH_Erase(uint32_t addr, uint32_t size);


/* Write len bytes from data[] to flash starting at addr */

uint8_t FLASH_Write(uint32_t addr, uint8_t *data, uint32_t len);


/* Read a single 32-bit word directly from flash */

uint32_t FLASH_ReadWord(uint32_t addr);



#endif /* BOOTLOADER_INC_FLASH_IF_H_ */
