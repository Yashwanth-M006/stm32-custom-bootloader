/*
 * uart_update.h
 *
 *  Created on: 05-Mar-2026
 *      Author: Yashwanth
 */

#ifndef BOOTLOADER_INC_UART_UPDATE_H_
#define BOOTLOADER_INC_UART_UPDATE_H_

#include "utils.h"
#include "update_manager.h"
#include "main.h"


/* Packet constants */

#define UART_START_BYTE 	0xAA

#define CMD_START_UPDATE   0x01
#define CMD_FIRMWARE_DATA  0x02
#define CMD_END_UPDATE     0x03

#define CMD_ACK            0x79
#define CMD_NACK           0x1F

#define UART_MAX_DATA      256


/* UART Packet */

typedef struct __attribute__((packed))
{
    uint8_t start;
    uint8_t cmd;
    uint16_t length;
    uint8_t data[UART_MAX_DATA];
    uint32_t crc;

} UART_Packet_t;


/* UART functions */

void UART_Update_Init(void);

uint8_t UART_Update_Send(uint8_t *data, uint16_t len);

uint8_t UART_Update_Receive(uint8_t *data, uint16_t len);

uint8_t UART_Update_ReceiveTimeout(uint8_t *data, uint16_t len, uint32_t timeout);


/* Helper functions */

void UART_Send_ACK(void);

void UART_Send_NACK(void);

#endif /* BOOTLOADER_INC_UART_UPDATE_H_ */
