/**
 * @addtogroup module_DncpStack
 * @{
 */

/**
 * @file
 * @brief 设备命令表。
 * @details 命令表中保存了命令码和对应的处理函数。
 *  各应用相关的命令的入口函数都需要添加到表中，DNCP协议栈会根据接收到的
 *  命令自动调用相关的处理函数。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2013-3-19
 */

#ifndef DNCP_STACK_FT_CMD_TABLE_H_
#define DNCP_STACK_FT_CMD_TABLE_H_

#include "DNCP/App/DscpDevice.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 获取设备的DNCP命令表。
 */
extern DscpCmdTable DevCmdTable_GetTable(void);

/**
 * @brief 获取设备命令表的版本信息。
 */
extern DscpVersion DevCmdTable_GetVersion(void);


#ifdef __cplusplus
}
#endif

#endif  // DNCP_STACK_FT_CMD_TABLE_H_

/** @} */
