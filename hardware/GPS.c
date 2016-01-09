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
#include "display.h"


static const uint8_t* 		gps_msg_header[] = {"$GPGGA", "$GPGSA", "$GPGSV", "$GPRMC", "$GPVTG", "$PGTOP"};
static uint8_t				gps_msg_frequency[] = "1,1,1,1,1,5,0,0,0,0,0,0,0,0,0,0,0,0,0";
uint8_t 					gps_msg_byte_index;
uint8_t						gps_msg_size;
volatile uint8_t 			gps_msg_received;
uint16_t					gps_msg_checksum;
gps_gga_msg_t				gga_message;
uint32_t					gps_sample_nr;
uint32_t					gps_sample_timestmap;
volatile uint8_t			gps_message_sample_storage_time;
uint32_t					gps_distance_meters;


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
	UART_Enable();
	UART_Start_Rx();

	NRF_GPIO->OUTCLR = 1 << GPS_ON_PIN;
}

/**
 * This function turns off the GPS module
 */
inline void GPS_Turn_Off()
{
	UART_Stop_Rx();
	UART_Disable();
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
static gps_msg_header_e GPS_Get_Message_Type(uint8_t* gps_msg)
{
	uint32_t ret_val = 0;

	if(gps_msg[0] != '$')
		return UNKNOWN_HEADER;

	if(gps_msg[1] == 'G')	///	If it is GPS protocol command
	{

		ret_val = memcmp(gps_msg, gps_msg_header[TIME_POS_FIX_MSG], 6);
		if(ret_val == 0)
			return TIME_POS_FIX_MSG;

		ret_val = memcmp(gps_msg, gps_msg_header[SATS_IN_VIEW], 6);
		if(ret_val == 0)
			return SATS_IN_VIEW;

		ret_val = memcmp(gps_msg, gps_msg_header[MIN_NAVI_DATA], 6);
		if(ret_val == 0)
			return MIN_NAVI_DATA;

		ret_val = memcmp(gps_msg, gps_msg_header[DOP_AND_ACTIVE_SATS], 6);
		if(ret_val == 0)
			return DOP_AND_ACTIVE_SATS;

		ret_val = memcmp(gps_msg, gps_msg_header[COURSE_AND_SPEED], 6);
		if(ret_val == 0)
			return COURSE_AND_SPEED;

		ret_val = memcmp(gps_msg, gps_msg_header[ANT_ADVISOR], 6);
		if(ret_val == 0)
			return ANT_ADVISOR;
	}
	else
	if(gps_msg[1] == 'P')	///	Else if it is chip command
	{


		ret_val = memcmp(gps_msg, "$PMTK001", sizeof(gps_msg));
		if(ret_val == 0)
			return ACK_MSG;
	}

	return UNKNOWN_HEADER;
}

/**
 * \brief This function checks if the message was received correctly. If so, it returns 0. If non-zero value returned - there is an error
 *
 * \param message - pointer to the message on which the checksum is to be calculated
 */
static uint32_t GPS_Checksum_Check(uint8_t* message, uint8_t calc_checksum)
{
	uint16_t msg_checksum = message[gps_msg_size - 8] + message[gps_msg_size - 9]*10;

	return calc_checksum ^ msg_checksum;
}

/**
 * \brief This function parses the GGA message from the GPS module into the given structure
 *
 * \param msg - pointer to the structure where the parsed data is to be stored
 *
 * \return NRF_SUCCESS
 */
__attribute__((optimize("O0")))
uint32_t GPS_Parse_GGA_Message(gps_gga_msg_t* msg, uint8_t* msg_copy)
{
	uint8_t message_length = gps_msg_size - 2;
	int8_t	currently_parsed_field_ind = -1;
	uint8_t temp_byte = 0;
	uint8_t temp_index = 0;

	///	Check if the checksum is correct
	//if(!GPS_Checksum_Check(msg_copy, gps_msg_checksum))
	//{
		temp_index = 6;
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
				memcpy(msg->utc_time.hour, &msg_copy[temp_index], 2);
				memcpy(msg->utc_time.min, &msg_copy[temp_index+2],2);
				memcpy(msg->utc_time.sec, &msg_copy[temp_index+4],2);
				memcpy(msg->utc_time.msec, &msg_copy[temp_index+7], 3);
				temp_index = temp_index + 10;
				break;
			}
			case 1:	///	Latitude
				memcpy(&msg->latitude.deg[1], &msg_copy[temp_index], 2);
				memcpy(msg->latitude.min_int, &msg_copy[temp_index+2],2);
				memcpy(msg->latitude.min_fract, &msg_copy[temp_index+5], 4);
				temp_index += 9;
				break;

			case 2:	///	Latitude indi
				msg->latitude_indi = msg_copy[temp_index++];
				break;

			case 3:	/// Longtitude
				memcpy(msg->longtitude.deg, &msg_copy[temp_index], 3);
				memcpy(msg->longtitude.min_int, &msg_copy[temp_index+3], 2);
				memcpy(msg->longtitude.min_fract, &msg_copy[temp_index+6], 4);
				temp_index += 10;
				break;

			case 4:	///	Longtitude indi
				msg->longtitude_indi = msg_copy[temp_index++];
				break;

			case 5:	///	Position fix indi
			{
				uint8_t previous_fix = msg->fix_indi;
				msg->fix_indi = msg_copy[temp_index++];

				///	If the current fix state is different than the previous one, then notify the central aobut the fix change
				if(msg->fix_indi != previous_fix)
					Ble_Uart_Notify_Central(0, &msg->fix_indi, sizeof(msg->fix_indi), false);
				break;
			}
			case 6: /// Satellites used
			{
				uint8_t field_size = 0;
				while(msg_copy[temp_index + field_size] != ',')
					field_size++;
				memcpy(&msg->sats_used, &msg_copy[temp_index], field_size);
				temp_index += field_size;
				break;
			}
			case 7: /// HDOP
				memcpy(&msg->horizontal_dilution_of_precision, &msg_copy[temp_index], 4);
				temp_index += 4;
				break;

			case 8: /// MSL Altitude
			{
				uint8_t field_size = 0;
				while(msg_copy[temp_index + field_size] != ',')
					field_size++;
				memcpy(&msg->altitude, &msg_copy[temp_index], field_size);

				temp_index += field_size;
				break;
			}
			case 9: /// Units
				msg->altitude_unit = msg_copy[temp_index++];
				break;

			case 10:
				memcpy(&msg->geoidal_separation, &msg_copy[temp_index], 4);
				temp_index += 4;
				break;

			case 11:
				msg->geoidal_separ_units = msg_copy[temp_index++];
				break;

			case 12: /// Age of diff coor
			{
				uint8_t field_size = 0;
				while(msg_copy[temp_index + field_size] != ',')
					field_size++;
				memcpy(&msg->age_of_diff_corr, &msg_copy[temp_index], field_size);
				temp_index += field_size;
				break;
			}
			case 13: /// Checksum
				++temp_index;
				memcpy(&msg->checksum, &msg_copy[temp_index], 2);
				temp_index += 2;
				break;
			}

		}while(temp_index < message_length);

		gps_sample_timestmap = RTC_Get_Timestamp();
		gps_sample_nr++;

		gps_message_sample_storage_time = 1;
		disp_updt_time = 1;
		return NRF_SUCCESS;

