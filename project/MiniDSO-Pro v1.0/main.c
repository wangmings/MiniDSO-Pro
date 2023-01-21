/*

    ����ʾ����

    ����˵����
        ������-ѡ������ģʽ����Option��������
            �������������л���ʼ/��ͣ����
            ����ͬʱ��ת���������ڲ������л�

        ������-����ˮƽ����ģʽ��
            �������������л���ʼ/��ͣ����
            ��ת��������ˮƽ�������Σ�����ͣ����״̬����Ч��
            ����ͬʱ��ת����������ֱ�������Σ�������ͣ����״̬����Ч��
            
        ���ý��棨��Setting��������

            ����������������������
            ��ת���������޸ĵ�ǰѡ��
            ����ͬʱ��ת���������л�ѡ��

    �����������
        ����ʱ�����䣺"500ms", "200ms", "100ms", "50ms", "20ms", "10ms","5ms", "2ms", "1ms", "500us", "200us", "100us"(100us�����Զ�����ģʽ�¿���)
        �����ѹ���䣺�趨��ѹ�������ޣ����30V��˳ʱ����ת����������ѹ��������Ϊ0������Զ����̣���ʱ����ת�����ֶ�����
        ����ֵ���趨������ѹֵ������λ�ñ�������Ļ����
        �����������¼�ͷ���ֱ���������ش������½��ش���
        ������ʽ��
            Auto(�Զ�����)  ������������ָʾ��ʼ�յ�����ͨ������������������ͣ�������ɹ�ʱ��������λ����Ļ���ģ�����ʧ�������޹��ɹ�������Ļ�����ʾFail��־
            Normal(��ͨ����)���ȴ�Ԥ��������ָʾ�������ʾԤ������������ʱ���������źţ�
                              �����ɹ�����ʾ���Σ��������ȴ��´δ�����
                              ������µĴ����ɹ������Զ���ʾ�µĲ��Σ���û���µĴ����ɹ�����Ļ���α��ֲ�һֱ�ȴ��´δ�����
                              ��ע��������ȷ�Ĵ���ֵ������һֱ���ڵȴ�����״̬����Ļ������ʾ����
            Single(���δ���)���ȴ�Ԥ��������ָʾ�������ʾԤ������������ʱ���������źţ�
                              �����ɹ�����ʾ���Σ���ֹͣ���������û�������������ʼ�´β���	
                              ��ע��������ȷ�Ĵ���ֵ������һֱ���ڵȴ�����״̬����Ļ������ʾ����
            
    ������״̬��ʾ��	
        Run�����ڲ���
        Stop��ֹͣ����
        Fail���Զ�ģʽ�£�����ֵ�������η�Χ���𴥷�ʧ��
		Succe���Զ�ģʽ�£�����ֵ�ڲ��η�Χ��ʾ�����ɹ�
        Auto���Զ�����
            
    ���ý���ѡ�
        PMode(��ͼģʽ)��
            Vector��������ʸ����ʾ
            Dots  �������Ե�����ʾ
        LSB(����ϵ��)����ѹϵ����100����������10k��2k�ĵ�����з�ѹ����ѹϵ��Ϊ(10+2/2)=6��LSB����ȡֵΪ6x100=600������ʵ�ʵ��辫�Ƚ���΢��	
        BRT��OLED����

    �������ã������������˳����ý���ʱ�����ý��������������в��������浽EEPROM


	
	
*/

#include <reg51.h>
#include "global.h"
#include "delay.h"
#include "adc_stc8.h"
#include "config_stc.h"
#include "settings.h"
#include "chart.h"
#include "ssd1306.h"
#include "stdio.h"

sfr P5 = 0xC8;

char putchar (char c)
{
    if (c == '\n')
    {
        SBUF = 0x0d;
		while (!TI);	//�ȴ����ͳɹ�
		TI = 0;	  //��������жϱ�־
        /* output CR  */
    }
    SBUF = c;
    while (!TI);	//�ȴ����ͳɹ�
    TI = 0;	  //��������жϱ�־
    return c;
}

