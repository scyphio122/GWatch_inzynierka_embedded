#include "stdint.h"

#define BATTERY_VOLTAGE_THRESHOLD	(uint32_t)(CRITICAL_BATTERY_VOLTAGE * 0.33333 * 255 / 1200)

/****************************************************************
*   ADC algorithm:                                           	*
*	V = result*(1/prescaler)*(V_ref / voltage_level_numbers) 	*
*   prescaler = 1/3, V_ref = 1.2 V, voltage_level_numbers = 255 *
*****************************************************************/



void ADC_IRQHandler(void);
void ADC_Init(void);
void ADC_Get_Bat_Voltage(uint8_t* voltage_buffer);
