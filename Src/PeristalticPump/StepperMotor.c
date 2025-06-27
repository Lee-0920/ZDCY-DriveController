/*
 * StepperMotor.c
 *
 *  Created on: 2016年5月30日
 *      Author: Administrator
 */
#include "LiquidDriver/StepperMotorTimer.h"
#include "tracer/trace.h"
#include "System.h"
#include "StepperMotor.h"
#include "TMCConfig.h"

StepperMotor* g_stepperMotor[STEPPERMOTOR_TOTAL_NUMBER] = {NULL};

static void StepperMotor_MoveHandler(StepperMotor* stepperMotor);

void StepperMotor_Init(StepperMotor* stepperMotor, StepperMotorParam param)
{
    stepperMotor->status = StepperMotor_IDLE;
    stepperMotor->flashParam = param;

    stepperMotor->remainStep = 0;
    stepperMotor->alreadyStep = 0;
    stepperMotor->speedDownTotalStep = 0;

    stepperMotor->moveStatus = MOVE_STATUS_IDLE;
    stepperMotor->isStatusSwitchStart = FALSE;

    stepperMotor->result = RESULT_FINISHED;
    stepperMotor->isParamChange = FALSE;
    stepperMotor->isExcessiveSpeed = FALSE;
    stepperMotor->isLowSpeed = FALSE;
    stepperMotor->isPosLock = FALSE;
    //加速逻辑
    stepperMotor->upSpeed = 0;
    stepperMotor->alreadyUpTimer = 0;
    stepperMotor->upParam = stepperMotor->flashParam;

    stepperMotor->upInitSpeed = 0;
    stepperMotor->alreadyUpStep = 0;

    //减速逻辑
    stepperMotor->dowmSpeed = 0;
    stepperMotor->alreadyDownTimer = 0;
    stepperMotor->downParam = stepperMotor->flashParam;

    stepperMotor->downInitSpeed = 0;
    stepperMotor->alreadyDownStep = 0;

    stepperMotor->uniformSpeed = 0;
    stepperMotor->uniformParam = stepperMotor->flashParam;
    stepperMotor->otherMoveHandler = NULL;
    stepperMotor->finishHandler = NULL;
}

void StepperMotor_SetNumber(StepperMotor* stepperMotor, Uint8 num)
{
    stepperMotor->number = num;
    g_stepperMotor[num] = stepperMotor;  //保存指针
}

Uint8 StepperMotor_GetNumber(StepperMotor* stepperMotor)
{
    return stepperMotor->number;
}

void StepperMotor_SetSubdivision(StepperMotor* stepperMotor, Uint8 subdivision)
{
    stepperMotor->subdivision = subdivision;
}

Uint8 StepperMotor_GetSubdivision(StepperMotor* stepperMotor)
{
    return (stepperMotor->subdivision);
}

void StepperMotor_SetPosLockStatus(StepperMotor* stepperMotor, Bool isPosLock)
{
    stepperMotor->isPosLock = isPosLock;
}

Bool StepperMotor_GetPosLockStatus(StepperMotor* stepperMotor)
{
    return (stepperMotor->isPosLock);
}

Uint32 StepperMotor_GetAlreadyStep(StepperMotor* stepperMotor)
{
    return (stepperMotor->alreadyStep / stepperMotor->subdivision);
}

StepperMotorParam StepperMotor_GetDefaultMotionParam(StepperMotor* stepperMotor)
{
    return stepperMotor->flashParam;
}

Bool StepperMotor_SetDefaultMotionParam(StepperMotor* stepperMotor, StepperMotorParam param)
{
    if (StepperMotor_IDLE == stepperMotor->status)
    {
        TRACE_INFO("\n set default acc : ");
        System_PrintfFloat(TRACE_LEVEL_INFO, param.acceleration, 4);
        TRACE_INFO(" step/(s^2) ,maxSpeed :");
        System_PrintfFloat(TRACE_LEVEL_INFO, param.maxSpeed, 4);
        TRACE_INFO(" step/s");
        stepperMotor->flashParam = param;
        return TRUE;
    }
    else
    {
        TRACE_ERROR("\n The stepperMotor is running, prohibiting the change of the flash to save the movement parameters.");
        return FALSE;
    }
}

StepperMotorParam StepperMotor_GetCurrentMotionParam(StepperMotor* stepperMotor)
{
    return stepperMotor->setParam;
}

