/*
 * usart.c
 *
 *  Created on: 10.07.2023
 *      Author: moritz
 */


#include "usart.h"



int8_t usart_cmd[4] = { 0 };
uint8_t cmd_index = 0;

void usart_init()
{
	SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOAEN);

	// Use system clock for USART1
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_USART1EN);

	// PA9 AF7 tx
	MODIFY_REG(GPIOA->AFR[1], GPIO_AFRH_AFRH1,  7 << GPIO_AFRH_AFSEL9_Pos);
	SET_BIT(GPIOA->MODER, GPIO_MODER_MODER9_1);

	// PA10 AF7 rx
	MODIFY_REG(GPIOA->AFR[1], GPIO_AFRH_AFRH2,   7 << GPIO_AFRH_AFSEL10_Pos);
	SET_BIT(GPIOA->MODER, GPIO_MODER_MODER10_1);

	// Set baudrate
	USART1->BRR = (SystemCoreClock / 9600);

	// Enable receiver and receive-interrupt of USART1
	USART1->CR1 = USART_CR1_UE + USART_CR1_RE + USART_CR1_RXNEIE;

	// Enable interrupt in NVIC
	NVIC_EnableIRQ(USART1_IRQn);
}

void USART1_EXTI25_IRQHandler()
{
	usart_cmd[cmd_index] = (int8_t)USART1->RDR;

	if(cmd_index < 3)
		cmd_index++;
	else
		cmd_index = 0;
}
