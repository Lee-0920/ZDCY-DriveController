/**
 * @addtogroup module_Manufacture
 * @{
 */

/**
 * @file
 * @brief 产品的版本相关信息。
 * @details 提供软件版本号、硬件版本号、BOM版本号支持。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2015-3-19
 */

#ifndef MANUFACTURE_VERSION_INFO_H_
#define MANUFACTURE_VERSION_INFO_H_

#include "ManufTypes.h"

//static const char softwarelabel[64]="STD-[SVN Revision]-[2000.01.01]";


extern ManufVersion VersionInfo_GetSoftwareVersion(void);
extern ManufVersion VersionInfo_GetHardwareVersion(void);
extern const char* VersionInfo_GetSoftwareLabel(void);

#endif  // MANUFACTURE_VERSION_INFO_H_

/** @} */
