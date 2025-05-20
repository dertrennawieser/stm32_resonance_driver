/*
 * i2c.h
 *
 *  Created on: 27.03.2021
 *      Author: moritz
 */

#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>
#include <stdbool.h>

#include "stm32g4xx.h"


#ifndef I2C_BUFFER_SIZE
#define I2C_BUFFER_SIZE 256		//n. of reserved Bytes for receive buffer
#endif


bool i2c_busy(I2C_TypeDef*);
void i2c_init(I2C_TypeDef*, bool, uint32_t);
void i2c_start(I2C_TypeDef*, uint8_t, uint8_t[], int, uint8_t[], int );

#endif /* I2C_H_ */
