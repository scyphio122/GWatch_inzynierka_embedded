/*
 * GPS.h
 *
 *  Created on: 11 gru 2015
 *      Author: Konrad
 */

#ifndef HARDWARE_GPS_H_
#define HARDWARE_GPS_H_

typedef enum
{
	TIME_POS_FIX_MSG = 0,
	DOP_AND_ACTIVE_SATS,
	SATS_IN_VIEW,
	MIN_NAVI_DATA,
	COURSE_AND_SPEED,
	ANT_ADVISOR,

	UNKNOWN_HEADER
}gps_msg_header_e;

typedef struct
{
	const uint8_t time_pos_fix[] = "$GPGGA";
	const uint8_t dop_active_sat[] = "$GPGSA";
	const uint8_t sats_in_view[] = "$GPGSV";
	const uint8_t min_navi_data[] = "$GPRMC";
	const uint8_t course_speed[] = "$GPVTG";
	const uint8_t ant_advisor[] = "$PGTOP";


}gps_msg_header_t;

extern const gps_msg_header_t gps_msg_header;

void GPS_Init();
void GPS_Turn_On();
void GPS_Turn_Off();
void GPS_Reset();
void GPS_Get_Message_Type();



#endif /* HARDWARE_GPS_H_ */
