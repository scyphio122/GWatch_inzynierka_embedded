/*
 * display.h
 *
 *  Created on: 15 gru 2015
 *      Author: Konrad
 */

#ifndef HARDWARE_DISPLAY_H_
#define HARDWARE_DISPLAY_H_

#include "stdint.h"
#include "stdbool.h"


#define SHARP_WRITE_LINE_CMD				(uint8_t)0x01//0xC0
#define SHARP_WRITA_MULTIPLE_LINES_CMD		(uint8_t)0x01
#define SHARP_CLEAR_SCREEN					(uint8_t)0x04

#define DISPLAY_CLOCK_START_LINE			(uint8_t)9
#define DISPLAY_LATITUDE_DESC_START_LINE	(uint8_t)25
#define DISPLAY_LATITUDE_START_LINE			(uint8_t)33
#define DISPLAY_LONGTITUDE_DESC_START_LINE 	(uint8_t)49
#define DISPLAY_LONGTITUDE_START_LINE		(uint8_t)57
#define DISPLAY_SAMPLING_STATUS_LINE		(uint8_t)88


extern uint8_t display_array[14*96];
extern volatile uint8_t disp_updt_time;

void Display_Config();

void Display_Write_Line(uint8_t line_number);

void Display_Write_Consecutive_Lines(uint8_t start_line, uint8_t end_line);

void Display_Clear();

void Display_Test();

void Display_Write_Text(uint8_t* text, uint8_t text_size, uint8_t line_number, uint8_t char_index, bool inverted, bool dyn_alloc_buf);

void Display_Write_Buffer(uint8_t* text, uint8_t text_size, uint8_t line_number, uint8_t char_index, bool inverted);

void Display_Flush_Buffer();

void Display_Update_BLE_Conn(uint16_t ble_conn_status);

void Display_Update_GPS_Power_On();

void Display_Write_Time();

void Display_Write_Longtitude();

void Display_Write_Latitude();

void Display_Update_Battery_Level();


#endif /* HARDWARE_DISPLAY_H_ */
