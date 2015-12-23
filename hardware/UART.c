/*
 * UART.c
 *
 *  Created on: 10 gru 2015
 *      Author: Konrad
 */

//#include <core_cmInstr.h>
#include <libraries/fifo.h>
#include <hardware_settings.h>
#include <nrf_gpio.h>
#include <nrf51.h>
#include <nrf51_bitfields.h>
#include <RTC.h>
#include <stdbool.h>
#include <sys/_stdint.h>
#include <UART.h>
#include "GPS.h"

app_fifo_t 				uart_rx_fifo;
uint8_t 				uart_fifo_buffer[UART_FIFO_SIZE];

static uint8_t* 		tx_buffer;
static uint16_t			tx_byte_counter;
static uint16_t 		tx_byte_counter;
static uint16_t 		tx_data_size;
static volatile uint8_t	uart_tx_transmission_in_progress;

void UART0_IRQHandler()
{
	static uint8_t data_byte = 0;
	static uint8_t end_of_checksum_index = 255;
	if(NRF_UART0->EVENTS_RXDRDY)
	{
		///	Clear the rx interrupt flag
		NRF_UART0->EVENTS_RXDRDY = 0;
		data_byte = NRF_UART0->RXD;
		///	Put the data byte in the fifo
		app_fifo_put(&uart_rx_fifo, data_byte);
		if(data_byte == '*')
			end_of_checksum_index = gps_msg_byte_index;
		if((gps_msg_byte_index > 0) && (gps_msg_byte_index < end_of_checksum_index))
			gps_msg_checksum ^= data_byte;
		if(data_byte == '\n')
		{
			gps_msg_size = gps_msg_byte_index + 1;
			gps_msg_byte_index = 0;
			gps_msg_received = 1;

			GPS_Parse_Message();
			gps_msg_received = 0;
			end_of_checksum_index = 255;
		}
		else
			++gps_msg_byte_index;
	}

	if(NRF_UART0->EVENTS_TXDRDY)
	{
		///	Clear the interrupt flag
		NRF_UART0->EVENTS_TXDRDY = 0;
		///	If there is still more data to send...
		if(tx_byte_counter < tx_data_size)
		{
			/// ...then put it on the UART TX line
			NRF_UART0->TXD = tx_buffer[tx_byte_counter++];
		}
		else	///	If all the data has been send...
		{
			///	Clear the transmission in progrss flag
			uart_tx_transmission_in_progress = false;
			///	Turn off the transmitter in order to save some current
			UART_Stop_Tx();
		}
	}

	if(NRF_UART0->EVENTS_ERROR)
	{
		NRF_UART0->EVENTS_ERROR = 0;
		switch(NRF_UART0->ERRORSRC)
		{
		case UART_ERRORSRC_BREAK_Msk:
		{
			break;
		}
		case UART_ERRORSRC_FRAMING_Msk:
		{

			break;
		}
		case UART_ERRORSRC_OVERRUN_Msk:
		{
			break;
		}

		}
	}

}

/**
 * This function configures the UART peripheral
 */
void UART_Init()
{
	/// Configure pins
	nrf_gpio_cfg_output(UART_TX_PIN);
	nrf_gpio_cfg_input(UART_RX_PIN, NRF_GPIO_PIN_NOPULL);
	NRF_GPIO->OUTSET = 1 << UART_TX_PIN;

	NRF_UART0->PSELRXD = UART_RX_PIN;
	NRF_UART0->PSELTXD = UART_TX_PIN;

	/// Disable the parity check
	NRF_UART0->CONFIG = 0;
	///	Configure the baudrate
	NRF_UART0->BAUDRATE = UART_BAUDRATE_BAUDRATE_Baud9600;

	/// Initialize the UART fifo
	Fifo_Init(&uart_rx_fifo, uart_fifo_buffer, sizeof(uart_fifo_buffer));

}

/**
 * This function enables the peripheral
 */
inline void UART_Enable()
{
	NRF_UART0->ENABLE = 4;
}

/**
 * This function turns on the transmitter
 */
inline void UART_Start_Tx()
{
	NRF_UART0->INTENSET = UART_INTENSET_TXDRDY_Msk;
	NRF_UART0->TASKS_STARTTX = 1;
}

/**
 * This function turns off the transmitter
 */
inline void UART_Stop_Tx()
{
	NRF_UART0->INTENCLR = UART_INTENCLR_TXDRDY_Msk;
	NRF_UART0->TASKS_STOPTX = 1;
}

/**
 * This function turns on the receiver
 */
inline void UART_Start_Rx()
{
	NRF_UART0->INTENSET = UART_INTENSET_RXDRDY_Msk;
	NRF_UART0->TASKS_STARTRX = 1;
}

/**
 * This function turns off the receiver
 */
inline void UART_Stop_Rx()
{
	NRF_UART0->INTENCLR = UART_INTENCLR_RXDRDY_Msk;
	NRF_UART0->TASKS_STOPRX = 1;
}

/**
 * This function disables the UART peripheral
 */
inline void UART_Disable()
{
	NRF_UART0->ENABLE = 0;
}

/**
 * This function sends the data via UART
 *
 * \param data_to_send - pointer to the data which are to be sent
 * \param data_size - size of data(in bytes) which are to be sent
 */
void UART_Send_String(uint8_t* data_to_send, uint16_t data_size)
{
	///	Set the pointer to the data
	tx_buffer = data_to_send;
	///	Set the data size
	tx_data_size = data_size;
	///	Set the transmission in progress flag
	uart_tx_transmission_in_progress = 1;
	///	Send the first byte
	tx_byte_counter = 1;
	///	Enable the transmitter
	UART_Start_Tx();
	///	Send the first byte of data
	NRF_UART0->TXD = data_to_send[0];
}

/**
 * This function waits untill all the data are send via UART
 */
void UART_Wait_For_Transmission_End()
{
	while(uart_tx_transmission_in_progress)
	{
		///__WFE();
	}
}

/**
 * \brief This function changes baudrate to the given one
 * \param baudrate - the UART_BAUDRATE_Baud... macro from nrf51.h module
 */
inline void UART_Change_Baudrate(uint32_t baudrate)
{
	NRF_UART0->BAUDRATE = baudrate;
}
