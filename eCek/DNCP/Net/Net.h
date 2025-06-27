/**
 * @addtogroup module_DNCP
 * @{
 */

/**
 * @file
 * @brief 网络层抽象接口定义。
 * @details 统一定义网络层的操作接口。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2013-2-1
 */

#ifndef DNCP_NET_NET_H_
#define DNCP_NET_NET_H_

#include "Common/Types.h"
#include "DNCP/Base/Frame.h"
#include "DNCP/Ll/Ll.h"
#include "DNCP/Net/NetPack.h"

#ifdef __cplusplus
extern "C" {
#endif


//*******************************************************************
// 基本常数定义

// 网络层各协议号分配
#define NET_PROTOCAL_TTP                0   // 树网隧道协议
#define NET_PROTOCAL_TRP                1   // 树网路由协议

//*******************************************************************
// INetComm 接口定义


/**
 * @brief INetComm 接口函数：查询接口是否可发送数据。
 * @details 通常在 Send 之前调用。
 *  <p>可发送表示：本实体可立即发送一包新数据，此时上层可直接调用 Send()
 *      函数把数据发送出去；
 *  <p>不可发送表示：本实体正在发送数据，或者发送缓冲中无剩余空间可供等待发送。
 *      此时上层实体应该等待一段时间再尝试发送。
 * @param net 具体的Net层协议实体，如 Trp 。
 */
typedef Bool (* IfNetIsSendable)(void* net);

/**
 * @brief INetComm 接口函数：发送一包数据。
 * @details 请先调用 IsSendable() 查询是否能发送。
 * @param net 具体的Net层协议实体，如 Dslp 。
 * @param npn 调用者的网络层协议号。
 * @param destAddr 本包要发往的目标地址。
 *  上行地址请使用 @ref NET_ADDRESS_UPLINK_MAKE 宏生成；
 *  下行地址请使用 @ref NET_ADDRESS_DOWNLINK_MAKE 宏生成。
 * @param frameToSned 要发送的数据包。
 */
typedef void (* IfNetSend)(void* net, Uint8 npn, NetAddress destAddr, Frame* frameToSned);


/**
 * @brief 链路适配层通信接口，仅支持异步调用。
 */
typedef struct INetCommStruct
{
    IfNetIsSendable IsSendable;
    IfNetSend Send;
}
INetComm;


//*******************************************************************
// INetHandle 接口定义

/**
 * @brief INetHandle 接口函数：收到新包通知。
 * @details 当 Net 层收到一包数据时，通知上层实体及时处理新包。
 * @param[in] app 上层协议实体。
 * @param[in] frame 接收到的包对象。请调用 Frame_GetPackData() 取得
 *  有效负载的数据，调用 Frame_GetPackSize() 查询有效数据长度。
 * @param[in] addr 路由地址。
 */
typedef void (* IfNetOnReceived)(void* app, Frame* frame, NetAddress addr);

/**
 * @brief INetHandle 接口函数：发送报告。
 * @details 本层每次发送一个数据包，都要通告上层发送者该包是否成功发送出去。
 *  <p> 因为本层协议均采用单包等停协议，上层每次只能发送一个包。
 * @param[in] app 上层协议实体。
 * @param[in] isSendout 是否成功发送出去。
 */
typedef void (* IfNetOnSendReport)(void* app, Bool isSendout);

/**
 * @brief Net 层处理器接口。
 * @details 由逻辑链路层协议实体实现，Net 层接口实体调用。
 */
typedef struct INetHandleStruct
{
    IfNetOnReceived OnReceived;
    IfNetOnSendReport OnSendReport;
}
INetHandle;


//*******************************************************************
// NetComm 接口定义

/**
 * @brief Net 实体错误统计。
 * @details 统计Net 协议实体的运行情况。
 */
typedef struct NetStatisticsStruct
{
    unsigned int sendPacks;             ///< 发送包统计
    unsigned int recvPacks;             ///< 接收包统计
    unsigned int lostPacks;             ///< 路由时，因为底层接口的缓冲队列已满造成丢包的数目
}
NetStatistics;

/**
 * @brief Net实体基础数据结构，每个具体的Net实体都必须在结构初始处定义它。
 */
typedef struct NetStruct
{
    ILlHandle llHandle;                 ///< 下层处理接口，必须在结构初始化处定义。具体实现时必须初始化它
    INetComm * comm;                    ///< 本层通信接口，具体实现时必须初始化它
    NetStatistics commStat;             ///< 见： @ref NetStatistics
}
Net;


#ifdef __cplusplus
}
#endif

#endif  // DNCP_NET_NET_H_

/** @} */
