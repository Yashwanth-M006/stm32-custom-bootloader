/*
 * bootloader_config.h
 *
 *  Created on: 05-Mar-2026
 *      Author: Yashwanth
 */

#ifndef BOOTLOADER_INC_BOOTLOADER_CONFIG_H_
#define BOOTLOADER_INC_BOOTLOADER_CONFIG_H_




/********************************** Config *******************************************/

#define BOOT_TIMEOUT_MS      3000
#define BOOT_ATTEMPTS_MAX    3

/********************************** State Machine **************************************/

typedef enum
{
    BL_STATE_IDLE,
    BL_STATE_UPDATE,
    BL_STATE_VERIFY,
    BL_STATE_ACTIVATE,
    BL_STATE_ROLLBACK,
    BL_STATE_BOOT
}BL_State_t;



#endif /* BOOTLOADER_INC_BOOTLOADER_CONFIG_H_ */
