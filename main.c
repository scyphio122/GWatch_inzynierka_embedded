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
#include "adc.h"
#include "hardware_settings.h"
#include "nrf_gpio.h"

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

static uint8_t 	bat_voltage = 100;
uint8_t 		battery_level;

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

	sd_nvic_SetPriority(ADC_IRQn, 3);
	sd_nvic_EnableIRQ(ADC_IRQn);
}

void Periph_Config()
{
	RTC_Config();
	RTC_Start();

	UART_Init();
	GPS_Init();
	Display_Config();
	Timer1_Init();
	ADC_Init();
	RTC_Schedule_IRQ(RTC_US_TO_TICKS(1000000), &NRF_RTC1->CC[2]);
	Ext_Flash_Init();

	nrf_gpio_cfg_output(ACCELER_MAG_ON_PIN);
	NRF_GPIO->OUTSET = 1 << ACCELER_MAG_ON_PIN;
	nrf_gpio_cfg_output(ACCELER_CS_PIN);
	NRF_GPIO->OUTSET = 1 << ACCELER_CS_PIN;
	}

void Calculate_Battery_Level()
{
	battery_level = ((bat_voltage - ADC_RESULT_3_VOLT)*100) / ADC_WORK_RANGE;
}

//#define NO_BLE

int main()
{
	uint8_t sample_stored = 0;

	BLE_Init();
	NVIC_Config();

	Periph_Config();
	Advertising_Init();

	Scheduler_Init();
	Mem_Org_Init();

#ifndef NO_BLE
	Advertising_Start();
#endif

	///	Set default timestamp to 01/01/2016 00:00:00
	RTC_Set_Timestamp(1451606400);

	while(1)
	{
		__WFE();
		if(gps_message_sample_storage_time && (mem_org_track_samples_storage_enabled == 1) && (gps_sample_nr % mem_org_gps_sample_storage_interval == 0))//if(gps_message_sample_storage_time && (mem_org_track_samples_storage_enabled == 1) && (gga_message.fix_indi != '0') && (gps_sample_nr % mem_org_gps_sample_storage_interval == 0))
		{
			Mem_Org_Store_Sample(gps_sample_timestmap);
			Ble_Uart_Data_Send(0xFF, NULL, 0, false);
			gps_message_sample_storage_time = 0;
			sample_stored = 1;
		}
		else
		{
			sample_stored = 0;
		}

		Ble_Uart_Execute_Ble_Requests_If_Available();

		if(disp_updt_time)
		{
			ADC_Get_Bat_Voltage(&bat_voltage);
			Calculate_Battery_Level();

			Display_Write_Time();
			Display_Write_Latitude();
			Display_Write_Longtitude();
			Display_Update_GPS_Power_On();
			Display_Update_Battery_Level();
			Display_Update_Sampling_Status(sample_stored);

			Display_Flush_Buffer();
			disp_updt_time = 0;
		}
	}

	return 0;
}