void Timer1Init(void)		//20����@27.000MHz
{
	AUXR &= 0xBF;		//��ʱ��ʱ��12Tģʽ
	TMOD &= 0x0F;		//���ö�ʱ��ģʽ
	TL1 = 0x38;		//���ö�ʱ��ʼֵ
	TH1 = 0x50;		//���ö�ʱ��ʼֵ
	TF1 = 0;		//���TF1��־
	TR1 = 1;		//��ʱ��1��ʼ��ʱ
}

sfr T2H = 0xD6;
sfr T2L = 0xD7;

void UartInit(void)		//9600bps@24.000MHz
{
	SCON = 0x50;		//8λ����,�ɱ䲨����
	AUXR |= 0x01;		//����1ѡ��ʱ��2Ϊ�����ʷ�����
	AUXR |= 0x04;		//��ʱ��ʱ��1Tģʽ
	T2L = 0x8F;		//���ö�ʱ��ʼֵ
	T2H = 0xFD;		//���ö�ʱ��ʼֵ
	AUXR |= 0x10;		//��ʱ��2��ʼ��ʱ
	
}


void init()
{
	
    InSettings = 0;         //��ʼ����������
    OptionInSettings = 0;   //��ʼ�����ý����ѡ����
    OptionInChart = 0;      //��ʼ���������ѡ����
    WaveScroll = 0;         //���㲨�ι�����־
    OptionChanged = 0;      //���������޸ı�־
    ADCRunning = 1;         //��λ������־
    ADCInterrupt = 0;       //����ADC�жϱ�־
    ADCComplete = 0;        //����ADC��ɱ�־
    WaveUpdate = 0;         //���㲨�θ��±�־
    ClearWave = 0;          //����������α�־
    ClearDisplay = 1;       //��λ�����Ļ��־
    EC11PressAndRotate = 0; //�����������ת��־
    UpdateVbat = 0;         //���������ص�ѹ��־
    ADCComplete = 0;        //����ADC��ɲ�����־
    WaveFreq = 0;           //���㲨��Ƶ��
    TriPos = 50;            //��ʼ��������λ��
    TriPosOffset = 0;       //��ʼ��������ƫ����
    TriFail = 0;            //���㴥��ʧ�ܱ�־
    VMax = 0;               //���㲨�ε�ѹ���ֵ
    VMin = 0;               //���㲨�ε�ѹ��Сֵ
    WaveLengthSumNum = 0;   //���㲨�γ�����ʹ���
    WaveLengthSum = 0;      //���㲨�γ������
	
	PlotMode = 0;
	
	Bias_Voltage = 8900; //ƫ�õ�ѹȨ��
	
	TriLevel_m = 2000;
	
	VMax_m = 0;
	VMin_m = 0;
	Mvoltage = 0;
	RulerVMax_m = 0;
	
	MeaWay = Option_MeaWay_AC;
	
    /* ���ι���ģʽ�����ڲ���������ⲿ�ж�0����������ת��
       Disable external interrupt 0(Encoder rotation) in waveform scroll mode when sampling*/
    if (ADCRunning && WaveScroll)
        EX0 = 0;

    /* ��ȡ�ڲ��ο���ѹֵ
       Get internal reference voltage */
    BGV = GetBGV();
}

void runWhenInSettrings()
{
    P_Ready = 0;
    ClearDisplay = 1;
    while (InSettings)
    {
        /* ���µ�ص�ѹ��Ϣ
           Update voltage infor of battery */
        if (UpdateVbat)
        {
            UpdateVbat = 0;
            VBat = GetVoltage(ADC_BAT, VBAT_LSB);
        }
		
        /* ������Ļ����
           Update OLED Brightness */
        if (OLED_BrightnessChanged)
        {
            OLED_BrightnessChanged = 0;
            OLED_Write_Command(0x81);
            OLED_Write_Command(OLED_Brightness * 10);
        }

        /* ���²���
           Update parameters */
        if (DisplayUpdate)
        {
            DisplayUpdate = 0;
            PlotSettings();
            OLED_Display();
        }
    }

    /* ��������
       Save Settings */
    EA = 0;
    if (OptionChanged)
    {
        OptionChanged = 0;
        ClearDisplay = 0;
        /* �������ò���ʾ����״̬
           Save settings and display saving status*/
        PlotSaveStatus(Save_Options());
        OLED_Display();
        Delay1000ms();
    }

    /* �˳�settings��ˢ����Ļ
       Redraw display after exit settings */
    ClearDisplay = 1;
    PlotChart();
    OLED_Display();
    IE0 = 0;
    IE1 = 0;
    EA = 1;
}

