/*
 * systeminit.c
 *
 *  Created on: 21.07.2022
 *      Author: moritz
 */

#include "systeminit.h"

uint32_t SystemCoreClock=16000000;
uint32_t APB1Clock=16000000;

volatile uint32_t systick_count = 0;


// Change system clock to 170 MHz using external 24 MHz crystal
// Called by Assembler startup code
void SystemInit(void)
{
	// Because the debugger switches PLL on, we may need to switch
	// back to the HSI oscillator before we can configure the PLL

	// debug
	SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOBEN);
	MODIFY_REG(GPIOB->MODER, GPIO_MODER_MODER5, GPIO_MODER_MODER5_0);
	SET_BIT(GPIOB->BSRR, GPIO_BSRR_BS5);

	// Enable HSI oscillator
	SET_BIT(RCC->CR, RCC_CR_HSION);

	// Wait until HSI oscillator is ready
	while(!READ_BIT(RCC->CR, RCC_CR_HSIRDY));

	// Switch to HSI oscillator
	MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_HSI);

	// Wait until the switch is done
	while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_HSI);

	// Disable the PLL
	CLEAR_BIT(RCC->CR, RCC_CR_PLLON);

	// Wait until PLL is fully stopped
	while(READ_BIT(RCC->CR, RCC_CR_PLLRDY));

	// enable pwr clock
	SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_PWREN);

	// divide AHB clock by 2 before switching to higher frequency
	MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE, RCC_CFGR_HPRE_3);

	// range 1 boost mode
	CLEAR_BIT(PWR->CR5, PWR_CR5_R1MODE);

	// Flash latency 4 wait states
	MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, FLASH_ACR_LATENCY_4WS);

	// Enable HSE oscillator
	SET_BIT(RCC->CR, RCC_CR_HSEON);

	// Wait until HSE oscillator is ready
	while(!READ_BIT(RCC->CR, RCC_CR_HSERDY));

	// 170 MHz using 24 MHz crystal with HSE/4 * 85/2
	WRITE_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC_HSE + RCC_PLLCFGR_PLLM_2 + RCC_PLLCFGR_PLLM_0 + RCC_PLLCFGR_PLLN_6 + RCC_PLLCFGR_PLLN_4 + RCC_PLLCFGR_PLLN_2 + RCC_PLLCFGR_PLLN_0);

	// enable PLL R
	SET_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLREN);

	// Enable PLL
	SET_BIT(RCC->CR, RCC_CR_PLLON);

	// Wait until PLL is ready
	while(!READ_BIT(RCC->CR, RCC_CR_PLLRDY));

	// Select PLL as clock source
	MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_PLL);

	// Update variables
	APB1Clock=170000000;
	SystemCoreClock=170000000;

	// Disable the HSI oscillator
	CLEAR_BIT(RCC->CR, RCC_CR_HSION);

	// FPU einschalten
	SCB->CPACR = 0x00F00000;

	// systick interrupt every ms
	SysTick_Config(SystemCoreClock/1000);

	wait_ms(1);

	// switch AHB back to 170 MHz
	CLEAR_BIT(RCC->CFGR, RCC_CFGR_HPRE_3);
}


/*
// Change system clock to 170 MHz using internal 16 MHz R/C oscillator TODO set pll correctlu
// Called by Assembler startup code
void SystemInit(void)
{
	// Enable HSI oscillator
	SET_BIT(RCC->CR, RCC_CR_HSION);

	// Wait until HSI oscillator is ready
	while(!READ_BIT(RCC->CR, RCC_CR_HSIRDY));

	// Switch to HSI oscillator
	MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_HSI);

	// Wait until the switch is done
	while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_HSI);

	// Disable the PLL
	CLEAR_BIT(RCC->CR, RCC_CR_PLLON);

	// Wait until PLL is fully stopped
	while(READ_BIT(RCC->CR, RCC_CR_PLLRDY));

	// enable pwr clock
	SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_PWREN);

	// divide AHB clock by 2 before switching to higher frequency
	MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE, RCC_CFGR_HPRE_3);

	// range 1 boost mode
	CLEAR_BIT(PWR->CR5, PWR_CR5_R1MODE);

	// Flash latency 4 wait states
	MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, FLASH_ACR_LATENCY_4WS);

	// 170 MHz using 16 MHz crystal with HSE/2 * 85/2
	WRITE_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC_HSE + RCC_PLLCFGR_PLLM_0 + RCC_PLLCFGR_PLLN_6 + RCC_PLLCFGR_PLLN_4 + RCC_PLLCFGR_PLLN_2 + RCC_PLLCFGR_PLLN_0);

	// enable PLL R
	SET_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLREN);

	// Enable PLL
	SET_BIT(RCC->CR, RCC_CR_PLLON);

	// Wait until PLL is ready
	while(!READ_BIT(RCC->CR, RCC_CR_PLLRDY));

	// Select PLL as clock source
	MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_PLL);

	// Update variables
	APB1Clock=170000000;
	SystemCoreClock=170000000;

	// FPU einschalten
	SCB->CPACR = 0x00F00000;

	// systick interrupt every ms
	SysTick_Config(SystemCoreClock/1000);

	wait_ms(1);

	// switch AHB back to 170 MHz
	CLEAR_BIT(RCC->CFGR, RCC_CFGR_HPRE_3);
}
*/
void sleeponexit()
{
	// disable deepsleep mode
	CLEAR_BIT(SCB->SCR, SCB_SCR_SLEEPDEEP_Msk);

	// enable sleep on exit
	SET_BIT(SCB->SCR, SCB_SCR_SLEEPONEXIT_Msk);
}

/*
void stoponexit()
{
	// set v reg low power mode when cpu enters deepsleep
	SET_BIT(PWR->CR, PWR_CR_LPDS);

	// enter stop mode when cpu enters deepsleep
	CLEAR_BIT(PWR->CR, PWR_CR_PDDS);

	// clear wake up flag
	SET_BIT(PWR->CR, PWR_CR_CWUF);

	// enable deepsleep mode
	SET_BIT(SCB->SCR, SCB_SCR_SLEEPDEEP_Msk);

	// enable sleep on exit
	SET_BIT(SCB->SCR, SCB_SCR_SLEEPONEXIT_Msk);
}
*/

void wait_ms(uint32_t ticks) {
	uint64_t end = systick_count + ticks;
	while (systick_count < end);
}

uint64_t systick_gettick() {
	return systick_count;
}

void SysTick_Handler(void)
{
    systick_count++;
}

void iwdg_init()
{
	// disable iwdg in debug mode
	SET_BIT(DBGMCU->APB1FZR1, DBGMCU_APB1FZR1_DBG_IWDG_STOP);

	// enable iwdg
	IWDG->KR = 0x0000CCCC;

	// enable register write access
	IWDG->KR = 0x00005555;

	// set prescaler
	IWDG->PR = 0;

	// set reload value
	IWDG->RLR = 30;

	// wait for the registers to be updated
	uint32_t start = systick_gettick();
	while(IWDG->SR)
	{
		if((systick_gettick() - start) > TIMEOUT_MS)
			fault_handler();
	}

	// set window and refresh iwdg
	IWDG->KR = 0x0000AAAA;
}

void iwdg_refresh()
{
	IWDG->KR = 0x0000AAAA;
}

void fault_handler()
{

	while(1);
}
