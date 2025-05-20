/*
 * resonance_driver.h
 *
 *  Created on: 02.09.2024
 *      Author: moritz
 */

/*
 * PIN CONNECTIONS
 *
 * PA0 	COMP3_INP 	- OCD
 * PA1 	COMP1_INP 	- Current feedback 1
 * PA2  TIM5 CH3	- burst mode timer output
 * PA3 	COMP2_INP	- Current feedback 2
 * PA10 TIMB CH1	- Gate drive Bridge 1 high
 * PA11 TIMB CH2	- Gate drive Bridge 1 low
 * PB0	COMP4_INP	- Delayed protection for OCD/fault
 * PB12 TIMC CH1 	- Gate drive Bridge 2 high
 * PB13 TIMC CH2	- Gate drive Bridge 2 low
 *
 * to use burst mode PA2 and PB0 must be connected
 *
 */

#ifndef RESONANCE_DRIVER_H_
#define RESONANCE_DRIVER_H_

#include "dac.h"
#include "hrtim.h"
#include "systeminit.h"
#include "tim.h"

#define F_MAX_MULTIPLIER 3.0f
#define F_MIN_DIVISOR 3.0f

#define PULSE_SKIPPING true
#define PULSE_SKIPPING_IDLE_PERIODS 2
#define PULSE_SKIPPING_TOTAL_PERIODS (PULSE_SKIPPING_IDLE_PERIODS + 1)

#define BURST_RELOAD_START_FREQ true
#define BURST_DMARQ_LEADTIME_US 10


void driver_init(uint32_t leadtime_ns, float start_freq, uint32_t timeout_us);
void driver_arm();
void driver_disarm();
void driver_startburstoperation();
void driver_startburstoperation_openloop();
void driver_stopburstoperation();
void driver_setburstparams(uint32_t ontime_us, uint32_t period_us);

#endif /* RESONANCE_DRIVER_H_ */
