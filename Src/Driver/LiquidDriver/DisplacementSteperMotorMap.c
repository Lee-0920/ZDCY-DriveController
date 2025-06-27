/*
#include <LiquidDriver/ConstantDCMotorDriver.h>
 * DisplacementSteperMotorMap.c
 *
 *  Created on: 2023年03月02日
 *      Author: hgq
 */

#include "DisplacementSteperMotorMap.h"


void StepperMotorMap_DisplacementMotorInit(DisplacementSteperMotor *displacementMotor)
{
    displacementMotor[0].stepperMotor.driver.pinClock = GPIO_Pin_14;
    displacementMotor[0].stepperMotor.driver.portClock = GPIOD;
    displacementMotor[0].stepperMotor.driver.rccClock = RCC_AHB1Periph_GPIOD;

    displacementMotor[0].stepperMotor.driver.pinDir = GPIO_Pin_13;
    displacementMotor[0].stepperMotor.driver.portDir = GPIOD;
    displacementMotor[0].stepperMotor.driver.rccDir = RCC_AHB1Periph_GPIOD;

    displacementMotor[0].stepperMotor.driver.pinDiag = GPIO_Pin_15;
    displacementMotor[0].stepperMotor.driver.portDiag = GPIOD;
    displacementMotor[0].stepperMotor.driver.rccDiag = RCC_AHB1Periph_GPIOD;

    displacementMotor[0].stepperMotor.driver.pinEnable = 0;
    displacementMotor[0].stepperMotor.driver.portEnable = NULL;
    displacementMotor[0].stepperMotor.driver.rccEnable = 0;
    
    displacementMotor[0].stepperMotor.driver.pinReset = 0;
	displacementMotor[0].stepperMotor.driver.portReset = NULL;
	displacementMotor[0].stepperMotor.driver.rccReset = 0;

    DisplaceStepperMotorDriver_Init(&displacementMotor[0].stepperMotor.driver);
    StepperMotorDriver_PullLow(&displacementMotor[0].stepperMotor.driver);
    StepperMotorDriver_Disable(&displacementMotor[0].stepperMotor.driver);
    //StepperMotorDriver_Enable(&displacementMotor[0].stepperMotor.driver);
    StepperMotorDriver_SetForwardLevel(&displacementMotor[0].stepperMotor.driver, Bit_SET);

}
