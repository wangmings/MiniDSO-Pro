/*
    chart.c

    Sampling Data Analyse and Plot Interface

    Copyright (c) 2020 Creative Lau (creativelaulab@gmail.com)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

    Note:
    2020-04-30����
    1. ������������ѹ����С��ѹֵ��֮ǰ��ʾ�������в������е�������Сֵ����������ʾΪ��Ļ��Χ�ڵ����ֵ����С�?
    2. ����λ�ô�֮ǰ�Ĺ̶������е���?�û��Զ���
    3. ���Ӽ��㲨��Ƶ�ʺ���
*/
#include "chart.h"
#include "bmp.h"
#include "math.h"


uint16 *pADCSampling;       //ָ��ADCʵʱ������ָ��
int32 waveBuf[SAMPLE_NUM]; //��������������Ĳ�������?
int32 triPre;               //����λ��ǰ����ֵ��
int32 triAft;               //����λ�ú󷽵�ֵ��
int32 plotADCMax, plotADCMin;

void GetWaveData()
{
    pADCSampling = GetWaveADC(ADC_DSO, ScaleH);
}

/* ����Զ���������������?
   Calculate voltage range for Auto Range*/
void getRulerV()
{
    //�Զ����̣����ݲ�����������Сֵ����500mV����Χȡ������Ϊ��ֱ��߷��?mV
    if (ScaleV_Auto == 1)
    {
        if (VMax / 100 % 10 >= 5)
		{
            RulerVMax = (VMax + 500) / 1000 * 1000;
		}
        else
		{
            RulerVMax = VMax / 1000 * 1000 + 500;
		}
		
        if (Mvoltage / 100 % 10 >= 5)
		{
            RulerVMax_m = (Mvoltage + 500) / 1000 * 1000;
		}
        else
		{
            RulerVMax_m = Mvoltage / 1000 * 1000 + 500;
		}
		
        if (VMin / 100 % 10 >= 5)
            RulerVMin = VMin / 1000 * 1000 + 500;
        else
            RulerVMin = VMin / 1000 * 1000;

        if (RulerVMax > MAX_V)
            RulerVMax = MAX_V;

        if (RulerVMin < MIN_V)
            RulerVMin = MIN_V;

    }
}

/* ���㲨��
   Calculate wave length*/
bit getWaveLength(int32 triLevel, bit right_or_left)
{
    int16 i;
    uint8 tri_pre = 255;
    uint8 tri_aft = 255;
    uint8 triPos_tmp;
    bit triSlope_tmp;
    bit triFail = 1;
    /* ���Ҿ�����Ļ��������Ĵ�����?
       Search the trigger poiont closest to the center of chart */
    if (right_or_left) //���Ҳ�����ʱ������ / Search right side
    {
        for (i = TriPos + TriPosOffset; i < SAMPLE_NUM - 2; i++)
        {
            if (GetTriggerPos(*(ADCbuf + i), *(ADCbuf + i + 1), triLevel, 1)) //�������ز��� / Search on rising edge
            {
                triPos_tmp = i;
                triSlope_tmp = 1;
                triFail = 0;
                break;
            }
            else if (GetTriggerPos(*(ADCbuf + i), *(ADCbuf + i + 1), triLevel, 0)) //���½��ز��� / Search on falling edge
            {
                triPos_tmp = i;
                triSlope_tmp = 0;
                triFail = 0;
                break;
            }
        }
    }
    else //���������ʱ������? / Search left side
    {
        for (i = TriPos + TriPosOffset; i > 0; i--)
        {
            if (GetTriggerPos(*(ADCbuf + i), *(ADCbuf + i + 1), triLevel, 1)) //�������ز��� / Search on rising edge
            {
                triPos_tmp = i;
                triSlope_tmp = 1;
                triFail = 0;
                break;
            }
            else if (GetTriggerPos(*(ADCbuf + i), *(ADCbuf + i + 1), triLevel, 0)) //���½��ز��� / Search on falling edge
            {
                triPos_tmp = i;
                triSlope_tmp = 0;
                triFail = 0;
                break;
            }
        }
    }

    if (!triFail)
    {
        for (i = triPos_tmp; i >= 0; i--) //��ѯ����λ�����ĵ�ֵ�� / Search equal point left side
        {
            if (triSlope_tmp) //��������,�����½��� / Trigger on rising edge, search on falling edge
            {
                if (*(ADCbuf + i) >= triLevel && *(ADCbuf + i + 1) <= triLevel)
                {
                    tri_pre = i;
                    break;
                }
            }
            else //�½����������������� / Trigger on falling edge, search on rising edge
            {
                if (*(ADCbuf + i) <= triLevel && *(ADCbuf + i + 1) >= triLevel)
                {
                    tri_pre = i;
                    break;
                }
            }
        }

        for (i = triPos_tmp; i < SAMPLE_NUM - 1; i++) //��ѯ����λ���Ҳ�ĵ�ֵ��? / Search equal point right side
        {
            if (triSlope_tmp) //��������,�����½��� / Trigger on rising edge, search on falling edge
            {
                if (*(ADCbuf + i) >= triLevel && *(ADCbuf + i + 1) <= triLevel)
                {
                    tri_aft = i;
                    break;
                }
            }
            else //�½����������������� / Trigger on falling edge, search on rising edge
            {
                if (*(ADCbuf + i) <= triLevel && *(ADCbuf + i + 1) >= triLevel)
                {
                    tri_aft = i;
                    break;
                }
            }
        }
    }
    if (tri_pre == 255 || tri_aft == 255 || (tri_pre == tri_aft))
    {
        WaveLength = 0;
        return 0;
    }
    else
    {
        WaveLength = tri_aft - tri_pre;
        return 1;
    }
}

