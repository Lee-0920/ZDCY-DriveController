/*
 * DigitalControl.h
 *
 *  Created on: 2020年5月21日
 *      Author: Administrator
 */

#ifndef SRC_DIGITALCONTROL_DIGITALCONTROL_H_
#define SRC_DIGITALCONTROL_DIGITALCONTROL_H_

void DigitalController_Init(void);
Uint16 DigitalController_GetDINumber(void);
Bool DigitalController_GetDIStatus(Uint16 index);
Uint32 DigitalController_GetDIStatusMap(void);
Uint16 DigitalController_GetDONumber(void);
Bool DigitalController_GetDOStatus(Uint16 index);
void DigitalController_SetDOStatus(Uint16 index, Bool status);
Uint32 DigitalController_GetDOStatusMap(void);
void DigitalController_SetDOStatusMap(Uint32 map);

#endif /* SRC_DIGITALCONTROL_DIGITALCONTROL_H_ */
