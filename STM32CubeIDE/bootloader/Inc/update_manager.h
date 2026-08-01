/*
 * update_manager.h
 *
 *  Created on: 05-Mar-2026
 *      Author: Yashwanth
 */

#ifndef BOOTLOADER_INC_UPDATE_MANAGER_H_
#define BOOTLOADER_INC_UPDATE_MANAGER_H_


#include "utils.h"
#include "flash_if.h"
#include "firmware_image.h"
#include "metadata.h"
#include "crc_verify.h"
#include "crypto_verify.h"

/*
 * receive firmware
 * write firmware to inactive slot
 * verify firmware (CRC + signature)
 * update metadata
 * activate new slot
 */


typedef enum
{
    UPDATE_OK = 0,
    UPDATE_ERROR,
    UPDATE_TIMEOUT,
    UPDATE_INVALID_PACKET,
    UPDATE_INVALID_FIRMWARE,
    UPDATE_ROLLBACK_DETECTED

} UpdateStatus_t;


/* Update manager functions */

void UpdateManager_Init(void);

UpdateStatus_t UpdateManager_Start(void);

UpdateStatus_t UpdateManager_Receive(void);

UpdateStatus_t UpdateManager_Finalize(void);

void UpdateManager_Activate(void);

#endif /* BOOTLOADER_INC_UPDATE_MANAGER_H_ */
