/*
 * AnalogDriver.c
 *
 *  Created on: 2020年5月22日
 *      Author: Administrator
 */
#include "AnalogDriver.h"

void AnalogSignal_ADC1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_InitTypeDef ADC_InitStructure;

	//1 开启 ADC 和 GPIO 相关时钟和初始化 GPIO
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//使能 GPIOC时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//使能 GPIOC时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); //使能 ADC1 时钟

	//先初始化 ADC1 通道 15和通道11的 IO 口
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;//PC5 通道 15
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;//模拟输入
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;//不带上下拉
	GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;//PA7通道7
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;//模拟输入
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;//不带上下拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化

	//RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1,ENABLE);  //ADC1 复位
	//RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1,DISABLE);  //复位结束

	//2 初始化通用配置
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;//独立模式
	ADC_CommonInitStructure.ADC_TwoSamplingDelay =ADC_TwoSamplingDelay_5Cycles;//两个采样阶段之间的延迟 5 个时钟
	ADC_CommonInitStructure.ADC_DMAAccessMode =ADC_DMAAccessMode_Disabled; //DMA 失能
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;//预分频 4 分频。//ADCCLK=PCLK2/4=84/4=21Mhz,ADC 时钟最好不要超过 36Mhz
	ADC_CommonInit(&ADC_CommonInitStructure);//初始化

	//3 初始化 ADC1 相关参数
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;//12 位模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;//非扫描模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;//关闭连续转换
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//禁止触发检测，使用软件触发
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//右对齐
	ADC_InitStructure.ADC_NbrOfConversion = 1;//1 个转换在规则序列中
	ADC_Init(ADC1, &ADC_InitStructure);//ADC 初始化

	//4 开启 ADC 转换
	ADC_Cmd(ADC1, ENABLE);//开启 AD 转换器
}

Uint16 Get_ADC1(Uint8 ch)
{
	//设置指定 ADC 的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_480Cycles );
	ADC_SoftwareStartConv(ADC1); //使能指定的 ADC1 的软件转换启动功能
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束
	return ADC_GetConversionValue(ADC1);  //返回最近一次 ADC1 规则组的转换结果
}