Bool StepperMotor_SetCurrentMotionParam(StepperMotor* stepperMotor, StepperMotorParam param)
{
    TRACE_DEBUG("\n set current acc : ");
    System_PrintfFloat(TRACE_LEVEL_DEBUG, param.acceleration, 4);
    TRACE_DEBUG(" step/(s^2) ,maxSpeed :");
    System_PrintfFloat(TRACE_LEVEL_DEBUG, param.maxSpeed, 4);
    TRACE_DEBUG(" step/s \n");
    stepperMotor->setParam = param;
    stepperMotor->isParamChange = TRUE;
    stepperMotor->setParam = param;
    return TRUE;
}

StepperMotorStatus StepperMotor_GetStatus(StepperMotor* stepperMotor)
{
    return (stepperMotor->status);
}

static void StepperMotor_MoveStatusSwitch(StepperMotor* stepperMotor, StepperMotorMoveStatus moveStatus)
{
    // 参数检查
    if (NULL == stepperMotor)
    {
        return;
    }
    // 状态切换
    stepperMotor->moveStatus = moveStatus;
    stepperMotor->isStatusSwitchStart = TRUE;
}

static void StepperMotor_MoveStatusSwitchEnd(StepperMotor* stepperMotor)
{
    // 参数检查
    if (NULL == stepperMotor)
    {
        return;
    }
    // 状态切换
    stepperMotor->isStatusSwitchStart = FALSE;
}

Bool StepperMotor_AbsolutelyMove(Uint8 index, Direction dir,  Uint32 step,  Uint32 speed,  Uint32 acc)
{
    if(index >= STEPPERMOTOR_TOTAL_NUMBER)
    {
        TRACE_ERROR("\n Invalid StepperMotor Index %d", index);
        return FALSE;
    }

    StepperMotor* stepperMotor = g_stepperMotor[index];

    if(stepperMotor == NULL)
    {
        TRACE_ERROR("\n Invalid StepperMotor Pointer");
        return FALSE;
    }

    StepperMotorParam setparam;
    setparam.acceleration = speed;
    setparam.maxSpeed = acc;

    StepperMotor_SetCurrentMotionParam(stepperMotor, setparam);
    if(TRUE == StepperMotor_Start(stepperMotor, dir, step, FALSE, NULL))
    {
        return TRUE;
    }
    else
    {
        TRACE_ERROR("StepperMotor Absolutely Move Start Fail!");
        return FALSE;
    }

}

Bool StepperMotor_AbsoultelyStop(Uint8 index)
{
    if(index >= STEPPERMOTOR_TOTAL_NUMBER)
    {
        TRACE_ERROR("\n Invalid StepperMotor Index %d", index);
        return FALSE;
    }

    StepperMotor* stepperMotor = g_stepperMotor[index];

    if(stepperMotor == NULL)
    {
        TRACE_ERROR("\n Invalid StepperMotor Pointer");
        return FALSE;
    }

    if(TRUE == StepperMotor_ImmediatelyStop(stepperMotor))
    {
        return TRUE;
    }
    else
    {
        TRACE_ERROR("StepperMotor Absolutely Stop Fail!");
        return FALSE;
    }
}

Uint8 StepperMotor_GetTotalNumber(void)
{
    return STEPPERMOTOR_TOTAL_NUMBER;
}

