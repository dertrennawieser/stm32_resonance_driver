/*
 * tim.c
 *
 *  Created on: 21.07.2022
 *      Author: moritz
 */

#include "tim.h"


void tim2_init(uint16_t psc, uint16_t arr)
{
	//enable Clock für Timer 2
	SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM2EN);

	//enable prescaler
	TIM2->PSC = psc-1;
	//generate update event -> load prescaler
	SET_BIT(TIM2->EGR, TIM_EGR_UG);

	//auto reload register (max value)
	TIM2->ARR = arr;

	// config as downcounter
	SET_BIT(TIM2->CR1, TIM_CR1_DIR);

	// config slave mode for internal trigger 10, hrtim syncout 2
	MODIFY_REG(TIM2->SMCR, TIM_SMCR_TS, TIM_SMCR_TS_3 + TIM_SMCR_TS_2 + TIM_SMCR_TS_1);

	// set trigger to reset
	MODIFY_REG(TIM2->SMCR, TIM_SMCR_SMS, 0b0100<<TIM_SMCR_SMS_Pos); //TODO test vs combined reset+trigger mode

	// set trigger source to comp1 out
	// MODIFY_REG(TIM2->AF1, TIM1_AF1_ETRSEL, TIM1_AF1_ETRSEL_0);

	// send trgo signal pulse on compare 1 match
	MODIFY_REG(TIM2->CR2, TIM_CR2_MMS, 0b011<<TIM_CR2_MMS_Pos);

	//generate update event -> load prescaler
	SET_BIT(TIM2->EGR, TIM_EGR_UG);

	//Timer enable auto-preload
	SET_BIT(TIM2->CR1, TIM_CR1_ARPE);

}

void tim3_pwminit(uint16_t psc, uint16_t arr)
{
	//enable Clock für Timer 3
	SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM3EN);

	SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOBEN);

	// select slave mode trigger tim2 trgo
	//MODIFY_REG(TIM3->SMCR, TIM_SMCR_TS, TIM_SMCR_TS_0);
	// slave mode select external clock mode
	//MODIFY_REG(TIM3->SMCR, TIM_SMCR_SMS, TIM_SMCR_SMS_0 + TIM_SMCR_SMS_1 + TIM_SMCR_SMS_2);

	// CH4 - PB1
	MODIFY_REG(GPIOB->AFR[0], GPIO_AFRL_AFRL1, 2<<GPIO_AFRL_AFSEL1_Pos);	// PB1 -> AF2, CH4
	MODIFY_REG(GPIOB->MODER, GPIO_MODER_MODER1, GPIO_MODER_MODER1_1);

	// Timer 3 channel 4 compare mode = PWM1, preload buffer enabled
	MODIFY_REG(TIM3->CCMR2, TIM_CCMR2_OC4M + TIM_CCMR2_OC4PE, TIM_CCMR2_OC4M_2 + TIM_CCMR2_OC4M_1 + TIM_CCMR2_OC4PE);

	// Enable compare output
	SET_BIT(TIM3->CCER, TIM_CCER_CC4E);


	//invert
	//SET_BIT(TIM3->CCER, TIM_CCER_CC1P + TIM_CCER_CC2P + TIM_CCER_CC3P + TIM_CCER_CC4P);

	//auto reload register (max value)
	TIM3->ARR = arr;

	TIM3->CCR4 = 0;

	/*
	//enable compare interrupt
	SET_BIT(TIM3->DIER, TIM_DIER_CC1IE);
	SET_BIT(TIM3->DIER, TIM_DIER_CC2IE);
	SET_BIT(TIM3->DIER, TIM_DIER_CC3IE);
	SET_BIT(TIM3->DIER, TIM_DIER_CC4IE);

	//enable update interrupt
	//SET_BIT(TIM3->DIER, TIM_DIER_UIE);
	*/
	// enable update dma request
	SET_BIT(TIM3->DIER, TIM_DIER_UDE);

	//enable prescaler
	TIM3->PSC = psc-1;
	//generate update event -> load prescaler
	SET_BIT(TIM3->EGR, TIM_EGR_UG);

	//Timer enable auto-preload
	SET_BIT(TIM3->CR1, TIM_CR1_ARPE);
}

