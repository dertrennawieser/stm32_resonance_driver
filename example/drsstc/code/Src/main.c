/*
 * main.c
 *
 *  Created on: 18.01.2024
 *      Author: Moritz
 */

#include "stm32g4xx.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "systeminit.h"
#include "hrtim.h"
#include "tim.h"
#include "itm.h"
#include "adc.h"
#include "dac.h"

#include "resonance_driver.h"


#define PERIOD_MIN 800
#define ONTIME_MAX 100



volatile bool ocd_triggered = false;

uint32_t ontime = 6;
uint32_t period = 1000;

uint32_t pot_ontime = 0;
float pot_freq = 0;

bool adc_m1_done = false;
bool idle = true;

uint32_t temp = 0;

// timeout interrupt - gets called when no feedback signal is received before timeout
void HRTIM_TIMF_IRQn_IRQHandler()
{
	// clear irq flag
	SET_BIT(HRTIM1_TIMF->TIMxICR, HRTIM_TIMICR_CMP3C);

	SET_BIT(GPIOB->BSRR, GPIO_BSRR_BS4);

	// debug
	GPIOC->ODR ^= GPIO_ODR_OD0;
	//SET_BIT(GPIOC->BSRR, GPIO_BSRR_BR0);

	CLEAR_BIT(HRTIM1->sMasterRegs.MCR, HRTIM_MCR_TFCEN);

	// disable timb set output 1 on event 1,2,4
	CLEAR_BIT(HRTIM1_TIMB->SETx1R, HRTIM_SET1R_CMP2);
	// disable timb set output 2 on event 1,2,4
	CLEAR_BIT(HRTIM1_TIMB->SETx2R, HRTIM_SET2R_EXTVNT2 + HRTIM_SET2R_EXTVNT6);
	// disable timc set output 1 on event 1,2,4
	CLEAR_BIT(HRTIM1_TIMC->SETx1R, HRTIM_SET1R_EXTVNT2 + HRTIM_SET1R_EXTVNT6);
	// disable timc set output 2 on event 1,2,4
	CLEAR_BIT(HRTIM1_TIMC->SETx2R, HRTIM_SET2R_CMP2);

	while(READ_BIT(HRTIM1_TIMB->TIMxISR, HRTIM_TIMISR_O1CPY) || READ_BIT(HRTIM1_TIMB->TIMxISR, HRTIM_TIMISR_O2CPY) ||
			READ_BIT(HRTIM1_TIMC->TIMxISR, HRTIM_TIMISR_O1CPY) || READ_BIT(HRTIM1_TIMC->TIMxISR, HRTIM_TIMISR_O2CPY));

	driver_stopburstoperation();
	driver_disarm();
}

// adc interrupt for reading bps and ontime potentiometers
void ADC1_2_IRQHandler()
{
	if(READ_BIT(ADC1->ISR, ADC_ISR_EOC))
	{
		// clear ADC busy flag
		CLEAR_BIT(ADC1->CR, ADC_CR_ADSTART);

		// read DR (clears EOC)
		pot_freq = (4095-ADC1->DR) / 4095.0f;
		pot_freq = powf(10.0f, pot_freq*3.0f);
		period = (uint32_t)(1000000.0f/pot_freq);
		adc_m1_done = false;
	}
	if(READ_BIT(ADC2->ISR, ADC_ISR_EOC))
	{
		// clear ADC busy flag
		CLEAR_BIT(ADC2->CR, ADC_CR_ADSTART);

		// read DR (clears EOC)
		pot_ontime = 4096 - ADC2->DR;
		ontime = (uint32_t)(pot_ontime*ONTIME_MAX/4096.0f);
		adc_m1_done = true;

		adc_startconversion(ADC1, 12);
	}
}

