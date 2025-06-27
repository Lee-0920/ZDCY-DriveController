/**
 * @addtogroup module_DNCP
 * @{
 */

/**
 * @file
 * @brief 链路适配接口基础定义。
 * @details 链路适配接口通用操作的实现。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2012-12-20
 */

#include "Lai.h"

/**
 * @brief 配置本层协议的基本参数。
 * @details 本机地址仅对共享总线的通信链路有效，其它链路可以不使用该地址。
 * @param[in] self 要操作的Lai实体对象。
 * @param[in] local 本机的链路地址。
 */
void Lai_Setup(Lai* self, LinkAddress local)
{
    self->address = local;
}

/**
 * @brief 注册上层协议（Lai处理器）。
 * @param[in] self 要操作的Lai实体对象。
 * @param[in] handle Lai处理器。
 */
void Lai_Register(Lai* self, ILaiHandle* handle)
{
    self->handle = handle;
}


/**
 * @brief 设置一次能连续传输的最大帧数。
 * @details 用于控制传输过程，仅主从链路需要实现。
 * @param[in] self 要操作的Lai实体对象。
 * @param[in] num 最大传输的帧数，系统最多只支持255，默认为0，表示一直传输直到无帧待发。
 */
void Lai_SetMaxTransNum(Lai* self, Uint8 num)
{
    self->maxTransmitUnit = num;
}


/** @} */
