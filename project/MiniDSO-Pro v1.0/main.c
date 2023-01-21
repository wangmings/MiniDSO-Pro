/*

    简易示波器

    操作说明：
        主界面-选项设置模式（按Option按键）：
            单击编码器：切换开始/暂停采样
            按下同时旋转编码器：在参数间切换

        主界面-波形水平滚动模式：
            单击编码器：切换开始/暂停采样
            旋转编码器：水平滚动波形（在暂停采样状态下有效）
            按下同时旋转编码器：垂直滚动波形（仅在暂停采样状态下有效）
            
        设置界面（按Setting按键）：

            长按编码器：返回主界面
            旋转编码器：修改当前选项
            按下同时旋转编码器：切换选项

    主界面参数：
        横轴时间区间："500ms", "200ms", "100ms", "50ms", "20ms", "10ms","5ms", "2ms", "1ms", "500us", "200us", "100us"(100us仅在自动触发模式下可用)
        纵轴电压区间：设定电压量程上限，最大30V，顺时针旋转编码器将电压上限设置为0则进入自动量程，逆时针旋转进入手动量程
        触发值：设定触发电压值，触发位置被放在屏幕中心
        触发方向：上下箭头，分别代表上升沿触发和下降沿触发
        触发方式：
            Auto(自动触发)  ：连续采样，指示灯始终点亮，通过单击编码器控制启停；触发成功时，触发点位于屏幕中心，触发失败则波形无规律滚动，屏幕左侧显示Fail标志
            Normal(普通触发)：等待预采样，待指示灯亮起表示预采样结束，这时可以输入信号；
                              触发成功后，显示波形，并继续等待下次触发；
                              如果有新的触发成功，则自动显示新的波形，如没有新的触发成功则屏幕波形保持并一直等待下次触发；
                              需注意设置正确的触发值，否则一直处于等待触发状态，屏幕不会显示波形
            Single(单次触发)：等待预采样，待指示灯亮起表示预采样结束，这时可以输入信号；
                              触发成功后，显示波形，并停止采样，需用户单击编码器开始下次采样	
                              需注意设置正确的触发值，否则一直处于等待触发状态，屏幕不会显示波形
            
    主界面状态显示：	
        Run：正在采样
        Stop：停止采样
        Fail：自动模式下，触发值超出波形范围引起触发失败
		Succe：自动模式下，触发值在波形范围显示触发成功
        Auto：自动量程
            
    设置界面选项：
        PMode(绘图模式)：
            Vector：波形以矢量显示
            Dots  ：波形以点阵显示
        LSB(采样系数)：分压系数的100倍，比如用10k加2k的电阻进行分压，分压系数为(10+2/2)=6，LSB理论取值为6x100=600，根据实际电阻精度进行微调	
        BRT：OLED亮度

    保存设置：长按编码器退出设置界面时，设置界面和主界面的所有参数被保存到EEPROM


	
	
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
		while (!TI);	//等待发送成功
		TI = 0;	  //清除发送中断标志
        /* output CR  */
    }
    SBUF = c;
    while (!TI);	//等待发送成功
    TI = 0;	  //清除发送中断标志
    return c;
}

void Timer1Init(void)		//20毫秒@27.000MHz
{
	AUXR &= 0xBF;		//定时器时钟12T模式
	TMOD &= 0x0F;		//设置定时器模式
	TL1 = 0x38;		//设置定时初始值
	TH1 = 0x50;		//设置定时初始值
	TF1 = 0;		//清除TF1标志
	TR1 = 1;		//定时器1开始计时
}

sfr T2H = 0xD6;
sfr T2L = 0xD7;

void UartInit(void)		//9600bps@24.000MHz
{
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x01;		//串口1选择定时器2为波特率发生器
	AUXR |= 0x04;		//定时器时钟1T模式
	T2L = 0x8F;		//设置定时初始值
	T2H = 0xFD;		//设置定时初始值
	AUXR |= 0x10;		//定时器2开始计时
	
}


void init()
{
	
    InSettings = 0;         //初始化开机界面
    OptionInSettings = 0;   //初始化设置界面的选项编号
    OptionInChart = 0;      //初始化主界面的选项标号
    WaveScroll = 0;         //清零波形滚动标志
    OptionChanged = 0;      //清零设置修改标志
    ADCRunning = 1;         //置位采样标志
    ADCInterrupt = 0;       //清零ADC中断标志
    ADCComplete = 0;        //清零ADC完成标志
    WaveUpdate = 0;         //清零波形更新标志
    ClearWave = 0;          //清零清除波形标志
    ClearDisplay = 1;       //置位清空屏幕标志
    EC11PressAndRotate = 0; //清零编码器旋转标志
    UpdateVbat = 0;         //清零采样电池电压标志
    ADCComplete = 0;        //清零ADC完成采样标志
    WaveFreq = 0;           //清零波形频率
    TriPos = 50;            //初始化触发点位置
    TriPosOffset = 0;       //初始化触发点偏移量
    TriFail = 0;            //清零触发失败标志
    VMax = 0;               //清零波形电压最大值
    VMin = 0;               //清零波形电压最小值
    WaveLengthSumNum = 0;   //清零波形长度求和次数
    WaveLengthSum = 0;      //清零波形长度求和
	
	PlotMode = 0;
	
	Bias_Voltage = 8900; //偏置电压权重
	
	TriLevel_m = 2000;
	
	VMax_m = 0;
	VMin_m = 0;
	Mvoltage = 0;
	RulerVMax_m = 0;
	
	MeaWay = Option_MeaWay_AC;
	
    /* 波形滚动模式下正在采样则禁用外部中断0（编码器旋转）
       Disable external interrupt 0(Encoder rotation) in waveform scroll mode when sampling*/
    if (ADCRunning && WaveScroll)
        EX0 = 0;

    /* 获取内部参考电压值
       Get internal reference voltage */
    BGV = GetBGV();
}

