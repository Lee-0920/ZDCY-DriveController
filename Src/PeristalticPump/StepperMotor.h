/*
 * StepperMotor.h
 *
 *  Created on: 2016年5月30日
 *      Author: Administrator
 */

#ifndef SRC_PERISTALTICPUMP_STEPPERMOTOR_H_
#define SRC_PERISTALTICPUMP_STEPPERMOTOR_H_

#include "LiquidDriver/StepperMotorDriver.h"
#include "Common/Types.h"

#ifdef __cplusplus
extern "C"
{
#endif
#define STEPPERMOTOR_TOTAL_NUMBER   4
#define STEPPERMOTOR_INIT_SPEED 10   //未细分的初始化速度
#define STEPPERMOTOR_MIN_SUBDIVISION_SPEED  1600 //已细分的最小速度
#define STEPPERMOTOR_MAX_SUBDIVISION_SPEED  999999 //已细分的最大速度
/**
 * @brief 步进电机的忙碌状态
 */
typedef enum
{
    StepperMotor_IDLE,	//空闲状态
    StepperMotor_BUSY
} StepperMotorStatus;


/**
 * @brief 步进电机运动参数
 */
typedef struct
{
    float acceleration;        //单位为 步/平方秒 没有细分
    float maxSpeed;           //单位为步/秒 没有细分（注:上位机发送过来的ml/秒的最大速度会在PumpManager上转换）
} StepperMotorParam;

/**
 * @brief 步进电机转动的状态机
 */
typedef enum
{
    MOVE_STATUS_IDLE,                // 空闲
    MOVE_STATUS_START,               // 开始
    MOVE_STATUS_SPEED_UP,            // 加速
    MOVE_STATUS_SPEED_NOT_CHANGE,    // 匀速
    MOVE_STATUS_PEED_DOWM,           // 减速
    MAX_MOVE_STATUS
} StepperMotorMoveStatus;

/**
 * @brief 步进电机运行结果
 */
typedef enum
{
    RESULT_FINISHED,                // 操作正常完成。
    RESULT_FAILED,                  // 操作中途出现故障，未能完成。
    RESULT_STOPPED,                 // 操作被停止。
    RESULT_MOVE_OUT_SENSOR_FAIL_C,  //C移不出传感器
    RESULT_MOVE_IN_SENSOR_FAIL_C,    //C找不到传感器
    RESULT_DRIVER_ERROR,            //驱动错误
    MAX_MOVE_RESULT
} MoveResult;

typedef void (*StepperMotor_OtherMoveHandler)(void *stepperMotor);//步进电机每走一步（未细分）的附加处理
typedef void (*StepperMotor_FinishHandler)(void *stepperMotor);//步进电机完全停止的附加处理

/**
 * @brief 步进电机实体
 */
typedef struct
{
    StepperMotorDriver driver;               //泵的驱动引脚

    //基本参数
    Uint8 number;                                   //编号  = 驱动芯片地址slaveAddr
    StepperMotorStatus status;               //泵的忙碌状态
    StepperMotorParam flashParam;

    //运动状态共用逻辑
    Uint8 subdivision;               //泵的细分系数
    Uint32 remainStep;               //泵的剩余步数  已细分
    Uint32 alreadyStep;              //泵已转动的步数 已细分
    Uint32 speedDownTotalStep;
    __IO StepperMotorParam setParam;

    __IO StepperMotorMoveStatus moveStatus;       //泵转动的状态机
    __IO Bool isStatusSwitchStart;        //泵状态切换标志，TRUE为开始切换

    MoveResult result;
    Bool isParamChange;
    Bool isExcessiveSpeed;
    Bool isLowSpeed;
    Bool isPosLock;			//定位锁
    //加速逻辑
    float upSpeed;
    float alreadyUpTimer;                   //加速阶段已过的时间
    StepperMotorParam upParam;
    float upInitSpeed;              //加速阶段的初始速度 没有细分
    Uint32 alreadyUpStep;         //泵加速总步数

    //减速逻辑
    float dowmSpeed;
    float alreadyDownTimer;                 //减速阶段已过的时间
    StepperMotorParam downParam;
    float downInitSpeed;             //减速阶段的初始速度 没有细分
    Uint32 alreadyDownStep;

    //匀速逻辑
    float uniformSpeed;
    StepperMotorParam uniformParam;

    StepperMotor_OtherMoveHandler otherMoveHandler;
    StepperMotor_FinishHandler finishHandler;//步进电机完全停止的附加处理

} StepperMotor;

void StepperMotor_Init(StepperMotor* stepperMotor, StepperMotorParam param);
void StepperMotor_SetNumber(StepperMotor* stepperMotor, Uint8 num);
Uint8 StepperMotor_GetNumber(StepperMotor* stepperMotor);
void StepperMotor_SetPosLockStatus(StepperMotor* stepperMotor, Bool isPosLock);
Bool StepperMotor_GetPosLockStatus(StepperMotor* stepperMotor);
void StepperMotor_SetSubdivision(StepperMotor* stepperMotor, Uint8 subdivision);
Uint8 StepperMotor_GetSubdivision(StepperMotor* stepperMotor);
StepperMotorParam StepperMotor_GetDefaultMotionParam(StepperMotor* stepperMotor);
Bool StepperMotor_SetDefaultMotionParam(StepperMotor* stepperMotor, StepperMotorParam param);
StepperMotorParam StepperMotor_GetCurrentMotionParam(StepperMotor* stepperMotor);
Bool StepperMotor_SetCurrentMotionParam(StepperMotor* stepperMotor, StepperMotorParam param);
StepperMotorStatus StepperMotor_GetStatus(StepperMotor* stepperMotor);
Uint32 StepperMotor_GetAlreadyStep(StepperMotor* stepperMotor);
Bool StepperMotor_AbsolutelyMove(Uint8 index, Direction dir,  Uint32 step,  Uint32 speed,  Uint32 acc);
Bool StepperMotor_AbsoultelyStop(Uint8 index);
Uint8 StepperMotor_GetTotalNumber(void);
Bool StepperMotor_Start(StepperMotor* stepperMotor, Direction dir, Uint32 step, Bool isUseDefaultParam, StepperMotor_OtherMoveHandler otherMoveHandler);
Bool StepperMotor_RequestStop(StepperMotor* stepperMotor);
Bool StepperMotor_ImmediatelyStop(StepperMotor* stepperMotor);
Bool StepperMotor_ReadDiagnostic(StepperMotor* stepperMotor);
void StepperMotor_DiagnosticCheck(void *obj);
void StepperMotor_DriverCheck(void *obj);
void StepperMotor_ChangeStep(StepperMotor* stepperMotor, Uint32 step);
MoveResult StepperMotor_GetMoveResult(StepperMotor* stepperMotor);
void StepperMotor_RegisterFinishHandle(StepperMotor* stepperMotor, StepperMotor_FinishHandler handler);
#ifdef __cplusplus
}
#endif

#endif /* SRC_PERISTALTICPUMP_STEPPERMOTOR_H_ */
