#ifndef CHART_H
#define CHART_H

#include "ssd1306.h"
#include "adc_stc8.h"
#include "settings.h"
#include "config_stc.h"

#ifndef uint8
#define uint8 unsigned char
#endif

#ifndef uint16
#define uint16 unsigned int
#endif

#ifndef int16
#define int16 int
#endif

#ifndef uint32
#define uint32 unsigned long int
#endif

/* 屏幕上波形显示区域
   Region for waveform on display*/
#define CHART_H_MAX 127
#define CHART_H_MIN 32
#define CHART_V_MAX 53
#define CHART_V_MIN 10

void PlotChart(void);
void PlotSettings(void);
void PlotSaveStatus(bit _saveStatus);
void GetWaveData(void);
bit GetTriggerPos(uint16 d1, uint16 d2, uint16 dTrigger, bit triSlope);
int32 Convert_mv_ADC(int32 mv, uint16 *BGV, uint16 ADCbg, uint16 lsb);
void AnalyseData();
void PlotWave();

#endif