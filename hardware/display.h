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

#define DISPLAY_CLOCK_START_LINE			(uint8_t)1
#define DISPLAY_LATITUDE_DESC_START_LINE	(uint8_t)25
#define DISPLAY_LATITUDE_START_LINE			(uint8_t)33
#define DISPLAY_LONGTITUDE_DESC_START_LINE 	(uint8_t)49
#define DISPLAY_LONGTITUDE_START_LINE		(uint8_t)57



extern uint8_t display_array[14*96];
extern volatile uint8_t disp_updt_time;

void Display_Config();

void Display_Write_Line(uint8_t line_number);

void Display_Write_Consecutive_Lines(uint8_t start_line, uint8_t end_line);

void Display_Clear();

void Display_Test();

void Display_Write_Text(uint8_t* text, uint8_t text_size, uint8_t line_number, bool inverted, bool dyn_alloc_buf);







#endif /* HARDWARE_DISPLAY_H_ */
