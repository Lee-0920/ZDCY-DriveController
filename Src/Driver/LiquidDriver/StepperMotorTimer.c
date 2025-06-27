/*
 * StepperMotorTimer.c
 *
 *  Created on: 2016年5月31日
 *      Author: Administrator
 */

#include "string.h"
#include "tracer/trace.h"
#include "SystemConfig.h"
#include "StepperMotorTimer.h"

//中断时间计算
//Tout= ((TIM_PERIOD + 1) * (TIM_PRESCALER + 1)) / Tclk
//Tclk 定时器的输入时钟频率（单位为 Mhz）,APB1为90M, APB2为180M
//TIM_PERIOD 重装载寄存器周期值
//TIM_PRESCALER 时钟频率的预分频值

#define STEPPERMOTOR_TIMER_HZ                 1         // 每秒中断频率
#define STEPPERMOTOR_TIMER_PERIOD             36000     // 重载计数
#define STEPPERMOTOR_TIMER_PRESCALER          ((90000000/STEPPERMOTOR_TIMER_PERIOD/STEPPERMOTOR_TIMER_HZ)-1) // 预分频

#define STEPPERMOTOR_TIMERA_RCC                RCC_APB1Periph_TIM6
#define STEPPERMOTOR_TIMERA_RCC_CONFIG         RCC_APB1PeriphClockCmd(STEPPERMOTOR_TIMERA_RCC,ENABLE)
#define STEPPERMOTOR_TIMERA_IRQn               TIM6_DAC_IRQn
#define STEPPERMOTOR_TIMERA_IRQHANDLER         TIM6_DAC_IRQHandler
#define STEPPERMOTOR_TIMERA                    TIM6

#define STEPPERMOTOR_TIMERB_RCC                RCC_APB1Periph_TIM4
#define STEPPERMOTOR_TIMERB_RCC_CONFIG         RCC_APB1PeriphClockCmd(STEPPERMOTOR_TIMERB_RCC,ENABLE)
#define STEPPERMOTOR_TIMERB_IRQn               TIM4_IRQn
#define STEPPERMOTOR_TIMERB_IRQHANDLER         TIM4_IRQHandler
#define STEPPERMOTOR_TIMERB                    TIM4

#define STEPPERMOTOR_TIMERC_RCC                RCC_APB1Periph_TIM13
#define STEPPERMOTOR_TIMERC_RCC_CONFIG         RCC_APB1PeriphClockCmd(STEPPERMOTOR_TIMERC_RCC,ENABLE)
#define STEPPERMOTOR_TIMERC_IRQn               TIM8_UP_TIM13_IRQn
#define STEPPERMOTOR_TIMERC_IRQHANDLER         TIM8_UP_TIM13_IRQHandler
#define STEPPERMOTOR_TIMERC                    TIM13

#define STEPPERMOTOR_TIMERD_RCC                RCC_APB1Periph_TIM14
#define STEPPERMOTOR_TIMERD_RCC_CONFIG         RCC_APB1PeriphClockCmd(STEPPERMOTOR_TIMERB_RCC,ENABLE)
#define STEPPERMOTOR_TIMERD_IRQn               TIM8_TRG_COM_TIM14_IRQn
#define STEPPERMOTOR_TIMERD_IRQHANDLER         TIM8_TRG_COM_TIM14_IRQHandler
#define STEPPERMOTOR_TIMERD                    TIM14

//各个定时器中断服务程序回调函数指针
static void (*StepperMotorTimerA_Handle)(void *obj) = NULL;
static void (*StepperMotorTimerB_Handle)(void *obj) = NULL;
static void (*StepperMotorTimerC_Handle)(void *obj) = NULL;
static void (*StepperMotorTimerD_Handle)(void *obj) = NULL;

//传入中断服务程序回调函数参数
static void* s_timerAObj = NULL;
static void* s_timerBObj = NULL;
static void* s_timerCObj = NULL;
static void* s_timerDObj = NULL;

