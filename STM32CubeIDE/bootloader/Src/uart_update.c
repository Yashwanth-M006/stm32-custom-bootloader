/*
 * uart_update.c
 *
 *  Created on: 05-Mar-2026
 *      Author: Yashwanth
 */


#include "uart_update.h"
#include "update_manager.h"
#include "bootloader_config.h"


/*
 * Maximum time to wait for a chunk of UART data.
 * If the host does not send data within this window the receive returns 0
 * and the caller treats it as a communication error.
 */
#define UART_RX_TIMEOUT_MS  5000U


void UART_Update_Init(void)
{
    MX_USART1_UART_Init();
}



uint8_t UART_Update_Send(uint8_t *data, uint16_t len)
{
    if(HAL_UART_Transmit(&huart1, data, len, 1000U) == HAL_OK)
        return 1;

    return 0;
}



/*
 * Receive len bytes with a 5-second timeout.
 * Returns 1 on success, 0 on timeout or UART error.
 */
uint8_t UART_Update_Receive(uint8_t *data, uint16_t len)
{
    if(HAL_UART_Receive(&huart1, data, len, UART_RX_TIMEOUT_MS) == HAL_OK)
        return 1;

    return 0;
}



uint8_t UART_Update_ReceiveTimeout(uint8_t *data, uint16_t len, uint32_t timeout)
{
    if(HAL_UART_Receive(&huart1, data, len, timeout) == HAL_OK)
        return 1;

    return 0;
}



void UART_Send_ACK(void)
{
    uint8_t ack = CMD_ACK;
    UART_Update_Send(&ack, 1);
}



void UART_Send_NACK(void)
{
    uint8_t nack = CMD_NACK;
    UART_Update_Send(&nack, 1);
}
