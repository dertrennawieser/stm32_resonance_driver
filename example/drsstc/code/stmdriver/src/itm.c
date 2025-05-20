/*
 * itm.c
 *
 *  Created on: 17.08.2020
 *      Author: Moritz
 */

#include <stdint.h>
#include "stm32g4xx.h"

#include "itm.h"


static void ITM_itoa(int32_t z, char* buffer)
{
	int8_t i=0, j;
	char temp[LEN];

	if(z<0)
	{
		buffer[0] = '-';
		z = (-(z+1))+1;
	}
	else
	{
		buffer[0] = ' ';
	}

	do
	{
		temp[i] = (z%10) + '0';
		z /= 10;
		i++;
	}while(z>0);
	i--;
	for(j=1; i>=0; j++)
	{
		buffer[j] = temp[i];
		i--;
	}
	buffer[j] = '\0';
}

void ITM_SendString(char *ptr)
{
    while (*ptr)
    {
        ITM_SendChar(*ptr);
        ptr++;
    }
}

void ITM_SendInt(int32_t o)
{
	char arr[LEN];
	ITM_itoa(o, arr);
	ITM_SendString(arr);
}
