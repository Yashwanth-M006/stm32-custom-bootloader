/*
 * recovery_mode.h
 *
 *  Created on: 05-Mar-2026
 *      Author: Yashwanth
 */

#ifndef BOOTLOADER_INC_RECOVERY_MODE_H_
#define BOOTLOADER_INC_RECOVERY_MODE_H_

#include "utils.h"

/* Recovery GPIO configuration */

#define RECOVERY_GPIO_PORT   GPIOA
#define RECOVERY_GPIO_PIN    GPIO_PIN_0


/* Initialize recovery GPIO */

void Recovery_Init(void);


/* Check if recovery mode is requested */

uint8_t Recovery_IsRequested(void);


#endif /* BOOTLOADER_INC_RECOVERY_MODE_H_ */
