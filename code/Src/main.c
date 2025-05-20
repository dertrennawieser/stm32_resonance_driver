/*
 * main.c
 *
 *  Created on: 19.05.2025
 *      Author: Moritz
 *
 *  Empty main file with the relevant includes and functions.
 *  /Inc, /stmdriver/inc, /stmdriver/CMSIS/Include, /stmdriver/CMSIS/Device must be added to the Include path
 *
 *  The driver uses the following peripherals (that cannot be used by the application):
 *  HRTIM1
 *  TIM2, TIM5
 *  COMP1, COMP2, COMP3, COMP4
 *  DAC1, DAC3
 *  DMA2 (all channels and associated DMAMUX Channels)
 *
 */

#include "stm32g4xx.h"
#include <stdint.h>
#include <stdbool.h>

#include "systeminit.h"
#include "hrtim.h"
#include "tim.h"
#include "dac.h"

#include "resonance_driver.h"


// timeout interrupt - gets called when no feedback signal is received (timeout)
void HRTIM_TIMF_IRQn_IRQHandler()
{
	// clear irq flag
	SET_BIT(HRTIM1_TIMF->TIMxICR, HRTIM_TIMICR_CPT2C);

}

/*
 * main is called from startup file startup_stm32g474rbtx.s
 * before main is called, the startup file calls systeminit in /stmdriver/systeminit.c,
 * in order to increase the processor frequency to 170 MHz
 */
int main(void)
{
	/*
	 * driver functions must be called in the following order:
	 *
	 * driver_init(lead time in ns, start frequency in Hz, timeout in us);
	 * driver_setburstparams(ontime in us, period(time between bursts) in us); // (can be called whenever)
	 * driver_arm();
	 * driver_startburstoperation();
	 *
	 * driver_stopburstoperation();
	 * driver_disarm();
	 */

	while(1)
	{

	}
}
