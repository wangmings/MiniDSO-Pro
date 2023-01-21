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

#define SAMPLE_NUM 220      //���������������ʾ������101���㣬���Ҵ�����ӵ�51���㵽��SAMPLE_NUM-50���㣬
#define PRE_BUF_NUM 62      //���δ�������ͨ������Ԥ����������ڵ���52
#define AFT_BUF_NUM 158     //���δ�������ͨ�����Ĵ����󻺴��������SAMPLE_NUM-PRE_BUF_NUM


#define VBAT_UPDATE_FREQ 40 //��ص�ѹ��Ϣ�ĸ���Ƶ�� VBAT_UPDATE_FREQ*25ms
#define VBAT_LSB 590        //��ص�ѹ������ѹϵ��*100
#define CHX_VBAT_LSB 460    //��ص�ѹ������ѹϵ��*100
#define INT_0 0             //�ⲿ�ж�0��ţ���������ת
#define INT_1 2             //�ⲿ�ж�1��ţ�����������
#define TIMER_0 1           //��ʱ��0�ж����

/**
 * @brief   ������ʽ��
 * @details Method of measurement.
**/
typedef enum
{
    Option_MeaWay_DC = 0x00,  /*!< ����ֱ����Measure the DC. */
    Option_MeaWay_AC = 0x01,  /*!< ����������Measurement of AC. */
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
 * @brief   ����������Ļ���ε�������С��ѹmV�����ޡ�327675mV��
 * @details The maximum and minimum voltage of the corrected screen waveform is mV, 
 *          and the upper limit is ��327675mV.
**/
extern int32 VMax_m, VMin_m;


/**
 * @brief   ������ʽ��
 * @details Method of measurement.
**/
extern Option_InMain_MeaWay_t MeaWay;


/**
 * @brief   �������Ĳ�����ѹ������������ֱ���ͽ�����
 * @details Corrected measurement voltage, positive and negative, DC and AC.
**/
extern int32 Mvoltage;

/**
 * @brief   ƫ�õ�ѹ��
 * @details Bias voltage.
**/
extern int32 Bias_Voltage;

#endif