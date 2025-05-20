/*
 * tim.h
 *
 *  Created on: 21.07.2022
 *      Author: moritz
 */

#ifndef TIM_H_
#define TIM_H_

#include "stm32g4xx.h"

void tim2_init(uint16_t psc, uint16_t arr);
void tim3_pwminit(uint16_t psc, uint16_t arr);
void tim5_pwminit(uint16_t psc, uint16_t arr);

void tim6_init(uint16_t psc, uint16_t arr);
void tim7_init(uint16_t psc, uint16_t arr);

#endif /* TIM_H_ */
