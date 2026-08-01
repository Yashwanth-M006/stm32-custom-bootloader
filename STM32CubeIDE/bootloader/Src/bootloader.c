/*
 * bootloader.c
 *
 *  Created on: 05-Mar-2026
 *      Author: Yashwanth
 */

#ifndef BOOTLOADER_SRC_BOOTLOADER_C_
#define BOOTLOADER_SRC_BOOTLOADER_C_

#include "bootloader.h"
#include "bootloader_config.h"
#include "update_manager.h"
#include "metadata.h"
#include "flash_if.h"
#include "recovery_mode.h"
#include "watchdog_manager.h"
#include "uart_update.h"
#include "can_update.h"
#include "ota_update.h"



/* ------------------------------------------------ */
/* Bootloader initialization                        */
/* ------------------------------------------------ */

void Bootloader_Init(void)
{
    HAL_Init();

    Watchdog_Init();

    Recovery_Init();

    Metadata_Init();
}



/* ------------------------------------------------ */
/* Validate firmware slot                           */
/* ------------------------------------------------ */

uint8_t Bootloader_ValidateSlot(uint32_t slot_addr)
{
    if(Firmware_Validate(slot_addr) == FW_STATUS_VALID)
        return 1;

    return 0;
}



/* ------------------------------------------------ */
/* Jump to application                              */
/* ------------------------------------------------ */

void Bootloader_JumpToApplication(uint32_t app_addr)
{
    uint32_t msp_value;
    uint32_t reset_handler;

    typedef void (*app_entry_t)(void);

    app_entry_t app_entry;

    /* Disable interrupts */

    __disable_irq();

    /* De-initialize Peripherals */
    HAL_DeInit();
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    /* Vector Table Relocation */
    SCB->VTOR = app_addr;


    /* Get stack pointer */

    msp_value = *(volatile uint32_t*)app_addr;

    /* Get reset handler */

    reset_handler = *(volatile uint32_t*)(app_addr + 4);


    /* Set MSP */

    __set_MSP(msp_value);


    /* Jump */

    app_entry = (app_entry_t)reset_handler;

    app_entry();
}



/* ------------------------------------------------ */
/* Rollback firmware                                */
/* ------------------------------------------------ */

void Bootloader_Rollback(void)
{
    uint32_t active = Metadata_GetActiveSlot();

    if(active == SLOT_A)
    {
        Metadata_SetActiveSlot(SLOT_B);
    }
    else
    {
        Metadata_SetActiveSlot(SLOT_A);
    }
}



/* ------------------------------------------------ */
/* Bootloader main logic                            */
/* ------------------------------------------------ */

void Bootloader_Run(void)
{
    uint32_t active_slot;
    uint32_t slot_addr;



    /* -------- Recovery mode (UART update) -------- */

    if(Recovery_IsRequested())
    {
        UpdateManager_Init();

        if(UpdateManager_Start() == UPDATE_OK)
        {
            /* Receive firmware via UART */

            if(UpdateManager_Receive() == UPDATE_OK)
            {
                if(UpdateManager_Finalize() == UPDATE_OK)
                {
                    UpdateManager_Activate();
                }
            }
        }

        HAL_NVIC_SystemReset();
    }



    /* -------- Normal boot -------- */

    active_slot = Metadata_GetActiveSlot();

    if(active_slot == SLOT_A)
        slot_addr = SLOT_A_START_ADDR;
    else
        slot_addr = SLOT_B_START_ADDR;



    /* Validate firmware */

    if(Firmware_Validate(slot_addr) == FW_STATUS_VALID)
    {
        Bootloader_JumpToApplication(
            slot_addr + sizeof(FirmwareHeader_t));
    }



    /* -------- Try rollback -------- */

    if(active_slot == SLOT_A)
        slot_addr = SLOT_B_START_ADDR;
    else
        slot_addr = SLOT_A_START_ADDR;



    if(Firmware_Validate(slot_addr) == FW_STATUS_VALID)
    {
        Metadata_SetActiveSlot(
            (active_slot == SLOT_A) ? SLOT_B : SLOT_A);

        Bootloader_JumpToApplication(
            slot_addr + sizeof(FirmwareHeader_t));
    }



    /* -------- Both slots invalid → force update -------- */

    UpdateManager_Init();

    UpdateManager_Start();

    UpdateManager_Receive();
}

#endif /* BOOTLOADER_SRC_BOOTLOADER_C_ */
