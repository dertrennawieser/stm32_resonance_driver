/*
 * dac.c
 *
 *  Created on: Aug 29, 2022
 *      Author: moritz
 */

#include "dac.h"
#include "systeminit.h"


void dac1_init()
{
	SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOAEN);
	SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_DAC1EN);

	// configure pin as analog
	//MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER4, GPIO_MODER_MODER4_0 + GPIO_MODER_MODER4_1);

	// enable high freq interface to AHB >160 MHz
	SET_BIT(DAC1->MCR, DAC_MCR_HFSEL_1);

	// connect DAC1 to output pin and on chip peripherals with output buffer
	SET_BIT(DAC1->MCR, DAC_MCR_MODE1_0);
	SET_BIT(DAC1->MCR, DAC_MCR_MODE1_1);

	SET_BIT(DAC1->MCR, DAC_MCR_MODE2_0);
	SET_BIT(DAC1->MCR, DAC_MCR_MODE2_1);

	// enable dac1 ch1
	SET_BIT(DAC1->CR, DAC_CR_EN1);
	SET_BIT(DAC1->CR, DAC_CR_EN2);
	// disable ch1 output buffer
	//SET_BIT(DAC1->CR, DAC_CR_BOFF1);
	wait_ms(1);
}

void dac3_init()
{
	SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_DAC3EN);

	// enable high freq interface to AHB >160 MHz
	SET_BIT(DAC3->MCR, DAC_MCR_HFSEL_1);

	// connect DAC3 to on chip peripherals
	SET_BIT(DAC3->MCR, DAC_MCR_MODE2_0);
	SET_BIT(DAC3->MCR, DAC_MCR_MODE2_1);
	// connect DAC3 to on chip peripherals
	SET_BIT(DAC3->MCR, DAC_MCR_MODE1_0);
	SET_BIT(DAC3->MCR, DAC_MCR_MODE1_1);

	// enable dac3 ch2
	SET_BIT(DAC3->CR, DAC_CR_EN2);
	// enable dac3 ch1
	SET_BIT(DAC3->CR, DAC_CR_EN1);

	wait_ms(1);
}

void dac1_write(uint16_t value)
{
	DAC1->DHR12R1 = value & 0xFFF;
	DAC1->DHR12R2 = value & 0xFFF;
}

void dac3_write(uint16_t value)
{
	DAC3->DHR12R1 = value & 0xFFF;
	DAC3->DHR12R2 = value & 0xFFF;
}
