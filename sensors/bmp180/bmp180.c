

#include "bmp180.h"


void BMP180_Init(uint32_t i2c_speed)
{
	BMP180_CLK_Config();
	BMP180_GPIO_Config();
	BMP180_I2C_Config(i2c_speed);
}

void BMP180_GPIO_Config()
{
	GPIO_InitTypeDef I2C1_GPIO_Config;

	I2C1_GPIO_Config.GPIO_Mode = GPIO_Mode_AF_OD;
	I2C1_GPIO_Config.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	I2C1_GPIO_Config.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOB, &I2C1_GPIO_Config);

}

void BMP180_CLK_Config()
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}

void BMP180_I2C_Config(uint32_t i2c_speed)
{
	I2C_InitTypeDef  I2C_InitStructure;

	    /* I2C configuration */
	    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;

	    I2C_InitStructure.I2C_OwnAddress1 = 0x00;
	    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	    I2C_InitStructure.I2C_ClockSpeed = i2c_speed;

	    /* I2C Peripheral Enable */
	    I2C_Cmd(I2C1, ENABLE);
	    /* Apply I2C configuration after enabling it */
	    I2C_Init(I2C1, &I2C_InitStructure);
}
