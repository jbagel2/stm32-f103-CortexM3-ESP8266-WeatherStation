#include "globalDefines.h"
#include "custom_libs/mytime.h"
#include "USART3_Config.h"
#include "stm32f10x_flash.h"
#include "stm32f10x_rcc.h"


volatile char USART3_RxBuffer[RxBuffSize];


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


int main(void)
{
	SystemInit();

	Configure_HSI_Clock();

	Init_Time(MILLISEC,64);

	Init_USART3_DMA(2000000,USART3_RxBuffer, RxBuffSize);

    while(1)
    {
    }
}
