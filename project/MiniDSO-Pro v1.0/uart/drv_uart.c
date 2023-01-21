/*------------------------------------------------------------------------
|                            FILE DESCRIPTION                            |
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
|  - File name     : drv_uart.c
|  - Author        : zeweni
|  - Update date   : 2021.05.08
|  - Copyright(C)  : 2021-2021 zeweni. All rights reserved.
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
|                            COPYRIGHT NOTICE                            |
------------------------------------------------------------------------*/
/*
 * Copyright (C) 2021, zeweni (17870070675@163.com)

 * This file is ELL oscilloscope System.

 * ELL oscilloscope System is free software: you can redistribute it and/or 
 * modify it under the terms of the Apache-2.0 License.

 * ELL oscilloscope System is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTA-
 * BILITY or FITNESS FOR A PARTICULAR PURPOSE. See the Apache-2.0 License 
 * for more details.

 * You should have received a copy of the Apache-2.0 License.ELL oscillos- 
 * cope System. If not, see <http://www.apache.org/licenses/>.
**/
/*------------------------------------------------------------------------
|                                INCLUDES                                |
------------------------------------------------------------------------*/

#include "drv_uart.h"
#include "stdio.h"
#include "reg52.h"
/*------------------------------------------------------------------------
|                                  MACRO                                 |
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
|                                  DATA                                  |
------------------------------------------------------------------------*/

COMx_Define	COM1;

uint8 xdata TX1_Buffer[COM_TX1_Lenth];

/*------------------------------------------------------------------------
|                              API FUNCTIONS                             |
------------------------------------------------------------------------*/

void TX1_write_buff(uint8 dat)	
{
    TX1_Buffer[COM1.TX_write] = dat;	
    if(++COM1.TX_write >= COM_TX1_Lenth) COM1.TX_write = 0;

    if(COM1.B_TX_busy == 0) 
	{	
        COM1.B_TX_busy = 1;
		TI = 1;	
    }
}

void UART1_Print_String(uint8 *puts)
{
    for (; *puts != 0;	puts++)  
		TX1_write_buff(*puts); 	
	
}

/**
  * @name    UART1_ISRQ_Handler
  * @brief   MCU UART1 Interrupt request service function
  * @param   None
  * @return  None
***/
void UART1_ISRQ_Handler(void) interrupt  4
{

	if((SCON  & 0x02))
	{
        SCON  &= ~0x02;
        
        if(COM1.TX_read != COM1.TX_write)
        {
        	SBUF = TX1_Buffer[COM1.TX_read];
            TX1_Buffer[COM1.TX_read] = 0;
            
        	if(++COM1.TX_read >= COM_TX1_Lenth)	COM1.TX_read = 0;
        }
        else	COM1.B_TX_busy = 0;

	}
	

}


char putchar(char c)
{

    TX1_write_buff((uint8)c);
	
	return c;
}
/*------------------------------------------------------------------------
|                    END OF FLIE.  (C) COPYRIGHT zeweni                  |
------------------------------------------------------------------------*/