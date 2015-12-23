/*
 * display.h
 *
 *  Created on: 15 gru 2015
 *      Author: Konrad
 */

#ifndef HARDWARE_DISPLAY_H_
#define HARDWARE_DISPLAY_H_

#include "stdint.h"

#define SHARP_WRITE_LINE_CMD				(uint8_t)0xC0
#define SHARP_WRITA_MULTIPLE_LINES_CMD
#define SHARP_CLEAR_SCREEN					(uint8_t)0x20

extern uint8_t display_array[13*96];


void Display_Config();

#endif /* HARDWARE_DISPLAY_H_ */
