/*------------------------------------------------------------------------
|                            FILE DESCRIPTION                            |
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
|  - File name     : drv_uart.h
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
#ifndef __DRV_UART_H_
#define __DRV_UART_H_


#include "global.h"

/*------------------------------------------------------------------------
|                                  MACRO                                 |
------------------------------------------------------------------------*/

#define COM_TX1_Lenth 255

/*------------------------------------------------------------------------
|                                  DATA                                  |
------------------------------------------------------------------------*/

typedef struct
{ 
	uint8	id;				//串口号

	uint8	TX_read;		//发送读指针
	uint8	TX_write;		//发送写指针
	uint8	B_TX_busy;		//忙标志

	uint8	RX_Cnt;			//接收字节计数
	uint8	RX_TimeOut;		//接收超时
	uint8	B_RX_OK;		//接收块完成
} COMx_Define; 

extern uint8	xdata TX1_Buffer[COM_TX1_Lenth];	//发送缓冲

/*------------------------------------------------------------------------
|                              API FUNCTIONS                             |
------------------------------------------------------------------------*/

void UART1_Print_String(uint8 *puts);
void TX1_write_buff(uint8 dat);


#endif
/*------------------------------------------------------------------------
|                    END OF FLIE.  (C) COPYRIGHT zeweni                  |
------------------------------------------------------------------------*/