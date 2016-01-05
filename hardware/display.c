/*
 * display.c
 *
 *  Created on: 15 gru 2015
 *      Author: Konrad
 */


#include "display.h"
#include "spi.h"
#include "hardware_settings.h"
#include "nrf_gpio.h"
#include "nrf_soc.h"
#include "nrf51.h"
#include "stddef.h"
#include "stdint.h"
#include "RTC.h"
#include "sharp_display_font.h"
#include "stdlib.h"

/* Structure : first byte - command
 * 				line 1    -	line_number, data,
 * 				line 2	  -	line_number, data
 * 					.
 * 					.
 * 					.
 */
uint8_t display_array[96*14] = {0};


/**
 * \brief This function configures the SPI module for communication with Sharp 96x96 memory display
 */
static void Display_SPI_Config()
{
	nrf_gpio_cfg_output(DISP_CS);
	nrf_gpio_cfg_output(DISP_MOSI);
	nrf_gpio_cfg_output(DISP_SCK);

	NRF_SPI1->PSELSCK = DISP_SCK;
	NRF_SPI1->PSELMOSI = DISP_MOSI;

	///	Enable the SPI Interrupt
	NRF_SPI1->INTENSET = SPI_INTENSET_READY_Msk;

	///	Clear configuration
	NRF_SPI1->CONFIG = 0;

	///	Set the CPHA0 and CPOL0 and MSB bit first
	NRF_SPI1->CONFIG = (SPI_CONFIG_CPHA_Leading << SPI_CONFIG_CPHA_Pos) | (SPI_CONFIG_CPOL_ActiveHigh << SPI_CONFIG_CPOL_Pos) | (SPI_CONFIG_ORDER_LsbFirst << SPI_CONFIG_ORDER_Pos);

	///	Set the Display SPI CLK freqency to 0.5 MHz
	NRF_SPI1->FREQUENCY = SPI_FREQUENCY_FREQUENCY_M1;
}


/**
 * \brief This function configures the EXTCOMIN pin to generate 50% duty cycle PWM signal needed to refresh the LCD Sharp Memory Display
 */
static void Sharp_VCOM_Config()
{
	nrf_gpio_cfg_output(DISP_TOGGLE_PIN);
	///	Set the pin in gpiote mode
	NRF_GPIOTE->CONFIG[0] = (GPIOTE_CONFIG_MODE_Task << GPIOTE_CONFIG_MODE_Pos) | (GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos) | (GPIOTE_CONFIG_OUTINIT_Low << GPIOTE_CONFIG_OUTINIT_Pos);
	/// Set the pin
	NRF_GPIOTE->CONFIG[0] |= DISP_TOGGLE_PIN << GPIOTE_CONFIG_PSEL_Pos;
	///	Configure the PPI channel
	uint32_t err_code = sd_ppi_channel_assign(0, &NRF_RTC1->EVENTS_COMPARE[3], &NRF_GPIOTE->TASKS_OUT[0]);
	///	Enable the PPI channel
	err_code = sd_ppi_channel_enable_set(1);

	///	Set the value to RTC CC which triggers VCOM signal toggling (16Hz)
	NRF_RTC1->CC[3] = NRF_RTC1->COUNTER + 1024;
	///	Enable the RTC CC which triggers VCOM signal toggling
	NRF_RTC1->EVTENSET = RTC_EVTENSET_COMPARE3_Msk;

	NRF_RTC1->INTENSET = RTC_INTENSET_COMPARE3_Msk;
}

void Display_Config()
{
	Display_SPI_Config();
	Sharp_VCOM_Config();
}

/**
 * line number from 0 to 95
 */
void Display_Write_Line(uint8_t line_number)
{
	static uint8_t* line_buffer = NULL;

	line_buffer = malloc(16);

	///	Copy the write command
	line_buffer[0] = SHARP_WRITE_LINE_CMD;
	///	Copy the line number and pixel data
	memcpy(&line_buffer[1], &display_array[line_number*14], 13);
	line_buffer[14] = 0;
	line_buffer[15] = 0;
	SPI_Transfer_Non_Blocking(NRF_SPI1, line_buffer, 16, NULL, 0, DISP_CS, true);
	//SPI_Transfer_Blocking(NRF_SPI1, line_buffer, 16, NULL, 0, DISP_CS);
}

void Display_Write_Consecutive_Lines(uint8_t start_line, uint8_t end_line)
{
	uint8_t cmd = SHARP_WRITA_MULTIPLE_LINES_CMD;

	NRF_GPIO->OUTSET = 1 << DISP_CS;
	SPI_Transfer_Non_Blocking(NRF_SPI1, &cmd, sizeof(cmd), NULL, 0, SPI_CS_MANUALLY_CHANGED, false);

	SPI_Wait_For_Transmission_End(NRF_SPI1);

	SPI_Transfer_Non_Blocking(NRF_SPI1, &display_array[start_line*14], (end_line - start_line)*14, NULL, 0, DISP_CS, false);

}

void Display_Clear()
{
	uint8_t* ptr;
	ptr = malloc(2);
	ptr[0] = SHARP_CLEAR_SCREEN;
	ptr[1] = 0;
	SPI_Transfer_Non_Blocking(NRF_SPI1, ptr, 2, NULL, 0, DISP_CS, true);
}

void Display_Write_Text(uint8_t* text, uint8_t text_size, uint8_t line_number)
{
	for(uint8_t i=0; i< text_size; i++)
	{
		for(uint8_t char_line = 0; char_line < 8; char_line++)
			display_array[(char_line + line_number)*14 + i + 1] = font8x8[text[i] - 32 + char_line];
	}

	for(uint8_t i=0; i < 8; i++)
	{
		//Display()
	}

}



void Display_Test()
{
	Display_Clear();
		uint8_t byte = 0x0F;
		//Display_
		for(uint8_t i=0; i<96; i++)
		{
			display_array[i*14] = i+1;
			for(uint8_t j = 1; j<13; j++)
				display_array[ i*14+ j] = byte;
			display_array[i*14 + 13] = 0;
		}

		Display_Write_Consecutive_Lines(0, 95);
		SPI_Wait_For_Transmission_End(NRF_SPI1);
		RTC_Wait(1);
		/*	for(uint8_t i=0; i<96;i++)
			{
				Display_Write_Line(i);
				RTC_Wait(RTC_MS_TO_TICKS(1));
			}*/

}