/* ���㲨��Ƶ��
   Calculate the wave frequency*/
void getWaveFreq()
{
    uint16 m_num; //��ʹ���?
    uint8 n_num;  //��ƽ��ֵ����λ��

    //ADCֹͣ����ʱ��ÿ�ζ���������ʾ����Ƶ�ʣ���˲���ƽ���?
    if (!ADCRunning)
    {
        WaveLengthSumNum = 0;
        WaveLengthSum = 0;
    }

    //�Զ�ģʽ��Ϊ����Ƶ�ʲ�Ƶ���䶯��ȡƽ��ֵ�����ڵ���100ms���䲻ȡƽ��ֵ��һ��ԭ����100ms���α仯��������Ƶ��������Ƶ����
    //��һ����Խ����ʱ������ȡƽ��ֵ���ӳ�Խ�ߣ�Ӱ��ʹ������
    else if (TriMode == 0 && ScaleH > 2)
    {
        switch (ScaleH)
        {
        case 3: //50ms
            m_num = 2;
            n_num = 1;
            break;

        case 4: //20ms
            m_num = 4;
            n_num = 2;
            break;

        default: //<10ms
            m_num = 8;
            n_num = 3;
            break;
        }

        WaveLengthSum += WaveLength;

        //�ﵽ��ʹ�����?
        if (++WaveLengthSumNum >= m_num)
        {
            WaveLength = WaveLengthSum >> n_num; //�󲨳�ƽ��ֵ
            WaveLengthSumNum = 0;                //���㲨����ʹ���?
            WaveLengthSum = 0;                   //���㲨���ܺ�
        }

        //û�ﵽ��ʹ������򷵻أ�WaveFreq���ֲ���
        else
            return;
    }

    switch (ScaleH)
    {
    case 0:                                      //500ms
        WaveFreq = 50.0f / (float)WaveLength + 0.5; //WaveFreq=25000/(500*WaveLength);
        break;

    case 1:                          //200ms
        WaveFreq = (125.0f / (float)WaveLength); //WaveFreq=25000/(200*WaveLength);
        break;

    case 2:                          //100ms
        WaveFreq = (250.0f / (float)WaveLength); //WaveFreq=25000/(100*WaveLength);
        break;

    case 3:                          //50ms
        WaveFreq = (500.0f / (float)WaveLength); //WaveFreq=25000/(50*WaveLength);
        break;

    case 4:                           //20ms
        WaveFreq = (1250.0f / (float)WaveLength); //WaveFreq=25000/(20*WaveLength);
        break;

    case 5:                           //10ms
        WaveFreq = (2500.0f / (float)WaveLength); //WaveFreq=25000/(10*WaveLength);
        break;

    case 6:                           //5ms
        WaveFreq = (5000.0f / (float)WaveLength); //WaveFreq=25000/(5*WaveLength);
        break;

    case 7:                            //2ms
        WaveFreq = (12500.0f / (float)WaveLength); //WaveFreq=25000/(2*WaveLength);
        break;

    case 8:                            //1ms
        WaveFreq = (25000.0f / (float)WaveLength); //WaveFreq=25000/(1*WaveLength);
        break;

    case 9:                            //500us
        WaveFreq = (50000.0f / (float)WaveLength); //WaveFreq=25000000/(500*WaveLength);
        break;

    case 10:                            //200us
        WaveFreq = (125000.0f / (float)WaveLength); //WaveFreq=25000000/(200*WaveLength);
        break;

    case 11:                            //100us
        WaveFreq = (250000.0f / (float)WaveLength); //WaveFreq=25000000/(100*WaveLength);
        break;
    }
	
	/* �ڽ������ݵ��������ʱ����Ҫ������������Σ������ϣ��滻�����ʽ */
	WaveFreq = (float)WaveFreq /  (0.0162f * (log10(WaveFreq) /log10(2.7) ) + 0.9398f);

}

