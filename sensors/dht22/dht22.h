

#ifndef __DHT22__
#define __DHT22__

typedef struct
{
	uint8_t Response;
	float Temp;
	float Humid;
	uint8_t ValidChecksum;
}DHT22_Data;


//uint32_t DHT_Error_Since_Boot = 0;


void DHT22_Init();
void DHT22_Start_Read(DHT22_Data *tempAndHumid);
void DHT22_Config_CLK();
void DHT22_Config_GPIO_INPUT();
void DHT22_Config_GPIO_OUTPUT();
void DHT22_Config_EXTInterrupt_Enable();
void DHT22_Config_EXTInterrupt_Disable();
void DHT22_Config_NVIC();
void DHT_Value_Checksum();


#endif
