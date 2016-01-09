#include "stdint.h"

#define BATTERY_VOLTAGE_THRESHOLD	(uint32_t)(CRITICAL_BATTERY_VOLTAGE * 0.33333 * 255 / 1200)

/****************************************************************
*   ADC algorithm:                                           	*
*	V = result*(1/prescaler)*(V_ref / voltage_level_numbers) 	*
*   prescaler = 1/3, V_ref = 1.2 V, voltage_level_numbers = 255 *
*****************************************************************/

#define ADC_RESULT_3_VOLT 			(uint8_t)106
#define ADC_RESULT_4p2_VOLT			(uint8_t)148
#define ADC_WORK_RANGE				(uint8_t)42

void ADC_IRQHandler(void);
void ADC_Init(void);
void ADC_Get_Bat_Voltage(uint8_t* voltage_buffer);
