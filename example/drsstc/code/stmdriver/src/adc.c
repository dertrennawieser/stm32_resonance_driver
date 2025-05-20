/*
 * adc.c
 *
 *  Created on: 21.07.2022
 *      Author: moritz
 */

#include "adc.h"

uint16_t data = 0;
bool new = false;

void adc_init()
{
	data = 0;

	if(READ_BIT(ADC1->CR, ADC_CR_ADEN))
		return;

	SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOAEN);
	SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOBEN);

	// PB1 - ADC1 Analog in 12
	MODIFY_REG(GPIOB->MODER, GPIO_MODER_MODER1, GPIO_MODER_MODER1_0 + GPIO_MODER_MODER1_1);
	// PB2 - ADC2 Analog in 12
	MODIFY_REG(GPIOB->MODER, GPIO_MODER_MODER2, GPIO_MODER_MODER2_0 + GPIO_MODER_MODER2_1);

	// Enable clock for ADC
	SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADC12EN);

	// ADC Clock = HCLK/4
	MODIFY_REG(ADC12_COMMON->CCR, ADC_CCR_CKMODE, ADC_CCR_CKMODE_0 + ADC_CCR_CKMODE_1);

	// exit deep power down mode
	CLEAR_BIT(ADC1->CR, ADC_CR_DEEPPWD);
	CLEAR_BIT(ADC2->CR, ADC_CR_DEEPPWD);

	// Enable ADC voltage regulator
	MODIFY_REG(ADC1->CR, ADC_CR_ADVREGEN, 0);
	SET_BIT(ADC1->CR, ADC_CR_ADVREGEN);
	MODIFY_REG(ADC2->CR, ADC_CR_ADVREGEN, 0);
	SET_BIT(ADC2->CR, ADC_CR_ADVREGEN);

	// Delay 1-2 ms
	wait_ms(2);

	// Start calibration for single ended mode
	CLEAR_BIT(ADC1->CR, ADC_CR_ADCALDIF);
	SET_BIT(ADC1->CR, ADC_CR_ADCAL);

	// Wait until the calibration is finished
	while (READ_BIT(ADC1->CR, ADC_CR_ADCAL));

	// Clear the ready flag
	SET_BIT(ADC1->ISR, ADC_ISR_ADRDY);

	// Enable the ADC repeatedly until success (workaround from errata)
	do
	{
		SET_BIT(ADC1->CR, ADC_CR_ADEN);
	}
	while (!READ_BIT(ADC1->ISR, ADC_ISR_ADRDY));

	// Set sample time to 47.5 cycles
	MODIFY_REG(ADC1->SMPR2, ADC_SMPR2_SMP15, ADC_SMPR2_SMP15_2);

	wait_ms(2);

	// Start calibration for single ended mode
	CLEAR_BIT(ADC2->CR, ADC_CR_ADCALDIF);
	SET_BIT(ADC2->CR, ADC_CR_ADCAL);

	// Wait until the calibration is finished
	while (READ_BIT(ADC2->CR, ADC_CR_ADCAL));

	// Clear the ready flag
	SET_BIT(ADC2->ISR, ADC_ISR_ADRDY);

	// Enable the ADC repeatedly until success (workaround from errata)
	do
	{
		SET_BIT(ADC2->CR, ADC_CR_ADEN);
	}
	while (!READ_BIT(ADC2->ISR, ADC_ISR_ADRDY));

	// Set sample time to 47.5 cycles
	MODIFY_REG(ADC1->SMPR2, ADC_SMPR2_SMP12, ADC_SMPR2_SMP12_2);
	MODIFY_REG(ADC2->SMPR2, ADC_SMPR2_SMP12, ADC_SMPR2_SMP12_2);

	wait_ms(2);

	// enable EOC interrupt
	SET_BIT(ADC1->IER, ADC_IER_EOCIE);
	SET_BIT(ADC2->IER, ADC_IER_EOCIE);
	NVIC_SetPriority(ADC1_2_IRQn, 4);
	NVIC_EnableIRQ(ADC1_2_IRQn);
}

void adc_startconversion(ADC_TypeDef* adc, uint8_t ch)
{
	if(READ_BIT(adc->CR, ADC_CR_ADSTART))
		return;

	// convert 1 channel
	MODIFY_REG(adc->SQR1, ADC_SQR1_L, 0);

	// select channel
    MODIFY_REG(adc->SQR1, ADC_SQR1_SQ1, ch<<ADC_SQR1_SQ1_Pos);

	// start conversion
    SET_BIT(adc->CR, ADC_CR_ADSTART);
}

uint16_t adc_getnewdata()
{
	if(new)
	{
		new = false;
		return data;
	}
	else
	{
		return -1;
	}
}

bool adc_busy()
{
	if(READ_BIT(ADC1->CR, ADC_CR_ADSTART))
		return true;
	else
		return false;
}

