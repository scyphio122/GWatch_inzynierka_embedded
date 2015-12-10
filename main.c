/*
 * main.c

 *
 *  Created on: 10 gru 2015
 *      Author: Konrad
 */

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

int main()
{
	RTC_Config();
	UART_Init();
	GPS_Init();

	while(1);
	{

	}
	return 0;
}