void runWhenInSettrings()
{
    P_Ready = 0;
    ClearDisplay = 1;
    while (InSettings)
    {
        /* 更新电池电压信息
           Update voltage infor of battery */
        if (UpdateVbat)
        {
            UpdateVbat = 0;
            VBat = GetVoltage(ADC_BAT, VBAT_LSB);
        }
		
        /* 更新屏幕亮度
           Update OLED Brightness */
        if (OLED_BrightnessChanged)
        {
            OLED_BrightnessChanged = 0;
            OLED_Write_Command(0x81);
            OLED_Write_Command(OLED_Brightness * 10);
        }

        /* 更新参数
           Update parameters */
        if (DisplayUpdate)
        {
            DisplayUpdate = 0;
            PlotSettings();
            OLED_Display();
        }
    }

    /* 保存设置
       Save Settings */
    EA = 0;
    if (OptionChanged)
    {
        OptionChanged = 0;
        ClearDisplay = 0;
        /* 保存设置并显示保存状态
           Save settings and display saving status*/
        PlotSaveStatus(Save_Options());
        OLED_Display();
        Delay1000ms();
    }

    /* 退出settings后刷新屏幕
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

    /* ADC采样运行
       ADC Sampling Running */
    if (ADCRunning)
    {
        EX0 = 0;

        if (WaveUpdate)
        {
            WaveUpdate = 0;
            ClearDisplay = 1;

            /* 清空波形缓存
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
            /* 显示缓存中的波形
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

        /* ADCRuning和WaveScroll同时置位时，不开启外部中断0（编码器旋转）
           Not to enable external interrupt 0(Encoder Rotation) when both of ADCRunning and WaveScroll set. */
        if (!WaveScroll)
        {
            IE0 = 0;
            EX0 = 1;
        }
    }

    /* ADC采样停止
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
                    /* 分析缓存中完成采样的采样数据
                       Analyse completed sampling data in buffer */
                    AnalyseData();
                    ClearDisplay = 1;

                    /* 清空屏幕，绘制波形
                       Clear display and draw waveform */
                    if (ScaleH == ScaleH_tmp)
                    {
                        /* 采样点数有限，不支持横向缩放波形，只有时间区间与完成采样的时间区间相同才绘制波形
                           Since the sampling points limitation, scaling waveform along horizontal is not support.
                           Show waveform only when time scale is same as that when sampling completed */
                        PlotChart();
                        PlotWave();
                    }

                    /* 清空屏幕，不绘制波形
                       Clear display and no waveform */
                    else
                    {
                        WaveFreq = 0; //清零WaveFreq，绘制界面时，频率位置会显示****Hz
                        PlotChart();
                    }
                }
                /* 只更新屏幕上的参数，波形不变
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
    /*  ADC采样完成 - 单次触发
        ADC Sampling Complete - Single Trigger Mode */
    if (TriMode == 2)
    {
        EX0 = 0;
        ADCInterrupt = 1;
        DisplayUpdate = 0;
        WaveUpdate = 0;
        ADCRunning = 0; //清零ADC运行标志，停止采样
        AnalyseData();
        ClearDisplay = 1;
        PlotChart();
        PlotWave();
        OLED_Display();
        IE0 = 0;
        EX0 = 1;
    }

    /*  ADC采样完成 - 自动触发或普通触发
        ADC Sampling Complete - Auto or Normal Trigger Mode */
    else
    {
        //EX0 = 0;
        ClearDisplay = 1; //置位清空屏幕标志
        AnalyseData();    //分析采样数据
        PlotChart();      //绘制主界面
        PlotWave();       //绘制波形
        OLED_Display();
        //IE0 = 0;
        //EX0 = 1;
    }
}

/* 程序入口
   Program Entry */
