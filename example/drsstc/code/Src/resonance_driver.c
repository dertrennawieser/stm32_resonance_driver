/*
 * resonance_driver.c
 *
 *  Created on: 02.09.2024
 *      Author: moritz
 */

#include "resonance_driver.h"


volatile uint32_t start_period = 0;
uint32_t bm_trig_enable=0, bm_trig_disable=0;
uint32_t output_enable=0;

uint32_t current_ontime_us = 0;
uint32_t current_period_us = 0;
float current_timeout = 0;


static void config_dma(DMA_Channel_TypeDef* dma_ch, DMAMUX_Channel_TypeDef* dmamux_ch, uint8_t dmamux_sel, uint32_t ma,
		uint32_t pa, bool mem2per);

static void init_common(uint32_t leadtime_ns, float start_freq);

void driver_init(uint32_t leadtime_ns, float start_freq, uint32_t timeout_us)
{
	// enable comp clock
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN);

	SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOAEN);
	SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOBEN);

	// comparator input pins
	// zcd input pins
	MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER1, GPIO_MODER_MODER1_1 + GPIO_MODER_MODER1_0);
	MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER3, GPIO_MODER_MODER3_1 + GPIO_MODER_MODER3_0);
	//MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER7, GPIO_MODER_MODER7_1 + GPIO_MODER_MODER7_0);
	// ocd input pin
	MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER0, GPIO_MODER_MODER0_1 + GPIO_MODER_MODER0_0);


	// comp1 (zcd) configuration
	// comparator 1 negative input DAC1 CH1
	MODIFY_REG(COMP1->CSR, COMP_CSR_INMSEL, COMP_CSR_INMSEL_2 + COMP_CSR_INMSEL_0);
	// configure hysteresis 50 mV
	MODIFY_REG(COMP1->CSR, COMP_CSR_HYST, COMP_CSR_HYST_2 + COMP_CSR_HYST_0);

	// comp2 (zcd) configuration
	// comparator 2 positive input PA3
	SET_BIT(COMP2->CSR, COMP_CSR_INPSEL);
	// comparator 2 negative input DAC1 CH2
	MODIFY_REG(COMP2->CSR, COMP_CSR_INMSEL, COMP_CSR_INMSEL_2 + COMP_CSR_INMSEL_0);
	// comp2 output PA7 debugging TODO remove
	MODIFY_REG(GPIOA->AFR[0], GPIO_AFRL_AFRL7, 8<<GPIO_AFRL_AFSEL7_Pos);	//PA12 -> AF8, COMP2_OUT
	MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER7, GPIO_MODER_MODER7_1);
	// configure hysteresis 50 mV
	MODIFY_REG(COMP2->CSR, COMP_CSR_HYST, COMP_CSR_HYST_2 + COMP_CSR_HYST_0);

	// comp3 (ocd) configuration
	// comp 3 pos input pc1
	//MODIFY_REG(COMP3->CSR, COMP_CSR_INPSEL, COMP_CSR_INPSEL);
	// comparator 3 negative input DAC3 CH1
	MODIFY_REG(COMP3->CSR, COMP_CSR_INMSEL, COMP_CSR_INMSEL_2);// + COMP_CSR_INMSEL_0);
	// comp3 output PB7 debugging TODO remove
	MODIFY_REG(GPIOB->AFR[0], GPIO_AFRL_AFRL7, 8<<GPIO_AFRL_AFSEL7_Pos);	//PB7 -> AF8, COMP3_OUT
	MODIFY_REG(GPIOB->MODER, GPIO_MODER_MODER7, GPIO_MODER_MODER7_1);
	// configure hysteresis 50 mV
	MODIFY_REG(COMP3->CSR, COMP_CSR_HYST, COMP_CSR_HYST_2 + COMP_CSR_HYST_0);

	// dac for zcd
	dac1_init();
	// dac for ocd
	dac3_init();

	dac1_write(400);
	dac3_write(2000); // 3548

	init_common(leadtime_ns, start_freq);

	// set feedback timeout (time at which burst is terminated if no zcd feedback is received)
	// subtract one cycle to allow finishing current cycle and account for irq entry delay
	current_timeout = (timeout_us - 1000000/start_freq)*1000/5.882f;
	// set to 3 (miimum compare value) if timeout is too small
	if(current_timeout > 3.0f)
		HRTIM1_TIME->CMP3xR = (uint16_t)(current_timeout);
	else
		HRTIM1_TIME->CMP3xR = 3;

	// set timd period to maximum possible period, this is used for pulse skipping
	HRTIM1_TIMD->PERxR = start_period*F_MAX_MULTIPLIER;

	// timF output 2 goes high between this two cmp events
	// this is used as a blanking signal for the tim E extev (zcd) capture (for frequency tracking)
	HRTIM1_TIMF->CMP1xR = start_period/F_MIN_DIVISOR;
	HRTIM1_TIMF->CMP3xR = start_period*F_MAX_MULTIPLIER;

	// tim 5 cc3 channel is used as the burst signal
	tim5_pwminit(170, 10);

	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA2EN);
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMAMUX1EN);

	// DMA configuration for lead time frequency tracking
	config_dma(DMA2_Channel1, DMAMUX1_Channel8, 100, (uint32_t)&(TIM2->ARR), (uint32_t)&(HRTIM1_TIME->CPT1xR), false);

	// DMA configuration for enabling gate drive after burst start (exit from delayed protection)
	output_enable = (HRTIM_OENR_TB1OEN) + (HRTIM_OENR_TB2OEN) + (HRTIM_OENR_TC1OEN) + (HRTIM_OENR_TC2OEN);
	config_dma(DMA2_Channel5, DMAMUX1_Channel12, 76, (uint32_t)&(output_enable), (uint32_t)&(HRTIM1_COMMON->OENR), true);

	// configuration for pulse skipping
	if(PULSE_SKIPPING)
	{
		// burst mode trigger on eev8
		//SET_BIT(HRTIM1_COMMON->BMTRGR, HRTIM_BMTRGR_EEV8); // not working, TODO change to method 2 and test
		//hrtim_enableburst(2, 4);

		// method 2: tim d single shot, start timer with comp 3, comp 3 capture on tim B, dma request set burst trigger to timb reset
		// second dma request on timd rollover/compare, disable burst trigger

		bm_trig_enable = HRTIM_BMTRGR_TBRST;
		config_dma(DMA2_Channel2, DMAMUX1_Channel9, 97, (uint32_t)&bm_trig_enable, (uint32_t)&(HRTIM1_COMMON->BMTRGR), true);

		bm_trig_disable = 0x00000000;
		config_dma(DMA2_Channel3, DMAMUX1_Channel10, 99, (uint32_t)&bm_trig_disable, (uint32_t)&(HRTIM1_COMMON->BMTRGR), true);

		// dma2 channel 4 update timd compare with time capture value
		config_dma(DMA2_Channel4, DMAMUX1_Channel11, 98, (uint32_t)&(HRTIM1_TIMD->CMP1xR), (uint32_t)&(HRTIM1_TIME->CPT1xR), false);

		hrtim_enableburst(PULSE_SKIPPING_IDLE_PERIODS, PULSE_SKIPPING_TOTAL_PERIODS);

		// method 3: use delayed protection and dma
		// configure DMA2 CH2 to load repetition counter with 2 to skip one switching period in delayed idle
		// configure DMA2 CH3 to reactivate outputs and change repetition counter in DMA burst mode
	}

	// configuration for reloading the start frequency after every burst
	if(BURST_RELOAD_START_FREQ)
	{
		// update tim2 arr with original start period at tim5 ch2 dma request
		config_dma(DMA2_Channel6, DMAMUX1_Channel13, 73, (uint32_t)&start_period, (uint32_t)&(TIM2->ARR), true);
	}

	// software update to load all the register values from preload registers to active registers
	SET_BIT(HRTIM1_COMMON->CR2, HRTIM_CR2_TASWU + HRTIM_CR2_TBSWU + HRTIM_CR2_TCSWU + HRTIM_CR2_TDSWU + HRTIM_CR2_TESWU
			+ HRTIM_CR2_TFSWU);

	SET_BIT(COMP4->CSR, COMP_CSR_EN);
	wait_ms(1);
}

