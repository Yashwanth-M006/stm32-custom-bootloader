/*
 * watchdog_manager.c
 *
 *  Created on: 05-Mar-2026
 *      Author: Yashwanth
 */


#include "watchdog_manager.h"
#include "bootloader_config.h"
#include "main.h"   /* extern IWDG_HandleTypeDef hiwdg */


/*
 * Watchdog_Init — the IWDG is started by MX_IWDG_Init() in main().
 * This function is kept as a logical init point for the bootloader
 * module boundary; no re-initialization is needed since the hardware
 * handle (hiwdg) is shared via main.h.
 */
void Watchdog_Init(void)
{
    /* IWDG already configured and started by MX_IWDG_Init() */
}


void Watchdog_Refresh(void)
{
    HAL_IWDG_Refresh(&hiwdg);
}


void Watchdog_Disable(void)
{
    /* IWDG cannot be disabled once started on STM32 */
}
