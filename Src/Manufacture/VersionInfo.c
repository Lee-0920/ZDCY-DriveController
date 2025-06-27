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

//#include "Driver/HardVersion.h"
#include "VersionInfo.h"

/**
 * @details 提供本软件的版本号。
 */
static const ManufVersion s_kSoftwareVersion =
{
        3,      // 主版本号
        0,      // 次版本号
        3,      // 修订版本号
        3      // 编译版本号
};
/**
 * @details 提供本PCB板的版本号。
 */
static const ManufVersion pcbVersion =
{
        1,      // 主版本号
        4,      // 次版本号
        0,      // 修订版本号
        0       // 编译版本号
};

/**
 * @details 提供软件标识。
 */
static const char s_softwarelabel[64]="STD";


/**
 * @brief 获取软件版本号。
 * @return 软件版本号。
 */
ManufVersion VersionInfo_GetSoftwareVersion(void)
{
    return s_kSoftwareVersion;
}

const char* VersionInfo_GetSoftwareLabel(void)
{
    return s_softwarelabel;
}
/**
 * @brief 获取硬件（电路）版本号。
 * @return 硬件版本号。
 */
ManufVersion VersionInfo_GetHardwareVersion(void)
{
    return pcbVersion;
}


/** @} */
