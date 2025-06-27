/**
 * @addtogroup module_DNCP
 * @{
 */

/**
 * @file
 * @brief 逻辑链路回调接口。
 * @details LLCI（Logic Link Callback Interface）下面不设Lai实体，
 *  通过函数回调的形式，进行直接的本地数据包交换。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2013-3-12
 */

#include <string.h>

#include "Llci.h"

//*******************************************************************
// DSLP 实现

static void Llci_Reset(Llci* self, Uint16 action);
static Bool Llci_IsSendable(Llci* self);
static void Llci_Send(Llci* self, Uint8 npn, Uint8 destAddr, Frame* frame);

static const ILlComm s_kCommInterface =
{
        (IfLlReset) Llci_Reset,
        (IfLlIsSendable) Llci_IsSendable,
        (IfLlSend) Llci_Send
};

void Llci_Reset(Llci* ll, Uint16 action)
{
}

Bool Llci_IsSendable(Llci* ll)
{
    return TRUE;
}

void Llci_Send(Llci* self, Uint8 npn, Uint8 destAddr, Frame* frame)
{
    ILlHandle* handle;

    // 本地回调，不需要检验帧的完整性

    /// @todo 可能造成多线程安全问题
    // 直接扔到对端的接口上，调用对端接口的回调处理函数
    handle = self->oppo->handles[npn];
    if (handle)
    {
        handle->OnReceived(handle, frame, (Uint8) self->base.address);
    }
}


//*******************************************************************
// DSLP 实现相关


/**
 * @brief Llci 协议实体初始化。
 * @param self 自身对象。
 * @param laiToUse 协议使用的Lai实体，该对象必须已经被初始化。
 */
void Llci_Init(Llci* self, Llci* opposite)
{
    memset(self, 0, sizeof(Llci));

    // 初始化各接口
    self->base.comm = (ILlComm*) &s_kCommInterface;

    self->oppo = (Ll*) opposite;
}

/**
 * @brief Llci 协议实体结束化，释放相关资源。
 */
void Llci_Uninit(Llci* self)
{
}


/** @} */