/* ��mvת��ΪADC
   Convert voltage in mV to ADC*/
int32 Convert_mv_ADC(int32 mv, uint16 *BGV, uint16 ADCbg, uint16 lsb)
{
    int32 ADC;
    ADC = (int32)mv * ADCbg * 100 / (*BGV) / lsb;
    return ADC;
}

/* ��ADCת��Ϊmv
   Convert ADC to voltage in mV */
int32 c_ADC_mv(int32 ADC, uint16 *BGV, uint16 BGADC, uint16 lsb)
{
    int32 mv;
    mv = (int32)ADC * *BGV * lsb / BGADC / 100;
    return mv;
}

/* ��uint16��ʽ��mVת��Ϊ�ַ���V
   Convert voltage in mV to string*/
uint8 *c_mv_V(int32 mv)
{
    static uint8 s[6];

     if (mv > -10000l && mv < 0 )
    {
        s[0] = '-';
        s[1] = mv / -1000l + '0';
        s[2] = '.';
        s[3] = mv / -100l % -10l + '0';
        s[4] = mv / -10l % -10l + '0';
        s[5] = '\0';
    }
    else if( mv <= -10000l)
    {
        s[0] = '-';
        s[1] = mv / -10000l + '0';
        s[2] = mv / -1000l % -10l + '0';
        s[3] = '.';
        s[4] = mv / -100l % -10l + '0';
        s[5] = '\0';
    }
    else if ( mv >= 0 && mv < 10000l)
    {
        s[0] = '+';
        s[1] = mv / 1000l + '0';
        s[2] = '.';
        s[3] = mv / 100l % 10 + '0';
        s[4] = mv / 10l % 10 + '0';
        s[5] = '\0';
    }
    else
    {
        s[0] = '+';
        s[1] = mv / 10000l + '0';
        s[2] = mv / 1000l % 10l + '0';
        s[3] = '.';
        s[4] = mv / 100l % 10l + '0';
        s[5] = '\0';
    }

    return s;
}

/* ת������Ƶ��Ϊ�ַ�
   Convert frequency to string */
uint8 *c_WaveFreq_Str()
{
    static uint8 s[5];
if (WaveFreq == 0)
    {
        s[0] = '-';
        s[1] = '-';
        s[2] = '-';
        s[3] = '-';
        s[4] = '\0';
    }
    else if (WaveFreq >= 10000000)
    {
        s[0] = ' ';
        s[1] = WaveFreq / 10000000 + '0';
        s[2] = ((WaveFreq) / 1000000) % 10 + '0';
        s[3] = 'M';
        s[4] = '\0';
    }
    else if (WaveFreq >= 1000000)
    {
        s[0] = WaveFreq / 1000000 + '0';
        s[1] = '.';
        s[2] = ((WaveFreq) / 100000) % 10 + '0';
        s[3] = 'M';
        s[4] = '\0';
    }
    else if (WaveFreq >= 100000)
    {
        s[0] = WaveFreq / 100000 + '0';
        s[1] = (WaveFreq / 10000) % 10 + '0';
        s[2] = ((WaveFreq) / 1000) % 10 + '0';
        s[3] = 'k';
        s[4] = '\0';
    }
    else if (WaveFreq >= 10000)
    {
        s[0] = ' ';
        s[1] = WaveFreq / 10000 + '0';
        s[2] = ((WaveFreq) / 1000) % 10 + '0';
        s[3] = 'k';
        s[4] = '\0';
    }
    else if (WaveFreq >= 1000)
    {
        s[0] = WaveFreq / 1000 + '0';
        s[1] = '.';
        s[2] = ((WaveFreq) / 100) % 10 + '0';
        s[3] = 'k';
        s[4] = '\0';
    }
    else if (WaveFreq >= 100)
    {
        s[0] = ' ';
        s[1] = WaveFreq / 100 + '0';
        s[2] = (WaveFreq / 10) % 10 + '0';
        s[3] = (WaveFreq) % 10 + '0';
        s[4] = '\0';
    }
    else if (WaveFreq >= 10)
    {
        s[0] = ' ';
        s[1] = ' ';
        s[2] = WaveFreq / 10 + '0';
        s[3] = (WaveFreq) % 10 + '0';
        s[4] = '\0';
    }
    else if (WaveFreq >= 1)
    {
        s[0] = ' ';
        s[1] = ' ';
        s[2] = ' ';
        s[3] = (WaveFreq) % 10 + '0';
        s[4] = '\0';
    }
    return s;
}

