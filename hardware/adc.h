#include "stdint.h"

#define BATTERY_VOLTAGE_THRESHOLD	(uint32_t)(CRITICAL_BATTERY_VOLTAGE * 0.33333 * 255 / 1200)

/****************************************************************
*   ADC algorithm:                                           	*
*	V = result*(1/prescaler)*(V_ref / voltage_level_numbers) 	*
*   prescaler = 1/3, V_ref = 1.2 V, voltage_level_numbers = 255 *
*****************************************************************/
typedef struct
{
	uint8_t 	wakeup_measurement;
	uint8_t	    comm_measurement;
	uint8_t		comm_measurement_delayed;
}bat_voltage_t;


typedef struct
{
	uint8_t			v_bat_after_sleep;
	uint8_t			v_bat_after_meter_comm;
	uint8_t			v_bat_after_comm_1_sec_delay;
	uint8_t			v_bat_before_sleep;
}battery_voltage_timeline_t;


void ADC_IRQHandler(void);
void ADC_Init(void);
void ADC_Start_Bat_Volt_Conversion(uint8_t* voltage_buffer);