Bool StepperMotor_Start(StepperMotor* stepperMotor, Direction dir, Uint32 step, Bool isUseDefaultParam, StepperMotor_OtherMoveHandler otherMoveHandler)
{																//定时器中断回调函数
    if ( FALSE == StepperMotorTimer_RegisterHandle((TimerHandle) StepperMotor_MoveHandler, stepperMotor))
    {
        return FALSE;
    }
    stepperMotor->otherMoveHandler = otherMoveHandler;//定位电机每走一步附加处理的函数，检测是否遮住传感器
    StepperMotor_MoveStatusSwitch(stepperMotor, MOVE_STATUS_START);

    stepperMotor->remainStep = step * stepperMotor->subdivision;//泵的剩余步数  已细分

    stepperMotor->status = StepperMotor_BUSY;

    stepperMotor->alreadyStep = 0;
    stepperMotor->speedDownTotalStep = 0;

    stepperMotor->isParamChange = FALSE;
    stepperMotor->isExcessiveSpeed = FALSE;
    stepperMotor->isLowSpeed = FALSE;
    //加速逻辑
    stepperMotor->upSpeed = 0;
    stepperMotor->alreadyUpTimer = 0;

    stepperMotor->upInitSpeed = 0;
    stepperMotor->alreadyUpStep = 0;

    //减速逻辑
    stepperMotor->dowmSpeed = 0;
    stepperMotor->alreadyDownTimer = 0;

    stepperMotor->downInitSpeed = 0;
    stepperMotor->alreadyDownStep = 0;
    stepperMotor->uniformSpeed = 0;

    stepperMotor->result = RESULT_FINISHED;

    if(isUseDefaultParam)
    {
        stepperMotor->upParam = stepperMotor->flashParam;
        stepperMotor->downParam = stepperMotor->flashParam;
        stepperMotor->uniformParam = stepperMotor->flashParam;
    }
    else
    {
        stepperMotor->upParam = stepperMotor->setParam;
        stepperMotor->downParam = stepperMotor->setParam;
        stepperMotor->uniformParam = stepperMotor->setParam;
        stepperMotor->isParamChange = FALSE;
    }
    StepperMotorDriver_SetDirection(&stepperMotor->driver, dir);//设置电机的方向
    StepperMotorTimer_SpeedSetting(stepperMotor, STEPPERMOTOR_INIT_SPEED * stepperMotor->subdivision);//设置对应定时器的中断频率

    StepperMotorDriver_Enable(&stepperMotor->driver);
    StepperMotorTimer_Start(stepperMotor);//启动对应的定时器

    TRACE_INFO("\n ###########StepperMotor %d", stepperMotor->number);
    TRACE_INFO("\n ###########StepperMotor_Start");
    TRACE_INFO("\n ***********step %d ,subdivision remainStep %d, dir %d", step, stepperMotor->remainStep, dir);
    TRACE_INFO(" maxSpeed:");
    System_PrintfFloat(TRACE_LEVEL_INFO, stepperMotor->upParam.maxSpeed, 4);
    TRACE_INFO(" step/s,acc:");
    System_PrintfFloat(TRACE_LEVEL_INFO, stepperMotor->upParam.acceleration, 4);
    TRACE_INFO(" step/(s^2)");
    return TRUE;
}
/**
 * @brief 计算减速到初始速度需要的步数，保存到stepperMotor.speedDownTotalStep
 * @note 根据t = (V - V0) / a 和 s = V * t - 1/2 * a * t^2 公式算出s即步数。
 *  V是当前速度,步/秒。 V0是本系统泵启动的初始速度(运行后不能改变)。a是设置的加速度，步/平方秒。
 *  速度和加速度都是没有细分的，剩余步数是细分后的，算出s后需要乘以细分系数。
 *  减速步数是状态切换的判断条件，切换状态前必须为最新加速度和当前速度计算出来的剩余步数
 * @param stepperMotor 泵实体
 */
static void StepperMotor_UpdateDownTotalStep(StepperMotor* stepperMotor)
{
    float speed, acceleration, downTimer;
    switch (stepperMotor->moveStatus)
    {
    case MOVE_STATUS_SPEED_NOT_CHANGE:
        speed = stepperMotor->uniformSpeed;
        acceleration = stepperMotor->uniformParam.acceleration;
        break;
    case MOVE_STATUS_SPEED_UP:
        speed = stepperMotor->upSpeed;
        acceleration = stepperMotor->upParam.acceleration;
        break;
    case MOVE_STATUS_PEED_DOWM:
        speed = stepperMotor->dowmSpeed;
        acceleration = stepperMotor->downParam.acceleration;
        break;
    }
    downTimer = (speed - STEPPERMOTOR_INIT_SPEED) * 1.0 / acceleration;
    stepperMotor->speedDownTotalStep =
            (Uint32) ((speed - 0.5 * acceleration * downTimer) * downTimer
                    * stepperMotor->subdivision);
}

Bool StepperMotor_RequestStop(StepperMotor* stepperMotor)
{
    switch (stepperMotor->moveStatus)
    {
    case MOVE_STATUS_SPEED_NOT_CHANGE:
        StepperMotor_MoveStatusSwitch(stepperMotor, MOVE_STATUS_PEED_DOWM);

        stepperMotor->dowmSpeed = stepperMotor->uniformSpeed;
        stepperMotor->downInitSpeed = stepperMotor->uniformSpeed;
        stepperMotor->downParam = stepperMotor->uniformParam;
        break;

    case MOVE_STATUS_SPEED_UP:
        StepperMotor_MoveStatusSwitch(stepperMotor, MOVE_STATUS_PEED_DOWM);

        stepperMotor->dowmSpeed = stepperMotor->upSpeed;
        stepperMotor->downInitSpeed = stepperMotor->upSpeed;
        stepperMotor->downParam = stepperMotor->upParam;

        break;
    case MOVE_STATUS_PEED_DOWM:

        break;
    default:
        return FALSE;
    }

    stepperMotor->result = RESULT_STOPPED;
    stepperMotor->isExcessiveSpeed = FALSE;
    stepperMotor->isLowSpeed = FALSE;
    StepperMotor_UpdateDownTotalStep(stepperMotor);
    stepperMotor->remainStep = stepperMotor->speedDownTotalStep;
    TRACE_INFO("\n ###########StepperMotor %d", stepperMotor->number);
    TRACE_INFO("\n ###########StepperMotor_RequestStop");
    return TRUE;
}