/* ������ֵ��ӳ�䵽��Ļ����ʾ��Χ������ת
   Remap sampling data to display range and inverse */
uint16 remap(uint16 val, uint16 rangeMax, uint16 rangeMin, uint16 rangeMaxNew, uint16 rangeMinNew)
{
    if (val > rangeMax)
        val = rangeMax;
    else if (val < rangeMin)
        val = rangeMin;

    val = rangeMinNew + (uint32)(rangeMax - val) * (rangeMaxNew - rangeMinNew) / (rangeMax - rangeMin);
    return val;
}

/* ��ȡ������λ��
   Get Trigger Position */
bit GetTriggerPos(uint16 d1, uint16 d2, uint16 dTrigger, bit triSlope)
{
    /*  �����ش���
        Trigger on Rising Edge  */
    if (triSlope)
    {
        if (d1 <= dTrigger && d2 >= dTrigger)
        {
            return 1;
        }
    }

    /*  �½��ش���
        Trigger on Falling Edge */
    else
    {
        if (d1 >= dTrigger && d2 <= dTrigger)
        {
            return 1;
        }
    }

    return 0;
}

int32 Max = 0;
int32 Min = 4095;

/* ������������
   Analyse sampling date */
void AnalyseData()
{
    int16 i;
    int32 tmp;
    int32 adcMax = 0;
    int32 adcMin = 4095;
    int32 adcMid = 0;
    int32 plotADCMid = 0;
	
	static int32 Mvoltage_Aver = 0;
	static int32 Mvoltage_Buf = 0;
	static int8 Aver_Cnt = 0;
	
    if (ADCComplete)
    {
        ScaleH_tmp = ScaleH; //��¼��ɲ�����ʱ������?���ڲ�������������٣���˲�֧��ʵʱ����ʱ���������Ų��Σ�ʱ������ı�����ղ���
        //�������㸴�Ƶ���һ�����飬��������ж�������ݻ���
        //���������жϣ���ʹ�û����оɲ�������ʾ����
        for (i = 0; i < SAMPLE_NUM; i++)
        {
            *(ADCbuf + i) = *(pADCSampling + i);
        }

        //���㴥����λ��
        //ADC����ֹͣʱ��TriPos���䣬���Բ��������м���
        TriPos = SAMPLE_NUM / 2;
        TriFail = 1; //��λ����ʧ�ܱ�־
        for (i = ((CHART_H_MAX - CHART_H_MIN) >> 1); i < SAMPLE_NUM - ((CHART_H_MAX - CHART_H_MIN) >> 1); i++)
        {
            if (GetTriggerPos(*(ADCbuf + i), *(ADCbuf + i + 1), TriggerADC, TriSlope))
            {
                TriPos = i;
                TriFail = 0; //
                break;
            }
        }
        TriPosOffset = 0;
    }

    /*  ��ȡ��Ļ��ʾ���ε�������Сֵ
        Get the MAX and MIN value of waveform on display*/
    for (i = 0; i <= 100; i++)
    {
        tmp = *(ADCbuf + TriPos + TriPosOffset - 50 + i);
        if (tmp > adcMax)  adcMax = tmp;
        else if (tmp < adcMin)  adcMin = tmp;
    }
	
    Max = adcMax;   //
	Min = adcMin;
    
	//��������������С����ֵת���ɵ�ѹֵmV
    
    VMax = c_ADC_mv(adcMax, BGV, ADCbg, Lsb);
    VMin = c_ADC_mv(adcMin, BGV, ADCbg, Lsb);
    
	VMax_m = c_ADC_mv(adcMax - Bias_Voltage, BGV, ADCbg, Lsb);
	VMin_m = c_ADC_mv(adcMin - Bias_Voltage, BGV, ADCbg, Lsb);
    
    /* �ڽ������ݵ��������ʱ����Ҫ������������Σ������ϣ��滻�����ʽ */
	if(VMax_m > 0  && VMin_m > 0)
	{
		VMax_m = VMax_m * (0.0003f * (double)VMax_m + 0.2085f);
		VMin_m = VMin_m * (0.0003f * (double)VMin_m + 0.2085f);
	}
	else if(VMax_m < 0  && VMin_m < 0)
	{
		VMax_m = VMax_m * (0.0002f * (0-(double)VMax_m) + 0.2065f);
		VMin_m = VMin_m * (0.0002f * (0-(double)VMin_m) + 0.2065f);		
	}
	else if(VMax_m > 0  && VMin_m < 0)
	{
		VMax_m = VMax_m * (0.00041f * (double)VMax_m + 0.2064f);
		VMin_m = VMin_m * (0.00031f * (0-(double)VMin_m) + 0.2064f);
	}
	
    if(MeaWay == Option_MeaWay_DC)
	{
		Mvoltage_Aver += (VMax_m + VMin_m) >> 1;
		
		if(++Aver_Cnt >= 8)
		{
			Aver_Cnt = 0;
			
			Mvoltage = (float)Mvoltage_Aver / 8.0f;
			Mvoltage_Aver = 0;
		}
		
	}
    else if(MeaWay == Option_MeaWay_AC)
	{
		Mvoltage_Buf = (VMax_m - VMin_m);
		
		Mvoltage_Buf = Mvoltage_Buf / ( 0.3722f * (log10(Mvoltage_Buf) / log10(2.728f)) - 2.0842f );
		
		Mvoltage_Aver += Mvoltage_Buf;

		if(++Aver_Cnt >= 8)
		{
			Aver_Cnt = 0;
			
			Mvoltage = (float)Mvoltage_Aver / 8.0f;
			
			Mvoltage_Aver = 0;
		}
	}
	
    //��ô�ֱ��ߵ�������
    getRulerV();

    //�ô�ֱ���mV��Χ�����ADCֵ�ķ�Χ��Ϊͼ������ʾ������
    plotADCMax = Convert_mv_ADC(RulerVMax, BGV, ADCbg, Lsb);
    plotADCMin = Convert_mv_ADC(RulerVMin, BGV, ADCbg, Lsb);

    //���㲨�ε�Ƶ��
    //�����ǰ��ʱ������Ͳ������ݵ�ʱ����һ�������Ƶ�ʼ���?
    //����STC8A8K�ڸ�Ƶ����ʱ��ADCֵ����Ī����������䵽������?4095����ѯSTC����ʦ�޹�
    //Ϊ����ADC��������ʱƵ���������������㲨��ʱʹ�õ�ѹ��ߵ��е�Ͳ����е�Ľ�С�?
    //�������Ƶ�������޷���ȡ�����Զ������л������ʵ��ֶ����̼���?
    adcMid = (adcMax + adcMin) >> 1;
    plotADCMid = (plotADCMax + plotADCMin) >> 1;
    if (getWaveLength(adcMid < plotADCMid ? adcMid : plotADCMid, 1) || getWaveLength(adcMid < plotADCMid ? adcMid : plotADCMid, 0))
    {
        getWaveFreq();
    }
    else
    {
        WaveFreq = 0;
    }

    //ӳ�����ֵ����Ļ����ʾ���?
    for (i = 0; i < SAMPLE_NUM; i++)
    {
        waveBuf[i] = remap(*(ADCbuf + i), plotADCMax, plotADCMin, CHART_V_MAX, CHART_V_MIN);
    }
}

