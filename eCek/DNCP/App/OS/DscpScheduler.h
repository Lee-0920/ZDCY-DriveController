/*
 * DNCPScheduler.h
 *
 *  Created on: 2016年5月3日
 *      Author: Administrator
 */

#ifndef DNCP_PORT_OS_DSCPSCHEDULER_H_
#define DNCP_PORT_OS_DSCPSCHEDULER_H_

#ifdef __cplusplus
extern "C"
{
#endif

void DscpScheduler_Init(DscpDevice* dscp);
void DscpScheduler_Active(void);

#ifdef __cplusplus
}
#endif

#endif /* DNCP_PORT_OS_DSCPSCHEDULER_H_ */