/*
    ADCRuning=1
    |	WaveUpdate=1
    |	|	ClearWave=1
    |	|	ClearWave=0
    |	WaveUpdate=0
    ADCRuning=0
        DisplayUpdate=1
            WaveUpdate=1
            |	ClearWave=1
            |	ClearWave=0
            WaveUpdate=0
*/
void runWhenADCInterrupt()
{
    ADCInterrupt = 0;
    P_Ready = 0;

    /* ADC��������
       ADC Sampling Running */
    if (ADCRunning)
    {
        EX0 = 0;

        if (WaveUpdate)
        {
            WaveUpdate = 0;
            ClearDisplay = 1;

            /* ��ղ��λ���
               Clear waveform in buffer*/
            if (ClearWave)
            {
                ClearWave = 0;
                memset(ADCbuf, 0x00, SAMPLE_NUM * 2);
                WaveFreq = 0;
                TriFail = 0;
                VMax = 0;
                VMin = 0;
                PlotChart();
            }
            /* ��ʾ�����еĲ���
               Display waveform in buffer*/
            else
            {
                AnalyseData();
                PlotChart();
                PlotWave();
            }
        }
        else
        {
            ClearDisplay = 0;
            PlotChart();
        }
        OLED_Display();

        /* ADCRuning��WaveScrollͬʱ��λʱ���������ⲿ�ж�0����������ת��
           Not to enable external interrupt 0(Encoder Rotation) when both of ADCRunning and WaveScroll set. */
        if (!WaveScroll)
        {
            IE0 = 0;
            EX0 = 1;
        }
    }

    /* ADC����ֹͣ
       ADC Sampling Stopped */
    else
    {
        while (!ADCRunning && !InSettings)
        {
            if (DisplayUpdate)
            {
                EX0 = 0;
                DisplayUpdate = 0;
                if (WaveUpdate)
                {
                    WaveUpdate = 0;
                    /* ������������ɲ����Ĳ�������
                       Analyse completed sampling data in buffer */
                    AnalyseData();
                    ClearDisplay = 1;

                    /* �����Ļ�����Ʋ���
                       Clear display and draw waveform */
                    if (ScaleH == ScaleH_tmp)
                    {
                        /* �����������ޣ���֧�ֺ������Ų��Σ�ֻ��ʱ����������ɲ�����ʱ��������ͬ�Ż��Ʋ���
                           Since the sampling points limitation, scaling waveform along horizontal is not support.
                           Show waveform only when time scale is same as that when sampling completed */
                        PlotChart();
                        PlotWave();
                    }

                    /* �����Ļ�������Ʋ���
                       Clear display and no waveform */
                    else
                    {
                        WaveFreq = 0; //����WaveFreq�����ƽ���ʱ��Ƶ��λ�û���ʾ****Hz
                        PlotChart();
                    }
                }
                /* ֻ������Ļ�ϵĲ��������β���
                   Update parameters on display only, keep waveform */
                else
                {
                    ClearDisplay = 0;
                    PlotChart();
                }

                OLED_Display();
                IE0 = 0;
                EX0 = 1;
            }
        }
    }
}

void runWhenADCComplete()
{
    /*  ADC������� - ���δ���
        ADC Sampling Complete - Single Trigger Mode */
    if (TriMode == 2)
    {
        EX0 = 0;
        ADCInterrupt = 1;
        DisplayUpdate = 0;
        WaveUpdate = 0;
        ADCRunning = 0; //����ADC���б�־��ֹͣ����
        AnalyseData();
        ClearDisplay = 1;
        PlotChart();
        PlotWave();
        OLED_Display();
        IE0 = 0;
        EX0 = 1;
    }

    /*  ADC������� - �Զ���������ͨ����
        ADC Sampling Complete - Auto or Normal Trigger Mode */
    else
    {
        //EX0 = 0;
        ClearDisplay = 1; //��λ�����Ļ��־
        AnalyseData();    //������������
        PlotChart();      //����������
        PlotWave();       //���Ʋ���
        OLED_Display();
        //IE0 = 0;
        //EX0 = 1;
    }
}

