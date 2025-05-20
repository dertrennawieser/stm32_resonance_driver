/*
 * hrtim.h
 *
 *  Created on: Jan 18, 2024
 *      Author: moritz
 */

#ifndef HRTIM_H_
#define HRTIM_H_

#include <stdbool.h>

#include "stm32g4xx.h"

void hrtim_init(bool pulse_skipping);
void hrtim_enableburst(uint16_t idle, uint16_t period);

#endif /* HRTIM_H_ */
