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
#include "Clock.h"

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)




int main()
{
	NVIC_SetPriority(UART0_IRQn, 0);
	NVIC_EnableIRQ(UART0_IRQn);
	 HFCLK_Clock_Configure(CLOCK_XTALFREQ_XTALFREQ_16MHz);
	LFCLK_Clock_Configure(1);
	RTC_Config();
	UART_Init();
	GPS_Init();
	RTC_Start();
	UART_Enable();
	UART_Start_Rx();
	GPS_Turn_On();

	while(1)
	{

	}

	return 0;
}
