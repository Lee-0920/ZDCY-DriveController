/*
 * TempCollecterMap.c
 *
 *  Created on: 2017年11月20日
 *      Author: LIANG
 */

#include "TempCollecterMap.h"
#include "Tracer/Trace.h"
#include "Driver/System.h"
#include <string.h>

#define TEMPADCOLLECT_AD_MAX  (4095)    // 24位AD
#define TEMPADCOLLECT_V_REF  (2.484)       // 单位(V)
#define TEMPAD_ENLARGEMENT_FACTOR 3.205  //AD放大倍数

//温度校准系统初始化参数
const static TempCalibrateParam s_kTempCalculateParam1 =
{ .negativeInput = 1.488, .vref = 2.4801, .vcal = 0, };

const static TempCalibrateParam s_kTempCalculateParam2 =
{ .negativeInput = 1.9905, .vref = 2.5001, .vcal = 0, };

static double TempCollecterMap_GetResistanceValue1(TempCalibrateParam *tempCalibrateParam, Uint16 ad);
static double TempCollecterMap_GetResistanceValue2(TempCalibrateParam *tempCalibrateParam, Uint16 ad);

void TempCollecterMap_Init(TempCollecter *tempCollecter)
{
    tempCollecter[MEAROOM_TEMP].getResistanceValueFunc = TempCollecterMap_GetResistanceValue1;
    TempCollecter_SetNumber(&tempCollecter[MEAROOM_TEMP], MEAROOM_TEMP);
    TempCollecter_Init(&tempCollecter[MEAROOM_TEMP], s_kTempCalculateParam1);
}

static double TempCollecterMap_GetResistanceValue1(TempCalibrateParam *tempCalibrateParam, Uint16 ad)
{
    double realV = ad * TEMPADCOLLECT_V_REF / TEMPADCOLLECT_AD_MAX; //根据AD值计算得到电压值
    double rt;
    if (realV >= 2.5)//超出PT1000计算范围
    {
        return 2000;
    }
    rt = ((tempCalibrateParam->vref - tempCalibrateParam->negativeInput)
            / (tempCalibrateParam->negativeInput - realV / TEMPAD_ENLARGEMENT_FACTOR)) * 1000;

    TRACE_DEBUG(" way 1 ad: %d ;", ad);
    TRACE_DEBUG("realV: ");
    System_PrintfFloat(TRACE_LEVEL_CODE, realV, 8);
    TRACE_DEBUG(" ; Rt: ");
    System_PrintfFloat(TRACE_LEVEL_CODE, rt, 8);
    TRACE_DEBUG(" ; Temp:");

    return rt;
}

static double TempCollecterMap_GetResistanceValue2(TempCalibrateParam *tempCalibrateParam, Uint16 ad)
{
    double realV = ad * TEMPADCOLLECT_V_REF / TEMPADCOLLECT_AD_MAX; //根据AD值计算得到电压值
    double rt;

    rt = (realV + 3.205 * tempCalibrateParam->negativeInput) * 150
            / (3.205 * (tempCalibrateParam->vref
                            - tempCalibrateParam->negativeInput) - realV);


    TRACE_DEBUG(" way 2 ad: %d ;", ad);
    TRACE_DEBUG("realV: ");
    System_PrintfFloat(TRACE_LEVEL_CODE, realV, 8);
    TRACE_DEBUG(" ; Rt: ");
    System_PrintfFloat(TRACE_LEVEL_CODE, rt, 8);
    TRACE_DEBUG(" ; Temp:");

    return rt;
}

char* TempCollecterMap_GetName(Uint8 index)
{
    static char name[20] = "";
    memset(name, 0, sizeof(name));
    switch(index)
    {
    case MEAROOM_TEMP:
        strcpy(name, "MeasureRoom");
        break;
    case BACTROOM_TEMP:
        strcpy(name, "BacteriaRoom");
        break;
    case MEAROOM_OUT_TEMP:
        strcpy(name, "MeasureRoomOut");
        break;
    case BACTROOM_OUT_TEMP:
        strcpy(name, "BacteriaRoomOut");
        break;
    default:
        strcpy(name, "NULL");
        break;
    }
    return name;
}
