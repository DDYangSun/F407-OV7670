#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "stdio.h"

#define BSP_USART_RCC       RCC_APB1Periph_USART2
#define BSP_USART_TX_RCC    RCC_AHB1Periph_GPIOD
#define BSP_USART_RX_RCC    RCC_AHB1Periph_GPIOD
 
#define BSP_USART				USART2
#define BSP_USART_TX_PORT       GPIOD
#define BSP_USART_TX_PIN        GPIO_Pin_5
#define BSP_USART_RX_PORT       GPIOD
#define BSP_USART_RX_PIN        GPIO_Pin_6
#define BSP_USART_AF            GPIO_AF_USART2
#define BSP_USART_TX_AF_PIN		GPIO_PinSource5
#define BSP_USART_RX_AF_PIN		GPIO_PinSource6


void uart1_init(uint32_t __Baud);
void usart_send_data(uint8_t ucch);
void usart_send_String(uint8_t *ucstr);
int fputc(int ch, FILE *f);


#endif
