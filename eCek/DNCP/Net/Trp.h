/**
 * @addtogroup module_DNCP
 * @{
 */

/**
 * @file
 * @brief 树网路由协议。
 * @details Trp 为上层提供无连接的、可靠的传输服务，并且实现包转发。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2013-2-1
 */

#ifndef DNCP_NET_TRP_H_
#define DNCP_NET_TRP_H_

#include "Container/Queue.h"
#include "DNCP/Base/Frame.h"
#include "DNCP/Net/Net.h"

#ifdef __cplusplus
extern "C" {
#endif


//*******************************************************************
// Trp 定义


typedef struct RouteStrategyStruct
{
    NetAddress subnetID;                ///< 子网（层级域段）标识
    NetAddress subnetMask;              ///< 子网路由掩码
    Ll* ll;                             ///< 路由策略命中时，所使用的链路接口
}
RouteStrategy;


/**
 * @brief Trp 协议实体对象。
 */
typedef struct TrpStruct
{
    Net base;                ///< 实现Net实体的基本数据结构

    // Trp 选路相关
    RouteStrategy* strategies;          ///< 路由策略
    Uint8 numOfStrategies;              ///< 路由策略的数量
    Uint8 numOfUpperHandles;            ///< 上层协议的数量
    INetHandle** handles;               ///< 注册的上层处理器列表，下标即为 uap，未注册的 uap 应为空
    NetAddress localAddr;               ///< 本节点的网络地址
    NetAddress downMask;                ///< 本节点的下行层级掩码
    Uint8 routeMode;                    ///< 选路模式

    // Trp实现相关
    // 只缓冲一个数据包，实现简单等停的确认协议
    Bool isFrameBuffered;   ///< 帧是否被缓冲，是的话需要持续尝试发送
    INetHandle* frameHandle;///< 发送该帧的上层协议处理器
    Frame* frameToSend;     ///< 待发送的包对象
    NetAddress addrToSend;  ///< 待发送的目的地址，下属层地址经过MASK减1修正
}
Trp;


//*******************************************************************
// DSCP 实现相关

#define TRP_ROUTE_MODE_MULTICAST        0x01    ///< 多播路由

extern void Trp_Init(Trp* self);
extern void Trp_Uninit(Trp* self);

extern void Trp_Setup(Trp* self, Uint8 numOfInterface, Uint8 numOfUpperHandles, NetAddress localAddr, NetAddress downMask);
extern void Trp_AddInterface(Trp* self, Uint8 index, NetAddress subnetID, NetAddress subnetMask, Ll* ll);
extern void Trp_Register(Trp* self, Uint8 uap, INetHandle* handle);

extern void Trp_SetRouteMode(Trp* self, Uint8 mode);

#ifdef __cplusplus
}
#endif

#endif  // DNCP_NET_TRP_H_

/** @} */
