

#include "dht22.h"


#define DHT22_Pin GPIO_Pin_6


GPIO_InitTypeDef DHT22_Pin_GPIO_Config;

volatile uint8_t DHT22_Buffer[6]; //5 byte array, to hold the 40 bits
volatile uint16_t DHT22_Buffer16[3];
volatile uint32_t upTimeStart = 0;
volatile uint32_t upTimeEnd = 0;
volatile uint32_t downTimeStart = 0;
volatile uint32_t downTimeEnd = 0;
volatile uint8_t DHT22_Bit_Time[45]; //testing as 8bit instead of 32 (to save memory)
volatile uint8_t currentBit = 0;


void DHT22_Init()
{
	DHT22_Config_CLK();
	DHT22_Config_GPIO_OUTPUT();//output for pulse start
	DHT22_Config_EXTInterrupt_Enable();
	DHT22_Config_NVIC();
}


uint32_t dhtTimeStamp = 0;
uint16_t tempTemp;
uint16_t humidTemp;
void DHT22_Start_Read(DHT22_Data *tempAndHumid)
{
	currentBit =0;
	upTimeStart = 0;
	upTimeEnd = 0;
	downTimeStart = 0;
	downTimeEnd = 0;

	//tempAndHumid->Humid = 0.0;
	//tempAndHumid->Temp = 0.0;
	DHT22_Buffer16[0] = 0;
	DHT22_Buffer16[1] = 0;
	DHT22_Buffer16[2] = 0;

	dhtTimeStamp = Micros();
	DHT22_Config_GPIO_OUTPUT();
	GPIOB->BRR = DHT22_Pin; //Pull pin LOW
	while((Micros() - dhtTimeStamp) < 1100){}
	GPIOB->BSRR = DHT22_Pin; //Pull pin HIGH
	DHT22_Config_GPIO_INPUT(); //Ready for incoming data
	//DHT22_Config_EXTInterrupt_Enable();
	dhtTimeStamp = Millis();
	while((Millis() - dhtTimeStamp) < 1000){}
	DHT22_Times_To_Bits16(DHT22_Bit_Time, 45);
	if(DHT22_Buffer16[0] < 999)
	{
		humidTemp = DHT22_Buffer16[0];
		tempAndHumid->Humid = humidTemp/10.0;

	}

	if(DHT22_Buffer16[1] < 999)
	{
		tempTemp = DHT22_Buffer16[1];
		tempAndHumid->Temp = tempTemp/10.0;

	}



	//DHT_Value_Checksum();
	//DHT22_Buffer16[3] = DHT22_Buffer16[3]| DHT22_Buffer16[3]>>8;
	dhtTimeStamp = Millis();

	//Do something to check for data transmission completion

//Pull line low for at least 1ms

}


void DHT22_Config_CLK()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

}


void DHT22_Config_GPIO_INPUT()
{
	DHT22_Pin_GPIO_Config.GPIO_Pin = DHT22_Pin;
	DHT22_Pin_GPIO_Config.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	DHT22_Pin_GPIO_Config.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&DHT22_Pin_GPIO_Config);
}

void DHT22_Config_GPIO_OUTPUT()
{
	DHT22_Pin_GPIO_Config.GPIO_Pin = DHT22_Pin;
	DHT22_Pin_GPIO_Config.GPIO_Mode = GPIO_Mode_Out_PP;
	DHT22_Pin_GPIO_Config.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&DHT22_Pin_GPIO_Config);
}


void DHT22_Config_EXTInterrupt_Enable()
{
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource6);
	EXTI_InitTypeDef DHT22_IntConfig;
	DHT22_IntConfig.EXTI_Line = EXTI_Line6;
	DHT22_IntConfig.EXTI_Mode = EXTI_Mode_Interrupt;
	DHT22_IntConfig.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	DHT22_IntConfig.EXTI_LineCmd = ENABLE;

	EXTI_Init(&DHT22_IntConfig);
}