/* �������
   Program Entry */
void main()
{
    /* ��ADC�˿�P0.4��P0.6����Ϊ����
       Set ADC port P0.4, P0.6 as high-impedance */
    P0M1 |= 0x50;
    P0M0 &= ~0x50;

    /* ����ָʾ�ƶ˿�P1.1Ϊ�������
       Set indicator port P1.1 as push-pull output */
    P0M1 &= ~0x02;
    P0M0 |= 0x02;

    /* ���ñ������˿�P3.2��P3.3��P3.4Ϊ׼˫��
       Set Encoder port P3.2, P3.3, P3.4 as quasi bidirectional */
    P3M1 &= ~0x1C;
    P3M0 &= ~0x1C;

    /* ����OLED�˿�P2.3��P2.4��P2.5��P2.6��P2.7Ϊ׼˫��
       Set OLED port P2.3, P2.4, P2.5, P2.6 as quasi bidirectional */
    P2M1 &= ~0xF8;
    P2M0 &= ~0xF8;
	
    P0M1 |= 0x03;
    P0M0 &= ~0x03;
	
    /* ��������ת�ⲿ�ж� 
       Interrupt for rotating of Encoder */
    IT0 = 0; //�ⲿ�ж�0������ʽ�������غ��½���
    PX0 = 1; //�ⲿ�ж�0�����ȼ����ɴ�ϰ����жϣ������жϰ���ͬʱ��ת
    EX0 = 1; //�����ⲿ�ж�0

    /* �����������ⲿ�ж� 
       Interrupt for clicking of Encoder */
    IT1 = 1; //�ⲿ�ж�1������ʽ���½���
    PX1 = 0; //�ⲿ�ж�1�����ȼ�
    EX1 = 1; //�����ⲿ�ж�1

    /* ��ʱ��0��Ϊ���µ�ص�ѹ��Ϣ��ʱ
       Timer 0, for updating voltage of battery */
    AUXR &= 0x7F; //��ʱ��ʱ��12Tģʽ
    TMOD &= 0xF0; //���ö�ʱ��ģʽ
    TL0 = 0x46;   //���ö�ʱ��ֵ //27MHz��25ms
    TH0 = 0x24;   //���ö�ʱ��ֵ
    TF0 = 0;      //���TF0��־

    ET0 = 1;      //����ʱ��0�ж�
	
	UartInit();
	Timer1Init();
	
    ET1 = 1;      //����ʱ��1�ж�	
	
	
    Read_Options();
    init();
	Delay18us();
    OLED_Init();
    OLED_SetFontWidth(6);
    PlotChart();
    OLED_Display();
	
	/* �����ж�
   Enable global interrupt */
	EA = 1;
	
	
    while (1)
    {
        P_Ready = 0;
		
        /* ���µ�ص�ѹ��Ϣ
           Update voltage infor of battery */
        if (UpdateVbat)
        {
            UpdateVbat = 0;
            VBat = GetVoltage(ADC_BAT, VBAT_LSB);
        }
		
        /* ��������
           Sampling*/
        GetWaveData();

        /* �������ý���
           Enter Settings Interface */
        if (InSettings)
        {
            runWhenInSettrings();
        }

        /* ADC�����ж�
           ADC Sampling Interrupt */
        else if (ADCInterrupt)
        {
            runWhenADCInterrupt();
        }

        /* ADC�������
           ADC Sampling Complete */
        else
        {
            runWhenADCComplete();
        }
    }
}

/* ������������ת
   Analyse Rotation of Encoder */
void Scan_EC11(void)
{
    /* ��ʱȥ����
       Delay to remove jitter */
    Delay500us();

    /* ��ת
       Clockwise */
    if (EC11_A != EC11_B)
    {
        Change_Val(1);
    }

    /* ��ת
       Anticlockwise */
    else if (EC11_A == EC11_B)
    {
        Change_Val(0);
    }
}

