/**
 * @file
 * @brief 环境温度采集驱动。
 * @details 提供采集环境温度功能接口。
 * @version 1.1.0
 * @author kim.xiejinqiang
 * @date 2015-06-04
 */
#include "string.h"
#include "EnvTempCollect.h"
#include "Tracer/Trace.h"
#include "System.h"

static int s_timeout;                   //I2C通信超时计数
#define I2C_TIMEOUT                     25000   //I2C通信超时计数值

#define I2C1_OWN_ADDRESS7               0X0A    //主机地址，这个地址只要与STM32外挂的I2C器件地址不一样即可
#define I2C_Speed                       10000  //STM32 I2C 快速模式,100 kb/s
#define I2C_SLAVE_ADDRESS               0x90    // 根据硬件实际地址A2A1A0 =000对应的从地址0B 0100 1000;

// Pointer Register/温度传感器功能码
#define TMP75_ADDR_TEMP                 0x00    // 温度寄存器         只读
#define TMP75_ADDR_CONF                 0x01    // 配置寄存器          读写
#define TMP75_ADDR_TLOW                 0x02    // 低温限值寄存器  读写
#define TMP75_ADDR_THIGH                0x03    // 高温限值寄存器  读写

// 温度传感器工作模式配置
// TMP75 配置为:AD转换12 bit（温度精度0.0625°C）、错误队列为0、比较模式、连续转换模式。
#define TMP75_MODE                      0x60    // B 01100000 OS R1 R0 F1 F0 POL TM SD
// #define TMP75_START_CONVERT          0xE1    // 触发单次测量 暂时不用
#define TMP75_RESOLUTION                0.0625  // AD转换分辨率

static bool EnvTempCollect_TMP75Init(void);

static void EnvTempCollect_GPIOConfig(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10|GPIO_Pin_11;
    GPIO_Init(GPIOB,&GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_I2C2);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_I2C2);
}

 void EnvTempCollect_I2CConfig()
{
    I2C_InitTypeDef I2C_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2,ENABLE);

    I2C_InitStructure.I2C_Ack=I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress=I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed=I2C_Speed;
    I2C_InitStructure.I2C_DutyCycle=I2C_DutyCycle_2;
    I2C_InitStructure.I2C_Mode=I2C_Mode_I2C;
    I2C_InitStructure.I2C_OwnAddress1=I2C1_OWN_ADDRESS7;

    I2C_Init(I2C2,&I2C_InitStructure);

    I2C_Cmd(I2C2,ENABLE);
}
/**
 * @brief 环境温度采集芯片初始化
 * @param
 */
static void EnvTempCollect_ChipConfig(void)
{
    EnvTempCollect_TMP75Init();
}
/**
 * @brief 环境温度采集初始化
 * @param
 */
void EnvTempCollect_Init(void)
{
    EnvTempCollect_GPIOConfig();
    EnvTempCollect_I2CConfig();
    EnvTempCollect_ChipConfig();  // 环境温度采集芯片初始化
}
/**
 * @briefSTM32 IIC接口初始化
 * @param
 */
void EnvTempCollect_ReInit(void)
{
    EnvTempCollect_GPIOConfig();
    EnvTempCollect_I2CConfig();
}
/**
 * @brief 重新加载超时计数值
 * @param times
 */
static void EnvTempCollect_loadTimeoutValve(int times)
{
    s_timeout = times;
}

/**
 * @brief 判断是否超时，判断时需要一直调用
 * @return TRUE 为超时
 */
static Bool EnvTempCollect_IsTimeout(void)
{
    if(s_timeout == 0)
    {
        return TRUE;
    }
    else
    {
        s_timeout--;
    }
    return FALSE;
}

/**
 * @brief 请求读测量数据
 * @note
 * @return TRUE
 */