static Bool StepperMotor_Stop(StepperMotor* stepperMotor)
{
    StepperMotorTimer_Stop(stepperMotor);
    StepperMotorDriver_PullLow(&stepperMotor->driver);
    if(TRUE == stepperMotor->isPosLock)
    {
        StepperMotorDriver_Enable(&stepperMotor->driver);
    }
    else
    {
        StepperMotorDriver_Disable(&stepperMotor->driver);
    }

    if (FALSE == StepperMotorTimer_CancelRegisterHandle(stepperMotor))
    {
        return FALSE;
    }
    TRACE_DEBUG("\n ***********subdivision alreadyStep = %d", stepperMotor->alreadyStep);
    TRACE_MARK("\n ***********alreadyDownStep = %d alreadyUpStep = %d",
            stepperMotor->alreadyDownStep, stepperMotor->alreadyUpStep);
    TRACE_MARK("\n ***********dowmSpeed:");
    System_PrintfFloat(TRACE_LEVEL_MARK, stepperMotor->dowmSpeed, 4);
    TRACE_MARK(" step/s");

    stepperMotor->remainStep = 0;
    stepperMotor->speedDownTotalStep = 0;

    stepperMotor->isStatusSwitchStart = FALSE;

    stepperMotor->isParamChange = FALSE;
    stepperMotor->isExcessiveSpeed = FALSE;
    stepperMotor->isLowSpeed = FALSE;
    //加速逻辑
    stepperMotor->upSpeed = 0;
    stepperMotor->alreadyUpTimer = 0;
    stepperMotor->upParam = stepperMotor->flashParam;
    stepperMotor->upInitSpeed = 0;
    stepperMotor->alreadyUpStep = 0;

    //减速逻辑
    stepperMotor->dowmSpeed = 0;
    stepperMotor->alreadyDownTimer = 0;
    stepperMotor->downParam = stepperMotor->flashParam;

    stepperMotor->downInitSpeed = 0;
    stepperMotor->alreadyDownStep = 0;

    stepperMotor->uniformSpeed = 0;
    stepperMotor->uniformParam = stepperMotor->flashParam;

    TRACE_INFO("\n ###########StepperMotor_Stop");
    TRACE_INFO("\n ***********lastStep: %d step", stepperMotor->alreadyStep / stepperMotor->subdivision);

    stepperMotor->status = StepperMotor_IDLE;
    stepperMotor->moveStatus = MOVE_STATUS_IDLE;
    stepperMotor->isStatusSwitchStart = FALSE;
    stepperMotor->otherMoveHandler = NULL;
    return TRUE;
}

Bool StepperMotor_ImmediatelyStop(StepperMotor* stepperMotor)
{
    TRACE_INFO("\n ###########StepperMotor %d", stepperMotor->number);
    TRACE_INFO("\n ###########StepperMotor_ImmediatelyStop");
    stepperMotor->result = RESULT_STOPPED;
//    StepperMotor_MoveStatusSwitch(stepperMotor, MOVE_STATUS_IDLE);
    StepperMotor_Stop(stepperMotor);
    if (NULL != stepperMotor->finishHandler)
    {
        stepperMotor->finishHandler(stepperMotor);
    }
    return TRUE;
}

Bool StepperMotor_ReadDiagnostic(StepperMotor* stepperMotor)
{
    return FALSE;
}

void StepperMotor_DiagnosticCheck(void *obj)
{
    StepperMotor *stepperMotor = (StepperMotor *)obj;
    if(TRUE == StepperMotorDriver_ReadDiagnostic(&stepperMotor->driver))
    {
        StepperMotor_ImmediatelyStop(stepperMotor);
        stepperMotor->result = RESULT_DRIVER_ERROR;   //驱动错误
        TRACE_ERROR("\n StepperMotor %d Driver Diagnostic Warning.", stepperMotor->number);
    }
}