/* ����������
   Draw main interface */
void PlotChart(void)
{
    uint8 i;
    uint8 *s;

    if (ClearDisplay)
    {
        ClearDisplay = 0;
        OLED_Clear();

        /* ͼ���߿�
           ���κ����ͼ��?101��,26~126
           ���������ͼ��?45��,8~52
           Frame of chart 45x101*/
        OLED_DrawHLine(CHART_H_MIN, CHART_V_MIN, 4);
        OLED_DrawHLine(CHART_H_MIN, CHART_V_MAX, 4);
        OLED_DrawHLine(CHART_H_MAX - 3, CHART_V_MIN, 4);
        OLED_DrawHLine(CHART_H_MAX - 3, CHART_V_MAX, 4);
        OLED_DrawHLine(CHART_H_MIN + 25 - 2, CHART_V_MIN, 5);
        OLED_DrawHLine(CHART_H_MIN + 25 - 2, CHART_V_MAX, 5);
        OLED_DrawHLine(CHART_H_MIN + 50 - 2, CHART_V_MIN, 5);
        OLED_DrawHLine(CHART_H_MIN + 50 - 2, CHART_V_MAX, 5);
        OLED_DrawHLine(CHART_H_MIN + 75 - 2, CHART_V_MIN, 5);
        OLED_DrawHLine(CHART_H_MIN + 75 - 2, CHART_V_MAX, 5);
        OLED_DrawVLine(CHART_H_MIN - 1, CHART_V_MIN, CHART_V_MAX - CHART_V_MIN + 1);
        OLED_DrawVLine(CHART_H_MAX + 1, CHART_V_MIN, CHART_V_MAX - CHART_V_MIN + 1);

        /* ͼ����������
           Grid of chart */
        for (i = 0; i < 15; i++)
        {
            OLED_DrawHLine(CHART_H_MIN + 7 * i, CHART_V_MIN + ((CHART_V_MAX - CHART_V_MIN) >> 1), 3);
        }
        for (i = 0; i < 6; i++)
        {
            OLED_DrawVLine(CHART_H_MIN + 25, CHART_V_MIN + 1 + i * 8, 3);
            OLED_DrawVLine(CHART_H_MIN + 50, CHART_V_MIN + 1 + i * 8, 3);
            OLED_DrawVLine(CHART_H_MIN + 75, CHART_V_MIN + 1 + i * 8, 3);
        }

        /* ���ε�ѹ��Χ
           Voltage range of waveform*/
        OLED_Set_Pos(18, 56);
        s = c_mv_V(VMin_m);
        OLED_DrawString(s);
		
        /* ����λ�ñ��?
           Ruler for waveform position*/
        OLED_DrawHLine(50, 66, 17);
        OLED_DrawVLine(50, 60, 3);
        OLED_DrawVLine(66, 60, 3);
        OLED_DrawVLine((TriPos + TriPosOffset - 80) * 0.5f / 4 + 55, 58, 5);
		
		OLED_Set_Pos(67, 56);
        s = c_mv_V(VMax_m);
        OLED_DrawString(s);
		
    }

    OLED_Overlap(0); //���û�ͼģʽΪ����

	
    /* �Զ����̱�־
       Flag for Auto Range*/
    if (ScaleV_Auto == 1)
    {
        OLED_Set_Pos(0, 0);
        OLED_DrawString("Auto");
    }
    else
    {
        OLED_Set_Pos(0, 0);
        OLED_DrawString("    ");
    }

    /* ����ֵ
       Trigger Level */
    if (OptionInChart == 2 && !WaveScroll)
    {
		OLED_DrawHLine(28, 0, 44);
        OLED_DrawVLine(34, 1, 8);
		OLED_DrawVLine(71, 1, 8);
        OLED_Reverse(1);
    }
    else
    {
        OLED_Reverse(1);
		OLED_DrawHLine(28, 0, 44);
        OLED_DrawVLine(34, 1, 8);
		OLED_DrawVLine(71, 1, 8);
        OLED_Reverse(0);
    }

	OLED_Set_Pos(28, 1);
    OLED_DrawString("T");
    s = c_mv_V(TriLevel_m);
	OLED_Set_Pos(35, 1);
    OLED_DrawString(s);
	OLED_Set_Pos(65, 1);
    OLED_DrawString("V");
	
    OLED_Reverse(0);
	
	
    /* ����������?
       Trigger Slope */
    if (OptionInChart == 3 && !WaveScroll)
    {
        OLED_DrawVLine(73, 1, 8);
        OLED_DrawVLine(80, 1, 8);
        OLED_Reverse(1);
    }
    else
    {
        OLED_Reverse(1);
        OLED_DrawVLine(73, 1, 8);
        OLED_DrawVLine(80, 1, 8);
        OLED_Reverse(0);
    }
	
    if (TriSlope)
    {
        OLED_DrawChar(75, 1, 123); //123�ϼ�ͷ�������ش���
    }
    else
    {
        OLED_DrawChar(75, 1, 124); //124�¼�ͷ���½��ش���
    }
    OLED_Reverse(0);
	   
	
    /* ������ʽ��־
       Trigger Mode */
    if (OptionInChart == 4 && !WaveScroll)
    {
        OLED_DrawVLine(88, 1, 8);
        OLED_Reverse(1);
    }
    else
    {
        OLED_Reverse(1);
        OLED_DrawVLine(88, 1, 8);
        OLED_Reverse(0);
    }
	
    OLED_Set_Pos(82, 1);
    if (TriMode == 0)
    {
        OLED_DrawString("A");
    }
    else if (TriMode == 1)
    {
        OLED_DrawString("N");
    }
    else if (TriMode == 2)
    {
        OLED_DrawString("S");
    }
    OLED_Reverse(0);
	
    /* Ƶ��
       Frequency */
    OLED_Set_Pos(92, 1);
    OLED_DrawString("          ");
    OLED_Set_Pos(92, 1);
    s = c_WaveFreq_Str();
    OLED_DrawString(s);
	OLED_Set_Pos(117, 1);
    OLED_DrawString("Hz");
	
	//��������/ֹͣ��־
	OLED_Set_Pos(0, 8);
    if (ADCRunning)
    {
        OLED_DrawString("Run ");
    }
    else
    {
        OLED_DrawString("Stop");
    }
	
    /* ����ʧ�ܱ�־
       Flag for Trigger Fail*/
	OLED_Set_Pos(0, 17);
    if (TriFail)
    {
        OLED_DrawString("Fail");
    }
    else
    {
        OLED_DrawString("Succe");
    }
	
    /* �����ѹ����?
       Ruler for Voltage */
    OLED_Set_Pos(0, 26);
    OLED_DrawString("    ");
    if (OptionInChart == 1 && !WaveScroll)
    {
		OLED_DrawHLine(0, 25, 30);  //��Ϊ���ϱ�һ�и��ǲ���,����ֱ�ӻ�����������ȫ.
        OLED_Reverse(1);
    }
	else
	{
        OLED_Reverse(1);
        OLED_DrawHLine(0, 25, 30);  //��Ϊ���ϱ�һ�и��ǲ���,����ֱ�ӻ�����������ȫ.
        OLED_Reverse(0);		
	}
	
    s = c_mv_V(RulerVMax_m);
    OLED_Set_Pos(0, 26);
    OLED_DrawString(s);
    OLED_Reverse(0);
	
     /* ������ʽ
        Correct the calculated waveform voltage value */   
    if (OptionInChart == 5 && !WaveScroll)
    {
        OLED_DrawVLine(25, 35, 9);  //��Ϊ���ұ�һ�и��ǲ���,����ֱ�ӻ�����������ȫ.
		OLED_DrawVLine(24, 35, 9);  //��Ϊ���ұ�һ�и��ǲ���,����ֱ�ӻ�����������ȫ.
        OLED_DrawHLine(0, 35, 25);  //��Ϊ���ϱ�һ�и��ǲ���,����ֱ�ӻ�����������ȫ.
        OLED_Reverse(1);
    }
    else
    {
        OLED_Reverse(1);
		OLED_DrawVLine(25, 35, 9);  //��Ϊ���ұ�һ�и��ǲ���,����ֱ�ӻ�����������ȫ.
        OLED_DrawVLine(24, 35, 9);  //��Ϊ���ұ�һ�и��ǲ���,����ֱ�ӻ�����������ȫ.
        OLED_DrawHLine(0, 35, 25);  //��Ϊ���ϱ�һ�и��ǲ���,����ֱ�ӻ�����������ȫ.
        OLED_Reverse(0);
    }
	
    OLED_Set_Pos(1, 36);
    if(MeaWay == Option_MeaWay_DC)       OLED_DrawString("M-DC");
    else if (MeaWay == Option_MeaWay_AC) OLED_DrawString("M-AC");
    OLED_Reverse(0);
    
    /* ��������������Ĳ��ε�ѹ�?
       Correct the calculated waveform voltage value */   
	s = c_mv_V(Mvoltage);
    OLED_Set_Pos(0, 46);
    OLED_DrawString(s);
    OLED_Reverse(0);
	

    /* ����ʱ������
       Seconds per division */
    OLED_Set_Pos(99, 56);
    OLED_DrawString("     ");
    if (OptionInChart == 0 && !WaveScroll)
    {
		 OLED_DrawHLine(99, 55, 30);  //��Ϊ���ϱ�һ�и��ǲ���,����ֱ�ӻ�����������ȫ.
        OLED_Reverse(1);
    }
	else
	{
        OLED_Reverse(1);
        OLED_DrawHLine(99, 55, 30);  //��Ϊ���ϱ�һ�и��ǲ���,����ֱ�ӻ�����������ȫ.
        OLED_Reverse(0);		
	}
    OLED_Set_Pos(99, 56);
    OLED_DrawString(ScaleHTxt[ScaleH]);
    OLED_Reverse(0);

	/* ��ʾ���� */
   
    OLED_DrawHLine(1, 56, 14);
    OLED_DrawHLine(14, 57, 2);

    //�����?
    OLED_DrawVLine(1, 56, 7);

    //��һ��
    OLED_DrawVLine(3, 56, 7);

    //�ڶ���
    OLED_DrawVLine(5, 56, 7);

    //������
    OLED_DrawVLine(7, 56, 7);
	
	if(VBat >=3500 && VBat < 4000)
	{
		//���ĸ�
		OLED_DrawVLine(9, 56, 7);
	}
	
	if(VBat >=4000 && VBat < 4500)
	{
		//���ĸ�
		OLED_DrawVLine(9, 56, 7);
		//�����?
		OLED_DrawVLine(11, 56, 7);
	}
	if(VBat >=4500)
	{
		//���ĸ�
		OLED_DrawVLine(9, 56, 7);
		//�����?
		OLED_DrawVLine(11, 56, 7);
		//ͷ��
		OLED_DrawVLine(13, 56, 7);
	}
    //�����?   
    OLED_DrawVLine(15, 57, 5);
	
    OLED_DrawHLine(14, 61, 2);
    OLED_DrawHLine(1, 62, 14);

 

    OLED_Overlap(1); //�ָ���ͼģʽΪ����

}

