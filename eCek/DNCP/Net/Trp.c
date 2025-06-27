/**
 * @addtogroup module_DNCP
 * @{
 */

/**
 * @file
 * @brief 树网路由协议。
 * @details Trp 为上层提供无连接的、可靠的传输服务，并且实现包转发。
 *  - 下行路由
 *    - 地址结构：数据包地址字段的下行地址是目的地址，上行地址是源地址。
 *  - 上行路由
 *    - 地址结构：数据包地址字段的上行地址是目的地址，下行地址是源地址。
 *    - 上行源地址追补：在发送方向上发送数据包时，只填充了目标地址，而源地址
 *      （下行地址）为全零；中间结点的TRP层接收到上行包时，需要把底层链路的
 *      源地址填补到本机所处层级对应的下行地址域中。这样层层填补，数据包最终
 *      路由到目标节点时，即完成了源地址的填补，可清楚知道是谁发了这个包。
 *      注意待追补的域段为0才需要追补，非0表示下层节点已经设置了自己的地址，
 *      不需要额外的追补。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2013-2-1
 */

#include <string.h>
#include "Trp.h"

#include "Common/Utils.h"
#include "Common/SafeHeap.h"
#include "Common/MessageDigest.h"
#include "Tracer/Trace.h"

#include "FreeRTOS.h"
#include "semphr.h"

static xSemaphoreHandle s_semTrp;
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

static void Trp_OnReceived(Trp* self, Frame* frame, Uint8 sourceAddr);
static void Trp_OnSendRequest(Trp* self);


//*******************************************************************
// TRP 通信接口声明

static Bool Trp_IsSendable(Trp* self);
static void Trp_Send(Trp* self, Uint8 npn, NetAddress destAddr, Frame* frame);


//*******************************************************************
// 内部函数声明

static Bool Trp_TransmitBuffered2Interface(Trp* self);

//*******************************************************************
// 实现

static const INetComm s_kCommInterface =
{
        (IfNetIsSendable) Trp_IsSendable,
        (IfNetSend) Trp_Send
};

Bool Trp_IsSendable(Trp* self)
{
    return (!self->isFrameBuffered);
}

void Trp_Send(Trp* self, Uint8 npn, NetAddress destAddr, Frame* frame)
{
    unsigned char dir = TRP_ROUTE_DIR_DOWNLINK;
    Uint8 localLayer;
    Uint8 subAddr;
    TrpCtrlWord tcw;
    Byte* data;
    Byte* p;
    int i;

    TRACE_MARK("\n#TrpSend");

    // 把数据包缓冲起来
    self->isFrameBuffered = TRUE;
    self->frameHandle = self->handles[npn];
    self->frameToSend = frame;
    self->addrToSend = destAddr;

    if (NET_ADDRESS_IS_UPLINK(destAddr))
        dir = TRP_ROUTE_DIR_UPLINK;

    // 封包由上层改为本层TRP封包
    frame->packOffset -= TRP_HEADER_SIZE;
    data = Frame_GetPackData(frame);

    // 处理 TRP 层头部
    tcw.dir = dir;
    tcw.seq = 0;
    tcw.uap = npn;
    *data++ = *((Byte*) &tcw);

    // 填充数据包中的地址域，并提析目标链路地址
    if (dir == TRP_ROUTE_DIR_UPLINK)
    {
        frame->destAddr = 0;

        p = (Byte*)&destAddr;
        *data++ = *p;
        p = (Byte*)&(self->localAddr) + 1;

        TRACE_MARK("Up");
    }
    else
    {
        localLayer = UPLINK_ADDR_GET_LAYER(self->localAddr);
        // 解析要转发的下属节点的链路地址
        if (localLayer < 4)
        {
            subAddr = NetAddress_GetDownlinkField(destAddr, localLayer + 1);
            frame->destAddr = subAddr;
            // 下属节点地址不为零，才是真正需要发给下行接口，才需要做MASK修正
            if (subAddr)
            {
                NetAddress_SetDownlinkField(&self->addrToSend, localLayer + 1, subAddr - 1);
            }
        }

        p = (Byte*)&(self->localAddr);
        *data++ = *p;
        p = (Byte*)&destAddr + 1;

        TRACE_MARK("Down");
    }
    i = 3;
    while (i--)
    {
        *data++ = *p++;
    }

    // 尝试向下层接口发送包，失败则等待异步通知继续发送
    if (Trp_TransmitBuffered2Interface(self))
    {
        /// @todo 实现等停协议时，清除下面代码
        self->isFrameBuffered = FALSE;  // 已经发送完成
        // self->frameHandle->OnSendReport(self->frameHandle, TRUE);            // modify by weizhe
    }
    else
    {
//        TRACE_ERROR("Lost");
        self->isFrameBuffered = FALSE;
        self->base.commStat.lostPacks++;
        Frame_Delete(frame);
    }
}

