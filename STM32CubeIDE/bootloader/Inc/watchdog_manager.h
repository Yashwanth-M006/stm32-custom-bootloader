/*
 * watchdog_manager.h
 *
 *  Created on: 05-Mar-2026
 *      Author: Yashwanth
 */

#ifndef BOOTLOADER_INC_WATCHDOG_MANAGER_H_
#define BOOTLOADER_INC_WATCHDOG_MANAGER_H_

#include "utils.h"

/* Watchdog timeout in milliseconds */

#define WDG_TIMEOUT_MS 5000


/* Initialize watchdog */

void Watchdog_Init(void);


/* Refresh watchdog */

void Watchdog_Refresh(void);


/* Disable watchdog (if possible) */

void Watchdog_Disable(void);


#endif /* BOOTLOADER_INC_WATCHDOG_MANAGER_H_ */
