/*
 * dac.h
 *
 *  Created on: Aug 29, 2022
 *      Author: moritz
 */

#ifndef DAC_H_
#define DAC_H_

#include "stm32g4xx.h"

void dac1_init();
void dac3_init();
void dac1_write(uint16_t value);
void dac3_write(uint16_t value);

#endif /* DAC_H_ */
