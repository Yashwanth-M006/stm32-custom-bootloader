/*
 * bootloader_config.h
 *
 *  Created on: 05-Mar-2026
 *      Author: Yashwanth
 */

#ifndef BOOTLOADER_INC_BOOTLOADER_CONFIG_H_
#define BOOTLOADER_INC_BOOTLOADER_CONFIG_H_




/********************************** Config *******************************************/

/*
 * BOOT_TIMEOUT_MS — how long (in ms) Bootloader_Run() waits for a UART
 * trigger byte (CMD_START_UPDATE) before proceeding with normal boot.
 * Set to 0 to disable the UART trigger window.
 */
#define BOOT_TIMEOUT_MS      3000U

/*
 * MAX_BOOT_ATTEMPTS — number of times a slot is tried before the
 * bootloader assumes it is crash-looping and triggers a rollback.
 * Single definition; metadata.h #includes this file to reference it.
 */
#define MAX_BOOT_ATTEMPTS    3U

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