/* ��������ת�ж�
   Interrput for Encoder Rotated */
void INT0_interrupt(void) interrupt INT_0
{
    Delay500us();

    /* �������Ƿ񱻰���
       Whether the Encoder is pressed */
    if (!EC11_KEY)
        EC11PressAndRotate = 1;
    else
        EC11PressAndRotate = 0;

    Scan_EC11();
    ADCInterrupt = 1;
    DisplayUpdate = 1;
	
    IE1 = 0; //�����ⲿ�ж�1��־λ
    IE0 = 0; //�����ⲿ�ж�0��־λ
}

/* ����������ж�
   Interrput for Encoder Pressed */
void INT1_interrupt(void) interrupt INT_1
{
    Delay50ms();
    if (!EC11_KEY)
    {
        EC11PressAndRotate = 0;
		
        /*  ��ס������ͬʱ��ת
            Rotate Encoder while pressing  */
        if (EC11PressAndRotate)
        {
            /* �ɱ�������ת�ж�ִ�в���
               Operations Performed by Interrupt of Encoder Rotation */
        }
		
        /*  �������������� - �������棬�л�Stop/Run״̬
            Single click Encoder - Switch Run/Stop in main interface    */
			
        else if (!InSettings)
        {
            EX0 = 0;
            ADCRunning = ~ADCRunning;
            if (ADCRunning)
            {
                WaveUpdate = 1;
                ClearWave = 1;
            }
            else
            {
                DisplayUpdate = 1;
                WaveUpdate = 1;
            }
            ADCInterrupt = 1;
            IE0 = 0;
            IE1 = 0;
        }
    }
    IE1 = 0;
}

/* Ϊ����Vbat��ʱ
   Timer for updating Vbat */
void TIMER0_interrupt(void) interrupt TIMER_0
{
    static uint8 n;

    if (++n >= VBAT_UPDATE_FREQ)
    {
        n = 0;
        UpdateVbat = 1;
        DisplayUpdate = 1;
    }
}


TIMER1_ISR(void) interrupt  3
{
	static uint8 Trg1,read1,cont1;
	static uint8 Trg2,read2,cont2;
	static uint8 Trg3,read3,cont3;
	
	read1 = (P2 & 0x02) ^ 0x02;
	Trg1  = (read1 ^ cont1)  &	read1; 
	cont1 = read1;
	 
	read2 = (P2 & 0x04) ^ 0x04;
	Trg2  = (read2 ^ cont2) & read2; 
	cont2 = read2;
	
	read3 = P5 ^ 0xff;
	Trg3  = (read3 ^ cont3) & read3; 
	cont3 = read3;
	
	if(Trg1)
	{
		/* ������
		   Main Interface */
		if (!InSettings)
		{
			WaveScroll = ~WaveScroll;
			OptionChanged = 1;
			ADCInterrupt = 1;
			DisplayUpdate = 1;
			ClearWave = 0;
		}

		Trg1 = 0;
	}
	
	if(Trg2)
	{
			
			InSettings = ~InSettings;

			/*  �������ý���
				Enter Settings  */
			if (InSettings)
			{
				DisplayUpdate = 1;
				UpdateVbat = 1;
				TF0 = 0; //���㶨ʱ��0�����־
				TR0 = 1; //��ʱ��0��ʼ��ʱ����ʼ��ص�ѹ��Ϣ���¼�ʱ
				IE0 = 0; //�����ⲿ�ж�0�жϱ�־
				EX0 = 1; //�����ⲿ�ж�0����������ת��
			}
			/*  �ص�������
				Retrurn to main interface   */
			else
			{
				TR0 = 0; //���㶨ʱ��0�����־
				TF0 = 0; //��ʱ��0ֹͣ��ʱ��ֹͣ��ص�ѹ��Ϣ���¼�ʱ
				WaveFreq = 0;
				TriFail = 0;
				VMax = 0;
				VMin = 0;
				DisplayUpdate = 1;
				WaveUpdate = 1;
				ClearWave = 0;
			}
			
			ADCInterrupt = 1;	

		Trg2 = 0;
	}

    if(Trg3)
    {

        SP = 0;
        Trg3 = 0;
    }
}