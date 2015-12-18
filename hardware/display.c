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


uint8_t display_line[12][96] = {0};

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
	NRF_SPI1->CONFIG = (SPI_CONFIG_CPHA_Leading << SPI_CONFIG_CPHA_Pos) | (SPI_CONFIG_CPOL_ActiveHigh << SPI_CONFIG_CPOL_Pos) | (SPI_CONFIG_ORDER_MsbFirst << SPI_CONFIG_ORDER_Pos);

	///	Set the Display SPI CLK freqency to 1 MHz
	NRF_SPI1->FREQUENCY = SPI_FREQUENCY_FREQUENCY_M1;
}


/**
 * \brief This function configures the EXTCOMIN pin to generate 50% duty cycle PWM signal needed to refresh the LCD Sharp Memory Display
 */
static void Sharp_VCOM_Config()
{
	nrf_gpio_cfg_output(DISP_TOGGLE_PIN);
	///	Set the pin in gpiote mode
	NRF_GPIOTE->CONFIG[0] = GPIOTE_CONFIG_MODE_Task | GPIOTE_CONFIG_POLARITY_Toggle | (GPIOTE_CONFIG_OUTINIT_Low << GPIOTE_CONFIG_OUTINIT_Pos);
	/// Set the pin
	NRF_GPIOTE->CONFIG[0] |= DISP_TOGGLE_PIN << GPIOTE_CONFIG_PSEL_Pos;
	///	Configure the PPI channel
	sd_ppi_channel_assign(0, &NRF_RTC1->EVENTS_COMPARE[3], &NRF_GPIOTE->TASKS_OUT[0]);
	///	Enable the PPI channel
	sd_ppi_channel_enable_set(0);

	///	Set the value to RTC CC which triggers VCOM signal toggling (32Hz)
	NRF_RTC1->CC[3] = NRF_RTC1->COUNTER + 1024;
	///	Enable the RTC CC which triggers VCOM signal toggling
	NRF_RTC1->INTENSET = RTC_INTENSET_COMPARE3_Msk;
}

void Display_Config()
{
	Display_SPI_Config();
	Sharp_VCOM_Config();
}



