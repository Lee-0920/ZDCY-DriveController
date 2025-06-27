/**
 * @file LaiRS485SlaveAddr.c
 * @brief 硬件地址的操作
 * @version 1.0.0
 * @author xingfan
 * @date 2016-05-25
 */
#include "LaiRS485SlaveAddr.h"
#include "stm32f4xx.h"

#define LAIRS485SLAVEADDR_ADR0_RCC       RCC_AHB1Periph_GPIOA
#define LAIRS485SLAVEADDR_ADR0_PORT      GPIOA
#define LAIRS485SlAVEADDR_ADR0_PIN       GPIO_Pin_2

//#define LAIRS485SLAVEADDR_ADR1_RCC       RCC_AHB1Periph_GPIOC
//#define LAIRS485SLAVEADDR_ADR1_PORT      GPIOC
//#define LAIRS485SlAVEADDR_ADR1_PIN       GPIO_Pin_3

//#define LAIRS485SLAVEADDR_ADR2_RCC       RCC_AHB1Periph_GPIOE
//#define LAIRS485SLAVEADDR_ADR2_PORT      GPIOE
//#define LAIRS485SlAVEADDR_ADR2_PIN       GPIO_Pin_5
//
//#define LAIRS485SLAVEADDR_ADR3_RCC       RCC_AHB1Periph_GPIOE
//#define LAIRS485SLAVEADDR_ADR3_PORT      GPIOE
//#define LAIRS485SlAVEADDR_ADR3_PIN       GPIO_Pin_6

#define ADR0 GPIO_ReadInputDataBit(LAIRS485SLAVEADDR_ADR0_PORT, LAIRS485SlAVEADDR_ADR0_PIN)
//#define ADR1 GPIO_ReadInputDataBit(LAIRS485SLAVEADDR_ADR1_PORT, LAIRS485SlAVEADDR_ADR1_PIN)
//#define ADR2 GPIO_ReadInputDataBit(LAIRS485SLAVEADDR_ADR2_PORT, LAIRS485SlAVEADDR_ADR2_PIN)
//#define ADR3 GPIO_ReadInputDataBit(LAIRS485SLAVEADDR_ADR3_PORT, LAIRS485SlAVEADDR_ADR3_PIN)

/**
 * @brief 对LaiRS485的硬件地址进行初始化
 */
void LaiRS485SlaveAddr_GPIOConfig(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(
            LAIRS485SLAVEADDR_ADR0_RCC
            /* |LAIRS485SLAVEADDR_ADR1_RCC
                 | LAIRS485SLAVEADDR_ADR2_RCC
                 | LAIRS485SLAVEADDR_ADR3_RCC*/,
            ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

    GPIO_InitStructure.GPIO_Pin = LAIRS485SlAVEADDR_ADR0_PIN;
    GPIO_Init(LAIRS485SLAVEADDR_ADR0_PORT, &GPIO_InitStructure);

//    GPIO_InitStructure.GPIO_Pin = LAIRS485SlAVEADDR_ADR1_PIN;
//    GPIO_Init(LAIRS485SLAVEADDR_ADR1_PORT, &GPIO_InitStructure);

//    GPIO_InitStructure.GPIO_Pin = LAIRS485SlAVEADDR_ADR2_PIN;
//    GPIO_Init(LAIRS485SLAVEADDR_ADR2_PORT, &GPIO_InitStructure);
//
//    GPIO_InitStructure.GPIO_Pin = LAIRS485SlAVEADDR_ADR3_PIN;
//    GPIO_Init(LAIRS485SLAVEADDR_ADR3_PORT, &GPIO_InitStructure);
}
/**
 * @brief 对取LaiRS485硬件地址设置的地址值
 * @return 地址值
 */

Uint8 LaiRS485SlaveAddr_GetAddr(void)
{
    Uint8 Addr = 1;

    if (ADR0)
    {
        Addr += 1;
    }
//    if (ADR1)
//    {
//        Addr += 2;
//    }
//    if (ADR2)
//    {
//        Addr += 4;
//    }
//    if (ADR3)
//    {
//        Addr += 8;
//    }
    return Addr;
}

