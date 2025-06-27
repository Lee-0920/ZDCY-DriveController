/*
 * ConstantDCMotorDriver.c
 *
 *  Created on: 2022年06月08日
 *      Author: hyz
 */

#include "ConstantDCMotorDriver.h"


void DisplacementMotorDriver_Init(ConstantDCMotor* Motor)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(
    		Motor->rccL | Motor->rccR,
            ENABLE);

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

    GPIO_InitStructure.GPIO_Pin = Motor->pinL;
    GPIO_Init(Motor->portL, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = Motor->pinR;
    GPIO_Init(Motor->portR, &GPIO_InitStructure);
}

void DisplacementMotorDriver_Lift(ConstantDCMotor* Motor)
{
	GPIO_SetBits(Motor->portR,Motor->pinR);
	GPIO_ResetBits(Motor->portL,Motor->pinL);

}

void DisplacementMotorDriver_Right(ConstantDCMotor* Motor)
{
	GPIO_SetBits(Motor->portL,Motor->pinL);
	GPIO_ResetBits(Motor->portR,Motor->pinR);
}

void DisplacementMotorDriver_Stop(ConstantDCMotor* Motor)
{
    /*
	if (GPIO_ReadOutputDataBit(Motor->portL,Motor->pinL))
	{
		GPIO_ResetBits(Motor->portL,Motor->pinL);
		GPIO_SetBits(Motor->portR,Motor->pinR);
	}
	else if (GPIO_ReadOutputDataBit(Motor->portR,Motor->pinR))
	{
		GPIO_ResetBits(Motor->portR,Motor->pinR);
		GPIO_SetBits(Motor->portL,Motor->pinL);
	}
	GPIO_ResetBits(Motor->portL,Motor->pinL);
	GPIO_ResetBits(Motor->portR,Motor->pinR);
    */
    GPIO_SetBits(Motor->portL,Motor->pinL);
	GPIO_SetBits(Motor->portR,Motor->pinR);
}

void DisplacementMotorDriver_Reset(ConstantDCMotor* Motor)
{ 
	if (GPIO_ReadOutputDataBit(Motor->portL,Motor->pinL))
	{
		GPIO_ResetBits(Motor->portL,Motor->pinL);
		GPIO_SetBits(Motor->portR,Motor->pinR);
	}
	else if (GPIO_ReadOutputDataBit(Motor->portR,Motor->pinR))
	{
		GPIO_ResetBits(Motor->portR,Motor->pinR);
		GPIO_SetBits(Motor->portL,Motor->pinL);
	}
	GPIO_ResetBits(Motor->portL,Motor->pinL);
	GPIO_ResetBits(Motor->portR,Motor->pinR);
}

