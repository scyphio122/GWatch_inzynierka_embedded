#include "nrf51.h"
#include "nrf51_bitfields.h"
#include "timer.h"
#include "interface.h"
#include "nrf_soc.h"
#include "nrf_gpio.h"
#include "nrf_soc.h"

volatile char timer_delay_completed = FALSE;
volatile char timer_overflow = 0;
volatile char timer_timeout_flag = 0;
/**
*	@brief This function initializes the TIMER1, which will be used for short delays
**/
void Timer1_Init()
{	
	//	Set the mode to Timer mode
	NRF_TIMER1->MODE = TIMER_MODE_MODE_Timer;
	//	Set the frequency to TIMER1_FREQUENCY
	NRF_TIMER1->PRESCALER = TIMER1_PRESCALER;
	//	Set the bitmode to 16bit, because TIMER1 can only be 8 or 16bit
	NRF_TIMER1->BITMODE = TIMER_BITMODE_BITMODE_16Bit;
	
	NRF_TIMER1->INTENCLR = TIMER_INTENCLR_COMPARE0_Msk | TIMER_INTENCLR_COMPARE1_Msk | TIMER_INTENCLR_COMPARE2_Msk |TIMER_INTENCLR_COMPARE3_Msk;  
}

void Timer2_Init()
{
		//	Set the mode to Timer mode
	NRF_TIMER2->MODE = TIMER_MODE_MODE_Timer;
	//	Set the frequency to TIMER1_FREQUENCY
	NRF_TIMER2->PRESCALER = TIMER2_PRESCALER;
	//	Set the bitmode to 16bit, because TIMER1 can only be 8 or 16bit
	NRF_TIMER2->BITMODE = TIMER_BITMODE_BITMODE_16Bit;
	
	NRF_TIMER2->INTENCLR = TIMER_INTENCLR_COMPARE0_Msk | TIMER_INTENCLR_COMPARE1_Msk | TIMER_INTENCLR_COMPARE2_Msk |TIMER_INTENCLR_COMPARE3_Msk;  
	
#ifdef BEACON_V2
	NRF_TIMER2->SHORTS = TIMER_SHORTS_COMPARE1_CLEAR_Msk;
#elif defined BEACON_V3
	NRF_TIMER2->SHORTS = TIMER_SHORTS_COMPARE0_CLEAR_Msk;
#endif
}

/**
*		@brief 	This function is used to wait for a small amount of time. It should be used rarely because timers have bigger power consumption than RTC. The frequency is 1000000Hz (T = 1us).
*						The param should be the number of miliseconds of demanded delay
*
*			
**/
void Timer_Delay(uint16_t delay_us)
{
	//	Enable the cc[0] interrupt
	NRF_TIMER1->INTENSET = TIMER_INTENSET_COMPARE0_Msk;
	//	Set the timeout value
	NRF_TIMER1->CC[0] = delay_us;
	//	Start the timer
	NRF_TIMER1->TASKS_START = 1;
	//	Wait for the demanded amount of time
	while(timer_delay_completed == FALSE)
	{
		__WFE();
	}
	/// @TODO Use shortcut insead of Task Shutdown
	//	Stop the timer
	NRF_TIMER1->TASKS_SHUTDOWN = 1;
	//	Disable the CC[0] interrupt 
	NRF_TIMER1->INTENCLR = TIMER_INTENSET_COMPARE0_Msk;
	//	Clear the wait flag
	timer_delay_completed = FALSE;
}

void Timer_Timeout(uint32_t ticks)
{
	timer_timeout_flag = 0;
	NRF_TIMER1->INTENSET = TIMER_INTENSET_COMPARE1_Msk;
	NRF_TIMER1->TASKS_CAPTURE[1] = 1;
	uint32_t timeout = NRF_TIMER1->CC[1] + ticks;
	NRF_TIMER1->CC[1] = timeout;
	NRF_TIMER1->TASKS_START = 1;
}

inline void Timer_Cancel_Timeout()
{
	NRF_TIMER1->INTENCLR = TIMER_INTENCLR_COMPARE1_Msk;
	NRF_TIMER1->TASKS_SHUTDOWN = 1;
}

void Timer_Start_Measure()
{
	NRF_TIMER1->INTENSET = TIMER_INTENSET_COMPARE1_Msk;
	NRF_TIMER1->CC[1] = 0xFFFF;
	NRF_TIMER1->TASKS_START = 1;
}
void Timer_Stop_Measure()
{
	timer_overflow = 0;
	NRF_TIMER1->INTENCLR = TIMER_INTENSET_COMPARE1_Msk;
	NRF_TIMER1->TASKS_STOP = 1;
	NRF_TIMER1->TASKS_CLEAR = 1;
	NRF_TIMER1->TASKS_SHUTDOWN = 1;
}
uint32_t Timer_Get_Timestamp()
{
	NRF_TIMER1->TASKS_CAPTURE[2] = 1;
	if(timer_overflow)
		__NOP();
	uint32_t retval = timer_overflow * (uint16_t)(0xFFFF) + NRF_TIMER1->CC[2];
	return retval;
}
