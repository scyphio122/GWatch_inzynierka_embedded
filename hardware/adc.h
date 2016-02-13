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

/**
*   @brief ADC End of Conversion interrupt handler. It gets the converted value and disables the ADC peripheral
**/
void ADC_IRQHandler(void);
/**
*	@brief This function is responsible for ADC module initialization. It utilizes it to measure the battery voltage. After conversion, the peripheral is disabled in its End of Conversion Interrupt Handler
*/
void ADC_Init(void);


/**
*  @brief This function starts conversion of the battery voltage. The value after conversion is not calculated to units of Volt
*	\parem voltage_buffer - the pointer to the buffer where the converted voltage will be stored
**/
void ADC_Get_Bat_Voltage(uint8_t* voltage_buffer);
