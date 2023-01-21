/* All global variables defined here*/
#ifndef GLOBAL_H
#define GLOBAL_H

#ifndef uint8
#define uint8 unsigned char
#endif

#ifndef uint16
#define uint16 unsigned int
#endif

#ifndef uint32
#define uint32 unsigned long int
#endif

#ifndef int8
#define int8 signed char
#endif

#ifndef int16
#define int16 signed int
#endif

#ifndef int32
#define int32 long int
#endif

#define SAMPLE_NUM 220      //采样缓存点数，显示波形需101个点，查找触发点从第51个点到第SAMPLE_NUM-50个点，
#define PRE_BUF_NUM 62      //单次触发和普通触发的预缓存点数大于等于52
#define AFT_BUF_NUM 158     //单次触发和普通触发的触发后缓存点数等于SAMPLE_NUM-PRE_BUF_NUM


#define VBAT_UPDATE_FREQ 40 //电池电压信息的更新频率 VBAT_UPDATE_FREQ*25ms
#define VBAT_LSB 590        //电池电压采样分压系数*100
#define CHX_VBAT_LSB 460    //电池电压采样分压系数*100
#define INT_0 0             //外部中断0序号，编码器旋转
#define INT_1 2             //外部中断1序号，编码器按下
#define TIMER_0 1           //定时器0中断序号

/**
 * @brief   测量方式。
 * @details Method of measurement.
**/
typedef enum
{
    Option_MeaWay_DC = 0x00,  /*!< 测量直流。Measure the DC. */
    Option_MeaWay_AC = 0x01,  /*!< 测量交流。Measurement of AC. */
}Option_InMain_MeaWay_t;


extern uint16 VBat;
extern uint16 Lsb;
extern int16 TriLevel;
extern int16 TriLevel_m;
extern uint8 TriPos;
extern int8 TriPosOffset;
extern uint8 WaveLength;
extern uint32 WaveFreq;

extern int32 RulerVMin, RulerVMax;
extern int32 RulerVMin_m, RulerVMax_m;
extern int32 VMax, VMin;

extern uint16 *BGV;
extern uint16 ADCbg;
extern uint16 TriggerADC;
extern uint16 WaveLengthSumNum;
extern uint32 WaveLengthSum;
extern int8 ScaleH_tmp;
extern uint8 OLED_Brightness;
extern bit OLED_BrightnessChanged;
extern bit EC11PressAndRotate;
extern bit DisplayUpdate;
extern bit ClearDisplay;
extern bit ClearWave;
extern bit ADCRunning;
extern bit ADCInterrupt;
extern bit UpdateVbat;
extern bit TriFail;
extern bit PlotMode;
extern bit OptionChanged;
extern bit TriSlope;
extern bit InSettings;
extern bit WaveScroll;
extern bit WaveUpdate;
extern bit ScaleV_Auto;
extern bit ADCComplete;
extern int8 OptionInSettings;
extern int8 OptionInChart;
extern int8 ScaleH;
extern int8 TriMode;
extern uint16 ADCbuf[SAMPLE_NUM];
extern uint8 code *PlotModeTxt[];
extern uint8 code *TriModeTxt[];
extern uint8 code *ScaleHTxt[];


/**
 * @brief   修正过的屏幕波形的最大和最小电压mV，上限±327675mV。
 * @details The maximum and minimum voltage of the corrected screen waveform is mV, 
 *          and the upper limit is ±327675mV.
**/
extern int32 VMax_m, VMin_m;


/**
 * @brief   测量方式。
 * @details Method of measurement.
**/
extern Option_InMain_MeaWay_t MeaWay;


/**
 * @brief   修正过的测量电压，有正负，分直流和交流。
 * @details Corrected measurement voltage, positive and negative, DC and AC.
**/
extern int32 Mvoltage;

/**
 * @brief   偏置电压。
 * @details Bias voltage.
**/
extern int32 Bias_Voltage;

#endif