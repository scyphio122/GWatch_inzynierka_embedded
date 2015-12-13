/*
 * main.c

 *
 *  Created on: 10 gru 2015
 *      Author: Konrad
 */

#include <libraries/fifo.h>
#include "UART.h"
#include "GPS.h"
#include "RTC.h"

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)




int main()
{
	RTC_Config();
	UART_Init();
	GPS_Init();
	Fifo_Init(&uart_rx_fifo, uart_fifo_buffer, sizeof(uart_fifo_buffer));

	UART_Enable();
	UART_Start_Rx();
	GPS_Turn_On();

	while(1)
	{

	}

	return 0;
}
