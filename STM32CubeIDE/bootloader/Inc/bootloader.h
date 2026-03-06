/*
 * bootloader.h
 *
 *  Created on: 05-Mar-2026
 *      Author: Yashwanth
 */

#ifndef BOOTLOADER_INC_BOOTLOADER_H_
#define BOOTLOADER_INC_BOOTLOADER_H_

#include "utils.h"

#include "bootloader_config.h"
#include "metadata.h"
#include "recovery_mode.h"
#include "update_manager.h"
#include "flash_if.h"

/******************************** APIs *************************************************/

/* Bootloader main entry */

void Bootloader_Init(void);
void Bootloader_Run(void);


/* Jump to application */

void Bootloader_JumpToApplication(uint32_t app_addr);


/* Validate slot */

uint8_t Bootloader_ValidateSlot(uint32_t slot_addr);


/* Rollback firmware */

void Bootloader_Rollback(void);



#endif /* BOOTLOADER_INC_BOOTLOADER_H_ */
