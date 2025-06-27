/**
 * @addtogroup module_DncpStack
 * @{
 */

/**
 * @file
 * @brief DNCP 协议栈。
 * @details 为设备构建一个静态全局的协议栈栈体。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2012-12-21
 */

#ifndef DNCP_STACK_DNCP_STACK_H_
#define DNCP_STACK_DNCP_STACK_H_

#include "Common/Types.h"
#include "DNCP/App/Dscp.h"

#ifdef __cplusplus
extern "C" {
#endif
    
/**
 * @brief 设备地址。
 * @details DSCP设备地址，属于DNCP网络的下行地址。
 */
typedef struct DeviceAddressStruct
{
    unsigned char d1;
    unsigned char d2;
    unsigned char d3;
    unsigned char d4;
}
DeviceAddress;

    
/**
 * @brief 初始化协议栈。
 * @param[in] ucLabel 上接的单控编号，即挂接在哪个单控下面。
 * @param[in] port 熔配上行接口的本地端口号。
 * @return 是否成功初始化。
 *  只有初始化成功，才能继续使用协议栈收发数据。
 */
extern Bool DncpStack_Init();
  
/**
 * @brief 结束协议栈。
 * @note 不使用协议栈时，必须结束它，以释放系统资源。
 */
extern void DncpStack_Uninit(void);

/**
 * @brief DSCP 命令队列处理函数。
 * @note 需要在后台中定时调用。
 */
extern void DncpStack_HandleDscpCmd(void);


extern void DncpStack_SetDestAddr(NetAddress destAddr);
extern Bool DncpStack_SendResp(RespCode resp, void* data, Uint16 len);
extern Bool DncpStack_SendStatus(RespCode resp, StatusCode status);
extern Bool DncpStack_SendEvent(EventCode event, void* data, Uint16 len);
extern void DncpStack_BufferEvent(EventCode event, void* data, Uint16 len);
extern void DncpStack_ClearBufferedEvent(void);
#ifdef __cplusplus
}
#endif

#endif  // DNCP_STACK_DNCP_STACK_H_

/** @} */
