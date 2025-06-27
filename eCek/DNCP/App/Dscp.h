/**
 * @addtogroup module_DNCP
 * @{
 */

/**
 * @file
 * @brief 设备简单控制协议 DSCP 的公共定义。
 * @details 
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2013-2-26
 */

#ifndef DNCP_APP_DSCP_H_
#define DNCP_APP_DSCP_H_

#include "Common/Types.h"
#include "DNCP/Net/NetPack.h"

#ifdef __cplusplus
extern "C" {
#endif


//*******************************************************************
// DSCP 包头定义

// DSCP 包类型定义
#define DSCP_TYPE_CMD               0
#define DSCP_TYPE_RESP_INFO         1
#define DSCP_TYPE_RESP_STATUS       2
#define DSCP_TYPE_EVENT             3

/**
 * @brief DSCP 控制字。
 */
typedef struct DscpCtrlWordStruct
{
    unsigned char type       : 2;        ///< DSCP包类型，可以是命令、回应或事件，如： @ref DSCP_TYPE_CMD
    unsigned char needFollow : 1;        ///< 该数据包之后是否还有数据包跟随。
    unsigned char reserved   : 5;        ///< 保留，暂时不用
}
DscpCtrlWord;



//*******************************************************************
// DSCP 应用接口的数据结构定义

typedef Uint16 DscpCode;        ///< 抽象码值类型。
typedef Uint16 CmdCode;         ///< 命令码类型。
typedef Uint16 RespCode;        ///< 回应码类型。
typedef Uint16 StatusCode;      ///< 状态码类型。
typedef Uint16 EventCode;       ///< 事件码类型。

/**
 * @brief DSCP 版本号
 */
typedef struct DscpVersionStruct
{
    unsigned char major;        ///< 主版本号
    unsigned char minor;        ///< 次版本号
    unsigned char revision;     ///< 修订版本号
    unsigned char build;        ///< 编译版本号
}
DscpVersion;


#ifdef __cplusplus
}
#endif

#endif  // DNCP_APP_DSCP_H_

/** @} */