// 把数据包转移到具体的底层链路接口（根据目的地址自动选路）
// 返回是否成功转移
Bool Trp_TransmitBuffered2Interface(Trp* self)
{
    Bool ret = FALSE;
    RouteStrategy* strategy;
    Ll* ll = NULL;
    int i;
    Frame* frame = self->frameToSend;
    NetAddress destAddr = self->addrToSend;
    Frame* frameNew;
    int sendCount;

    xSemaphoreTake(s_semTrp, portMAX_DELAY);

    // 根据目标地址，选择正确的底层（LL）接口来发送数据
    i = self->numOfStrategies;
    strategy = self->strategies;

    // 单播模式，一旦匹配立即退出
    if (0 == (self->routeMode & TRP_ROUTE_MODE_MULTICAST))
    {
        while (i--)
        {
            if ((strategy->subnetMask & destAddr) == strategy->subnetID)
            {
                ll = strategy->ll;
                break;
            }
            strategy++;
        }

        // 链路接口空闲则发送
        if (ll && ll->comm->IsSendable(ll) && self->isFrameBuffered == TRUE)
        {
            ret = TRUE;
            self->isFrameBuffered = FALSE;  // 已经发送完成
            TRACE_MARK("->%d", ll->address);
            ll->comm->Send(ll, NET_PROTOCAL_TRP, frame->destAddr, frame);
        }
    }

#ifndef _CS_TRP_ROUTING_DISABLE_MULTICAST
    // 多播模式，需要遍历完所有路由策略
    // 有些数据包（广播）可能被多次匹配而发送到多个接口，如上行事件
    else
    {
        sendCount = 0;
        Frame_AddRef(frame);    // 可能发往多个接口，增加引用，以避免被删掉
        frameNew = frame;
        while (i--)
        {
            if ((strategy->subnetMask & destAddr) == strategy->subnetID)
            {
                ll = strategy->ll;

                // 链路接口空闲则发送
                if (ll && ll->comm->IsSendable(ll))
                {
                    // 有发送过，需要重新分配数据包，以避免多个接口改写同一帧数据引起混乱
                    if (sendCount > 0)
                    {
                        frameNew = Frame_CopyNew(frame);
                    }
                    ret = TRUE;
                    self->isFrameBuffered = FALSE;  // 已经发送完成
                    ll->comm->Send(ll, NET_PROTOCAL_TRP, frameNew->destAddr, frameNew);
                    TRACE_MARK("->%d", ll->address);
                    sendCount++;
                }
            }
            strategy++;
        }
        Frame_Delete(frame);
    }
#endif

    xSemaphoreGive(s_semTrp);

    return ret;
}

//*******************************************************************
// TRP 初始化和配置


/**
 * @brief Trp 协议实体初始化。
 * @param self 自身对象。
 */
void Trp_Init(Trp* self)
{
    memset(self, 0, sizeof(Trp));

    // 初始化各接口
    self->base.llHandle.OnReceived = (IfLlOnReceived) Trp_OnReceived;
    self->base.llHandle.OnSendRequest = (IfLlOnSendRequest) Trp_OnSendRequest;
    self->base.comm = (INetComm*) &s_kCommInterface;

    self->isFrameBuffered = FALSE;

    vSemaphoreCreateBinary(s_semTrp);
}

/**
 * @brief Trp 协议实体结束化，释放相关资源。
 */
void Trp_Uninit(Trp* self)
{
}


/**
 * @brief 配置本层协议的基本参数。
 * @param[in] self 要操作的Trp实体对象。
 * @param[in] numOfInterface 底层使用的链路接口数目。
 * @param[in] numOfUpperHandles 上层协议体的数目，必须等于最大协议号加1。
 * @param[in] localAddr 要设置的本机的地址。
 *  <p> 需要同时指定上行地址和下行地址，可以使用这样的形式来设置：
 *      UPLINK_ADDR_MAKE(layer, obj) | DOWNLINK_ADDR_MAKE(d1, d2, d3, d4)
 * @param[in] downMask 下行层级掩码，仅作下行数据包选路用。
 *  downMask 的上行域应为0，下行域中，下层域段所有位都应为1，其余为0。
 *  如果下行数据包中的地址按位与上本掩码等于本机地址（ @p localAddr 中的下行地址域），
 *  说明该数据包是发往本节点的。
 * @note 配置时需要正确指定底层的接口数 @p numOfInterface 和上层的协议数
 *  @p numOfUpperHandles ，然后对于每一个接口和每一个协议，分别调用 
 *  Net_AddInterface() 和 Net_Register() 配置具体参数。
 */
