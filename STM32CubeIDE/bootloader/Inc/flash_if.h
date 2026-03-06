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


#define FLASH_WRITE_OK     0
#define FLASH_WRITE_ERROR  1


void FLASH_Erase(uint32_t addr, uint32_t size);
uint8_t FLASH_Write(uint32_t addr, uint8_t *data, uint32_t len);
uint8_t FLASH_Read(uint32_t addr);



#endif /* BOOTLOADER_INC_FLASH_IF_H_ */
