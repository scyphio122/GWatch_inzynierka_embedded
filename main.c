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

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in]   sys_evt   System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
	pstorage_sys_event_handler(sys_evt);
	SD_flash_operation_callback(sys_evt);
}
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

#ifndef NO_BLE
	Advertising_Start();
#endif
	UART_Enable();

	UART_Start_Rx();
	GPS_Turn_On();
	uint32_t start_timestamp = RTC_Get_Timestamp();
	while(gga_message.fix_indi == '0' || gga_message.fix_indi == 0)
	{
			__WFE();
	}

	Ble_Uart_Notify_Central(0, &gga_message.fix_indi, 1);


	uint32_t end_timestmap = RTC_Get_Timestamp();

	RTC_Wait(3);
	return 0;
}