void DHT22_Config_EXTInterrupt_Disable()
{
	EXTI_InitTypeDef DHT22_IntConfig;
	DHT22_IntConfig.EXTI_Line = EXTI_Line6;
	DHT22_IntConfig.EXTI_Mode = EXTI_Mode_Interrupt;
	DHT22_IntConfig.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	DHT22_IntConfig.EXTI_LineCmd = DISABLE;

	EXTI_Init(&DHT22_IntConfig);
}

void DHT22_Config_NVIC()
{
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	NVIC_InitTypeDef DHT22_Interrupt;

	DHT22_Interrupt.NVIC_IRQChannel = EXTI9_5_IRQn;
	DHT22_Interrupt.NVIC_IRQChannelPreemptionPriority = 4;
	DHT22_Interrupt.NVIC_IRQChannelSubPriority = 0;
	DHT22_Interrupt.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&DHT22_Interrupt);

	NVIC_SetPriority(EXTI9_5_IRQn, NVIC_EncodePriority(4,15,0));
}

void DHT22_Times_To_Bits(uint8_t bitTimesArray[], uint8_t arraySize)
{
	uint8_t count;
	uint8_t toValidate;
	uint8_t bitCount = 7;
	uint8_t byteNumber = 0;
	for(count = 0; count < arraySize; count++)
	{
		toValidate = bitTimesArray[count];
		if(toValidate < 55)
		{
			DHT22_Buffer[byteNumber] = DHT22_Buffer[byteNumber] | 0<<bitCount;
		}
		else {
			DHT22_Buffer[byteNumber] = DHT22_Buffer[byteNumber] | 1<<bitCount;
		}
		if(bitCount != 0)
		{
			bitCount--;
		}
		else
		{
			bitCount = 7;
			byteNumber++;
		}
	}
}


void DHT22_Times_To_Bits16(uint8_t bitTimesArray[], uint8_t arraySize)
{
	uint8_t count;
	uint8_t toValidate;
	uint8_t bitCount = 15;
	uint8_t byteNumber = 0;
	for(count = 2; count < arraySize; count++)
	{
		toValidate = bitTimesArray[count];
		if(toValidate < 55)
		{
			DHT22_Buffer16[byteNumber] = DHT22_Buffer16[byteNumber] | 0<<bitCount;
		}
		else {
			DHT22_Buffer16[byteNumber] = DHT22_Buffer16[byteNumber] | 1<<bitCount;
		}
		if(bitCount != 0)
		{
			bitCount--;
		}
		else
		{
			bitCount = 15;
			byteNumber++;
		}
	}
}



void DHT_Value_Checksum()
{
	uint8_t count;
	uint8_t buffCount = 0;
	for(count =0; count < 3; count++)
	{
		DHT22_Buffer[buffCount++] = DHT22_Buffer16[count] & 0xff;
		DHT22_Buffer[buffCount++] = (DHT22_Buffer16[count] >> 8) & 0xff;
	}

}


void EXTI9_5_IRQHandler(void)
{
if(EXTI_GetITStatus(EXTI_Line6) != RESET)
  {
	if(GPIO_ReadInputDataBit(GPIOB,DHT22_Pin)) //If pin high
	{
		//currentBit++;
		upTimeStart = Micros();
		downTimeEnd = Micros();
		//if(downTimeStart != 0)
		//{
			//DHT22_Bit_Time[currentBit] = downTimeEnd - downTimeStart;
		//}
	}
	else
	{

		downTimeStart = Micros();
		upTimeEnd = Micros();
		if(upTimeStart != 0)
		{
			DHT22_Bit_Time[currentBit] = upTimeEnd - upTimeStart;
			currentBit++;
		}
	}


	//Need to count the length of pulses for DHT22 Data

    /* Clear the  EXTI line 8 pending bit */
    EXTI_ClearITPendingBit(EXTI_Line6);
  }
}