/* ���Ʋ���
   Draw waveform*/
void PlotWave(void)
{
    uint8 i;
    //Vector Mode
    if (PlotMode == 0)
    {
        for (i = 0; i < (CHART_H_MAX - CHART_H_MIN); i++)
        {
            OLED_DrawLine(
                i + CHART_H_MIN,
                waveBuf[TriPos + TriPosOffset - ((CHART_H_MAX - CHART_H_MIN) >> 1) + i],
                i + CHART_H_MIN + 1,
                waveBuf[TriPos + TriPosOffset - ((CHART_H_MAX - CHART_H_MIN) >> 1) + i + 1]);
        }		

    }
    //Dots Mode
    if (PlotMode == 1)
    {
        for (i = 0; i <= (CHART_H_MAX - CHART_H_MIN); i++)
        {
            OLED_DrawPixel(i + CHART_H_MIN, waveBuf[TriPos + TriPosOffset - ((CHART_H_MAX - CHART_H_MIN) >> 1) + i]);
        }
    }
}

/* �������ý���
   Draw settings */
void PlotSettings()
{
    if (ClearDisplay)
    {
        ClearDisplay = 0;
        OLED_Clear();
		
	    OLED_DrawVLine(48, 0, 11);  //��߿�?
		OLED_DrawVLine(88, 0, 11);  //�ұ߿�
        OLED_DrawHLine(49, 0, 39);  //�ϱ߿�
		OLED_DrawHLine(49, 10, 39); //�±߿�
		
        OLED_Set_Pos(50, 2);
        OLED_DrawString("MinDSO");
    }

    OLED_Overlap(0);

    /* ѡ��
       Options */
    /* DrawMode */
    OLED_Set_Pos(38, 15);
    OLED_DrawString("PMode=");
    OLED_DrawString(PlotModeTxt[PlotMode]);

    /* LSB */
    OLED_Set_Pos(45, 25);
    OLED_DrawString("LSB=");
    OLED_DrawString("   ");
    OLED_Set_Pos(70, 25);
    OLED_DrawNum(Lsb, 3);

    /* OLED_Brightness */
    OLED_Set_Pos(45, 35);
    OLED_DrawString("BRT=");
    OLED_DrawString("   ");
    OLED_Set_Pos(70, 35);
    OLED_DrawNum(OLED_Brightness, 3);

    /* Vbat */
    OLED_Set_Pos(35, 45);
    OLED_DrawString("Vbat=");
    OLED_DrawString(c_mv_V(VBat));
    OLED_DrawString("V");
	
	OLED_Overlap(0);
    /* ѡ���־��?125��Ӧ����?
       Option indicator, 125 is Left arrow */
    /* PlotMode */
    if (OptionInSettings == 0)
    {
		
        OLED_DrawChar(101, 15, 125);
        OLED_DrawChar(95, 30, ' ');
        OLED_DrawChar(90, 45, ' ');
    }
    /* LSB */
    else if (OptionInSettings == 1)
    {
        OLED_DrawChar(101, 15, ' ');
        OLED_DrawChar(95, 25, 125);
        OLED_DrawChar(90, 35, ' ');
    }
    /* OLED_Brightness */
    else if (OptionInSettings == 2)
    {
        OLED_DrawChar(101, 15, ' ');
        OLED_DrawChar(95, 25, ' ');
        OLED_DrawChar(90, 35, 125);
    }
	
    OLED_Overlap(1);
}

/* ���Ʊ���״̬
   Draw saving status*/
void PlotSaveStatus(bit _saveStatus)
{
	OLED_Overlap(0); //���û�ͼģʽΪ����
    OLED_Set_Pos(30, 55);
    if (_saveStatus)
    {
        OLED_DrawString("Save Success!");
    }
    else
    {
        OLED_DrawString("Save Fail!");
    }
    OLED_Overlap(1); //���û�ͼģʽΪ����
}