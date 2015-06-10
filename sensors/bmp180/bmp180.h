

#ifndef __BMP180_H__
#define __BMP180_H__

#include "stm_lib/inc/stm32f10x_rcc.h"
#include "stm_lib/inc/stm32f10x_i2c.h"
#include "stm_lib/inc/misc.h"

typedef struct
{
	uint8_t Response;
	uint16_t Pressure;
	uint16_t Altitude;
	uint16_t Temp;
}BMP180_Data;


void BMP180_Init(uint32_t i2c_speed);



#endif //"__BMP180_H__"