void driver_init_openloop(float freq)
{
	init_common(0, freq);

	tim5_pwminit(170, 10);

	// DMA configuration for burst
	output_enable = (HRTIM_OENR_TB1OEN) + (HRTIM_OENR_TB2OEN) + (HRTIM_OENR_TC1OEN) + (HRTIM_OENR_TC2OEN);
	config_dma(DMA2_Channel5, DMAMUX1_Channel12, 76, (uint32_t)&(output_enable), (uint32_t)&(HRTIM1_COMMON->OENR), true);
}

void driver_init_cw(bool pulse_skipping, uint32_t leadtime_ns, float start_freq, uint32_t timeout_us)
{
	// TODO
}

void driver_arm()
{
	// enable timer B and C outputs
	SET_BIT(HRTIM1_COMMON->OENR, HRTIM_OENR_TB1OEN + HRTIM_OENR_TB2OEN + HRTIM_OENR_TC1OEN + HRTIM_OENR_TC2OEN);

	// enable delayed protection for burst end
	SET_BIT(HRTIM1_TIMB->OUTxR, HRTIM_OUTR_DLYPRTEN);
	SET_BIT(HRTIM1_TIMC->OUTxR, HRTIM_OUTR_DLYPRTEN);

	// enable counters
	SET_BIT(HRTIM1->sMasterRegs.MCR, HRTIM_MCR_TACEN + HRTIM_MCR_TBCEN + HRTIM_MCR_TCCEN + HRTIM_MCR_TDCEN + HRTIM_MCR_TECEN
			+ HRTIM_MCR_TFCEN);
}

