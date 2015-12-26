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


#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

void NVIC_Config()
{
	sd_nvic_SetPriority(UART0_IRQn, 3);
	sd_nvic_EnableIRQ(UART0_IRQn);

	sd_nvic_SetPriority(RTC1_IRQn, 1);
	sd_nvic_EnableIRQ(RTC1_IRQn);

	sd_nvic_SetPriority(SPI1_TWI1_IRQn, 1);
	sd_nvic_EnableIRQ(SPI1_TWI1_IRQn);
}

void Periph_Config()
{
	RTC_Config();
	UART_Init();
	GPS_Init();
	Display_Config();
}

int main()
{
	Periph_Config();
	BLE_Init();
	Advertising_Init();
	NVIC_Config();
	RTC_Start();
	Ext_Flash_Init();

	Ext_Flash_Turn_On(EXT_FLASH_PROGRAM_OP);
	Ext_Flash_Read_Status_Reg();

	while(1)
		__WFE();
/*	uint8_t byte = 0x0F;
	//Display_
	for(uint8_t i=0; i<96; i++)
	{
		display_array[i*13] = i+1;
		for(uint8_t j = 1; j<13; j++)
			display_array[ i*13+ j] = byte;
	}


		for(uint8_t i=0; i<96;i++)
		{
			Display_Write_Line(i);
			RTC_Wait(RTC_MS_TO_TICKS(5));
			RTC_Wait(2);
		}
*/
	/*Advertising_Start();

	UART_Enable();

	UART_Start_Rx();
	GPS_Turn_On();
	uint32_t start_timestamp = RTC_Get_Timestamp();
	while(gga_message.fix_indi == '0' || gga_message.fix_indi == 0)
	{
			__WFE();
	}
*/
	Ble_Uart_Notify_Central(0, &gga_message.fix_indi, 1);


	uint32_t end_timestmap = RTC_Get_Timestamp();

	RTC_Wait(3);
	return 0;
}
