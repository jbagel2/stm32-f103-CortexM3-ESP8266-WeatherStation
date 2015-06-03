

#ifndef __BMP180_H__
#define __BMP180_H__

#include "misc.h"

typedef struct
{
	uint8_t Response;
	uint16_t Pressure;
	uint16_t Altitude;
	uint16_t Temp;
}BMP180_Data;


void BMP180_Init();

#endif //"__BMP180_H__"
