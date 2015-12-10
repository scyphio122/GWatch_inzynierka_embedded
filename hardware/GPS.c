/*
 * GPS.c
 *
 *  Created on: 10 gru 2015
 *      Author: Konrad
 */

#include "GPS.h"
#include "nrf_gpio.h"
#include "hardware_settings.h"

/**
 * This function initializes the GPS pins
 */
void GPS_Init()
{
	/// GPS on/off configuration
	nrf_gpio_cfg_output(GPS_ON_PIN);
	GPS_Turn_Off();

	///	Fix pin configuration
	nrf_gpio_cfg_input(GPS_3D_FIX_PIN, NRF_GPIO_PIN_NOPULL);

	/// Reset pin configuration
	nrf_gpio_cfg_output(GPS_RESET_PIN);
	NRF_GPIO->OUTSET = 1 << GPS_RESET_PIN;

	/// the 1PPS pin configuration
	nrf_gpio_cfg_input(GPS_1_PPS_PIN, NRF_GPIO_PIN_NOPULL);

	///	RTCM is not enabled by default
	nrf_gpio_cfg_input(GPS_RTCM_PIN, NRF_GPIO_PIN_NOPULL);
}


/**
 * This function turns on the GPS module
 */
inline void GPS_Turn_On()
{
	NRF_GPIO->OUTCLR = 1 << GPS_ON_PIN;
}

/**
 * This function turns off the GPS module
 */
inline void GPS_Turn_Off()
{
	NRF_GPIO->OUTSET = 1 << GPS_ON_PIN;
}


/**
 * This function causes the gps module to reset
 */
void GPS_Reset()
{
	///	Force the gps module to reset
	NRF_GPIO->OUTCLR = 1 << GPS_RESET_PIN;
	///	Wait one second to be sure that it is reset
	RTC_Wait(RTC_S_TO_TICKS(1));
	///	Set the reset pin to the normal condition
	NRF_GPIO->OUTSET = 1 << GPS_RESET_PIN;
}

void GPS_Parse_Data()
{

}