// timer interrupt every ms for polling switch and starting adc conversion
void TIM6_DACUNDER_IRQHandler()
{
	CLEAR_BIT(TIM6->SR, TIM_SR_UIF);

	if(!READ_BIT(GPIOA->IDR, GPIO_IDR_ID4))
	{
		SET_BIT(GPIOC->BSRR, GPIO_BSRR_BS12);
		if(idle)
		{
			// disable timb set output 1 on event 1,2,4
			SET_BIT(HRTIM1_TIMB->SETx1R, HRTIM_SET1R_CMP2);
			// disable timb set output 2 on event 1,2,4
			SET_BIT(HRTIM1_TIMB->SETx2R, HRTIM_SET2R_EXTVNT2 + HRTIM_SET2R_EXTVNT6);
			// disable timc set output 1 on event 1,2,4
			SET_BIT(HRTIM1_TIMC->SETx1R, HRTIM_SET1R_EXTVNT2 + HRTIM_SET1R_EXTVNT6);
			// disable timc set output 2 on event 1,2,4
			SET_BIT(HRTIM1_TIMC->SETx2R, HRTIM_SET2R_CMP2);

			driver_arm();
			SET_BIT(GPIOC->BSRR, GPIO_BSRR_BR0);
			driver_startburstoperation();

			//debug
			SET_BIT(GPIOC->BSRR, GPIO_BSRR_BS0);
			idle = false;
		}
	}
	else
	{
		SET_BIT(GPIOC->BSRR, GPIO_BSRR_BR12);
		SET_BIT(GPIOB->BSRR, GPIO_BSRR_BR4);
		driver_stopburstoperation();
		driver_disarm();
		idle = true;
	}

	if(period < PERIOD_MIN)
		period = PERIOD_MIN;

	driver_setburstparams(ontime, period);
	adc_startconversion(ADC2, 12);
}



int main(void)
{
	ITM_SendString("test\n");

	SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOAEN);
	SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOBEN);
	SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOCEN);

	// board led 1
	MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER15, GPIO_MODER_MODER15_0);
	// board led 2
	MODIFY_REG(GPIOB->MODER, GPIO_MODER_MODER4, GPIO_MODER_MODER4_0);
	// board led 3
	MODIFY_REG(GPIOB->MODER, GPIO_MODER_MODER5, GPIO_MODER_MODER5_0);

	// debug
	MODIFY_REG(GPIOC->MODER, GPIO_MODER_MODER0, GPIO_MODER_MODER0_0);

	// taster pull up
	MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODE4, 0);
	MODIFY_REG(GPIOA->PUPDR, GPIO_PUPDR_PUPD4, GPIO_PUPDR_PUPD4_0);

	// ext led
	MODIFY_REG(GPIOC->MODER, GPIO_MODER_MODER12, GPIO_MODER_MODER12_0);

	SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR15);
	SET_BIT(GPIOB->BSRR, GPIO_BSRR_BR4);
	SET_BIT(GPIOB->BSRR, GPIO_BSRR_BR5);


	//NVIC_EnableIRQ(DMA1_Channel1_IRQn);

	adc_init();
	tim6_init(1700, 100);

	//driver_init_openloop(180000);
	driver_init(350, 180000, 20);
	//driver_arm();
	driver_setburstparams(20, 1000000);

	NVIC_EnableIRQ(TIM6_DAC_IRQn);
	SET_BIT(TIM6->CR1, TIM_CR1_CEN);

	SET_BIT(GPIOC->BSRR, GPIO_BSRR_BR0);

	SET_BIT(GPIOA->BSRR, GPIO_BSRR_BS15);

	//driver_arm();
	//driver_startburstoperation();
	while(1)
	{
		//debug
		temp = TIM2->ARR;
		//if(temp <  100 )
		if(READ_BIT(HRTIM1_COMMON->CR2, HRTIM_CR2_SWPB))
		{
			//temp = TIM2->ARR;
			SET_BIT(GPIOB->BSRR, GPIO_BSRR_BS5);
			__NOP();
		}
		/*
		wait_ms(500);
		SET_BIT(GPIOA->BSRR, GPIO_BSRR_BS15);
		//SET_BIT(GPIOB->BSRR, GPIO_BSRR_BS4);
		//SET_BIT(GPIOB->BSRR, GPIO_BSRR_BS5);
		wait_ms(500);
		SET_BIT(GPIOA->BSRR, GPIO_BSRR_BR15);
		//SET_BIT(GPIOB->BSRR, GPIO_BSRR_BR4);
		//SET_BIT(GPIOB->BSRR, GPIO_BSRR_BR5);
		*/
		/*
		wait_ms(100);
		SET_BIT(GPIOC->BSRR, GPIO_BSRR_BS12);
		wait_ms(100);
		SET_BIT(GPIOC->BSRR, GPIO_BSRR_BR12);
		*/
	}
}
