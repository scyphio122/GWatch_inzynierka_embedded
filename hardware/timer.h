#ifndef __TIMER__
#define __TIMER__

#include "nrf51.h"
#include "nrf51_bitfields.h"


#define TIMER1_FREQ 				(uint16_t)31250	//Hz - this macro is created only to provide information about timer frequency
#define TIMER1_PRESCALER			(uint8_t)8		//	16000000/(2^8) = 62500Hz

#define TIMER2_FREQ					(uint16_t)62500
#define TIMER2_PRESCALER			(uint8_t)8

#define TIMER_US_TO_TICKS(x)		(uint16_t)((x*0.000001)/((1.0/TIMER1_FREQ)))
#define TIMER2_US_TO_TICKS(x)	  	(uint16_t)((x*0.000001)/((1.0/TIMER2_FREQ)))

extern volatile char timer_delay_completed;
extern volatile char timer_timeout_flag;

void Timer1_Init(void);
void Timer2_Init(void);
void Timer_Delay(uint16_t delay_us);
void Timer_Timeout(uint32_t ticks);
void Timer_Cancel_Timeout();
void Timer_Start_Measure(void);
void Timer_Stop_Measure(void);
uint32_t Timer_Get_Timestamp(void);

#endif //__TIMER__