void driver_disarm()
{
	// disable timer B and C outputs
	SET_BIT(HRTIM1_COMMON->ODISR, HRTIM_ODISR_TB1ODIS + HRTIM_ODISR_TB2ODIS + HRTIM_ODISR_TC1ODIS + HRTIM_ODISR_TC2ODIS);

	// disable counters
	CLEAR_BIT(HRTIM1->sMasterRegs.MCR, HRTIM_MCR_TACEN + HRTIM_MCR_TBCEN + HRTIM_MCR_TCCEN + HRTIM_MCR_TDCEN + HRTIM_MCR_TECEN
			+ HRTIM_MCR_TFCEN);

	// disable delayed protection for burst
		// has to be done so burst operation can be restarted later
		CLEAR_BIT(HRTIM1_TIMB->OUTxR, HRTIM_OUTR_DLYPRTEN);
		CLEAR_BIT(HRTIM1_TIMC->OUTxR, HRTIM_OUTR_DLYPRTEN);

	CLEAR_BIT(TIM2->CR1, TIM_CR1_CEN);
	TIM2->CNT = 0;
}

void driver_startburstoperation()
{
	SET_BIT(TIM5->EGR, TIM_EGR_UG);
	// enable zcd, ocd and burst comparators
	SET_BIT(COMP1->CSR, COMP_CSR_EN);
	SET_BIT(COMP2->CSR, COMP_CSR_EN);
	SET_BIT(COMP3->CSR, COMP_CSR_EN);
	//SET_BIT(COMP4->CSR, COMP_CSR_EN);

	// load cmp2 registers with half of the period for blanking
	// this is updated during the burst by the triggered-half mode
	HRTIM1_TIMB->CMP2xR = start_period/2;
	HRTIM1_TIMC->CMP2xR = start_period/2;

	TIM5->CNT = 0;
	SET_BIT(TIM5->CR1, TIM_CR1_CEN);

	TIM2->CNT = 0;
	TIM2->ARR = start_period;
	SET_BIT(TIM2->EGR, TIM_EGR_UG);
	SET_BIT(HRTIM1_COMMON->CR2, HRTIM_CR2_TASWU + HRTIM_CR2_TBSWU + HRTIM_CR2_TCSWU);
	WRITE_REG(HRTIM1_COMMON->CR2, HRTIM_CR2_TARST + HRTIM_CR2_TBRST + HRTIM_CR2_TCRST + HRTIM_CR2_TERST + HRTIM_CR2_TFRST);
	SET_BIT(TIM2->CR1, TIM_CR1_CEN);

}

void driver_startburstoperation_openloop()
{
	//enable burst comparator
	SET_BIT(COMP4->CSR, COMP_CSR_EN);

	TIM5->CNT = 0;
	SET_BIT(TIM5->CR1, TIM_CR1_CEN);

	TIM2->CNT = 0;
	TIM2->ARR = start_period;
	WRITE_REG(HRTIM1_COMMON->CR2, HRTIM_CR2_TARST + HRTIM_CR2_TBRST + HRTIM_CR2_TCRST);
	SET_BIT(TIM2->CR1, TIM_CR1_CEN);

}

void driver_stopburstoperation()
{
	// disable zcd and ocd comparators
	CLEAR_BIT(COMP1->CSR, COMP_CSR_EN);
	CLEAR_BIT(COMP2->CSR, COMP_CSR_EN);
	CLEAR_BIT(COMP3->CSR, COMP_CSR_EN);

	// set cnt above cc3 value to force the output to low level
	TIM5->CNT = current_ontime_us + 1;
	CLEAR_BIT(TIM5->CR1, TIM_CR1_CEN);

	TIM2->CNT = 0;
	CLEAR_BIT(TIM2->CR1, TIM_CR1_CEN);

	// disable burst comparator
	//CLEAR_BIT(COMP4->CSR, COMP_CSR_EN);

}