/**
 * @brief 配置4个定时器的中断向量管理器
 */
static void StepperMotorTimer_NVICConfig(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_InitStructure.NVIC_IRQChannel = STEPPERMOTOR_TIMERA_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = STEPPERMOTOR_TIMERA_PRIORITY;
    NVIC_Init(&NVIC_InitStructure);

    //NVIC_InitStructure.NVIC_IRQChannel = STEPPERMOTOR_TIMERB_IRQn;
    //NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = STEPPERMOTOR_TIMERB_PRIORITY;
    //NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = STEPPERMOTOR_TIMERC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = STEPPERMOTOR_TIMERC_PRIORITY;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = STEPPERMOTOR_TIMERD_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = STEPPERMOTOR_TIMERD_PRIORITY;
    NVIC_Init(&NVIC_InitStructure);
}

/**
 * @brief 配置定时器的时钟
 */
static void StepperMotorTimer_RCCConfig(void)
{
    STEPPERMOTOR_TIMERA_RCC_CONFIG;
    //STEPPERMOTOR_TIMERB_RCC_CONFIG;
    STEPPERMOTOR_TIMERC_RCC_CONFIG;
    STEPPERMOTOR_TIMERD_RCC_CONFIG;
}

/**
 * @brief 配置定时器的工作参数
 */
static void StepperMotorTimer_Configuration(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    memset(&TIM_TimeBaseStructure, 0, sizeof(TIM_TimeBaseStructure));

    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseStructure.TIM_Period = STEPPERMOTOR_TIMER_PERIOD;//36000
    TIM_TimeBaseStructure.TIM_Prescaler = STEPPERMOTOR_TIMER_PRESCALER;//2500
    TIM_TimeBaseInit(STEPPERMOTOR_TIMERA, &TIM_TimeBaseStructure);
    TIM_ClearFlag(STEPPERMOTOR_TIMERA, TIM_IT_Update); //避免定时器启动时进入中断服务程序
    TIM_ITConfig(STEPPERMOTOR_TIMERA, TIM_IT_Update, ENABLE);
    TIM_Cmd(STEPPERMOTOR_TIMERA, DISABLE);

    //TIM_TimeBaseStructure.TIM_Period = STEPPERMOTOR_TIMER_PERIOD;
    //TIM_TimeBaseStructure.TIM_Prescaler = STEPPERMOTOR_TIMER_PRESCALER;
    //TIM_TimeBaseInit(STEPPERMOTOR_TIMERB, &TIM_TimeBaseStructure);
    //TIM_ClearFlag(STEPPERMOTOR_TIMERB, TIM_IT_Update);
    //TIM_ITConfig(STEPPERMOTOR_TIMERB, TIM_IT_Update, ENABLE);
    //TIM_Cmd(STEPPERMOTOR_TIMERB, DISABLE);

    TIM_TimeBaseStructure.TIM_Period = STEPPERMOTOR_TIMER_PERIOD;
    TIM_TimeBaseStructure.TIM_Prescaler = STEPPERMOTOR_TIMER_PRESCALER;
    TIM_TimeBaseInit(STEPPERMOTOR_TIMERC, &TIM_TimeBaseStructure);
    TIM_ClearFlag(STEPPERMOTOR_TIMERC, TIM_IT_Update);
    TIM_ITConfig(STEPPERMOTOR_TIMERC, TIM_IT_Update, ENABLE);
    TIM_Cmd(STEPPERMOTOR_TIMERC, DISABLE);

    TIM_TimeBaseStructure.TIM_Period = STEPPERMOTOR_TIMER_PERIOD;
    TIM_TimeBaseStructure.TIM_Prescaler = STEPPERMOTOR_TIMER_PRESCALER;
    TIM_TimeBaseInit(STEPPERMOTOR_TIMERD, &TIM_TimeBaseStructure);
    TIM_ClearFlag(STEPPERMOTOR_TIMERD, TIM_IT_Update);
    TIM_ITConfig(STEPPERMOTOR_TIMERD, TIM_IT_Update, ENABLE);
    TIM_Cmd(STEPPERMOTOR_TIMERD, DISABLE);
}

