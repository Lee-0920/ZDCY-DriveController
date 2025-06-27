/*
 * AnalogControl.c
 *
 *  Created on: 2020年5月22日
 *      Author: Administrator
 */
/*
 * AnalogControl.c
 *
 *  Created on: 2020年5月20日
 *      Author: Administrator
 */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "AnalogControl.h"
#include "Driver/System.h"
#include "Driver/System.h"
#include "SystemConfig.h"
#include "Tracer/Trace.h"
#include "DncpStack/DncpStack.h"
#include "LuipApi/AnalogControlInterface.h"
#include "Driver/AnalogDriver/AnalogDriver.h"

#define MAX_AI_DATA_BUFFER_NUM   16
#define AINumber  4
//采集AD滤波参数
#define FILTER_BUFFER_LEN   10
#define FILTER_HEAD_LEN  1
#define FILTER_TAIL_LEN    1

//采集任务间隔时间
#define COLLECT_WAIT_MS  500
//读写锁等待时间
#define MUTEX_WAIT_MS   COLLECT_WAIT_MS*2

typedef struct
{
    Uint32 data[MAX_AI_DATA_BUFFER_NUM];
    float currentTemp;
    TaskHandle_t analogControllerTask;
    Uint32 uploadPeriodMs;
    TimerHandle_t analogUploadTimer;
    SemaphoreHandle_t collectSemaphore;
}AnalogController;

static AnalogController s_analogController;

static void AnalogController_AIUploadTimerHandle(TimerHandle_t argument);
static void AnalogController_AICollectTask(void* argument);
static void WaterCount_TestTask(void* argument);
static Uint32 AnalogController_FilterData(Uint16 *inputData, Uint16 count, Uint16 filterHigh, Uint16 filterLow);
static void AnalogController_BubbleSort(Uint16 *dataBuff, Uint16 count);

static Bool s_startFlag = FALSE;
static Uint32 s_count = 0;
static Uint32 s_startFlow = 0;

void AnalogController_Init(void)
{
	AnalogSignal_ADC1_Init();

    memset(s_analogController.data, 0, sizeof(s_analogController.data));
    s_analogController.uploadPeriodMs = 5000;

    vSemaphoreCreateBinary(s_analogController.collectSemaphore);
    			//模拟信号采集任务
    xTaskCreate(AnalogController_AICollectTask, "AICollect",
            ANALOGCONTROLLER_AI_COLLECT_STK_SIZE, (void *)&s_analogController,
            ANALOGCONTROLLER_AI_COLLECT_TASK_PRIO, &s_analogController.analogControllerTask);

    xTaskCreate(WaterCount_TestTask, "WaterCount_TestTask", 128, NULL,5, NULL);

    			//模拟信号周期上传定时器
    s_analogController.analogUploadTimer = xTimerCreate("AnalogUpload",
            (uint32_t) (s_analogController.uploadPeriodMs / portTICK_RATE_MS), pdTRUE, (void *) ANALOGCONTROLLER_AI_UPLOAD_TIMER_PRIO,
            AnalogController_AIUploadTimerHandle);
    xTimerStart(s_analogController.analogUploadTimer, 0);
}


static void WaterCount_TestTask(void* argument)
{
	while(1)
	{
		if(s_startFlag)
		{
			s_count++;
		}
		vTaskDelay(6000 / portTICK_RATE_MS);
	}
}

/**
 * 对数据进行排序(冒泡法)
 * @param 数据
 * @param 数据个数
 */
void AnalogController_BubbleSort(Uint16 *dataBuff, Uint16 count)
{
    Uint16 i    = 0;
    Uint16 j    = 0;
    Uint16 temp = 0;

    for(i = 1; i < count; i++)
    {
        for(j = count - 1; j >= i; j--)
        {
            if(dataBuff[j] < dataBuff[j-1])
            {
                temp = dataBuff[j-1];
                dataBuff[j-1] = dataBuff[j];
                dataBuff[j] = temp;
            }
        }
    }
}

/**
 * @brief 对数据进行滤波处理，去掉最大的和最小的数据
 * @param  要进行滤波处理的数据
 * @param  数据个数
 * @param  去掉高端数据个数
 * @param  去掉低端数据个数
 * @return 滤波之后的数据
 */