static Bool EnvTempCollect_I2CRequestReadReg(void)
{
    EnvTempCollect_loadTimeoutValve(I2C_TIMEOUT);
    while(I2C_GetFlagStatus(I2C2,I2C_FLAG_BUSY))
    {
        if(TRUE == EnvTempCollect_IsTimeout())//避免传感器不存在，或者传感器损坏造成的通信异常
         {
//             TRACE_ERROR("\n Rouse I2C Get Flag Status error");
             I2C_GenerateSTOP(I2C2,ENABLE);
             return FALSE;
         }
    }

    I2C_GenerateSTART(I2C2, ENABLE);//发送起始信号

    EnvTempCollect_loadTimeoutValve(I2C_TIMEOUT);
    while(FALSE == I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if(TRUE == EnvTempCollect_IsTimeout())
        {
//            TRACE_ERROR("\n Rouse I2C START error");
            I2C_GenerateSTOP(I2C2,ENABLE);
            return FALSE;
        }
    }

    I2C_Send7bitAddress(I2C2, I2C_SLAVE_ADDRESS, I2C_Direction_Transmitter);// 1 发送从机地址
    while(FALSE == I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if(TRUE == EnvTempCollect_IsTimeout())
        {
//            TRACE_ERROR("\n Rouse I2C Send7bitAddress error");
            I2C_GenerateSTOP(I2C2,ENABLE);
            return FALSE;
        }
    }

    I2C_SendData(I2C2, TMP75_ADDR_TEMP);    // 2 发送寄存器地址

    EnvTempCollect_loadTimeoutValve(I2C_TIMEOUT);
    while(FALSE == I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if(TRUE == EnvTempCollect_IsTimeout())
        {
//            TRACE_ERROR("\n Rouse I2C Send cmd error");
            I2C_GenerateSTOP(I2C2,ENABLE);
            return FALSE;
        }
    }

    I2C_GenerateSTOP(I2C2,ENABLE);//发送停止信号
    return TRUE;
}

/**
 * @brief 读返回数据或确认信号
 * @note 发送读/写指令后，主机需等待至少220ms，然后再发送读取时序。返回报文格式：
 * 温度数据高（1字节） + 温度数据低（1字节）
 * @param numByteToRead 返回字节数, 字节数由 @see EnvTempCollect_I2CSend 确定
 * @param buffer 返回数据
 * @return TRUE 读取成功
 */
static Bool EnvTempCollect_I2CReceive(Uint8 numByteToRead, Uint8 * buffer)
{
    numByteToRead += 0;

    I2C_GenerateSTART(I2C2, ENABLE);

    EnvTempCollect_loadTimeoutValve(I2C_TIMEOUT);
    while(FALSE == I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if(TRUE == EnvTempCollect_IsTimeout())
        {
//            TRACE_ERROR("\n Receive I2C START error");
            I2C_GenerateSTOP(I2C2,ENABLE);
            return FALSE;
        }
    }

    I2C_Send7bitAddress(I2C2, I2C_SLAVE_ADDRESS, I2C_Direction_Receiver);

    EnvTempCollect_loadTimeoutValve(I2C_TIMEOUT);
    while(FALSE == I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    {
        if(TRUE == EnvTempCollect_IsTimeout())
        {
//            TRACE_ERROR("\n Receive I2C Send7bitAddress error");
            I2C_GenerateSTOP(I2C2,ENABLE);
            return FALSE;
        }
    }

    EnvTempCollect_loadTimeoutValve(I2C_TIMEOUT);
    while(numByteToRead)//连续读取数据
    {
        if(TRUE == I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED))
        {
            *buffer = I2C_ReceiveData(I2C2);
            buffer++;
            numByteToRead--;
        }

        if(1 == numByteToRead)
        {
            I2C_AcknowledgeConfig(I2C2,DISABLE);
            I2C_GenerateSTOP(I2C2,ENABLE);
        }

        if(TRUE == EnvTempCollect_IsTimeout())
        {
//            TRACE_ERROR("\n Receive I2C ReceiveData error");
            I2C_GenerateSTOP(I2C2,ENABLE);
            break;
        }
    }
    I2C_AcknowledgeConfig(I2C2,ENABLE);
    return TRUE;
}

/**
 * @brief TMP75初始化
 * @param
 */