/**
 * @brief 初始化定时器
 */
void StepperMotorTimer_Init(void)
{
    StepperMotorTimer_NVICConfig();
    StepperMotorTimer_RCCConfig();
    StepperMotorTimer_Configuration();
}

/**
 * @brief 定时器中断回调函数注册，使中断回调函数指向需要使用定时器的泵的运动算法处理
 * @param Handle 定时器中断回调函数指向的函数
 * @param obj 定时器中断回调函数传入的参数
 * @return TRUE 操作成功 FALSE 操作失败(所有定时器已用掉)
 */
Bool StepperMotorTimer_RegisterHandle(TimerHandle Handle, void* obj)
{
    if (StepperMotorTimerA_Handle == NULL)
    {
        StepperMotorTimerA_Handle = Handle;
        s_timerAObj = obj;
        TRACE_CODE("\n###########Start TIMERA: ");
    }
    //else if (StepperMotorTimerB_Handle == NULL)
    //{
    //    StepperMotorTimerB_Handle = Handle;
    //    s_timerBObj = obj;
    //    TRACE_CODE("\n###########Start TIMERB: ");
    //}
    else if (StepperMotorTimerC_Handle == NULL)
    {
        StepperMotorTimerC_Handle = Handle;
        s_timerCObj = obj;
        TRACE_CODE("\n###########Start TIMERC: ");
    }
    else if (StepperMotorTimerD_Handle == NULL)
    {
        StepperMotorTimerD_Handle = Handle;
        s_timerDObj = obj;
        TRACE_CODE("\n###########Start TIMERD: ");
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}

/**
 * @brief 取消中断回调函数指向的泵的运动算法处理，释放中断回调函数和传入参数
 * @param Handle 定时器中断回调函数指向的函数
 * @return TRUE 操作成功 FALSE 操作失败(找不到使用的定时器)
 */
Bool StepperMotorTimer_CancelRegisterHandle(void* obj)
{
    if (obj == s_timerAObj)
    {
        StepperMotorTimerA_Handle = NULL;
        s_timerAObj = NULL;
        TRACE_CODE("\n###########Discon TIMERA: ");
    }
    //else if (obj == s_timerBObj)
    //{
    //    StepperMotorTimerB_Handle = NULL;
    //    s_timerBObj = NULL;
    //    TRACE_CODE("\n###########Discon TIMERB: ");
    //}
    else if (obj == s_timerCObj)
    {
        StepperMotorTimerC_Handle = NULL;
        s_timerCObj = NULL;
        TRACE_CODE("\n###########Discon TIMERC: ");
    }
    else if (obj == s_timerDObj)
    {
        StepperMotorTimerD_Handle = NULL;
        s_timerDObj = NULL;
        TRACE_CODE("\n###########Discon TIMERD: ");
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}

/**
 * @brief 启动对应得定时器
 * @param obj 用于查找对应的定时器
 */
void StepperMotorTimer_Start(void* obj)
{
    TIM_TypeDef* TIMx;

    if (obj == s_timerAObj)
    {
        TIMx = STEPPERMOTOR_TIMERA;
    }
    //else if (obj == s_timerBObj)
    //{
    //    TIMx = STEPPERMOTOR_TIMERB;
    //}
    else if (obj == s_timerCObj)
    {
        TIMx = STEPPERMOTOR_TIMERC;
    }
    else if (obj == s_timerDObj)
    {
        TIMx = STEPPERMOTOR_TIMERD;
    }
    // 清计数器
    TIMx->CNT = 0;
    // 清除更新中断标志
    TIMx->SR &= (uint16_t) (~((uint16_t) TIM_IT_Update));
    TIM_ITConfig(TIMx, TIM_IT_Update, ENABLE);
    // 使能定时器
    TIMx->CR1 |= TIM_CR1_CEN;
}

/**
 * @brief 停止对应得定时器
 * @param obj 用于查找对应的定时器
 */
void StepperMotorTimer_Stop(void* obj)
{
    TIM_TypeDef* TIMx;
    if (obj == s_timerAObj)
    {
        TIMx = STEPPERMOTOR_TIMERA;
    }
    //else if (obj == s_timerBObj)
    //{
    //    TIMx = STEPPERMOTOR_TIMERB;
    //}
    else if (obj == s_timerCObj)
    {
        TIMx = STEPPERMOTOR_TIMERC;
    }
    else if (obj == s_timerDObj)
    {
        TIMx = STEPPERMOTOR_TIMERD;
    }
    TIM_ITConfig(TIMx, TIM_IT_Update, DISABLE);
    TIMx->CR1 &= (uint16_t) (~((uint16_t) TIM_CR1_CEN));
}

/**
 * @brief 设置对应定时器的中断频率
 * @param obj 用于查找对应的定时器
 * @param speed 速度便是中断频率 单位Hz(注意频率不能大于36KHz)
 */
void StepperMotorTimer_SpeedSetting(void* obj, float speed)
{
    uint16_t count = 0;
    TIM_TypeDef* TIMx;
    if (obj == s_timerAObj)
    {
        TIMx = STEPPERMOTOR_TIMERA;
    }
    //else if (obj == s_timerBObj)
    //{
    //    TIMx = STEPPERMOTOR_TIMERB;
    //}
    else if (obj == s_timerCObj)
    {
        TIMx = STEPPERMOTOR_TIMERC;
    }
    else if (obj == s_timerDObj)
    {
        TIMx = STEPPERMOTOR_TIMERD;
    }
    count = (uint16_t)(STEPPERMOTOR_TIMER_PERIOD * 1.0 / speed - 1);

    //     设置定时器重载计数
    TIM_SetAutoreload(TIMx, count);
}

void STEPPERMOTOR_TIMERA_IRQHANDLER(void)
{
    if ((STEPPERMOTOR_TIMERA->DIER & TIM_IT_Update)
            && (STEPPERMOTOR_TIMERA->SR & TIM_IT_Update))  //检查更新中断发生与否
    {
        // 清更新中断标志
        STEPPERMOTOR_TIMERA->SR &= (uint16_t) (~((uint16_t) TIM_IT_Update));
        StepperMotorTimerA_Handle(s_timerAObj);
    }
}

/*void STEPPERMOTOR_TIMERB_IRQHANDLER(void)
{
    if ((STEPPERMOTOR_TIMERB->DIER & TIM_IT_Update)
            && (STEPPERMOTOR_TIMERB->SR & TIM_IT_Update))  //检查更新中断发生与否
    {
        // 清更新中断标志
        STEPPERMOTOR_TIMERB->SR &= (uint16_t) (~((uint16_t) TIM_IT_Update));
        StepperMotorTimerB_Handle(s_timerBObj);
    }
}*/

void STEPPERMOTOR_TIMERC_IRQHANDLER(void)
{
    if ((STEPPERMOTOR_TIMERC->DIER & TIM_IT_Update)
            && (STEPPERMOTOR_TIMERC->SR & TIM_IT_Update))  //检查更新中断发生与否
    {
        // 清更新中断标志
        STEPPERMOTOR_TIMERC->SR &= (uint16_t) (~((uint16_t) TIM_IT_Update));
        StepperMotorTimerC_Handle(s_timerCObj);
    }
}

void STEPPERMOTOR_TIMERD_IRQHANDLER(void)
{
    if ((STEPPERMOTOR_TIMERD->DIER & TIM_IT_Update)
            && (STEPPERMOTOR_TIMERD->SR & TIM_IT_Update))  //检查更新中断发生与否
    {
        // 清更新中断标志
        STEPPERMOTOR_TIMERD->SR &= (uint16_t) (~((uint16_t) TIM_IT_Update));
        StepperMotorTimerD_Handle(s_timerDObj);
    }
}
