/**
 * @page page_Manufacture 生产支持
 *
 * 生产支持模块主要用于生产信息的管理，生产信息包括：
 * - 产品的版本信息，如固件版本、电路版本、BOM版本等
 * - 产品制造信息，如生产厂商、批次、序列号、UUID等
 * - 产品本身的管理信息，如产品ID、名称、型号等
 *
 * <br> <br>ManufInfo.h
 * @section sec_Manufacture_Function 功能描述
 *
 * 生产支持模块的功能为：
 * - 从存储介质中加载生产信息；
 * - 访问加载的生产信息；
 * - 设置具体的生产信息，然后保存；
 * - 能修改的信息存储需要做校验；
 *
 *
 * <br> <br>
 * @section sec_Manufacture_Design 设计说明
 *
 * 从应用的角度，生产信息包括两类：
 * - 只读信息，如各版本号、产品ID、名称等
 * - 可写信息，如序列号、UUID、产品型号等
 *
 * 从系统构架的角度，生产信息支持模块的层次结构为：
 * - 驱动层：在特定的存储介质上保存数据，并提供访问接口
 * - 应用层：生产信息的管理，如数据校验、具体生产信息的访问
 *
 * 综上，本模块包括以下几个子模块：
 * - VersionInfo：版本信息，用于管理产品的各个版本，供上层做差异化处理，只读
 * - ManufInfo：生产信息，用于控制生产的可追溯性，可读可写
 * - ProductInfo：产品信息，用于管理产品本身的信息，可读，某些信息可写（暂未实现）
 * - ManufAccesser：生产信息存储访问器，为生产信息的物理存储提供统一的接口。
 *   - 通常ManufAccesser管理一个可独立操作的存储单元，如 Flash 的一个Block，擦除的最小单位
 *
 * 上层模块调用时，对于只读的信息（VersionInfo），可直接调用其 Getor 函数获取需要的信息；
 * 对于可写的信息（ManufInfo、ProductInfo），在调用 Getor 之前需要先加载（Load），
 * 设置完可写的信息域后，也需要调用 Save 函数，把信息真正存储到介质中。
 *
 * 生产信息的存储介质使用 MSP430 内部的 information memory segment D.
 */


/**
* @defgroup module_Manufacture 生产支持
*/
