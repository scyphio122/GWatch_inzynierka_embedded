/*
 * GPS.h
 *
 *  Created on: 11 gru 2015
 *      Author: Konrad
 */

#ifndef HARDWARE_GPS_H_
#define HARDWARE_GPS_H_

#include "stdint-gcc.h"
#include "RTC.h"


typedef enum
{
	TIME_POS_FIX_MSG = 0,
	DOP_AND_ACTIVE_SATS,
	SATS_IN_VIEW,
	MIN_NAVI_DATA,
	COURSE_AND_SPEED,
	ANT_ADVISOR,
	ACK_MSG,

	UNKNOWN_HEADER
}gps_msg_header_e;

typedef struct
{
	uint16_t 	deg;		/*< degrees **/
	uint8_t 	min_int;	/*< integer part of minutes **/
	uint16_t 	min_fract;	/*< fraction part of minutes **/
}gps_coord_t;

typedef struct
{
	utc_time_t 		utc_time;
	gps_coord_t		latitude;
	uint8_t			latitude_indi;
	gps_coord_t		longtitude;
	uint8_t			longtitude_indi;
	uint8_t			fix_indi;
	uint8_t			sats_used;
	uint8_t			horizontal_dilution_of_precision[4];
	uint8_t			altitude[4];
	uint8_t			altitude_unit;
	uint8_t			geoidal_separation[4];
	uint8_t			geoidal_separ_units;
	uint8_t			age_of_diff_corr;
	uint16_t		checksum;

}gps_gga_msg_t;

extern uint8_t 					gps_msg_byte_index;
extern volatile uint8_t 		gps_msg_received;
extern uint8_t					gps_msg_size;
uint16_t						gps_msg_checksum;

void 		GPS_Init();
void 		GPS_Turn_On();
void 		GPS_Turn_Off();
void 		GPS_Reset();
void 		GPS_Parse_Message();
uint32_t	GPS_Parse_GGA_Message();


#endif /* HARDWARE_GPS_H_ */