void driver_setburstparams(uint32_t ontime_us, uint32_t period_us)
{
	if(ontime_us <= 0)
		current_ontime_us = 2;
	else
		current_ontime_us = ontime_us;


	current_period_us = period_us;

	TIM5->CCR2 = current_period_us - BURST_DMARQ_LEADTIME_US;
	TIM5->CCR3 = current_ontime_us;
	TIM5->CCR4 = current_period_us - BURST_DMARQ_LEADTIME_US;
	TIM5->ARR = current_period_us;

	//SET_BIT(TIM5->EGR, TIM_EGR_UG);
}

void driver_startcwoperation()
{
	// TODO
}

void driver_stopcwoperation()
{
	// TODO
}

static void config_dma(DMA_Channel_TypeDef* dma_ch, DMAMUX_Channel_TypeDef* dmamux_ch, uint8_t dmamux_sel, uint32_t ma,
		uint32_t pa, bool mem2per)
{
	// set periperal address
	MODIFY_REG(dma_ch->CPAR, DMA_CPAR_PA, pa);

	// set memory address
	MODIFY_REG(dma_ch->CMAR, DMA_CMAR_MA, ma);

	// configure total number of data
	MODIFY_REG(dma_ch->CNDTR, DMA_CNDTR_NDT, 1);

	// set size of memory size to 32 bit
	MODIFY_REG(dma_ch->CCR, DMA_CCR_MSIZE, DMA_CCR_MSIZE_1);

	// set size of peripheral size to 32 bit
	MODIFY_REG(dma_ch->CCR, DMA_CCR_PSIZE, DMA_CCR_PSIZE_1);

	if(mem2per)
	{
		// data direction memory -> peripheral
		SET_BIT(dma_ch->CCR, DMA_CCR_DIR);
	}
	else
	{
		// data direction memory -> peripheral
		CLEAR_BIT(dma_ch->CCR, DMA_CCR_DIR);
	}

	// set circular mode
	SET_BIT(dma_ch->CCR, DMA_CCR_CIRC);

	// enable transfer complete interrupt
	SET_BIT(dma_ch->CCR, DMA_CCR_TCIE);

	// config dmamux
	MODIFY_REG(dmamux_ch->CCR, DMAMUX_CxCR_DMAREQ_ID, dmamux_sel << DMAMUX_CxCR_DMAREQ_ID_Pos);

	// enable channel
	SET_BIT(dma_ch->CCR, DMA_CCR_EN);
}

static void init_common(uint32_t leadtime_ns, float start_freq)
{
	// enable comp clock
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN);

	SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOAEN);
	SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOBEN);

	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA2EN);
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMAMUX1EN);

	// burst comparator configuration
	// comparator input pins
	MODIFY_REG(GPIOB->MODER, GPIO_MODER_MODER0, GPIO_MODER_MODER0_1 + GPIO_MODER_MODER0_0);

	// comparator 4 negative input vrefint
	SET_BIT(COMP4->CSR, COMP_CSR_SCALEN);
	MODIFY_REG(COMP4->CSR, COMP_CSR_INMSEL, COMP_CSR_INMSEL_1 + COMP_CSR_INMSEL_0);
	// wait for scaler startup (see datasheet)
	wait_ms(1);
	// configure hysteresis 50 mV
	MODIFY_REG(COMP4->CSR, COMP_CSR_HYST, COMP_CSR_HYST_2 + COMP_CSR_HYST_0);

	// period here refers to the timer period (half of the resonance period)
	start_period = (uint32_t)(170000000.0f/(start_freq*2));

	hrtim_init(PULSE_SKIPPING);

	tim2_init(1, start_period);
	TIM2->CCR1 = (uint16_t)(leadtime_ns/5.882f);	// one timer incrementation at 170 MHz takes 5.882 ns

	// cmp1 is used for blanking (rejecting reset signals before the minimum period time elapsed)
	HRTIM1_TIMA->CMP1xR = start_period/F_MIN_DIVISOR;
	HRTIM1_TIMA->CMP2xR = 5;
	// blanking for rejecting tim2 trgo signals shortly after reset
	// should not happen anyway in principle but makes sure there is no reset from the signal delay hrtim->tim2->hrtim
	HRTIM1_TIMB->CMP1xR = 5;
	HRTIM1_TIMC->CMP1xR = 5;
}

