/*
 * GPS.c
 *
 *  Created on: 10 gru 2015
 *      Author: Konrad
 */

#include "GPS.h"
#include "nrf_gpio.h"
#include "hardware_settings.h"
#include "fifo.h"


const gps_msg_header_t gps_msg_header;

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

/**
 * This function checks the received gps message header with the known patterns.
 *
 * \return  *Enum which describes the result pattern
 * 			*UNKNOWN_MSG_HEADER - if message header not recognised
 */
static gps_msg_header_e GPS_Get_Message_Type()
{
	uint8_t msg_header[6];

	for(uint8_t i=0; i<sizeof(msg_header); i++)
	{
		msg_header[i] = Fifo_Get(uart_rx_fifo);
	}

	uint32_t ret_val = memcmp(msg_header, gps_msg_header.time_pos_fix);
	if(ret_val == 0)
		return TIME_POS_FIX_MSG;

	uint32_t ret_val = memcmp(msg_header, gps_msg_header.sats_in_view);
	if(ret_val == 0)
		return SATS_IN_VIEW;

	uint32_t ret_val = memcmp(msg_header, gps_msg_header.min_navi_data);
	if(ret_val == 0)
		return MIN_NAVI_DATA;

	uint32_t ret_val = memcmp(msg_header, gps_msg_header.dop_active_sat);
	if(ret_val == 0)
		return DOP_AND_ACTIVE_SATS;

	uint32_t ret_val = memcmp(msg_header, gps_msg_header.course_speed);
	if(ret_val == 0)
		return COURSE_AND_SPEED;

	uint32_t ret_val = memcmp(msg_header, gps_msg_header.ant_advisor);
	if(ret_val == 0)
		return ANT_ADVISOR;


	return UNKNOWN_HEADER;
}



void GPS_Parse_Message()
{
	switch(GPS_Get_Message_Type());
	{
	case TIME_POS_FIX_MSG:

		break;

	case MIN_NAVI_DATA:

		break;

	case SATS_IN_VIEW:

		break;

	case DOP_AND_ACTIVE_SATS:

		break;

	case COURSE_AND_SPEED:

		break;

	case ANT_ADVISOR:

		break;

	case UNKNOWN_HEADER:

		break;

	}

}


