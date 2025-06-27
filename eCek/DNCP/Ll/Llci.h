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

#ifndef DNCP_LL_LLCI_H_
#define DNCP_LL_LLCI_H_

#include "DNCP/Base/Frame.h"
#include "DNCP/Ll/LinkFrame.h"
#include "DNCP/Ll/Ll.h"

#ifdef __cplusplus
extern "C" {
#endif


//*******************************************************************
// Llci 定义

/**
 * @brief Llci 协议实体对象。
 */
typedef struct LlciStruct
{
    Ll base;                ///< 实现Lai实体的基本数据结构

    // Llci实现相关
    Ll* oppo;               ///< 对端通信接口
}
Llci;


//*******************************************************************
// DSCP 实现相关

extern void Llci_Init(Llci* self, Llci* opposite);
extern void Llci_Uninit(Llci* self);


#ifdef __cplusplus
}
#endif

#endif  // DNCP_LL_LLCI_H_

/** @} */
