/*
    adc_stc8.c

    Sampling with ADC built in MCU
    
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
*/

#include "adc_stc8.h"

#ifdef STC8

/* scale_h: Time scale 500ms, 200ms, 100ms, 50ms, 20ms, 10ms, 5ms, 2ms, 1ms, 500us, 200us, 100us */
void ADCInit(uint8 scale_h)
{
    uint8 ADC_SPEED;

    switch (scale_h)
    {
    case 0: //500ms
        ADC_SPEED = ADC_SPEED_512;
        break;

    case 1: //200ms
        ADC_SPEED = ADC_SPEED_512;
        break;

    case 2: //100ms
        ADC_SPEED = ADC_SPEED_512;
        break;

    case 3: //50ms
        ADC_SPEED = ADC_SPEED_512;
        break;

    case 4: //20ms
        ADC_SPEED = ADC_SPEED_512;
        break;

    case 5: //10ms
        ADC_SPEED = ADC_SPEED_512;
        break;

    case 6: //5ms
        ADC_SPEED = ADC_SPEED_512;
        break;

    case 7: //2ms
        ADC_SPEED = ADC_SPEED_512;
        break;

    case 8: //1ms
        ADC_SPEED = ADC_SPEED_352;
        break;

    case 9: //500us
        ADC_SPEED = ADC_SPEED_192;
        break;

    case 10: //200us
        ADC_SPEED = ADC_SPEED_32;
        break;

    case 11: //100us
        ADC_SPEED = ADC_SPEED_32;
        break;
    }

    ADCCFG = RESFMT | ADC_SPEED; //???????????????? ADC ??????????????/2/16/16
    ADC_CONTR = ADC_POWER;       //???? ADC ????
    Delay5ms();                  //ADC??????????
}

uint16 ADCRead(uint8 chx)
{
    uint16 res;
    ADC_RES = 0;            //??????????????
    ADC_RESL = 0;           //??????????????
    ADC_CONTR &= 0xF0;      //????ADC????????
    ADC_CONTR |= chx;       //????ADC????????
    ADC_CONTR |= ADC_START; //???? AD ????
    _nop_();
    _nop_();

    while (!(ADC_CONTR & ADC_FLAG))
        ; //???? ADC ????????

    ADC_CONTR &= ~ADC_FLAG;          //??????????
    res = (ADC_RES << 8) | ADC_RESL; //???? ADC ????
    return res;
}

uint16 *GetBGV(void)
{
    uint16 *BGV;
    BGV = (uint16 code *)BGV_ADR;
    return BGV;
}

uint16 GetADC_CHX(uint8 chx)
{
    uint16 ADCx;
    uint8 i;

    ADCInit(0); //ADC ??????

    //????ADC??chx????????Bandgap??????ADC??????
    ADCRead(chx); //??????????????
    ADCRead(chx);
    ADCx = 0;

    for (i = 0; i < 16; i++)
    {
        ADCx += ADCRead(chx);
    }

    ADCx >>= 4; //????????
    return ADCx;
}

uint16 GetVoltage(uint8 chx, uint16 lsb)
{
    uint16 ADCbg;
    uint16 *BGV;
    uint16 ADCx;
    uint16 Vx;

    ADCInit(0); //ADC ??????

    //????ADC????16????????Bandgap??????ADC??????
    ADCbg = GetADC_CHX(ADC_CHS_BG);

    //????ADC??chx??????????????????????ADC??????
    ADCx = GetADC_CHX(chx);

    //????????????????????????????????
    BGV = GetBGV();
    Vx = (uint32)*BGV * ADCx * lsb / ADCbg / 100;
    return Vx;
}

//	ADCRead(chx) Timing:500ms, 200ms, 100ms, 50ms, 20ms, 10ms, 5ms, 2ms, 1ms, 500us, 200us, 100us
//	????????????4??????????????????????????????25????????
//  24MHz
//	ADC_SPEED_512 28us
//	ADC_SPEED_480 26.2us
//	ADC_SPEED_448 25us
//	ADC_SPEED_416 23.8us
//	ADC_SPEED_384 22us
//	ADC_SPEED_352 20us
//	ADC_SPEED_320 18us
//	ADC_SPEED_288 17us
//	ADC_SPEED_256 15us
//	ADC_SPEED_224 14us
//	ADC_SPEED_192 12us
//	ADC_SPEED_160 10.4us
//	ADC_SPEED_128 9us
//	ADC_SPEED_96 7us
//	ADC_SPEED_64 5.6us
//	ADC_SPEED_32 4us
//
//  27MHz
//	ADC_SPEED_512 26us
//	ADC_SPEED_352 19us
//	ADC_SPEED_192 11us
//	ADC_SPEED_32 4us

