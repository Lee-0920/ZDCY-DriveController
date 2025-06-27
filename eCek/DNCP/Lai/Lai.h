/**
 * @addtogroup module_DNCP
 * @{
 */

/**
 * @file
 * @brief 链路适配接口基础定义。
 * @details 提供链路适配接口相关资源的统一定义。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2012-12-17
 */

#ifndef DNCP_LAI_LAI_H_
#define DNCP_LAI_LAI_H_

#include "Common/Types.h"
#include "DNCP/Ll/LinkFrame.h"

#ifdef __cplusplus
extern "C" {
#endif


//*******************************************************************
// 基本常数定义

#define FRAME_CHAR_SYNC             0x7E    ///< 帧同步符
#define FRAME_CHAR_ESC              0x7D    ///< 帧转义符
#define FRAME_CHAR_RAW_SYNC         0x5E    ///< 帧同步符的裸数据下半部
#define FRAME_CHAR_RAW_ESC          0x5D    ///< 帧转义符的裸数据下半部

#define FRAME_MAX_SIZE             255    ///< 最大帧长度

//*******************************************************************
// ILaiComm 接口定义

// 复位动作定义
#define LAI_TX_CLEAR                0x01    ///< 清空发送缓冲
#define LAI_RX_CLEAR                0x02    ///< 清空接收缓冲
#define LAI_TX_ABORT                0x04    ///< 中止当前发送操作
#define LAI_RX_ABORT                0x08    ///< 中止当前接收操作

/**
 * @brief ILaiComm 接口函数：复位Lai协议状态逻辑。
 * @details 当上层协议体因为通信异常（如等待超时）需要复位Lai实体时调用。
 *  在Lai协议体通信之前不能也没有必要调用本函数。
 * @param lai 具体的Lai层协议实体。
 * @param action 复位动作，可以是以下一个或多个动作的合并（或）：
 *        - @ref LAI_TX_CLEAR
 *        - @ref LAI_RX_CLEAR 
 *        - @ref LAI_TX_ABORT 
 *        - @ref LAI_RX_ABORT 
 * @note 实现时，应根据具体的底层协议，分别实现 @p action 指定的所有动作。
 */
typedef void (* IfLaiReset)(void* lai, Uint16 action);

/**
 * @brief ILaiComm 接口函数：查询接口是否可发送数据。
 * @details 通常在 Send 之前调用。
 *  <p>可发送表示：本实体可立即发送一帧新数据，此时上层可直接调用 Send()
 *      函数把数据发送出去；
 *  <p>不可发送表示：本实体正在发送数据，或者发送缓冲中无剩余空间可供等待发送。
 *      此时上层实体应该等待 OnSendRequest 的到来进行异步发送。
 *  <p>如果正在发送，意味着发送缓冲存在数据或者硬件正在发送数据，
 *      此时上层可等待异步的发送请求。
 * @param lai 具体的Lai层协议实体，如 LaiRs485 。
 */
typedef Bool (* IfLaiIsSendable)(void* lai);

/**
 * @brief ILaiComm 接口函数：发送一帧数据。
 * @details 请先调用 IsSendable() 查询发送缓冲是否足够，无剩余空间则应该等待
 *   OnSendRequest 的到来进行异步发送。
 * @param lai 具体的Lai层协议实体，如 LaiRs485 。
 * @param destAddr 本帧要发往的目标地址（链路地址）。
 * @param data 要发送的帧数据（逻辑帧的裸数据）。
 * @param len 要发送的帧长度。
 */
typedef void (* IfLaiSend)(void* lai, Uint8 destAddr, Byte* data, int len);


/**
 * @brief 链路适配层通信接口，仅支持异步调用。
 * @details 实现提示：
 *  <p>通信接口实体中至少需要一个能容纳最大长度帧的发送缓冲，用于保存待发送的帧数据。
 *      Send()调用只是把待发送的数据拷贝到缓冲中，并打开硬件模块开始发送流程。
 *      只有当发送缓冲中的所有数据都发送完后，IsSendable()才为真，否则为假。
 *  <p>为了效率考虑，发送缓冲也可以使用队列，以缓冲多帧数据。
 *      此时，IsSendable()实现可变通一下，即设置一个最大空闲门限，如4/8，那么，
 *      缓冲队列的空闲空间超过4/8进，IsSendable() 即为真，否则为假。
 */
typedef struct ILaiCommStruct
{
    IfLaiReset Reset;
    IfLaiIsSendable IsSendable;
    IfLaiSend Send;
}
ILaiComm;


//*******************************************************************
// ILaiHandle 接口定义

/**
 * @brief ILaiHandle 接口函数：收到新帧通知。
 * @details 当 Lai 层收到一帧数据时，通知上层实体及时处理新帧。
 * @param[in] ll 上层协议实体。
 * @param[in] data 接收到的帧数据（逻辑帧的裸数据）。
 * @param[in] len 接收到的帧长度。
 * @param[in] sourceAddr 源地址，即发送该帧的链路节点号。
 *  <p> 该参数最终将被传递到网络层，以实现上行源地址追补机制。除此之外，
 *      客户程序不要使用本参数实现其它功能。
 *  <p> 对于点对点链路，该地址应为1；对于点对多点的共享链路，
 *      主机实现时该地址是发帧节点的链路地址，从机实现时该地址无意义，
 *      可设为1。
 */
typedef void (* IfLaiOnReceived)(void* ll, Byte* data, Uint16 len, Uint8 sourceAddr);

/**
 * @brief ILaiHandle 接口函数：查询等待发送的帧数。
 * @details 如果Lai实体没有实现发送帧队列，可调用本函数预先计算待发送的帧数。
 *  通常用于对等网络中的令牌轮询策略。
 * @param[in] ll 上层协议实体。
 * @return 上层等待发送的帧数。
 */
typedef Uint16 (* IfLaiOnSendQuery)(void* ll);

/**
 * @brief ILaiHandle 接口函数：请求发送数据。
 * @details 发送缓冲中有剩余足够时，可向上层发出请求，通知上层发送新数据。
 <p>上层实现时，如果还有帧待发，可调用Send继续发送帧。
 * @param[in] ll 上层协议实体。
 */
typedef void (* IfLaiOnSendRequest)(void* ll);

/**
 * @brief Lai 层处理器接口。
 * @details 由逻辑链路层协议实体实现，Lai 层接口实体调用。
 */
typedef struct ILaiHandleStruct
{
    IfLaiOnReceived OnReceived;
    IfLaiOnSendQuery OnSendQuery;
    IfLaiOnSendRequest OnSendRequest;
}
ILaiHandle;


//*******************************************************************
// LaiComm 接口定义

/**
 * @brief Lai 实体错误统计。
 * @details 统计Lai 协议实体的运行情况。
 */
typedef struct LaiStatisticsStruct
{
    unsigned int sendFrames;            ///< 发送帧统计
    unsigned int recvFrames;            ///< 接收帧统计
    unsigned int outOfFrameBytes;       ///< 接收的帧外数据计数
    unsigned char outOfRamError;        ///< 内存分配失败（不足）的错误数
}
LaiStatistics;

/**
 * @brief Lai实体基础数据结构，每个具体的Lai实体都必须在结构初始处定义它。
 */
typedef struct LaiStruct
{
    ILaiComm * comm;                    ///< 本层通信接口，具体实现时必须初始化它
    ILaiHandle* handle;                 ///< 上层协议处理器
    LinkAddress address;                ///< 本地链路地址
    Uint8 maxTransmitUnit;              ///< 最大传输单元，仅主从链路有效
    LaiStatistics commStat;             ///< 通信统计对象
}
Lai;


extern void Lai_Setup(Lai* self, LinkAddress local);
extern void Lai_Register(Lai* self, ILaiHandle* handle);

extern void Lai_SetMaxTransNum(Lai* self, Uint8 num);


#ifdef __cplusplus
}
#endif

#endif  // DNCP_LAI_LAI_H_


/** @} */