void main()
{
    /* 将ADC端口P0.4，P0.6设置为高阻
       Set ADC port P0.4, P0.6 as high-impedance */
    P0M1 |= 0x50;
    P0M0 &= ~0x50;

    /* 设置指示灯端口P1.1为推挽输出
       Set indicator port P1.1 as push-pull output */
    P0M1 &= ~0x02;
    P0M0 |= 0x02;

    /* 设置编码器端口P3.2，P3.3，P3.4为准双向
       Set Encoder port P3.2, P3.3, P3.4 as quasi bidirectional */
    P3M1 &= ~0x1C;
    P3M0 &= ~0x1C;

    /* 设置OLED端口P2.3，P2.4，P2.5，P2.6，P2.7为准双向
       Set OLED port P2.3, P2.4, P2.5, P2.6 as quasi bidirectional */
    P2M1 &= ~0xF8;
    P2M0 &= ~0xF8;
	
    P0M1 |= 0x03;
    P0M0 &= ~0x03;
	
    /* 编码器旋转外部中断 
       Interrupt for rotating of Encoder */
    IT0 = 0; //外部中断0触发方式，上升沿和下降沿
    PX0 = 1; //外部中断0高优先级，可打断按键中断，用于判断按下同时旋转
    EX0 = 1; //开启外部中断0

    /* 编码器按键外部中断 
       Interrupt for clicking of Encoder */
    IT1 = 1; //外部中断1触发方式，下降沿
    PX1 = 0; //外部中断1低优先级
    EX1 = 1; //开启外部中断1

    /* 定时器0，为更新电池电压信息计时
       Timer 0, for updating voltage of battery */
    AUXR &= 0x7F; //定时器时钟12T模式
    TMOD &= 0xF0; //设置定时器模式
    TL0 = 0x46;   //设置定时初值 //27MHz，25ms
    TH0 = 0x24;   //设置定时初值
    TF0 = 0;      //清除TF0标志

    ET0 = 1;      //允许定时器0中断
	
	UartInit();
	Timer1Init();
	
    ET1 = 1;      //允许定时器1中断	
	
	
    Read_Options();
    init();
	Delay18us();
    OLED_Init();
    OLED_SetFontWidth(6);
    PlotChart();
    OLED_Display();
	
	/* 开总中断
   Enable global interrupt */
	EA = 1;
	
	
    while (1)
    {
        P_Ready = 0;
		
        /* 更新电池电压信息
           Update voltage infor of battery */
        if (UpdateVbat)
        {
            UpdateVbat = 0;
            VBat = GetVoltage(ADC_BAT, VBAT_LSB);
        }
		
        /* 采样波形
           Sampling*/
        GetWaveData();

        /* 进入设置界面
           Enter Settings Interface */
        if (InSettings)
        {
            runWhenInSettrings();
        }

        /* ADC采样中断
           ADC Sampling Interrupt */
        else if (ADCInterrupt)
        {
            runWhenADCInterrupt();
        }

        /* ADC采样完成
           ADC Sampling Complete */
        else
        {
            runWhenADCComplete();
        }
    }
}

/* 分析编码器旋转
   Analyse Rotation of Encoder */
void Scan_EC11(void)
{
    /* 延时去抖动
       Delay to remove jitter */
    Delay500us();

    /* 正转
       Clockwise */
    if (EC11_A != EC11_B)
    {
        Change_Val(1);
    }

    /* 反转
       Anticlockwise */
    else if (EC11_A == EC11_B)
    {
        Change_Val(0);
    }
}

/* 编码器旋转中断
   Interrput for Encoder Rotated */
void INT0_interrupt(void) interrupt INT_0
{
    Delay500us();

    /* 编码器是否被按下
       Whether the Encoder is pressed */
    if (!EC11_KEY)
        EC11PressAndRotate = 1;
    else
        EC11PressAndRotate = 0;

    Scan_EC11();
    ADCInterrupt = 1;
    DisplayUpdate = 1;
	
    IE1 = 0; //清零外部中断1标志位
    IE0 = 0; //清零外部中断0标志位
}

/* 编码器点击中断
   Interrput for Encoder Pressed */
void INT1_interrupt(void) interrupt INT_1
{
    Delay50ms();
    if (!EC11_KEY)
    {
        EC11PressAndRotate = 0;
		
        /*  按住编码器同时旋转
            Rotate Encoder while pressing  */
        if (EC11PressAndRotate)
        {
            /* 由编码器旋转中断执行操作
               Operations Performed by Interrupt of Encoder Rotation */
        }
		
        /*  单击编码器按键 - 在主界面，切换Stop/Run状态
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

/* 为更新Vbat计时
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
		/* 主界面
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

			/*  进入设置界面
				Enter Settings  */
			if (InSettings)
			{
				DisplayUpdate = 1;
				UpdateVbat = 1;
				TF0 = 0; //清零定时器0溢出标志
				TR0 = 1; //定时器0开始计时，开始电池电压信息更新计时
				IE0 = 0; //清零外部中断0中断标志
				EX0 = 1; //开启外部中断0（编码器旋转）
			}
			/*  回到主界面
				Retrurn to main interface   */
			else
			{
				TR0 = 0; //清零定时器0溢出标志
				TF0 = 0; //定时器0停止计时，停止电池电压信息更新计时
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