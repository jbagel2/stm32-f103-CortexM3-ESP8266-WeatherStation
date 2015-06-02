

#ifndef __DHT22__
#define __DHT22__

#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "misc.h"
#include "custom_libs/mytime.h"


typedef struct
{
	uint8_t Response;
	float Temp;
	float Humid;
	uint8_t ValidChecksum;
}DHT22_Data;



void DHT22_Init();
void DHT22_Start_Read(DHT22_Data *tempAndHumid);
void DHT22_Config_CLK();
void DHT22_Config_GPIO_INPUT();
void DHT22_Config_GPIO_OUTPUT();
void DHT22_Config_EXTInterrupt_Enable();
void DHT22_Config_EXTInterrupt_Disable();
void DHT22_Config_NVIC();
void DHT_Value_Checksum();
void DHT22_Times_To_Bits16(uint8_t bitTimesArray[], uint8_t arraySize);


#endif