void Trp_Setup(Trp* self, Uint8 numOfInterface, Uint8 numOfUpperHandles, NetAddress localAddr, NetAddress downMask)
{
    self->localAddr = localAddr;
    self->downMask = downMask;
    self->numOfStrategies = numOfInterface;
    self->numOfUpperHandles = numOfUpperHandles;

    self->strategies = (RouteStrategy*) 
        SafeHeap_Alloc(sizeof(RouteStrategy) * numOfInterface);
    self->handles = (INetHandle**) 
        SafeHeap_Alloc(sizeof(INetHandle*) * numOfUpperHandles);

    while (numOfUpperHandles--)
    {
        self->handles[numOfUpperHandles] = NULL;
    }
}

/**
 * @brief 添加链路接口，并配置数据包的路由策略。
 * @param[in] self 要操作的Trp实体对象。
 * @param[in] index 接口编号，0为基，不能超出 Net_Setup() 指定的接口数目。
 * @param[in] subnetID 目的子网段标识。
 *  <p> 上行接口的地址，仅上行域有效，其余为0；下行接口的地址，仅下行各域段有效，上行域为0。
 *  <p> 从数据包的地址域中抽取目的地址，先经过MASK减一修正，以方便路由决策的计算。
 *      （因为网络地址是1为基的，MASK算法是0为基的，所以需要MASK修正）。
 *      然后把MASK修正后的目的地址按位与上掩码 @p subnetMask ，如果等于本标识，
 *      那么数据包将路由到 @p ll 接口。注意抽取的目的地址仍为 NetAddress 类型，但源地址段
 *      无意义被置零，如上行数据包，下行地址段（源地址）被置零。
 * @param[in] subnetMask 子网段掩码。
 *  <p> 子网段掩码是屏蔽一个网络地址的各域段的全1比特标识，它不但能区分是上下行地址，
 *      还能区分下行各层级地址。如果同一层级还要区分不同的子网段，仍可通过掩码实现。
 *      注意上行接口subnetMask中的下行域应全为1；
 *      下行接口subnetMask中的上行域应全为1，下行域非下层段为全0。
 * @param[in] ll 路由策略（子网段+掩码）对应的链路接口。
 * @details 通过 @p subnetID 和 @p subnetMask 为标识一个LL接口的路由条件。例如：
 *  -  通用上行接口： subnetID=00:0.0.00.00，subnetMask=00:F.F.FF.FF （全部节点）
 *  - 第1层上行接口： subnetID=11:0.0.00.00，subnetMask=1F:F.F.FF.FF （标识为1的节点）
 *  - 第1层上行接口： subnetID=21:0.0.00.00，subnetMask=2F:F.F.FF.FF （标识为2的节点）
 *  - 第3层下行接口： subnetID=00:0.0.00.00，subnetMask=FF:0.0.00.00 （全部节点）
 *  - 第2层下行接口1：subnetID=00:0.0.00.00，subnetMask=FF:0.0.F8.00 （子网节点1-8）
 *  - 第2层下行接口2：subnetID=00:0.0.08.00，subnetMask=FF:0.0.F8.00 （子网节点9-16）
 *  - 第1层下行接口： subnetID=00:0.1.00.00，subnetMask=FF:0.F.00.00 （节点1）
 * @note 必须先调用 Net_Setup() 后再调用本函数。
 */
void Trp_AddInterface(Trp* self, Uint8 index, NetAddress subnetID, NetAddress subnetMask, Ll* ll)
{
    RouteStrategy* strategy = self->strategies + index;
    strategy->subnetID = subnetID;
    strategy->subnetMask = subnetMask;
    strategy->ll = ll;
}

/**
 * @brief 为特定的上层访问点注册上层协议（Net处理器）。
 * @param[in] self 要操作的Trp实体对象。
 * @param[in] uap 上层访问点，接收到的包通过该标识给对应的上层协议体分发包。
 *            受限于Net层协议规范， uap 只能取值为 0 - 7
 * @param[in] handle 上层访问点对应的Net处理器。
 * @note 必须先调用 Net_Setup() 后再调用本函数。
 */
void Trp_Register(Trp* self, Uint8 uap, INetHandle* handle)
{
    self->handles[uap] = handle;
}

/**
 * @brief 设置选路模式。
 * @param[in] self 要操作的Trp实体对象。
 * @param[in] mode 模式选项，多个模式可以用“|”操作符合并使用。
 *  <p>支持的模式选项有：
 *  - @ref TRP_ROUTE_MODE_MULTICAST 多播模式，选路时将遍历所有接口。
 *      该模式默认为关闭。
 */
