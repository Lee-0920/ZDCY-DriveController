/**
 * @addtogroup module_DNCP
 * @{
 */

/**
 * @file
 * @brief TRP末端节点。
 * @details TrpTail 是树形网络末端节点的 Trp 简化版，是为了减小代码尺寸
 *  而定制的一个网络层简单实体。
 *  <p> TrpTail 只支持一个下层链路接口和一个上层协议，并且只能处于最后一层
 *  （第4层级），因此不需要进行数据包的路由和分发。
 *  
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2013-3-19
 */

#ifndef DNCP_NET_TRP_TAIL_H_
#define DNCP_NET_TRP_TAIL_H_

#include "DNCP/Base/Frame.h"
#include "DNCP/Net/Net.h"

#ifdef __cplusplus
extern "C" {
#endif


//*******************************************************************
// TrpTail 定义

/**
 * @brief TrpTail 协议实体对象。
 */
typedef struct TrpTailStruct
{
    Net base;               ///< 实现Net实体的基本数据结构

    Ll* ll;                 ///< 本层所使用的底层协议对象
    INetHandle* handle;     ///< 注册的上层处理器
    Uint8 uap;

    // TrpTail实现相关
    // 只缓冲一个数据包，实现简单等停的确认协议
    Bool isFrameBuffered;   ///< 帧是否被缓冲，是的话需要持续尝试发送
    Frame* frameToSend;     ///< 待发送的包对象
    NetAddress addrToSend;  ///< 待发送的目的地址
}
TrpTail;


//*******************************************************************
// DSCP 实现相关

extern void TrpTail_Init(TrpTail* self);
extern void TrpTail_Uninit(TrpTail* self);

extern void TrpTail_Setup(TrpTail* self, Ll* llToUse);
extern void TrpTail_Register(TrpTail* self, Uint8 uap, INetHandle* handle);

#ifdef __cplusplus
}
#endif

#endif  // DNCP_NET_TRP_TAIL_H_

/** @} */
