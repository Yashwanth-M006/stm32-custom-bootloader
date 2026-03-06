/*
 * recovery_mode.c
 *
 *  Created on: 05-Mar-2026
 *      Author: Yashwanth
 */


#include "recovery_mode.h"
#include "bootloader_config.h"




/* ------------------------------------------------ */
/* Initialize recovery GPIO                         */
/* ------------------------------------------------ */

void Recovery_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIO clock */

    __HAL_RCC_GPIOA_CLK_ENABLE();


    /* Configure pin as input */

    GPIO_InitStruct.Pin  = RECOVERY_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;

    HAL_GPIO_Init(RECOVERY_GPIO_PORT, &GPIO_InitStruct);
}



/* ------------------------------------------------ */
/* Check recovery request                           */
/* ------------------------------------------------ */

uint8_t Recovery_IsRequested(void)
{
    if(HAL_GPIO_ReadPin(RECOVERY_GPIO_PORT, RECOVERY_GPIO_PIN) == GPIO_PIN_RESET)
    {
        /* Button pressed / recovery pin active */

        return 1;
    }

    return 0;
}