void Trp_SetRouteMode(Trp* self, Uint8 mode)
{
    self->routeMode = mode;
}


//*******************************************************************
// ILlHandle 接口实现

void Trp_OnReceived(Trp* self, Frame* frame, Uint8 sourceAddr)
{
    TrpCtrlWord tcw;
    Bool isRouteHere = FALSE;
    Byte* data;
    NetAddress addr;
    NetAddress destAddr;
    INetHandle* handle;
    Uint8 localLayer = UPLINK_ADDR_GET_LAYER(self->localAddr);
    Uint8 subAddr;

    TRACE_MARK("\n#TrpRecv");

    // 包解析
    data = Frame_GetPackData(frame);
    *((Byte*) &tcw) = *data++;
    memcpy(&addr, data, sizeof(addr));

    // 判断是否路由到本机
    if (tcw.dir == TRP_ROUTE_DIR_UPLINK)
    {
        destAddr = NET_ADDRESS_UPLINK_GET(addr);
        if (UPLINK_ADDR_GET_LAYER(destAddr) == localLayer)
        {
            isRouteHere = TRUE;
        }

        // 链路地址指向主机
        frame->destAddr = 0;

        // 路由转发前，先进行源地址追补
        // 待追补的源地址段为0才需要追补，非0表示下层节点已经设置了自己的地址，不需要上层追补
        subAddr = NetAddress_GetDownlinkField(addr, localLayer + 1);
        if (subAddr == 0)
        {
            // 追补下属层的地址
            NetAddress_SetDownlinkField((NetAddress*)&addr, localLayer + 1, sourceAddr);
            memcpy(data, &addr, sizeof(addr));
        }
    }
    else
    {
        // 是否路由到本机
        destAddr = NET_ADDRESS_DOWNLINK_GET(addr);
        if ((destAddr & self->downMask) == NET_ADDRESS_DOWNLINK_GET(self->localAddr))
        {
            isRouteHere = TRUE;
        }
        else
        {
            // 解析要转发的下属节点的链路地址
            if (localLayer < 4)
            {
                subAddr = NetAddress_GetDownlinkField(destAddr, localLayer + 1);
                frame->destAddr = subAddr;
                // 下属节点地址不为零，才是真正需要发给下行接口，才需要做MASK修正
                if (subAddr)
                {
                    NetAddress_SetDownlinkField(&destAddr, localLayer + 1, subAddr - 1);
                }
            }
        }
    }

    if (isRouteHere)
    {
        TRACE_MARK("Here");

        // 提交前，需要进行源地址追补（顶层控制器）
        // 仅顶层结点需要，顶层控制器编号固定为1
        if (localLayer == 1)
        {
            NetAddress_SetDownlinkField((NetAddress*)&addr, 1, 1);
        }

        // 路由到本机，向上层提交
        handle = self->handles[tcw.uap];
        if (handle)
        {
            // 封包由本层改为上层业务包
            frame->packOffset += TRP_HEADER_SIZE;
            handle->OnReceived(handle, frame, addr);
        }
    }
    else
    {
        TRACE_MARK("Route%s", (tcw.dir == TRP_ROUTE_DIR_UPLINK) ? "Up" : "Down");

        /// @todo 需要增加转发缓冲队列，不然有问题
        self->isFrameBuffered = TRUE;
        self->frameToSend = frame;
        self->addrToSend = destAddr;

        // 尝试转发，失败则丢弃该包，说明底层接口的缓冲已经不足
        if (!Trp_TransmitBuffered2Interface(self))
        {
//            TRACE_ERROR("Lost");
            self->isFrameBuffered = FALSE;
            self->base.commStat.lostPacks++;
            Frame_Delete(frame);
        }
    }
}

void Trp_OnSendRequest(Trp* self)
{
    // 被缓冲，尝试发送出去
    if (self->isFrameBuffered)
    {
        // 尝试向下层接口发送包，失败则继续等待异步通知
        TRACE_MARK("\n#TrpTry");
        if (Trp_TransmitBuffered2Interface(self))
        {
            /// @todo 实现等停协议时，清除下面代码
            self->isFrameBuffered = FALSE;  // 已经发送完成
            self->frameHandle->OnSendReport(self->frameHandle, TRUE);
        }
    }
    else
    {
        self->frameHandle->OnSendReport(self->frameHandle, TRUE);
    }

    // 不需要通知上层，因为架构中，上层协议是通过查询的方式发送数据包的
}


/** @} */
