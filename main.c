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
#include "ble_gwatch.h"
#include "nrf_soc.h"
#include "display.h"
#include "ext_flash.h"
#include "ble_uart.h"
#include "libraries/memory_organization.h"
#include "libraries/scheduler.h"
#include "timer.h"


#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

//#define NO_BLE

void NVIC_Config()
{
	sd_nvic_SetPriority(UART0_IRQn, 3);
	sd_nvic_EnableIRQ(UART0_IRQn);

	sd_nvic_SetPriority(RTC1_IRQn, 1);
	sd_nvic_EnableIRQ(RTC1_IRQn);

	sd_nvic_SetPriority(SPI1_TWI1_IRQn, 3);
	sd_nvic_EnableIRQ(SPI1_TWI1_IRQn);

	sd_nvic_SetPriority(TIMER1_IRQn, 3);
	sd_nvic_EnableIRQ(TIMER1_IRQn);
}

void Periph_Config()
{
	RTC_Config();
	RTC_Start();

	UART_Init();
	GPS_Init();
	Display_Config();
	Timer1_Init();
}

//#define NO_BLE

int main()
{
	BLE_Init();
	NVIC_Config();

	Periph_Config();
	Advertising_Init();
	Ext_Flash_Init();
	Scheduler_Init();
	Mem_Org_Init();

	//Display_Test();


#ifndef NO_BLE
	Advertising_Start();
#endif

	GPS_Turn_On();


	RTC_Set_Timestamp(1452341754);
	while(1)
	{

		if(gps_message_sample_storage_time && (mem_org_track_samples_storage_enabled == 1) && (gga_message.fix_indi != '0') && (gps_sample_nr % mem_org_gps_sample_storage_interval == 0))
		{
			Mem_Org_Store_Sample(gps_sample_timestmap);
			Ble_Uart_Data_Send(0xFF, NULL, 0, false);
			gps_message_sample_storage_time = 0;
		}

		__WFE();
		Ble_Uart_Execute_Ble_Requests_If_Available();
		if(disp_updt_time)
		{
			Display_Write_Time();
			Display_Write_Latitude();
			Display_Write_Longtitude();
			disp_updt_time = 0;
		}
	}

	return 0;
}
