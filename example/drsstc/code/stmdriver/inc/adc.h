/*
 * adc.h
 *
 *  Created on: 21.07.2022
 *      Author: moritz
 */

#ifndef ADC_H_
#define ADC_H_

#include "stm32g4xx.h"

#include "stdbool.h"

#include "systeminit.h"


void adc_init();
void adc_startconversion(ADC_TypeDef* adc, uint8_t ch);
uint16_t adc_getnewdata();
bool adc_busy();

#endif /* ADC_H_ */
