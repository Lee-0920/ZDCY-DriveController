/*
 * TMC2160SPIDriver.c
 *
 *  Created on: 2020年6月5日
 *      Author: Administrator
 */
#include "TMC2160SPIDriver.h"
#include "SystemConfig.h"
#include "Driver/System.h"
#include "Tracer/Trace.h"

static void SPI_SendByte(char data);

void TMC2160_SPIInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	//PA4:CS1
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//PA5:SCLK
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//PA6:MISO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//PA7:MOSI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//配置引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource5,GPIO_AF_SPI1); //PA5 复用为 SPI1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource6,GPIO_AF_SPI1); //PA6 复用为 SPI1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource7,GPIO_AF_SPI1); //PA7 复用为 SPI1
	//这里只针对 SPI 口初始化
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,ENABLE);//复位 SPI1
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,DISABLE);//停止复位 SPI1

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;   //STM32的SPI1用APB2的72MHz，TMC5130手册里P23指出TMC5130的SPI时钟不能超过4MHz和8MHz，所以注意此处的分频值
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);
	SPI_Cmd(SPI1, ENABLE);
}

void SPI_SendByte(char data)
{
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI1, data);

	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	SPI_I2S_ReceiveData(SPI1);
}

char SPI_ReceiveByte(void)
{
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI1, 0x0);

	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	return SPI_I2S_ReceiveData(SPI1);
}

//TMC5130 takes 40 bit data: 8 address and 32 data
void TMC2160_1_sendData(unsigned long address, unsigned long datagram)
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);  	//SPI_CS1片选拉低
	GPIO_WriteBit(GPIOC, GPIO_Pin_4, Bit_SET);  	//SPI_CS2片选拉高

	SPI_SendByte(address);
	SPI_SendByte((datagram >> 24) & 0xff);
	SPI_SendByte((datagram >> 16) & 0xff);
	SPI_SendByte((datagram >> 8) & 0xff);
	SPI_SendByte(datagram & 0xff);

	GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);  	//SPI_CS1片选拉高
}

void TMC2160_2_sendData(unsigned long address, unsigned long datagram)
{
	GPIO_WriteBit(GPIOC, GPIO_Pin_4, Bit_RESET);  	//SPI_CS2片选拉低
	GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);  	//SPI_CS1片选拉高

	SPI_SendByte(address);
	SPI_SendByte((datagram >> 24) & 0xff);
	SPI_SendByte((datagram >> 16) & 0xff);
	SPI_SendByte((datagram >> 8) & 0xff);
	SPI_SendByte(datagram & 0xff);

	GPIO_WriteBit(GPIOC, GPIO_Pin_4, Bit_SET);  	//SPI_CS片选拉高
}

unsigned long TMC2160_1_ReadData(unsigned long address)
{
	char data[4] = {0, 0, 0, 0};
	unsigned long datagram = 0;

	GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);  	//SPI_CS1片选拉低
	GPIO_WriteBit(GPIOC, GPIO_Pin_4, Bit_SET);  	//SPI_CS2片选拉高

	SPI_SendByte(address);
	data[0] = SPI_ReceiveByte();//SPI_ReceiveByte((datagram >> 24) & 0xff);
	data[1] = SPI_ReceiveByte();//SPI_ReceiveByte((datagram >> 16) & 0xff);
	data[2] = SPI_ReceiveByte();//SPI_ReceiveByte((datagram >> 8) & 0xff);
	data[3] = SPI_ReceiveByte();//SPI_ReceiveByte(datagram & 0xff);

	datagram = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];

	GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);  	//SPI_CS1片选拉高

	return datagram;
}

unsigned long TMC2160_2_ReadData(unsigned long address)
{
	char data[4] = {0, 0, 0, 0};
	unsigned long datagram = 0;

	GPIO_WriteBit(GPIOC, GPIO_Pin_4, Bit_RESET);  	//SPI_CS2片选拉低
	GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);  	//SPI_CS1片选拉高

	SPI_SendByte(address);
	data[0] = SPI_ReceiveByte();//SPI_ReceiveByte((datagram >> 24) & 0xff);
	data[1] = SPI_ReceiveByte();//SPI_ReceiveByte((datagram >> 16) & 0xff);
	data[2] = SPI_ReceiveByte();//SPI_ReceiveByte((datagram >> 8) & 0xff);
	data[3] = SPI_ReceiveByte();//SPI_ReceiveByte(datagram & 0xff);

	datagram = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];

	GPIO_WriteBit(GPIOC, GPIO_Pin_4, Bit_SET);  	//SPI_CS2片选拉高

	return datagram;
}
