/*
 * bootloader.c
 *
 *  Created on: 05-Mar-2026
 *      Author: Yashwanth
 */

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
#include "firmware_image.h"



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
    uint32_t fallback_slot;
    uint32_t fallback_addr;

    /* Determine the other slot */
    if(active == SLOT_A)
    {
        fallback_slot = SLOT_B;
        fallback_addr = SLOT_B_START_ADDR;
    }
    else
    {
        fallback_slot = SLOT_A;
        fallback_addr = SLOT_A_START_ADDR;
    }

    /* Only switch if the fallback slot actually contains valid firmware */
    if(Firmware_Validate(fallback_addr) == FW_STATUS_VALID)
    {
        Metadata_SetActiveSlot(fallback_slot);
        Metadata_ResetBootAttempts();
    }
    /* If fallback is also invalid, stay on current slot — force update path
       in Bootloader_Run() will handle it */
}



/* ------------------------------------------------ */
/* Helper: run a full UART update cycle             */
/* ------------------------------------------------ */

static void RunRecoveryUpdate(void)
{
    UpdateManager_Init();

    if(UpdateManager_Start() == UPDATE_OK)
    {
        if(UpdateManager_Receive() == UPDATE_OK)
        {
            if(UpdateManager_Finalize() == UPDATE_OK)
            {
                UpdateManager_Activate();
                Metadata_ResetBootAttempts();
            }
        }
    }

    /* Always reset so the new (or unchanged) firmware is booted cleanly */
    HAL_NVIC_SystemReset();
}



/* ------------------------------------------------ */
/* Bootloader main logic                            */
/* ------------------------------------------------ */

void Bootloader_Run(void)
{
    uint32_t active_slot;
    uint32_t slot_addr;


    /* -------- UART trigger window (BOOT_TIMEOUT_MS) -------- */
    /*
     * Wait up to BOOT_TIMEOUT_MS for a CMD_START_UPDATE byte on UART.
     * This allows a host tool to trigger a software-initiated update
     * without needing to physically toggle the recovery GPIO pin.
     * If no trigger arrives within the window, boot continues normally.
     */
    {
        uint8_t trigger = 0U;
        if(UART_Update_ReceiveTimeout(&trigger, 1U, BOOT_TIMEOUT_MS) &&
           trigger == CMD_START_UPDATE)
        {
            RunRecoveryUpdate();
            /* RunRecoveryUpdate() calls SystemReset — never returns */
        }
    }


    /* -------- Recovery mode (forced UART update via GPIO pin) -------- */

    if(Recovery_IsRequested())
    {
        RunRecoveryUpdate();
        /* RunRecoveryUpdate() calls SystemReset — never returns */
    }


    /* -------- Normal boot -------- */

    active_slot = Metadata_GetActiveSlot();

    if(active_slot == SLOT_A)
        slot_addr = SLOT_A_START_ADDR;
    else
        slot_addr = SLOT_B_START_ADDR;


    /* Check boot attempt counter — if exhausted, force rollback */

    if(Metadata_GetBootAttempts() == 0U)
    {
        Bootloader_Rollback();
        HAL_NVIC_SystemReset();
        /* After reset, the counter is fresh (reset by Rollback) and the
           other slot will be tried */
    }


    /* Validate and boot active slot */

    if(Firmware_Validate(slot_addr) == FW_STATUS_VALID)
    {
        /* Decrement counter before jumping so a crash-loop is detected */
        Metadata_DecrementBootAttempts();

        Bootloader_JumpToApplication(slot_addr + sizeof(FirmwareHeader_t));
        /* Never returns */
    }


    /* -------- Active slot invalid — try rollback -------- */

    if(active_slot == SLOT_A)
        slot_addr = SLOT_B_START_ADDR;
    else
        slot_addr = SLOT_A_START_ADDR;

    if(Firmware_Validate(slot_addr) == FW_STATUS_VALID)
    {
        Metadata_SetActiveSlot((active_slot == SLOT_A) ? SLOT_B : SLOT_A);
        Metadata_ResetBootAttempts();
        Metadata_DecrementBootAttempts();

        Bootloader_JumpToApplication(slot_addr + sizeof(FirmwareHeader_t));
        /* Never returns */
    }


    /* -------- Both slots invalid → force UART update -------- */

    RunRecoveryUpdate();
    /* RunRecoveryUpdate() calls SystemReset — never returns */
}