void tim5_pwminit(uint16_t psc, uint16_t arr)
{
	//enable Clock für Timer 3
	SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM5EN);

	SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOBEN);

	// select slave mode trigger tim2 trgo
	//MODIFY_REG(TIM5->SMCR, TIM_SMCR_TS, TIM_SMCR_TS_0);
	// slave mode select external clock mode
	//MODIFY_REG(TIM5->SMCR, TIM_SMCR_SMS, TIM_SMCR_SMS_0 + TIM_SMCR_SMS_1 + TIM_SMCR_SMS_2);

	// CH3 - PA2
	MODIFY_REG(GPIOA->AFR[0], GPIO_AFRL_AFRL2, 2<<GPIO_AFRL_AFSEL2_Pos);	// PA2 -> AF2, CH3
	MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER2, GPIO_MODER_MODER2_1);

	// Timer 5 channel 3 compare mode = PWM1, preload buffer enabled
	MODIFY_REG(TIM5->CCMR2, TIM_CCMR2_OC3M + TIM_CCMR2_OC3PE, TIM_CCMR2_OC3M_2 + TIM_CCMR2_OC3M_1 + TIM_CCMR2_OC3PE);

	// Enable compare output
	SET_BIT(TIM5->CCER, TIM_CCER_CC3E);


	//invert
	//SET_BIT(TIM5->CCER, TIM_CCER_CC1P + TIM_CCER_CC2P + TIM_CCER_CC3P + TIM_CCER_CC4P);

	//auto reload register (max value)
	TIM5->ARR = arr;

	TIM5->CCR3 = 0;

	/*
	//enable compare interrupt
	SET_BIT(TIM5->DIER, TIM_DIER_CC1IE);
	SET_BIT(TIM5->DIER, TIM_DIER_CC2IE);
	SET_BIT(TIM5->DIER, TIM_DIER_CC3IE);
	SET_BIT(TIM5->DIER, TIM_DIER_CC4IE);

	//enable update interrupt
	//SET_BIT(TIM5->DIER, TIM_DIER_UIE);
	*/
	// enable update dma request
	SET_BIT(TIM5->DIER, TIM_DIER_UDE);
	// enable ch2 dma request
	SET_BIT(TIM5->DIER, TIM_DIER_CC2DE);
	// enable ch3 dma request
	//SET_BIT(TIM5->DIER, TIM_DIER_CC3DE);
	// enable ch4 dma request
	//SET_BIT(TIM5->DIER, TIM_DIER_CC4DE);

	//enable prescaler
	TIM5->PSC = psc-1;
	//generate update event -> load prescaler
	SET_BIT(TIM5->EGR, TIM_EGR_UG);

	//Timer enable auto-preload
	SET_BIT(TIM5->CR1, TIM_CR1_ARPE);
}

void tim6_init(uint16_t psc, uint16_t arr)
{
	//TIM2 & TIM4 clock enable
	SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM6EN);

	//auto reload register (max value)
	TIM6->ARR = arr;

	//Precsaler for 100MHz -> 1MHz (/100)
	TIM6->PSC = psc-1;

	//generate update event -> load prescaler
	SET_BIT(TIM6->EGR, TIM_EGR_UG);

	//enable update interrupt
	SET_BIT(TIM6->DIER, TIM_DIER_UIE);
}

void tim7_init(uint16_t psc, uint16_t arr)
{
	//TIM2 & TIM4 clock enable
	SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM7EN);

	//auto reload register (max value)
	TIM7->ARR = arr;

	//Precsaler for 100MHz -> 1MHz (/100)
	TIM7->PSC = psc-1;

	// configure in one pulse mode - disable counter after update
	SET_BIT(TIM7->CR1, TIM_CR1_OPM);

	// trigger output trgo on update
	SET_BIT(TIM7->CR2, TIM_CR2_MMS_1);

	//generate update event -> load prescaler
	SET_BIT(TIM7->EGR, TIM_EGR_UG);

	//enable update interrupt
	SET_BIT(TIM7->DIER, TIM_DIER_UIE);
}



/*
void TIM3_IRQHandler()
{
	if(READ_BIT(TIM3->SR, TIM_SR_CC1IF))
	{
		CLEAR_BIT(TIM3->SR, TIM_SR_CC1IF);

	}
	else if(READ_BIT(TIM3->SR, TIM_SR_CC2IF))
	{
		CLEAR_BIT(TIM3->SR, TIM_SR_CC2IF);

	}
	else if(READ_BIT(TIM3->SR, TIM_SR_CC3IF))
	{
		CLEAR_BIT(TIM3->SR, TIM_SR_CC3IF);

	}
	else if(READ_BIT(TIM3->SR, TIM_SR_CC4IF))
	{
		CLEAR_BIT(TIM3->SR, TIM_SR_CC4IF);

	}
}
*/

