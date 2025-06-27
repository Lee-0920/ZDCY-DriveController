/**
 * @addtogroup module_DNCP
 * @{
 */

/**
 * @file
 * @brief 设备简单链路协议。
 * @details DSLP 为上层提供无连接的、不可靠的传输服务。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2013-1-23
 */

#ifndef DNCP_LL_DSLP_H_
#define DNCP_LL_DSLP_H_

#include "Container/Queue.h"
#include "DNCP/Base/Frame.h"
#include "DNCP/Ll/LinkFrame.h"
#include "DNCP/Ll/Ll.h"

#ifdef __cplusplus
extern "C" {
#endif


//*******************************************************************
// Dslp 定义

/**
 * @brief Dslp 协议实体对象。
 */
typedef struct DslpStruct
{
    Ll base;                ///< 实现Lai实体的基本数据结构

    // Dslp实现相关
    ILaiComm* laiComm;      ///< 底层通信接口
    Queue transQueue;       ///< 发送帧队列
    Frame* transFrames[32]; ///< 发送帧队列的缓冲
    Bool isSending;
}
Dslp;


//*******************************************************************
// DSCP 实现相关

extern void Dslp_Init(Dslp* self, Lai* laiToUse);
extern void Dslp_Uninit(Dslp* self);


#ifdef __cplusplus
}
#endif

#endif  // DNCP_LL_DSLP_H_

/** @} */
