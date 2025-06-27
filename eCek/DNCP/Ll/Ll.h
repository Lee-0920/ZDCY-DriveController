/**
 * @addtogroup module_DNCP
 * @{
 */

/**
 * @file
 * @brief 逻辑链路层抽象接口定义。
 * @details 统一定义逻辑链路层的操作接口。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2013-1-22
 */

#ifndef DNCP_LL_LL_H_
#define DNCP_LL_LL_H_

#include "Common/Types.h"
#include "DNCP/Base/Frame.h"
#include "DNCP/Ll/LinkFrame.h"
#include "DNCP/Lai/Lai.h"

#ifdef __cplusplus
extern "C" {
#endif


//*******************************************************************
// 基本常数定义

#define LL_UAP_SIZE                 4       ///< LL 层支持的上层协议数量

//*******************************************************************
// ILlComm 接口定义

// 复位动作定义
#define LL_TX_CLEAR                 0x01    ///< 清空发送缓冲
#define LL_RX_CLEAR                 0x02    ///< 清空接收缓冲
#define LL_TX_ABORT                 0x04    ///< 中止当前发送操作
#define LL_RX_ABORT                 0x08    ///< 中止当前接收操作

/**
 * @brief ILlComm 接口函数：复位Ll协议状态逻辑。
 * @details 当上层协议体因为通信异常（如等待超时）需要复位Ll实体时调用。
 *  在Ll协议体通信之前不能也没有必要调用本函数。
 * @param ll 具体的Ll层协议实体。
 * @param action 复位动作，可以是以下一个或多个动作的合并（或）：
 *        - @ref LL_TX_CLEAR
 *        - @ref LL_RX_CLEAR 
 *        - @ref LL_TX_ABORT 
 *        - @ref LL_RX_ABORT 
 * @note 实现时，应根据具体的底层协议，分别实现 @p action 指定的所有动作。
 */
typedef void (* IfLlReset)(void* ll, Uint16 action);

/**
 * @brief ILlComm 接口函数：查询接口是否可发送数据。
 * @details 通常在 Send 之前调用。
 *  <p>可发送表示：本实体可立即发送一帧新数据，此时上层可直接调用 Send()
 *      函数把数据发送出去；
 *  <p>不可发送表示：本实体正在发送数据，或者发送缓冲中无剩余空间可供等待发送。
 *      此时上层实体应该等待 OnSendRequest 的到来进行异步发送。
 *  <p>如果正在发送，意味着发送缓冲存在数据或者硬件正在发送数据，
 *      此时上层可等待异步的发送请求。
 * @param ll 具体的Ll层协议实体，如 Dslp 。
 */
typedef Bool (* IfLlIsSendable)(void* ll);

/**
 * @brief ILlComm 接口函数：发送一帧数据。
 * @details 请先调用 IsSendable() 查询发送缓冲是否足够，无剩余空间则应该等待
 *   OnSendRequest 的到来进行异步发送。
 * @param ll 具体的Ll层协议实体，如 Dslp 。
 * @param upn 调用者的上层协议号。
 * @param destAddr 本帧要发往的目标地址（链路地址）。
 * @param frameToSned 要发送的数据帧。
 */
typedef void (* IfLlSend)(void* ll, Uint8 upn, Uint8 destAddr, Frame* frameToSned);


/**
 * @brief 链路适配层通信接口，仅支持异步调用。
 */
typedef struct ILlCommStruct
{
    IfLlReset Reset;
    IfLlIsSendable IsSendable;
    IfLlSend Send;
}
ILlComm;


//*******************************************************************
// ILlHandle 接口定义

/**
 * @brief ILlHandle 接口函数：收到新帧通知。
 * @details 当 Ll 层收到一帧数据时，通知上层实体及时处理新帧。
 * @param[in] upo 上层协议实体。
 * @param[in] frame 接收到的帧对象。请调用 Frame_GetPackData() 取得
 *  有效负载的数据，调用 Frame_GetPackSize() 查询有效数据长度。
 * @param[in] sourceAddr 源地址，即发送该帧的链路节点号。
 *  <p> 该参数最终将被传递到网络层，以实现上行源地址追补机制。除此之外，
 *      客户程序不要使用本参数实现其它功能。
 */
typedef void (* IfLlOnReceived)(void* upo, Frame* frame, Uint8 sourceAddr);

/**
 * @brief ILlHandle 接口函数：请求发送帧。
 * @details 发送缓冲中有剩余足够时，可向上层发出请求，通知上层发送新数据。
 *  <p> 上层实现时，如果还有帧待发，可调用Send继续发送帧，且尽可能多的发送，
 *      直到 IsSendable() 为假。
 * @param[in] upo 上层协议实体。
 */
typedef void (* IfLlOnSendRequest)(void* upo);

/**
 * @brief Ll 层处理器接口。
 * @details 由逻辑链路层协议实体实现，Ll 层接口实体调用。
 */
typedef struct ILlHandleStruct
{
    IfLlOnReceived OnReceived;
    IfLlOnSendRequest OnSendRequest;
}
ILlHandle;


//*******************************************************************
// LlComm 接口定义

/**
 * @brief Ll 实体错误统计。
 * @details 统计Ll 协议实体的运行情况。
 */
typedef struct LlStatisticsStruct
{
    unsigned int sendFrames;            ///< 发送帧统计
    unsigned int recvFrames;            ///< 接收帧统计
    unsigned int outOfFrameBytes;       ///< 接收的帧外数据计数
    unsigned char outOfRamError;        ///< 内存分配失败（不足）的错误数
}
LlStatistics;

/**
 * @brief Ll实体基础数据结构，每个具体的Ll实体都必须在结构初始处定义它。
 */
typedef struct LlStruct
{
    ILaiHandle laiHandle;               ///< 下层处理接口，必须在结构初始化处定义。具体实现时必须初始化它
    ILlComm * comm;                     ///< 本层通信接口，具体实现时必须初始化它
    Lai* lai;                           ///< 本层所使用的底层协议对象
    ILlHandle* handles[LL_UAP_SIZE];    ///< 注册的上层处理器，下标即为 uap，未注册的 uap 应为空
    LinkAddress address;                ///< 本链路节点的地址
    LlStatistics commStat;              ///< 见： @ref LlStatistics
}
Ll;

extern void Ll_Setup(Ll* self, Lai* laiToUse, LinkAddress local);
extern void Ll_Register(Ll* self, Uint8 uap, ILlHandle* handle);


#ifdef __cplusplus
}
#endif

#endif  // DNCP_LL_LL_H_

/** @} */
