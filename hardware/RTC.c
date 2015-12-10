#include "nrf51.h"
#include "nrf51_bitfields.h"
#include "Clock.h"
#include "RTC.h"
#include "UART.h"
#include "WATCHDOG.h"
#include "nrf_soc.h"

volatile char second = 0;
volatile char minute = 0;
volatile char hour = 0;
volatile unsigned char day = 1;
volatile unsigned int month = 1;     //  The MSB of month variable is used to define whether the year is leap year
volatile unsigned int  year = 2000;
volatile unsigned char wait_flag = 0;
volatile uint8_t  timeout_flag = 0;
volatile uint32_t rtc_overflow_cnt;

volatile uint32_t partial_timestamp;
uint32_t rtc_last_synch_time;

void RTC_Config()
{
	///	Set the RTC1 prescaler
	NRF_RTC1->PRESCALER = RTC_PRESCALER;
	///	Enable the interrupts on CC[0] register
	NRF_RTC1->INTENSET = RTC_INTENSET_COMPARE0_Enabled<<RTC_INTENSET_COMPARE0_Pos;
	///	Enable the overflow interrupt
	NRF_RTC1->INTENSET = RTC_INTENSET_OVRFLW_Enabled<<RTC_INTENSET_OVRFLW_Pos;
}

__INLINE void RTC_Start()
{
	NRF_RTC1->TASKS_START = 1;
}


uint32_t RTC_Get_Timestamp()
{
	return (partial_timestamp + RTC_COUNTER_INT_SEC);
}


uint32_t RTC_Get_Counter()
{
	return NRF_RTC1->COUNTER;
}

uint8_t RTC_Set_Timestamp(uint32_t timestamp)
{
	///	Check if the timestamp we try to set isn't smaller then the current one (reversing the timestamp guard)
	long long timestamp_buffer = timestamp - RTC_COUNTER_INT_SEC;
	if(timestamp_buffer >= 0)
	{
		///	If the sub second part of COUNTER register is bigger than half second, then we round it up, so the partial_timestamp must be one second smaller
		if(RTC_COUNTER_SUB_SEC >= RTC_COUNTER_HALF_SECOND)
			partial_timestamp = timestamp_buffer - 1;
		else	///	Else we round it down so the partial_timestamp does not have to be changed
			partial_timestamp = timestamp_buffer;

		rtc_last_synch_time = timestamp;

		return RTC_OP_OK;
	}
	
	///	The user tried to "reverse" the timestamp
	return RTC_OP_ERROR;
}

void RTC_Wait(uint32_t time)
{
	// Zero-lenght delay will cause program hang!
	if(time < 1)
		return;
	wait_flag = 0;
    //  Get the current value in Counter Register and add the specified amount of time to the Compare register in order to wait specified amount of time
    NRF_RTC1->CC[0] = (NRF_RTC1->COUNTER + time);
    //  Turn on the Interrupt of Compare[1] register
    NRF_RTC1->INTENSET |= RTC_INTENSET_COMPARE0_Msk;
    //  Sleep until the wait_flag isn't raised
    while(wait_flag != RTC_DELAY_COMPLETED)
    {
    	__WFE();
    }
    //  Clear the interrupt for compare register
    NRF_RTC1->INTENCLR = RTC_INTENCLR_COMPARE0_Msk;
    //  Clear the wait_flag
    wait_flag = 0;
}

void RTC_Timeout(uint32_t time)
{
	if(NRF_RTC1->INTENSET & RTC_INTENSET_COMPARE2_Msk) /// return if timeout already started
	{
		return;
	}
	timeout_flag = 0;
	NRF_RTC1->CC[2] = (NRF_RTC1->COUNTER + time);
	NRF_RTC1->INTENSET |= RTC_INTENSET_COMPARE2_Msk;	
}
inline void RTC_Cancel_Timeout()
{
	NRF_RTC1->INTENCLR = RTC_INTENCLR_COMPARE2_Msk;
}

void GetDiffBetweenTwoTimestamps(uint32_t earlier_timestamp, uint32_t later_timestamp, date_t* date)
{
	//	Get the period of time in seconds, between the given two timestamps
	uint32_t diff_timestamp = later_timestamp - earlier_timestamp;
	
	//	Calculate days number between the given two timestamps
	date->day = (uint16_t)(diff_timestamp/(24*60*60));

	// Calculate hours diff between the calculated days
	diff_timestamp = diff_timestamp - (date->day*24*60*60);
	date->hour = (uint8_t)(diff_timestamp/3600);

	//	Calculate minutes between the above calculated hours
	diff_timestamp = diff_timestamp - (date->hour*60*60);
	date->minute = (uint8_t)(diff_timestamp/60);
	
	//	Calculate seconds between the above calculted minutes
	diff_timestamp = diff_timestamp - (date->minute*60);
	date->second = diff_timestamp;
}

uint16_t Get_Integer_Day_Number_Between_Timestamps(uint32_t earlier_timestamp, uint32_t later_timestamp)
{
		//	Get the period of time in seconds, between the given two timestamps
	uint32_t diff_timestamp = later_timestamp - earlier_timestamp;
	
	//	Calculate days number between the given two timestamps
	uint16_t day_diff = (uint16_t)(diff_timestamp/(24*60*60));
	return day_diff;
}

/**	@brief This function calculates an absolute date (since year 1970) from the given timestamp
 *	@param timestamp 	- 	The given date
 *	@param date 		-	Pointer to the structure where the date will be held */
void TranslateTimestampToDate(uint32_t timestamp, date_t* date)
{
	GetDiffBetweenTwoTimestamps(0, timestamp, date);
}

