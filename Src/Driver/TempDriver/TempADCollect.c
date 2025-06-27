/**
 * @file
 * @brief 消解温度采集驱动。
 * @details 提供采集消解温度功能接口。
 * @version 1.1.0
 * @author kim.xiejinqiang
 * @date 2015-06-04
 */


#include "TempADCollect.h"
#include "SystemConfig.h"

#define ADC_CHANNEL_NUM           1
#define TEMP_COLLECT_COUNT        10                 // 采集AD次数
#define DATA_FILTER_NUM(x)       (x/5)               // 数据过滤比例
static vu16 s_ADCData[TEMP_COLLECT_COUNT][ADC_CHANNEL_NUM];

#define ADCx                      ADC2
#define ADCx_CLK                  RCC_APB2Periph_ADC2

#define ADCx_CHANNELA_GPIO_CLK    RCC_AHB1Periph_GPIOC

#define ADC_CHANNELA              ADC_Channel_14

#define GPIO_CHANNELA_PIN         GPIO_Pin_4
#define GPIO_CHANNELA_PORT        GPIOC

#define DMA_CHANNELx              DMA_Channel_1
#define DMA_STREAMx               DMA2_Stream2

static void TempADCollect_RCCConfiguration(void)
{
    RCC_AHB1PeriphClockCmd(ADCx_CHANNELA_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(ADCx_CLK, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

    RCC_APB2PeriphResetCmd(ADCx_CLK, ENABLE);
    RCC_APB2PeriphResetCmd(ADCx_CLK, DISABLE);
}

static void TempADCollect_GPIOConfiguration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

    GPIO_InitStructure.GPIO_Pin = GPIO_CHANNELA_PIN;
    GPIO_Init(GPIO_CHANNELA_PORT, &GPIO_InitStructure);
}

static void TempADCollect_DMAConfiguration(void)
{
    DMA_InitTypeDef DMA_InitStructure;

    DMA_DeInit(DMA_STREAMx);
    while (DMA_GetCmdStatus(DMA_STREAMx) != DISABLE)
        ; //等待 DMA 可配置

    DMA_InitStructure.DMA_Channel = DMA_CHANNELx; //设置 DMA 数据流对应的通道
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &ADC2->DR; //设置 DMA 传输的外设基地址
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) &s_ADCData; //存放 DMA 传输数据的内存地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory; //设置数据传输方向
    DMA_InitStructure.DMA_BufferSize = ADC_CHANNEL_NUM * TEMP_COLLECT_COUNT; //设置一次传输数据量
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //设置传输数据的时候外设地址是不变
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; //设置传输数据时候内存地址递增
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //设置外设的数据长度
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //设置内存的数据长度
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //设置 DMA 模式循环采集
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //设置 DMA 通道的优先级
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable; //是否开启 FIFO 模式
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull; //用来选择 FIFO 阈值
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single; //配置存储器突发传输配置
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single; //配置外设突发传输配置
    DMA_Init(DMA_STREAMx, &DMA_InitStructure);
    DMA_Cmd(DMA_STREAMx, ENABLE);
    while ((DMA_GetCmdStatus(DMA_STREAMx) != ENABLE))
    {
    }
}


//ADC 的转换时间可以由以下公式计算： Tcovn = 采样时间 + 12 个周期
// 0.045us * (480 + 12 + 20) = 23.04us。
static void TempADCollect_ADCConfiguration(void)
{
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;//独立模式
    ADC_CommonInitStructure.ADC_TwoSamplingDelay =
    		ADC_TwoSamplingDelay_20Cycles;//设置两个采样阶段之间的延迟周期数
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;//DMA 模式禁止
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div8;//预分频为8分频，180M / 8 = 22.5M , 0.045us 最好不要超过36M
    ADC_CommonInit(&ADC_CommonInitStructure);

    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;//12 位模式
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;//扫描模式
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;//连续转换
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//禁止触发检测，使用软件触发
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//右对齐
    ADC_InitStructure.ADC_NbrOfConversion = ADC_CHANNEL_NUM;//1 个转换在规则序列中
    ADC_Init(ADCx, &ADC_InitStructure);

    ADC_RegularChannelConfig(ADCx, ADC_CHANNELA, 1, ADC_SampleTime_480Cycles);
	
    ADC_DMARequestAfterLastTransferCmd(ADCx, ENABLE);

    ADC_DMACmd(ADCx, ENABLE);

    ADC_Cmd(ADCx, ENABLE);

    ADC_SoftwareStartConv(ADCx);//使能指定ADC软件转换启动功能
}
/**
 * @brief 消解温度采集初始化
 * @param
 */
void TempADCollect_Init(void)
{
    TempADCollect_RCCConfiguration();
    TempADCollect_DMAConfiguration();
    TempADCollect_GPIOConfiguration();
    TempADCollect_ADCConfiguration();
}

/**
 * @brief 采集更新温度AD值
 * @param
 */
static uint16_t TempADCollect_ADFilter(uint16_t *temperatureAD)
{
    uint32_t i, j = 0;
    uint32_t temp = 0;
    uint32_t sumdata = 0;
    uint32_t filternum = 0;

    filternum = DATA_FILTER_NUM(TEMP_COLLECT_COUNT);

    // 冒泡
    for (i = 1; i < TEMP_COLLECT_COUNT; i++)
    {
        for (j = TEMP_COLLECT_COUNT - 1; j >= i; j--)
        {
            if (temperatureAD[j] < temperatureAD[j - 1])
            {
                temp = temperatureAD[j - 1];
                temperatureAD[j - 1] = temperatureAD[j];
                temperatureAD[j] = temp;
            }
        }
    }

    // 取平均值
    for (i = filternum; i < TEMP_COLLECT_COUNT - filternum; i++)
    {
        sumdata += temperatureAD[i];
    }
    return (sumdata / (TEMP_COLLECT_COUNT - filternum * 2));
}


/**
 * @brief 获取消解温度AD。
 * @param
 */
uint16_t TempADCollect_GetAD(uint8_t index)
{
    uint16_t temperatureAD[TEMP_COLLECT_COUNT];
    for (uint32_t i = 0; i < TEMP_COLLECT_COUNT; i++)
    {
        temperatureAD[i] = s_ADCData[i][index];
    }
    return TempADCollect_ADFilter(temperatureAD);
}
