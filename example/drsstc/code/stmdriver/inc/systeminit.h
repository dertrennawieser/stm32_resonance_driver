/*
 * systeminit.h
 *
 *  Created on: 21.07.2022
 *      Author: moritz
 */

#ifndef SYSTEMINIT_H_
#define SYSTEMINIT_H_

#include "stm32g4xx.h"

#define TIMEOUT_MS 1000

void SystemInit(void);

void sleeponexit();
void stoponexit();

void wait_ms(uint32_t);
uint64_t systick_gettick();

void iwdg_init();
void iwdg_refresh();

void fault_handler();

#endif /* SYSTEMINIT_H_ */
