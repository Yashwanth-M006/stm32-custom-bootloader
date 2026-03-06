/*
 * update_manager.c
 *
 *  Created on: 05-Mar-2026
 *      Author: Yashwanth
 */


#include "update_manager.h"
#include "flash_if.h"
#include "firmware_image.h"
#include "metadata.h"
#include "crc_verify.h"
#include "crypto_verify.h"
#include "uart_update.h"
#include "watchdog_manager.h"



static uint32_t inactive_slot_addr;
static uint32_t write_address;



/* Determine inactive slot */

static uint32_t UpdateManager_GetInactiveSlot(void)
{
    if(Metadata_GetActiveSlot() == SLOT_A)
        return SLOT_B_START_ADDR;
    else
        return SLOT_A_START_ADDR;
}



/* Initialize */

void UpdateManager_Init(void)
{
    inactive_slot_addr = UpdateManager_GetInactiveSlot();
    write_address = inactive_slot_addr;
}



/* Start update */

UpdateStatus_t UpdateManager_Start(void)
{
    inactive_slot_addr = UpdateManager_GetInactiveSlot();

    write_address = inactive_slot_addr;

    FLASH_Erase(inactive_slot_addr, SLOT_A_SIZE);

    return UPDATE_OK;
}



/* Read packet */

static uint8_t UpdateManager_ReadPacket(UART_Packet_t *pkt)
{
    UART_Update_Receive(&pkt->start,1);

    if(pkt->start != UART_START_BYTE)
        return 0;

    UART_Update_Receive(&pkt->cmd,1);

    UART_Update_Receive((uint8_t*)&pkt->length,2);

    if(pkt->length > UART_MAX_DATA)
        return 0;

    UART_Update_Receive(pkt->data,pkt->length);

    UART_Update_Receive((uint8_t*)&pkt->crc,4);

    return 1;
}



/* Receive firmware */

UpdateStatus_t UpdateManager_Receive(void)
{
    UART_Packet_t packet;

    uint32_t calc_crc;

    while(1)
    {
        if(!UpdateManager_ReadPacket(&packet))
        {
            UART_Send_NACK();
            return UPDATE_INVALID_PACKET;
        }


        if(packet.cmd == CMD_START_UPDATE)
        {
            UART_Send_ACK();
        }


        else if(packet.cmd == CMD_FIRMWARE_DATA)
        {
            calc_crc = CRC_Calculate(packet.data, packet.length);

            if(calc_crc != packet.crc)
            {
                UART_Send_NACK();
                continue;
            }

            FLASH_Write(write_address,
                        packet.data,
                        packet.length);

            write_address += packet.length;

            Watchdog_Refresh();

            UART_Send_ACK();
        }


        else if(packet.cmd == CMD_END_UPDATE)
        {
            UART_Send_ACK();
            return UPDATE_OK;
        }
    }
}



/* Verify firmware */

UpdateStatus_t UpdateManager_Finalize(void)
{
    if(Firmware_Validate(inactive_slot_addr) == FW_STATUS_VALID)
        return UPDATE_OK;

    return UPDATE_INVALID_FIRMWARE;
}



/* Activate new firmware */

void UpdateManager_Activate(void)
{
    if(inactive_slot_addr == SLOT_A_START_ADDR)
        Metadata_SetActiveSlot(SLOT_A);
    else
        Metadata_SetActiveSlot(SLOT_B);

    Metadata_ResetBootAttempts();
}
