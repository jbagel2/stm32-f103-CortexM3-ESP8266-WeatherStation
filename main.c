#include "globalDefines.h"
#include "custom_libs/mytime.h"
#include "USART3_Config.h"
#include "stm32f10x_flash.h"
#include "stm32f10x_rcc.h"
#include "esp8266.h"
#include "dht22.h"
#include "sensors/bmp180/bmp180.h"

uint32_t timeStamp = 0;
uint32_t i = 0;
#define TX_DELAY 1000
uint32_t lastDMABuffPoll = 0;
uint32_t lastESPResetPoll = 0;
uint32_t lastDHT22update = 0;
#define ESP_RESET_CHECK_INTERVAL 20000 //20 seconds

#define DHT_UPDATE_INTERVAL 20000 //10 seconds



volatile char USART3_RxBuffer[RxBuffSize];
extern char customRESTResponse[800];

IPD_Data currentIPD;
ESP_Status currentESPStatus;
DHT22_Data Current_DHT22_Reading;
BMP180_Data Current_BMP180_Reading;


void Configure_HSI_Clock()
{
	FLASH_SetLatency(FLASH_ACR_LATENCY_2);

	RCC_HSICmd(ENABLE);
	RCC_HSEConfig(DISABLE);
	RCC_PLLConfig(RCC_PLLSource_HSI_Div2,RCC_CFGR_PLLMULL16);
	RCC_PLLCmd(ENABLE);
	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {}
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

	RCC_HCLKConfig(RCC_SYSCLK_Div1);
	RCC_PCLK1Config(RCC_HCLK_Div1);
	RCC_PCLK2Config(RCC_HCLK_Div1);
	RCC_ADCCLKConfig(RCC_PCLK2_Div2);
}

void SetSystemClockOut()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2ENR_AFIOEN, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	RCC_MCOConfig(RCC_MCO_PLLCLK_Div2);
}


int main(void)
{
	SystemInit();

	Configure_HSI_Clock();

	Init_Time(MILLISEC,64);

	Init_USART3_DMA(2000000,USART3_RxBuffer, RxBuffSize);

	//SetSystemClockOut();
	BMP180_Init(400000);


	Wifi_Init();

	Wifi_OFF();

	for (i=0;i<1305000;i++);

	Wifi_ON();

	uint16_t WaitForReady_TimeStmp = Millis();
	while(!Wifi_CheckDMABuff_ForReady() && (Millis() - WaitForReady_TimeStmp) < ESP_ResponseTimeout_ms){}

	Wifi_SendCommand(WIFI_JOIN_NONYA);

	StartServer(1,80);

	Wifi_SendCommand(WIFI_GET_CURRENT_IP);
	uint8_t readyFound = 0;
	DHT22_Init();

    while(1)
    {
    	Wifi_CheckDMABuff_ForCIFSRData();
    	if((Millis() - lastDMABuffPoll) >= DMA_Rx_Buff_Poll_Int_ms)
    	{
    		lastDMABuffPoll = Millis();
    		currentIPD = Wifi_CheckDMABuff_ForIPDData(&Current_DHT22_Reading,&Current_BMP180_Reading);
    		if(currentIPD.Valid == 1)
				{
    				SendRESTResponse(currentIPD.ConnectionNum, RESTResponse_Headers_Test_OK, customRESTResponse);
				}
    	}
//
    	if((Millis() - lastDHT22update) >= DHT_UPDATE_INTERVAL)
    	    	{
    	    		lastDHT22update = Millis();
    	    		DHT22_Start_Read(&Current_DHT22_Reading);
    	    	}

    	    	if((Millis() - lastESPResetPoll) >= ESP_RESET_CHECK_INTERVAL)
    	    	{
    	    		lastESPResetPoll = Millis();
    	    		readyFound = Wifi_CheckDMABuff_ForReady();

    	    		if(readyFound)
    	    		{
    	    			StartServer(1,80);
    	    			Wifi_SendCommand(WIFI_GET_CURRENT_IP);
    	    		}


    	    		//Check for ip and mac address data (and update the current settings if found.)

    	    	}

    }
}