Uint32 AnalogController_FilterData(Uint16 *inputData, Uint16 count, Uint16 filterHigh, Uint16 filterLow)
{
    Uint16 i = 0;
    uint16_t avgData = 0;
    uint32_t sumData = 0;

    // 冒泡
    AnalogController_BubbleSort(inputData, count);

    // 过滤
    memcpy(inputData, (inputData + filterLow), (count - filterLow) * sizeof(Uint32));
    count -= (filterHigh + filterLow);

    // 取平均值
    for (i = 0; i < count; i++)
    {
        sumData += inputData[i];
    }

    avgData = sumData / count;
    TRACE_MARK("%d ", avgData);

    return avgData;
}

void AnalogController_AICollectTask(void* argument)
{
	Uint8 i=0;
	static Uint16 buffer[FILTER_BUFFER_LEN] = {0};
	AnalogController *analogController = (AnalogController *)argument;

	while(1)
	{
		xSemaphoreTake(analogController->collectSemaphore, portMAX_DELAY);
		TRACE_MARK("\nAI data : ");
		memset(buffer, 0, FILTER_BUFFER_LEN*sizeof(Uint16));

		for(i=0; i<FILTER_BUFFER_LEN; i++)
		{
			buffer[i] = Get_ADC1(7);
		}
		analogController->data[0] = AnalogController_FilterData(buffer, FILTER_BUFFER_LEN, FILTER_HEAD_LEN, FILTER_TAIL_LEN);

		System_DelayUs(10);

		for(i=0; i<FILTER_BUFFER_LEN; i++)
		{
			buffer[i] = Get_ADC1(15);
		}
		analogController->data[1] = AnalogController_FilterData(buffer, FILTER_BUFFER_LEN, FILTER_HEAD_LEN, FILTER_TAIL_LEN);

		xSemaphoreGive(analogController->collectSemaphore);
		System_Delay(COLLECT_WAIT_MS);
	}
}

void AnalogController_AIUploadTimerHandle(TimerHandle_t argument)
{
	Uint8 data[20];
	Uint16 num = AnalogController_GetAINumber();
	s_analogController.data[2] = s_count;
	s_analogController.data[3] = s_startFlow;
	xSemaphoreTake(s_analogController.collectSemaphore, MUTEX_WAIT_MS);

	memcpy(data, &num, sizeof(Uint16));
	memcpy(data+sizeof(Uint16), s_analogController.data, sizeof(Uint32)*num);

	xSemaphoreGive(s_analogController.collectSemaphore);

	DncpStack_SendEvent(DSCP_EVENT_ACI_AI_UPLOAD, data, sizeof(Uint16)+4*sizeof(Uint32));
}

Uint16 AnalogController_GetAINumber(void)
{
    return AINumber;
}

void AnalogController_StartFlow(void)
{
	Printf("\nWater Flow Start");
	s_startFlow = 1;
	s_count = 0;
	s_startFlag = TRUE;
}

void AnalogController_StopFlow(void)
{
	Printf("\nWater Flow Stop");
	s_startFlow = 0;
	s_count = 0;
	s_startFlag = FALSE;
}

Uint16 AnalogController_GetAIData(Uint16 index)
{
    Uint32 ad = 0;
    if(index < AINumber)
    {
        ad = s_analogController.data[index];
    }
    else
    {
        TRACE_ERROR("\nInvalid ad channel %d", index);
    }

    return ad;
}

Bool AnalogController_GetAllAIData(Uint16* num, Uint32* data)
{

	*num = AINumber;

    xSemaphoreTake(s_analogController.collectSemaphore, MUTEX_WAIT_MS);

    memcpy(data, s_analogController.data, (*num)*sizeof(Uint32));

    xSemaphoreGive(s_analogController.collectSemaphore);

    return TRUE;
}

void AnalogController_SetAIUploadPeriod(Uint32 time)
{
    TRACE_INFO("\n Set AI upload period = %d s", time);
    s_analogController.uploadPeriodMs = time*1000;
    if (s_analogController.uploadPeriodMs > 0)
    {
        xTimerChangePeriod(s_analogController.analogUploadTimer, s_analogController.uploadPeriodMs/ portTICK_RATE_MS,  0);
        xTimerStart(s_analogController.analogUploadTimer, 0);
    }
    else
    {
        xTimerStop(s_analogController.analogUploadTimer, 0);
    }
}



