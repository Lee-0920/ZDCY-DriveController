/**
 * @addtogroup module_DNCP
 * @{
 */

/**
 * @file
 * @brief 应用层规范定义。
 * @details 统一定义应用层的共享资源。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2013-2-27
 */

#ifndef DNCP_APP_APP_H_
#define DNCP_APP_APP_H_

#include "Common/Types.h"
#include "DNCP/Base/Frame.h"
#include "DNCP/Ll/Ll.h"
#include "DNCP/Net/NetPack.h"

#ifdef __cplusplus
extern "C" {
#endif


//*******************************************************************
// 基本常数定义

// 应用层各协议号分配
#define APP_PROTOCAL_DSCP               0   // 简单设备控制协议


#ifdef __cplusplus
}
#endif

#endif  // DNCP_APP_APP_H_

/** @} */
