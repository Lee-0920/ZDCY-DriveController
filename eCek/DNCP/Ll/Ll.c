/**
 * @addtogroup module_DNCP
 * @{
 */

/**
 * @file
 * @brief 逻辑链路层抽象接口定义。
 * @details 逻辑链路层通用操作的实现。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2012-12-20
 */

#include "Ll.h"

/**
 * @brief 配置本层协议的基本参数。
 * @details 本层的具体实现依赖于底层协议体提供的通信接口。
 * @param[in] self 要操作的Ll实体对象。
 * @param[in] laiToUse 本协议要使用的底层协议对象。
 * @param[in] local 要设置的本机地址。
 */
void Ll_Setup(Ll* self, Lai* laiToUse, LinkAddress local)
{
    self->lai = laiToUse;
    self->address = local;
}

/**
 * @brief 为特定的上层访问点注册上层协议（Ll处理器）。
 * @param[in] self 要操作的Ll实体对象。
 * @param[in] uap 上层访问点，接收到的帧通过该标识给对应的上层协议体分发帧。
 *            受限于LL层协议规范， uap 只能取值为 0 - 3
 * @param[in] handle 上层访问点对应的Ll处理器。
 */
void Ll_Register(Ll* self, Uint8 uap, ILlHandle* handle)
{
    self->handles[uap] = handle;
}


/** @} */
