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

#define NO_BLE

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
	UART_Init();
	GPS_Init();
	Display_Config();
	Timer1_Init();
}

//#define NO_BLE

int main()
{
	Periph_Config();
	BLE_Init();
	Advertising_Init();
	NVIC_Config();
	RTC_Start();
	Ext_Flash_Init();
	Scheduler_Init();
	Mem_Org_Init();

	//Display_Test();
	Display_Clear();
	/*uint8_t* ptr = malloc(12);
	memcpy(ptr, &"1", 1);
	while(1)
	{
		Display_Write_Text(ptr, 12, 16, true, false);
		RTC_Wait(RTC_S_TO_TICKS(2));
		Display_Write_Text(ptr, 12, 16, false, false);
		RTC_Wait(RTC_S_TO_TICKS(2));
	}*/


	/*Mem_Org_Track_Start_Storage();
	Mem_Org_Store_Sample(0x01234567);
	Mem_Org_Store_Sample(0x89ABCDEF);
	Mem_Org_Track_Stop_Storage();

	Mem_Org_Track_Start_Storage();
	Mem_Org_Store_Sample(0x01234567);
	Mem_Org_Store_Sample(0x89ABCDEF);
	Mem_Org_Track_Stop_Storage();

	uint32_t key = 0;
	uint32_t err_code = 0;
	err_code = Mem_Org_Find_Key(1, &key);
	err_code = Mem_Org_Find_Key(2, &key);
	err_code = Mem_Org_Find_Key(3, &key);
	err_code = Mem_Org_Find_Key(4, &key);
	err_code = Mem_Org_Find_Key(5, &key);
	err_code = Mem_Org_Find_Key(6, &key);
	err_code = Mem_Org_Find_Key(7, &key);
	err_code = Mem_Org_Find_Key(8, &key);
	err_code = Mem_Org_Find_Key(9, &key);
	err_code = Mem_Org_Find_Key(10, &key);
	err_code = Mem_Org_Find_Key(11, &key);
//		Mem_Org_Clear_Tracks_Memory();*/
#ifndef NO_BLE
	Advertising_Start();
#endif
	UART_Enable();

	UART_Start_Rx();
	GPS_Turn_On();
	/*uint32_t start_timestamp = RTC_Get_Timestamp();
	while(gga_message.fix_indi == '0' || gga_message.fix_indi == 0)
	{
			__WFE();
	}

	//Ble_Uart_Notify_Central(0, &gga_message.fix_indi, 1);


	uint32_t end_timestmap = RTC_Get_Timestamp();

	RTC_Wait(3);*/
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
			disp_updt_time = 0;
		}
	}

	return 0;
}