void StepperMotor_DriverCheck(void *obj)
{
    TMCConfig_DriverCheck((StepperMotor *)obj);
}

void StepperMotor_ChangeStep(StepperMotor* stepperMotor, Uint32 step)
{
    Uint32 lastStep = stepperMotor->alreadyStep / stepperMotor->subdivision;
    if (step > lastStep)
    {
        stepperMotor->remainStep =
                (Uint32) (step * stepperMotor->subdivision) - stepperMotor->alreadyStep;
        TRACE_DEBUG("\n ###########ChangeStep remainStep: %d step", stepperMotor->remainStep);
    }
    else
    {
        StepperMotor_RequestStop(stepperMotor);
    }
}

MoveResult StepperMotor_GetMoveResult(StepperMotor* stepperMotor)
{
    return stepperMotor->result;
}

void StepperMotor_RegisterFinishHandle(StepperMotor* stepperMotor, StepperMotor_FinishHandler handler)
{
    stepperMotor->finishHandler = handler;
}

static void StepperMotor_PrintfInfo(StepperMotor* stepperMotor)
{
    TRACE_CODE(
            "\n ***********remainStep = %d alreadyUpStep = %d speedDownTotalStep = %d",
            stepperMotor->remainStep, stepperMotor->alreadyUpStep, stepperMotor->speedDownTotalStep);

    TRACE_CODE("\n ***********upSpeed:");
    System_PrintfFloat(TRACE_LEVEL_CODE, stepperMotor->upSpeed, 4);
    TRACE_CODE(" step/s");
    TRACE_CODE("  uniformSpeed:");
    System_PrintfFloat(TRACE_LEVEL_CODE, stepperMotor->uniformSpeed, 4);
    TRACE_CODE(" step/s");
    TRACE_CODE("  dowmSpeed:");
    System_PrintfFloat(TRACE_LEVEL_CODE, stepperMotor->dowmSpeed, 4);
    TRACE_CODE(" step/s");
}

