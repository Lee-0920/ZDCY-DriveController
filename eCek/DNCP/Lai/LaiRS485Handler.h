/**
 * @addtogroup module_DNCP
 * @{
 */

/**
 * @file
 * @brief 链路适配接口LaiRS485的实现。
 * @details 在Socket上，通过UDP实现一个测试用的Lai层实体，作为上层协议的测试桩。
 * @version 1.0.0
 * @author xingfan
 * @date 2016-5-10
 */
#ifndef LAI_RS485_HANDLER_H_
#define LAI_RS485_HANDLER_H_

#include "Lai.h"
#include "Container/Queue.h"
#ifdef __cplusplus
extern "C"
{
#endif



#define POLLINGRFRAME_LCW 0x88  //令牌帧的控制字
//*******************************************************************
// LaiRS485 定义
typedef enum
{
    Idle,
    Requesting,
    Sending,
}LaiRS485Status;
/**
 * @brief UDP的Lai层实体对象。
 */
typedef struct LaiRS485Struct
{
    Lai base;                           ///< 实现Lai实体的基本数据结构

    LaiRS485Status RS485CurrentStatus;  ///< 从机RS485接口当前状态

    FrameCtrlWord ctrl;                 ///< 链路控制字
    Uint8 isPolling;                    ///< 链路控制字的状态机
    Uint8 pollSendNum;                  ///< 链路控制字的状态机

    // 接收逻辑
    Bool isParseTaskActive;             ///< 帧解析任务是否处于激活状态（帧解析状态机）
    Uint8 recvStatus;                   ///< 接收状态（帧解析状态机）
    Uint8 recvEscStatus;                ///< 接收转义状态
    Bool isFrameDetected;               ///< 标示是否接收到帧
    Byte recvBuf[FRAME_MAX_SIZE];       ///< 接收缓冲
    volatile Uint16 recvBytes;          ///< 接收到的字节长度
    Byte recvFrameData[FRAME_MAX_SIZE]; ///< 已经接收的帧缓冲
    volatile Uint16 recvFrameBytes;     ///< 已经接收的帧缓冲长度
    // 发送逻辑，只能缓冲一帧数据
    Bool isSending;                     ///< 帧发送请求，有帧需要发送时，值为TRUE
    Uint8 sendEscStatus;                ///< 发送转义字符状态机
    Bool isSendSync;                    ///< 发送转义字符状态机
    Bool isSendEnd;                     ///< 发送转义字符状态机
    Uint8 sendTo;                       ///< 要发往的地址
    Uint16 sendBytes;                   ///< 要发送的字节长度
    Byte sendBuffer[FRAME_MAX_SIZE];    ///< 发送缓冲FIFO
    volatile Uint16 idxTx;              ///< 发送缓冲指针
    Uint8 destAddr;                     ///< 正在通信的对方地址
    Uint8 checkSlave;                   ///< 用于检查从机是否存在
    volatile Uint8 txByteCount;         ///< 用于计数已发送的字节数
    Bool isSendLastFrame;               ///< 发送最后一帧标志,最后一帧为TRUE
    Bool isOnSendRequesting;            ///< 请求发送标志位，请求了为TRUE
    Bool isSendAllow;                   ///< 发送允许标志，允许为TRUE
    Bool isTokenTimeout;
    Uint8 tokenTimeoutCnt;
} LaiRS485;

/**
 * @brief 全局共享的 LaiRS485 实体的 LaiComm 接口。
 */

// LaiRS485 实现相关
void LaiRS485_Init(LaiRS485* lai);
void LaiRS485_Start(LaiRS485* lai);
void LaiRS485_Stop(LaiRS485* lai);
void LaiRS485_Uninit(LaiRS485* lai);
extern void LaiRS485_SendRequest(void);
extern void LaiRS485_CommitToUpper(void);
void LaiRS485_PollingTimeOut(void);
Bool LaiRS485_GetHostStatus(void);
#ifdef __cplusplus
}
#endif

#endif /* LAI_BLE_UART_H_ */