//	}

//	return NRF_ERROR_INVALID_DATA;
}

/**
 * \brief This function parses the return message sent from the GPS module to MTK message
 *
 * \return structure of command code on which response came and return flag
 */
gps_ack_msg_ret_val_t GPS_Parse_ACK_Message()
{
	uint8_t message_length = gps_msg_size - 8; /// minus size of header
	uint8_t msg_copy[32] = {0};
	uint8_t temp_index = 0;
	gps_ack_msg_ret_val_t ret_val;
	while(message_length-- > 0)
	{
		Fifo_Get(&uart_rx_fifo, &msg_copy[temp_index++]);
	}

	if(msg_copy[0] == ',')
	{
		///	Copy the command
		memcpy(ret_val.command, &msg_copy[1], 3);
	}

	/// Copy the ret_val flag
	memcpy(&ret_val.ret_val, &msg_copy[5], 1);

	return ret_val;
}

/**
 * \brief This function parses all the GPS messages which are serviced by the program
 */
void GPS_Parse_Message()
{
	uint8_t* gps_msg = malloc(gps_msg_size);
	for(uint8_t i=0; i< gps_msg_size; i++)
		Fifo_Get(&uart_rx_fifo, &gps_msg[i]);
	gps_msg_header_e msg_header = GPS_Get_Message_Type(gps_msg);
	switch(msg_header)
	{
	case TIME_POS_FIX_MSG:
	{
		GPS_Parse_GGA_Message(&gga_message, gps_msg);
		break;
	}

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
	case ACK_MSG:
	{
		gps_ack_msg_ret_val_t ret_val = GPS_Parse_ACK_Message();
		switch(ret_val.ret_val)
		{
		case INVALID_COMMAND_PACKET:
			break;

		case UNSUPPORTED_COMMAND:
			break;

		case VALID_COMMAND_ACTION_FAILED:
			break;

		case VALID_COMMAND_ACTION_SUCCESS:
			break;
		}
		break;
	}
	case UNKNOWN_HEADER:

		break;

	}
	free(gps_msg);
	///	Clear the gps message checksum
	gps_msg_checksum = 0;
	uint8_t dummy_byte = 0;
	for(uint16_t i=0; i<gps_msg_size; i++)
	{
		Fifo_Get(&uart_rx_fifo, &dummy_byte);
	}
}

uint32_t GPS_Change_Message_Frequency(gps_msg_rate_indexes_e msg_to_change, uint8_t rate)
{
	uint8_t msg[] = "$PMTK314,1,1,1,1,1,5,0,0,0,0,0,0,0,0,0,0,0,0,0*2C\r\n";
	uint8_t checksum = 0;
	gps_msg_frequency[msg_to_change] = rate;

	///	Set the message
	memcpy(&msg[9], gps_msg_frequency, sizeof(gps_msg_frequency));

	/// calculate the checksum
	for(uint8_t i=1; msg[i] != '*'; i++)
		checksum ^= msg[i];

	uint8_t first_byte = checksum >> 4;
	uint8_t second_byte = checksum & (0x0F);
	uint8_t char_checksum[2] = {0};
	sprintf(char_checksum,"%d%d", first_byte, second_byte);

	memcpy(msg[sizeof(msg) - 4], char_checksum, 2);
	UART_Start_Tx();
	UART_Send_String(msg, sizeof(msg));
	UART_Stop_Tx();

	return NRF_SUCCESS;
}

uint32_t GPS_Prepare_To_Sampling_Start()
{
	gps_sample_nr = 0;
	gps_distance_meters = 0;

	return NRF_SUCCESS;
}

