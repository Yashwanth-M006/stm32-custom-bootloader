/*
 * watchdog_manager.c
 *
 *  Created on: 05-Mar-2026
 *      Author: Yashwanth
 */


#include "watchdog_manager.h"
#include "bootloader_config.h"

static IWDG_HandleTypeDef hiwdg;

void Watchdog_Init(void)
{
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
    hiwdg.Init.Reload = 3125;

    HAL_IWDG_Init(&hiwdg);
}


void Watchdog_Refresh(void)
{
    HAL_IWDG_Refresh(&hiwdg);
}

void Watchdog_Disable(void)
{
    /* IWDG cannot be disabled once started */
}
