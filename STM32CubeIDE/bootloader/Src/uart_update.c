/*
 * uart_update.c
 *
 *  Created on: 05-Mar-2026
 *      Author: Yashwanth
 */


#include "uart_update.h"
#include "update_manager.h"
#include "bootloader_config.h"


void UART_Update_Init(void)
{
    MX_USART1_UART_Init();
}



uint8_t UART_Update_Send(uint8_t *data, uint16_t len)
{
    if(HAL_UART_Transmit(&huart1, data, len, 1000) == HAL_OK)
        return 1;

    return 0;
}



uint8_t UART_Update_Receive(uint8_t *data, uint16_t len)
{
    if(HAL_UART_Receive(&huart1, data, len, HAL_MAX_DELAY) == HAL_OK)
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
    UART_Update_Send(&ack,1);
}



void UART_Send_NACK(void)
{
    uint8_t nack = CMD_NACK;
    UART_Update_Send(&nack,1);
}
