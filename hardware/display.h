/*
 * display.h
 *
 *  Created on: 15 gru 2015
 *      Author: Konrad
 */

#ifndef HARDWARE_DISPLAY_H_
#define HARDWARE_DISPLAY_H_

#include "stdint.h"

#define SHARP_WRITE_LINE_CMD				(uint8_t)0x01//0xC0
#define SHARP_WRITA_MULTIPLE_LINES_CMD
#define SHARP_CLEAR_SCREEN					(uint8_t)0x04

extern uint8_t display_array[13*96];


void Display_Config();

void Display_Write_Line(uint8_t line_number);

void Display_Write_Consecutive_Lines(uint8_t start_line, uint8_t end_line);

void Display_Clear();

void Display_Test();

#endif /* HARDWARE_DISPLAY_H_ */