void switch_Dealy(uint8 scale_h)
{
    switch (scale_h)
    {
        //500ms ADC_SPEED_512
    case 0:
        Delay19971us();
        break;

        //200ms ADC_SPEED_512
    case 1:
        Delay7971us();
        break;

        //100ms	ADC_SPEED_512
    case 2:
        Delay3971us();
        break;

        //50ms ADC_SPEED_512
    case 3:
        Delay1971us();
        break;

        //20ms ADC_SPEED_512
    case 4:
        Delay771us();
        break;

        //10ms ADC_SPEED_512
    case 5:
        Delay371us();
        break;

        //5ms ADC_SPEED_512
    case 6:
        Delay171us();
        break;

        //2ms ADC_SPEED_512
    case 7:
        Delay51us();
        break;

        //1ms ADC_SPEED_352
    case 8:
        Delay18us();
        break;

        //500us	ADC_SPEED_192
    case 9:
        Delay6us();
        break;

        //200us ADC_SPEED_32
    case 10:
        /* ????????????????
           fine tuning the delay */
        _nop_();
        //_nop_();
        break;

        //100us ADC_SPEED_32
    case 11:

        break;
    }
}

uint16 *GetWaveADC(uint8 chx, uint8 scale_h)
{
    uint8 i, j;
    static uint16 ADCSampling[SAMPLE_NUM];
    uint16 ADCPreSampling[PRE_BUF_NUM + 1]; //??????????????????????????PRE_BUF_NUM??????????????????????????????????????????????????????????

    ADCComplete = 0; //????????????????
    if (ADCInterrupt)
        return ADCSampling;
    memset(ADCSampling, 0x00, SAMPLE_NUM * 2);
    memset(ADCPreSampling, 0x00, (PRE_BUF_NUM + 1) * 2);
    //BGV = GetBGV(); //??????????????
    ADCbg = GetADC_CHX(ADC_CHS_BG);                         //????????ADC??
    TriggerADC = Convert_mv_ADC(TriLevel, BGV, ADCbg, Lsb); //????????????????????????????ADC??

    //????????????A/D??????????????????????????????????????????????????????A/D????
    ADCInit(scale_h);
    //????ADC????x??????????????????????ADC??????
    ADCRead(chx); //??????????????
    ADCRead(chx);

    //????????100us????????????????????????????????????????????????????4us????????????????????????????
    if (scale_h == 11) //100 us
    {
        P_Ready = 1; //????????????????????
        for (i = 0; i < SAMPLE_NUM; i++)
        {
            if (ADCInterrupt)
                return ADCSampling;
            ADCSampling[i] = ADCRead(chx);
#ifdef DEBUG
            P15 = ~P15;
#endif
        }
        P_Ready = 0; //????????????????????
    }

    /* ??????????????????
       Single or Normal Trigger Mode */
    else if (TriMode)
    {
        P_Ready = 0;                       //????????????????????????????????????????????????????????????
        for (j = 1; j <= PRE_BUF_NUM; j++) //??????PRE_BUF_NUM????????????????????????????????????????????????????????????????????????????????????
        {
            if (ADCInterrupt)
                return ADCSampling;

            Delay3us();            //??????????????????????????????????3us
            switch_Dealy(scale_h); //????????
            ADCPreSampling[j] = ADCRead(chx);
#ifdef DEBUG
            P15 = ~P15;
#endif
        }
        P_Ready = 1; //??????????????????????????????????????

        //????????????????PRE_BUF_NUM??????????????????????????????????????
        while (1)
        {
            if (ADCInterrupt)
                return ADCSampling;

            //??????????????1????????????????????????????????????
            if (j > PRE_BUF_NUM)
            {
                j = 1;
                ADCPreSampling[0] = ADCPreSampling[PRE_BUF_NUM];
            }
            switch_Dealy(scale_h); //????????
            ADCPreSampling[j] = ADCRead(chx);
            if (GetTriggerPos(ADCPreSampling[j - 1], ADCPreSampling[j], TriggerADC, TriSlope)) //????????????????
            {
                P_Ready = 0; //??????????????while????????????????
                break;
            }
            j++;
#ifdef DEBUG
            P15 = ~P15;
#endif
        }

        //????????AFT_BUF_NUM????????
        for (i = 0; i < AFT_BUF_NUM; i++)
        {
            if (ADCInterrupt)
                return ADCSampling;

            Delay3us();            //??????????????????????????????????3us
            switch_Dealy(scale_h); //????????
            ADCSampling[i + PRE_BUF_NUM] = ADCRead(chx);
#ifdef DEBUG
            P15 = ~P15;
#endif
        }

        //????PRE_BUF_NUM??????PRE_BUF_NUM??????????????????????
        for (i = 0; i < PRE_BUF_NUM; i++) //????????????????????????????????????????????????????????PRE_BUF_NUM-1????????????????????????????ADCSampling????PRE_BUF_NUM-1????????
        {
            if (ADCInterrupt)
                return ADCSampling;
            if (++j > PRE_BUF_NUM) //??????????????????????????????????
                j = 1;

            ADCSampling[i] = ADCPreSampling[j];
        }
    }

    /* ????????
       Auto Trigger Mode */
    else
    {
        P_Ready = 1; //????????????????????
        for (i = 0; i < SAMPLE_NUM; i++)
        {
            if (ADCInterrupt)
                return ADCSampling;
            ADCSampling[i] = ADCRead(chx);
            Delay3us();            //??????????????????????????????3us
            switch_Dealy(scale_h); //????????
#ifdef DEBUG
            P15 = ~P15;
#endif
        }
        P_Ready = 0; //????????????????????
    }
    ADCComplete = 1; //????????????????
    return ADCSampling;
}

#endif
