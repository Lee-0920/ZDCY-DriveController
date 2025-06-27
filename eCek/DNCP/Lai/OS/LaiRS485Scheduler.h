/*
 * DNCPScheduler.h
 *
 *  Created on: 2016年5月3日
 *      Author: Administrator
 */

#ifndef ECEK_DNCP_LAI_OS_LAIRS485SCHEDULER_H_
#define ECEK_DNCP_LAI_OS_LAIRS485SCHEDULER_H_

#ifdef __cplusplus
extern "C"
{
#endif

extern void LaiRS485Scheduler_Init(void);

extern void LaiRS485CommitToUpperTask_Active(void);
extern void LaiRS485SendRequestTask_Active(void);
extern void LaiRS485SendingTask_Active(void);
#ifdef __cplusplus
}
#endif

#endif /* ECEK_DNCP_PORT_OS_DNCPSCHEDULER_H_ */
