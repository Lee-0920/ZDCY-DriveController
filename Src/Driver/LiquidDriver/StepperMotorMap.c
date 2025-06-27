/*
 * StepperMotorMap.c
 *
 *  Created on: 2016年5月30日
 *      Author: Administrator
 */

#include "stm32f4xx.h"
#include "StepperMotorDriver.h"
#include "StepperMotorMap.h"

void StepperMotorMap_PeristalticPumpInit(PeristalticPump *peristalticPump)
{
    peristalticPump[0].stepperMotor.driver.pinClock = GPIO_Pin_7;
    peristalticPump[0].stepperMotor.driver.portClock = GPIOD;
    peristalticPump[0].stepperMotor.driver.rccClock = RCC_AHB1Periph_GPIOD;

    peristalticPump[0].stepperMotor.driver.pinDir = GPIO_Pin_6;
    peristalticPump[0].stepperMotor.driver.portDir = GPIOD;
    peristalticPump[0].stepperMotor.driver.rccDir = RCC_AHB1Periph_GPIOD;

    peristalticPump[0].stepperMotor.driver.pinEnable = GPIO_Pin_5;
    peristalticPump[0].stepperMotor.driver.portEnable = GPIOD;
    peristalticPump[0].stepperMotor.driver.rccEnable = RCC_AHB1Periph_GPIOD;

    peristalticPump[0].stepperMotor.driver.pinReset = GPIO_Pin_3;
    peristalticPump[0].stepperMotor.driver.portReset = GPIOB;
    peristalticPump[0].stepperMotor.driver.rccReset = RCC_AHB1Periph_GPIOB;

//    peristalticPump[0].stepperMotor.driver.pinDiag = GPIO_Pin_6;
//    peristalticPump[0].stepperMotor.driver.portDiag = GPIOE;
//    peristalticPump[0].stepperMotor.driver.rccDiag = RCC_AHB1Periph_GPIOE;

    StepperMotorDriver_Init(&peristalticPump[0].stepperMotor.driver);
    StepperMotorDriver_PullLow(&peristalticPump[0].stepperMotor.driver);
    StepperMotorDriver_Disable(&peristalticPump[0].stepperMotor.driver);
    StepperMotorDriver_SetForwardLevel(&peristalticPump[0].stepperMotor.driver, Bit_RESET);


    peristalticPump[1].stepperMotor.driver.pinClock = GPIO_Pin_7;
    peristalticPump[1].stepperMotor.driver.portClock = GPIOB;
    peristalticPump[1].stepperMotor.driver.rccClock = RCC_AHB1Periph_GPIOB;

    peristalticPump[1].stepperMotor.driver.pinDir = GPIO_Pin_6;
    peristalticPump[1].stepperMotor.driver.portDir = GPIOB;
    peristalticPump[1].stepperMotor.driver.rccDir = RCC_AHB1Periph_GPIOB;

    peristalticPump[1].stepperMotor.driver.pinEnable = GPIO_Pin_5;
    peristalticPump[1].stepperMotor.driver.portEnable = GPIOB;
    peristalticPump[1].stepperMotor.driver.rccEnable = RCC_AHB1Periph_GPIOB;

    peristalticPump[1].stepperMotor.driver.pinReset = GPIO_Pin_8;
    peristalticPump[1].stepperMotor.driver.portReset = GPIOB;
    peristalticPump[1].stepperMotor.driver.rccReset = RCC_AHB1Periph_GPIOB;

//    peristalticPump[1].stepperMotor.driver.pinDiag = GPIO_Pin_5;
//    peristalticPump[1].stepperMotor.driver.portDiag = GPIOE;
//    peristalticPump[1].stepperMotor.driver.rccDiag = RCC_AHB1Periph_GPIOE;

    StepperMotorDriver_Init(&peristalticPump[1].stepperMotor.driver);
    StepperMotorDriver_PullLow(&peristalticPump[1].stepperMotor.driver);
    StepperMotorDriver_Disable(&peristalticPump[1].stepperMotor.driver);
    StepperMotorDriver_SetForwardLevel(&peristalticPump[1].stepperMotor.driver, Bit_RESET);
}

//void StepperMotorMap_DisplacementMotorInit(DisplacementMotor *displacementMotor)
//{
//	displacementMotor[0].stepperMotor.driver.pinClock = GPIO_Pin_14;
//	displacementMotor[0].stepperMotor.driver.portClock = GPIOD;
//	displacementMotor[0].stepperMotor.driver.rccClock = RCC_AHB1Periph_GPIOD;
//
//	displacementMotor[0].stepperMotor.driver.pinDir = GPIO_Pin_13;
//	displacementMotor[0].stepperMotor.driver.portDir = GPIOD;
//	displacementMotor[0].stepperMotor.driver.rccDir = RCC_AHB1Periph_GPIOD;
//
//	displacementMotor[0].stepperMotor.driver.pinDiag = GPIO_Pin_15;
//	displacementMotor[0].stepperMotor.driver.portDiag = GPIOD;
//	displacementMotor[0].stepperMotor.driver.rccDiag = RCC_AHB1Periph_GPIOD;

    ///displacementMotor[0].stepperMotor.driver.pinEnable = GPIO_Pin_0;
    ///displacementMotor[0].stepperMotor.driver.portEnable = GPIOC;
    ///displacementMotor[0].stepperMotor.driver.rccEnable = RCC_AHB1Periph_GPIOC;

//    StepperMotorDriver_Init(&displacementMotor[0].stepperMotor.driver);
//    StepperMotorDriver_PullLow(&displacementMotor[0].stepperMotor.driver);
    //StepperMotorDriver_Disable(&displacementMotor[0].stepperMotor.driver);
//    StepperMotorDriver_Enable(&displacementMotor[0].stepperMotor.driver);
//    StepperMotorDriver_SetForwardLevel(&displacementMotor[0].stepperMotor.driver, Bit_SET);

//}
