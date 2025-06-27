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

#include <string.h>
#include "TrpTail.h"

#include "Common/Utils.h"
#include "Common/SafeHeap.h"
#include "Common/MessageDigest.h"

//*******************************************************************
// TRP 包结构定义

#define TRP_HEADER_SIZE                 5

#define TRP_ROUTE_DIR_DOWNLINK          0
#define TRP_ROUTE_DIR_UPLINK            1

/**
 * @brief TRP 控制字。
 */
typedef struct TrpCtrlWordStruct
{
    unsigned char dir   : 1;        ///< 路由方向，见： @ref TRP_ROUTE_DIR_DOWNLINK 和 @ref TRP_ROUTE_DIR_UPLINK
    unsigned char seq   : 4;        ///< 期望或确认序号，用于维护滑动窗口
    unsigned char uap   : 3;        ///< 上层访问点
}
TrpCtrlWord;


//*******************************************************************
// ILlHandle 接口声明

static void TrpTail_OnReceived(TrpTail* self, Frame* frame, Uint8 sourceAddr);
static void TrpTail_OnSendRequest(TrpTail* self);


//*******************************************************************
// TRP 通信接口声明

static Bool TrpTail_IsSendable(TrpTail* self);
static void TrpTail_Send(TrpTail* self, Uint8 npn, NetAddress destAddr, Frame* frame);

//*******************************************************************
// 内部函数声明

static void TrpTail_TransmitBufferedFrame(TrpTail* self);


//*******************************************************************
// 实现

static const INetComm s_kCommInterface =
{
        (IfNetIsSendable) TrpTail_IsSendable,
        (IfNetSend) TrpTail_Send
};

Bool TrpTail_IsSendable(TrpTail* self)
{
    return (!self->isFrameBuffered);
}

void TrpTail_Send(TrpTail* self, Uint8 npn, NetAddress destAddr, Frame* frame)
{
    TrpCtrlWord tcw;
    Byte* data;

    // 把数据包缓冲起来
    self->isFrameBuffered = TRUE;
    self->frameToSend = frame;
    self->addrToSend = destAddr;

    // 封包由上层改为本层TRP封包
    frame->packOffset -= TRP_HEADER_SIZE;
    data = Frame_GetPackData(frame);

    // 处理 TRP 层头部
    tcw.dir = TRP_ROUTE_DIR_UPLINK;
    tcw.seq = 0;
    tcw.uap = npn;
    *data++ = *((Byte*) &tcw);

    // 填充数据包中的地址域，并提析目标链路地址
    frame->destAddr = 0;
    memcpy(data, &destAddr, sizeof(destAddr));

    // 尝试向下层接口发送包，失败则等待异步通知继续发送
    TrpTail_TransmitBufferedFrame(self);
}

// 把数据包转移到具体的底层链路接口（根据目换地址自动选路）
// 返回是否成功转移
void TrpTail_TransmitBufferedFrame(TrpTail* self)
{
    Ll* ll;
    ILlComm* comm;
    Frame* frame;

    // 被缓冲，尝试发送出去
    if (self->isFrameBuffered)
    {
        ll = self->ll;
        comm = ll->comm;
        frame = self->frameToSend;

        // 尝试向下层接口发送包，失败则等待异步通知继续发送
        if (comm->IsSendable(ll))
        {
            /// @todo 实现等停协议时，清除下面代码
            self->isFrameBuffered = FALSE;  // 已经发送完成
            comm->Send(ll, NET_PROTOCAL_TRP, frame->destAddr, frame);
            self->handle->OnSendReport(self->handle, TRUE);
        }
    }
}


//*******************************************************************
// TRP 初始化和配置


/**
 * @brief TrpTail 协议实体初始化。
 * @param self 自身对象。
 */
void TrpTail_Init(TrpTail* self)
{
    memset(self, 0, sizeof(TrpTail));

    // 初始化各接口
    self->base.llHandle.OnReceived = (IfLlOnReceived) TrpTail_OnReceived;
    self->base.llHandle.OnSendRequest = (IfLlOnSendRequest) TrpTail_OnSendRequest;
    self->base.comm = (INetComm*) &s_kCommInterface;

    self->isFrameBuffered = FALSE;
}

/**
 * @brief TrpTail 协议实体结束化，释放相关资源。
 */
void TrpTail_Uninit(TrpTail* self)
{
}


/**
 * @brief 配置本层协议的基本参数。
 * @param[in] self 要操作的Net实体对象。
 * @param[in] llToUse 使用的下层链路实体（接口）。
 */
void TrpTail_Setup(TrpTail* self, Ll* llToUse)
{
    self->ll = llToUse;
}

/**
 * @brief 为特定的上层访问点注册上层协议（Net处理器）。
 * @param[in] self 要操作的Net实体对象。
 * @param[in] uap 上层访问点，接收到的包通过该标识给对应的上层协议体分发包。
 *            受限于Net层协议规范， uap 只能取值为 0 - 7
 * @param[in] handle 上层访问点对应的Net处理器。
 */
void TrpTail_Register(TrpTail* self, Uint8 uap, INetHandle* handle)
{
    self->uap = uap;
    self->handle = handle;
}


//*******************************************************************
// ILlHandle 接口实现

void TrpTail_OnReceived(TrpTail* self, Frame* frame, Uint8 sourceAddr)
{
    TrpCtrlWord tcw;
    Byte* data;
    NetAddress addr;
    INetHandle* handle;

    // 包解析
    data = Frame_GetPackData(frame);
    *((Byte*) &tcw) = *data++;
    memcpy(&addr, data, sizeof(addr));

    // 路由到本机，向上层提交
    if (self->uap == tcw.uap)
    {
        handle = self->handle;

        // 封包由本层改为上层业务包
        frame->packOffset += TRP_HEADER_SIZE;
        handle->OnReceived(handle, frame, addr);
    }
}

void TrpTail_OnSendRequest(TrpTail* self)
{
    // 被缓冲，尝试发送出去
    if (self->isFrameBuffered)
    {
        TrpTail_TransmitBufferedFrame(self);
    }

    // 不需要通知上层，因为架构中，上层协议是通过查询的方式发送数据包的
}


/** @} */
