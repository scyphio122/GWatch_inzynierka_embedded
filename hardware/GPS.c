/*
 * GPS.c
 *
 *  Created on: 10 gru 2015
 *      Author: Konrad
 */

#include "GPS.h"
#include "nrf_gpio.h"
#include "hardware_settings.h"
#include "libraries/fifo.h"
#include "UART.h"

static const uint8_t* 		gps_msg_header[] = {"$GPGGA", "$GPGSA", "$GPGSV", "$GPRMC", "$GPVTG", "$PGTOP"};
uint8_t 					gps_msg_byte_index;
uint8_t						gps_msg_size;
volatile uint8_t 			gps_msg_received;
uint16_t					gps_msg_checksum;

static gps_gga_msg_t		gga_message;
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
	uint32_t ret_val = 0;

	for(uint8_t i=0; i<sizeof(msg_header); i++)
	{
		Fifo_Get(&uart_rx_fifo, &msg_header[i]);
	}

	ret_val = memcmp(msg_header, gps_msg_header[TIME_POS_FIX_MSG], 6);
	if(ret_val == 0)
		return TIME_POS_FIX_MSG;

	ret_val = memcmp(msg_header, gps_msg_header[SATS_IN_VIEW], 6);
	if(ret_val == 0)
		return SATS_IN_VIEW;

	ret_val = memcmp(msg_header, gps_msg_header[MIN_NAVI_DATA], 6);
	if(ret_val == 0)
		return MIN_NAVI_DATA;

	ret_val = memcmp(msg_header, gps_msg_header[DOP_AND_ACTIVE_SATS], 6);
	if(ret_val == 0)
		return DOP_AND_ACTIVE_SATS;

	ret_val = memcmp(msg_header, gps_msg_header[COURSE_AND_SPEED], 6);
	if(ret_val == 0)
		return COURSE_AND_SPEED;

	ret_val = memcmp(msg_header, gps_msg_header[ANT_ADVISOR], 6);
	if(ret_val == 0)
		return ANT_ADVISOR;


	return UNKNOWN_HEADER;
}

uint32_t GPS_Parse_GGA_Message(gps_gga_msg_t* msg)
{
	uint8_t message_length = gps_msg_size;
	int8_t	currently_parsed_field_ind = -1;
	uint8_t temp_byte = 0;
	uint8_t msg_copy[255] = {0};
	uint8_t temp_index = 0;
	while(message_length-- > 0)
	{
		Fifo_Get(&uart_rx_fifo, &msg_copy[temp_index++]);
	}

	temp_index = 0;
	message_length = gps_msg_size - 6;	///	size minus msg header size
	do
	{
		///	If it is the separator, then continue
		if(msg_copy[temp_index] == ',')
		{
			temp_index++;
			currently_parsed_field_ind++;
			continue;
		}

		switch(currently_parsed_field_ind)
		{
		case 0:	///	TIME
		{

			msg->utc_time.hour = msg_copy[temp_index] + (msg_copy[temp_index+1] << 8);
			msg->utc_time.min  = msg_copy[temp_index+2] + (msg_copy[temp_index+3] << 8);
			msg->utc_time.sec  = msg_copy[temp_index+4] + (msg_copy[temp_index+5] << 8);
			msg->utc_time.msec = msg_copy[temp_index+7] + (msg_copy[temp_index+8] << 8) + (msg_copy[temp_index + 9] << 16) + (msg_copy[temp_index + 10] << 24);;
			temp_index = temp_index + 11;
			break;
		}
		case 1:	///	Latitude
			msg->latitude.deg = msg_copy[temp_index] + (msg_copy[temp_index+1] << 8);
			msg->latitude.min_int = msg_copy[temp_index+2] + (msg_copy[temp_index+3] << 8);
			msg->latitude.min_fract = msg_copy[temp_index+5] + (msg_copy[temp_index+6] << 8) + (msg_copy[temp_index + 7] << 16) + (msg_copy[temp_index + 8] << 24);
			temp_index += 9;
			break;

		case 2:	///	Latitude indi
			msg->latitude_indi = msg_copy[temp_index++];
			break;

		case 3:	/// Longtitude
			msg->longtitude.deg = msg_copy[temp_index] + (msg_copy[temp_index+1] << 8) + (msg_copy[temp_index+2] << 16);
			msg->longtitude.min_int = msg_copy[temp_index+3] + (msg_copy[temp_index+4] << 8);
			msg->longtitude.min_fract = msg_copy[temp_index+5] + (msg_copy[temp_index+6] << 8) + (msg_copy[temp_index + 7] << 16) + (msg_copy[temp_index + 8] << 24);
			temp_index += 10;
			break;

		case 4:	///	Longtitude indi
			msg->longtitude_indi = msg_copy[temp_index++];
			break;

		case 5:	///	Position fix indi
			msg->fix_indi = msg_copy[temp_index++];
			break;

		case 6: /// Satellites used
			msg->sats_used = msg_copy[temp_index++];
			break;

		case 7: /// HDOP
			*(msg->horizontal_dilution_of_precision) = msg_copy[temp_index] + (msg_copy[temp_index+1] << 8) + (msg_copy[temp_index + 2] << 16) + (msg_copy[temp_index + 3] << 24);
			temp_index += 4;
			break;

		case 8: /// MSL Altitude
			*(msg->altitude) = msg_copy[temp_index] + (msg_copy[temp_index+1] << 8) + (msg_copy[temp_index + 2] << 16) + (msg_copy[temp_index + 3] << 24);
			temp_index += 4;
			break;

		case 9: /// Units
			msg->altitude_unit = msg_copy[temp_index++];
			break;

		case 10: /// Age of diff coor
			msg->age_of_diff_corr = msg_copy[temp_index] + (msg_copy[temp_index+1] << 8) + (msg_copy[temp_index + 2] << 16) + (msg_copy[temp_index + 3] << 24);
			temp_index += 4;
			break;

		case 11: /// Checksum
			temp_index++;		///	'*' character
			msg->checksum = msg_copy[temp_index] + (msg_copy[temp_index+1] << 8);
			temp_index += 2;
		}

	}while(temp_index < gps_msg_size);

	return NRF_SUCCESS;
}

void GPS_Parse_Message()
{
	gps_msg_header_e msg_header = GPS_Get_Message_Type();
	switch(msg_header)
	{
	case TIME_POS_FIX_MSG:
		GPS_Parse_GGA_Message(&gga_message);
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

	uint8_t dummy_byte = 0;
	for(uint16_t i=0; i<gps_msg_size; i++)
	{
		Fifo_Get(&uart_rx_fifo, &dummy_byte);
	}
}