static bool EnvTempCollect_TMP75Init(void)
{

    EnvTempCollect_loadTimeoutValve(I2C_TIMEOUT);
    while(I2C_GetFlagStatus(I2C2,I2C_FLAG_BUSY))
    {
        if(TRUE == EnvTempCollect_IsTimeout())
         {
//             TRACE_ERROR("\n Init I2C Get Flag Status Init_error");
             I2C_GenerateSTOP(I2C2,ENABLE);
             return FALSE;
         }
    }

    I2C_GenerateSTART(I2C2, ENABLE);

    EnvTempCollect_loadTimeoutValve(I2C_TIMEOUT);
    while(FALSE == I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if(TRUE == EnvTempCollect_IsTimeout())
        {
//            TRACE_ERROR("\n Init I2C START Init_error");
            I2C_GenerateSTOP(I2C2,ENABLE);
            return FALSE;
        }
    }
    I2C_Send7bitAddress(I2C2, I2C_SLAVE_ADDRESS, I2C_Direction_Transmitter);// 1 发送IIC从地址

    EnvTempCollect_loadTimeoutValve(I2C_TIMEOUT);
    while(FALSE == I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if(TRUE == EnvTempCollect_IsTimeout())
        {
//            TRACE_ERROR("\n Init I2C Send7bitAddress Init_error");
            I2C_GenerateSTOP(I2C2,ENABLE);
            return FALSE;
        }
    }
    I2C_SendData(I2C2, TMP75_ADDR_CONF);    // 2 发送配置寄存器地址

    EnvTempCollect_loadTimeoutValve(I2C_TIMEOUT);
    while(FALSE == I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if(TRUE == EnvTempCollect_IsTimeout())
        {
//            TRACE_ERROR("\n Init I2C Send cmd Init_error");
            I2C_GenerateSTOP(I2C2,ENABLE);
            return FALSE;
        }
    }
    I2C_SendData(I2C2, TMP75_MODE); // 3 配置工作模式

    EnvTempCollect_loadTimeoutValve(I2C_TIMEOUT);
    while(FALSE == I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if(TRUE == EnvTempCollect_IsTimeout())
        {
//            TRACE_ERROR("\n Init I2C Send mode Init_error");
            I2C_GenerateSTOP(I2C2,ENABLE);
            return FALSE;
        }
    }
    I2C_GenerateSTOP(I2C2, ENABLE);
    return TRUE;
}

/**
 * @brief 获取环境温度
 * @note  连续读取最小间隔为220ms
 * @param 无
 */
float EnvironmentTemp_Get(void)
{
    Uint8 buffer[4] = {0};
    float temperature = -3000;
    if(TRUE == EnvTempCollect_I2CRequestReadReg()) // 1 请求
    {
        System_Delay(2);
        if(TRUE == EnvTempCollect_I2CReceive(2, buffer))    // 2 读取AD值
        {
            temperature =(float)((buffer[0]<<4 | buffer[1]>>4) & 0xfff) * TMP75_RESOLUTION;    // 3 转换
            System_PrintfFloat(TRACE_LEVEL_MARK, temperature, 1);
        }
    }
    if((-3000) == temperature)  // 说明IIC通信错误或者传感器件工作不正常
    {
        // *1 传感器复位 暂无

        // *2 STM32的IIC接口模块初始化
        I2C_DeInit(I2C2);
        System_Delay(2);
        EnvTempCollect_ReInit();
        System_Delay(2);
//        TRACE_ERROR("\n  I2C ReInit_OK");

        if(TRUE == EnvTempCollect_I2CRequestReadReg()) // 1 请求
        {
            System_Delay(2);
            if(TRUE == EnvTempCollect_I2CReceive(2, buffer))    // 2 读取AD值
            {
                temperature =(float)((buffer[0]<<4 | buffer[1]>>4) & 0xfff) * TMP75_RESOLUTION;    // 3 转换
                System_PrintfFloat(TRACE_LEVEL_MARK, temperature, 1);
            }
        }
        return temperature;
    }
    return temperature;
}