void StepperMotor_MoveHandler(StepperMotor* stepperMotor)
{
    // 参数检查
    if (NULL == stepperMotor)
    {
        return;
    }
    if (0 == stepperMotor->remainStep)
    {
        stepperMotor->moveStatus = MOVE_STATUS_IDLE;
        StepperMotor_Stop(stepperMotor);
        if (NULL != stepperMotor->finishHandler)
        {
            stepperMotor->finishHandler(stepperMotor);
        }
        return;
    }

    switch (stepperMotor->moveStatus)
    {
    	//空闲
    case MOVE_STATUS_IDLE:
        StepperMotor_Stop(stepperMotor);
        if (NULL != stepperMotor->finishHandler)
        {
            stepperMotor->finishHandler(stepperMotor);
        }
        break;
        //开始
    case MOVE_STATUS_START:
        if (TRUE == stepperMotor->isStatusSwitchStart)
        {
            StepperMotor_MoveStatusSwitchEnd(stepperMotor);

            stepperMotor->upSpeed = STEPPERMOTOR_INIT_SPEED;
            stepperMotor->upInitSpeed = STEPPERMOTOR_INIT_SPEED;

            StepperMotor_MoveStatusSwitch(stepperMotor, MOVE_STATUS_SPEED_UP);
            TRACE_DEBUG("\n ###########START TO SPEED_UP");

            StepperMotorDriver_PullLow(&stepperMotor->driver);
        }
        break;
        //加速
    case MOVE_STATUS_SPEED_UP:
        //速度计算设置逻辑================================================================================================================
        if (TRUE == stepperMotor->isStatusSwitchStart)
        {
            StepperMotor_MoveStatusSwitchEnd(stepperMotor);
            stepperMotor->alreadyUpTimer = 0; //每次进来的加速度或者速度可能不一样，Vt=a*t+V0直线不一样，t重新计数
        }
        else
        {
            //加速度或速度改变逻辑************************************************************************************
            if (TRUE == stepperMotor->isParamChange)
            {
                stepperMotor->isParamChange = FALSE;
                TRACE_CODE("\n @@@@@ParamSet");
                //加速度改变，设置初始速度为当前速度和清零加速时间
                if (stepperMotor->upParam.acceleration != stepperMotor->setParam.acceleration)
                {
                    stepperMotor->upInitSpeed = stepperMotor->upSpeed;
                    stepperMotor->alreadyUpTimer = 0;
                    TRACE_CODE("acc");
                }
                //当前速度大于最大速度，设置超速标志量
                if (stepperMotor->upSpeed > stepperMotor->setParam.maxSpeed)
                {
                    stepperMotor->isExcessiveSpeed = TRUE;
                    stepperMotor->isLowSpeed = FALSE; //把上一次因速度过低进入加速的标志量去掉
                    TRACE_CODE("ExcessiveSpeed");
                }
                stepperMotor->upParam = stepperMotor->setParam;
            }
            stepperMotor->alreadyUpStep++;
            //根据Vt=a*t+V0算出本脉冲的速度即频率(Vt是当前速度, a是加速度, t是使用此加速度的加速时间，V0是此加速度的)
            stepperMotor->alreadyUpTimer += 1.0 / (stepperMotor->upSpeed * stepperMotor->subdivision); //计算累积加速时间
            stepperMotor->upSpeed = stepperMotor->upParam.acceleration * stepperMotor->alreadyUpTimer
                    + stepperMotor->upInitSpeed; //计算设置速度
        }
        //设置对应定时器的中断频率
        StepperMotorTimer_SpeedSetting(stepperMotor, stepperMotor->upSpeed * stepperMotor->subdivision); //设置细分后的速度

        //输出脉冲和整体状态计数逻辑=======================================================================================================
        StepperMotorDriver_PullHigh(&stepperMotor->driver);
        stepperMotor->alreadyStep++;//泵已转动的步数
        stepperMotor->remainStep--; //泵的剩余步数
        for (int i = 0; i < 100; i++)
            ;
        StepperMotorDriver_PullLow(&stepperMotor->driver);
        StepperMotor_UpdateDownTotalStep(stepperMotor);//计算减速到初始速度需要的步数，保存到stepperMotor.speedDownTotalStep
        //状态切换逻辑====================================================================================================================
        //如果减速步数等于小于剩余步数则进入减速阶段(加速度减少时可能出现小于情况)
        if (stepperMotor->speedDownTotalStep >= stepperMotor->remainStep)
        {
            stepperMotor->dowmSpeed = stepperMotor->upSpeed;
            stepperMotor->downInitSpeed = stepperMotor->upSpeed;
            stepperMotor->downParam = stepperMotor->upParam;

            stepperMotor->isExcessiveSpeed = FALSE;
            stepperMotor->isLowSpeed = FALSE;

            StepperMotor_MoveStatusSwitch(stepperMotor, MOVE_STATUS_PEED_DOWM);
            TRACE_DEBUG(
                    "\n ###########DownTotalStep = remainStep SPEED_UP TO PEED_DOWM");
            StepperMotor_PrintfInfo(stepperMotor);
        }
        //如果下一次再加速剩余步数不足以减速则进入匀速阶段
        else if (stepperMotor->speedDownTotalStep == (stepperMotor->remainStep - 1))
        {
            stepperMotor->uniformSpeed = stepperMotor->upSpeed;
            stepperMotor->uniformParam = stepperMotor->upParam;

            stepperMotor->isExcessiveSpeed = FALSE;
            stepperMotor->isLowSpeed = FALSE;

            StepperMotor_MoveStatusSwitch(stepperMotor, MOVE_STATUS_SPEED_NOT_CHANGE);
            TRACE_DEBUG(
                    "\n ###########DownTotalStep = remainStep-1 SPEED_UP TO PEED_DOWM");
            StepperMotor_PrintfInfo(stepperMotor);
        }
        else if (stepperMotor->upSpeed >= stepperMotor->upParam.maxSpeed) //当前速度大于等于最大速度(当前速度有很大几率大于最大速度)
        {
            if (TRUE == stepperMotor->isExcessiveSpeed)        //由设置最大速度引起的，需要进入减速
            {
                StepperMotor_MoveStatusSwitch(stepperMotor, MOVE_STATUS_PEED_DOWM);

                stepperMotor->dowmSpeed = stepperMotor->upSpeed;
                stepperMotor->downInitSpeed = stepperMotor->upSpeed;
                stepperMotor->downParam = stepperMotor->upParam;

                TRACE_DEBUG(
                        "\n ###########isExcessiveSpeed =TRUE SPEED_UP TO PEED_DOWM");
                StepperMotor_PrintfInfo(stepperMotor);
            }
            else        //加速到最大速度，进入匀速
            {
                stepperMotor->uniformSpeed = stepperMotor->upSpeed;
                stepperMotor->uniformParam = stepperMotor->upParam;
                stepperMotor->isLowSpeed = FALSE;

                StepperMotor_MoveStatusSwitch(stepperMotor, MOVE_STATUS_SPEED_NOT_CHANGE);
                TRACE_DEBUG(
                        "\n ###########speed >= maxSpeed SPEED_UP TO NOT_CHANGE");
                StepperMotor_PrintfInfo(stepperMotor);
            }
        }
        break;
        //匀速
    case MOVE_STATUS_SPEED_NOT_CHANGE:
        //速度计算设置逻辑================================================================================================================
        if (TRUE == stepperMotor->isStatusSwitchStart)
        {
            StepperMotor_MoveStatusSwitchEnd(stepperMotor);
        }
        //加速度或速度改变逻辑************************************************************************************
        if (TRUE == stepperMotor->isParamChange)
        {
            stepperMotor->isParamChange = FALSE;
            TRACE_CODE("\n @@@@@ParamSet");
            //最大速度改变，设置超速或者慢速标志
            if (stepperMotor->uniformParam.maxSpeed != stepperMotor->setParam.maxSpeed)
            {
                if (stepperMotor->uniformSpeed > stepperMotor->setParam.maxSpeed)
                {
                    stepperMotor->isExcessiveSpeed = TRUE;
                    stepperMotor->isLowSpeed = FALSE;
                    TRACE_CODE("ExcessiveSpeed");
                }
                else if (stepperMotor->uniformSpeed < stepperMotor->setParam.maxSpeed)
                {
                    stepperMotor->isLowSpeed = TRUE;
                    stepperMotor->isExcessiveSpeed = FALSE;
                    TRACE_CODE("LowSpeed");
                }
            }
            stepperMotor->uniformParam = stepperMotor->setParam;
            StepperMotor_UpdateDownTotalStep(stepperMotor);        //在加速度改变后重新计算剩余步数即可
        }

        //输出脉冲和整体状态计数逻辑=======================================================================================================
        StepperMotorDriver_PullHigh(&stepperMotor->driver);
        stepperMotor->alreadyStep++;
        stepperMotor->remainStep--;
        for (int i = 0; i < 100; i++)
            ;
        StepperMotorDriver_PullLow(&stepperMotor->driver);

        //状态切换逻辑====================================================================================================================
        if (stepperMotor->speedDownTotalStep >= stepperMotor->remainStep)
        {
            StepperMotor_MoveStatusSwitch(stepperMotor, MOVE_STATUS_PEED_DOWM);

            stepperMotor->dowmSpeed = stepperMotor->uniformSpeed;
            stepperMotor->downInitSpeed = stepperMotor->uniformSpeed;
            stepperMotor->downParam = stepperMotor->uniformParam;

            stepperMotor->isExcessiveSpeed = FALSE;
            stepperMotor->isLowSpeed = FALSE;
            TRACE_DEBUG(
                    "\n ###########UpTotalStep == remainStep NOT_CHANGE TO PEED_DOWM");
            StepperMotor_PrintfInfo(stepperMotor);
        }
        else if (TRUE == stepperMotor->isExcessiveSpeed)        //需要减速
        {
            StepperMotor_MoveStatusSwitch(stepperMotor, MOVE_STATUS_PEED_DOWM);

            stepperMotor->dowmSpeed = stepperMotor->uniformSpeed;
            stepperMotor->downInitSpeed = stepperMotor->uniformSpeed;
            stepperMotor->downParam = stepperMotor->uniformParam;

            TRACE_DEBUG(
                    "\n ###########isExcessiveSpeed =TRUE NOT_CHANGE TO PEED_DOWM");
            StepperMotor_PrintfInfo(stepperMotor);
        }
        else if ((TRUE == stepperMotor->isLowSpeed))        //需要加速
        {
            StepperMotor_MoveStatusSwitch(stepperMotor, MOVE_STATUS_SPEED_UP);

            stepperMotor->upSpeed = stepperMotor->uniformSpeed;
            stepperMotor->upInitSpeed = stepperMotor->uniformSpeed;
            stepperMotor->upParam = stepperMotor->uniformParam;

            TRACE_DEBUG("\n ###########isLowSpeed =TRUE NOT_CHANGE TO PEED_UP");
            StepperMotor_PrintfInfo(stepperMotor);
        }
        break;
        //减速
    case MOVE_STATUS_PEED_DOWM:
        //速度计算设置逻辑================================================================================================================
        if (TRUE == stepperMotor->isStatusSwitchStart)
        {
            StepperMotor_MoveStatusSwitchEnd(stepperMotor);
            stepperMotor->alreadyDownTimer = 0;
        }
        //加速度或速度改变逻辑************************************************************************************
        if (TRUE == stepperMotor->isParamChange)
        {
            stepperMotor->isParamChange = FALSE;
            TRACE_CODE("\n @@@@@ParamSet");
            //加速度改变，设置初始速度为当前速度和清零加速时间
            if (stepperMotor->downParam.acceleration != stepperMotor->setParam.acceleration)
            {
                stepperMotor->downInitSpeed = stepperMotor->dowmSpeed;
                stepperMotor->alreadyDownTimer = 0;
                TRACE_CODE("acc");
            }
            //当前速度小于最大速度，设置慢速标志量
            if (stepperMotor->dowmSpeed < stepperMotor->setParam.maxSpeed)
            {
                stepperMotor->isLowSpeed = TRUE;
                stepperMotor->isExcessiveSpeed = FALSE;
                TRACE_CODE("isLowSpeed");
            }
            stepperMotor->downParam = stepperMotor->setParam;
        }
        if (stepperMotor->dowmSpeed > STEPPERMOTOR_INIT_SPEED)
        {
            stepperMotor->alreadyDownStep++;
            stepperMotor->alreadyDownTimer += 1.0
                    / (stepperMotor->dowmSpeed * stepperMotor->subdivision); //计算累积减速时间
            stepperMotor->dowmSpeed = stepperMotor->downInitSpeed
                    - stepperMotor->downParam.acceleration * stepperMotor->alreadyDownTimer; //根据Vt=V0-a*t算出本脉冲的速度即频率
            if(stepperMotor->dowmSpeed < STEPPERMOTOR_INIT_SPEED)
            {
                stepperMotor->dowmSpeed = STEPPERMOTOR_INIT_SPEED;
            }
            StepperMotorTimer_SpeedSetting(stepperMotor, stepperMotor->dowmSpeed * stepperMotor->subdivision);
        }

        //输出脉冲和整体状态计数逻辑=======================================================================================================
        StepperMotorDriver_PullHigh(&stepperMotor->driver);
        stepperMotor->alreadyStep++;
        stepperMotor->remainStep--;
        for (int i = 0; i < 100; i++)
            ;
        StepperMotorDriver_PullLow(&stepperMotor->driver);
        StepperMotor_UpdateDownTotalStep(stepperMotor);
        //状态切换逻辑====================================================================================================================
        if (0 == stepperMotor->remainStep)
        {
            StepperMotor_MoveStatusSwitch(stepperMotor, MOVE_STATUS_IDLE);
            TRACE_DEBUG("\n ########### 0 == stepperMotor->remainStep ");
            StepperMotor_Stop(stepperMotor);
            if (NULL != stepperMotor->finishHandler)
            {
                stepperMotor->finishHandler(stepperMotor);
            }
        }
        else if (stepperMotor->speedDownTotalStep < stepperMotor->remainStep)
        {
            //减速完成
            if (TRUE == stepperMotor->isExcessiveSpeed
                    && stepperMotor->dowmSpeed <= stepperMotor->downParam.maxSpeed)
            {
                stepperMotor->isExcessiveSpeed = FALSE;

                StepperMotor_MoveStatusSwitch(stepperMotor, MOVE_STATUS_SPEED_NOT_CHANGE);
                stepperMotor->uniformSpeed = stepperMotor->dowmSpeed;
                stepperMotor->uniformParam = stepperMotor->downParam;

                TRACE_DEBUG(
                        "\n ###########isExcessiveSpeed = FALSE  SPEED_DOWM TO NOT_CHANGE");
                StepperMotor_PrintfInfo(stepperMotor);
            }
            else if (TRUE == stepperMotor->isLowSpeed) //需要加速
            {
                StepperMotor_MoveStatusSwitch(stepperMotor, MOVE_STATUS_SPEED_UP);

                stepperMotor->upSpeed = stepperMotor->dowmSpeed;
                stepperMotor->upInitSpeed = stepperMotor->dowmSpeed;
                stepperMotor->upParam = stepperMotor->downParam;

                TRACE_DEBUG(
                        "\n ###########isLowSpeed = TRUE  SPEED_DOWM TO SPEED_UP");
                StepperMotor_PrintfInfo(stepperMotor);
            }
        }
        break;
    default:
        break;
    }

    if (stepperMotor->otherMoveHandler != NULL)
    {
        if (0 == (stepperMotor->alreadyStep % stepperMotor->subdivision) && stepperMotor->alreadyStep != 0)
        {
            stepperMotor->otherMoveHandler(stepperMotor);
        }
    }
}

