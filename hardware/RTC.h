#ifndef RTC_H_INCLUDED
#define RTC_H_INCLUDED
#include "Clock.h"
#include "app_error.h"
#include <string.h>

#define RTC_FREQ                       	(32768) 						/*< RTC Frequency in Hz. If 32.768 kHz -> 30,517us for a tick  */
#define RTC_PRESCALER                   (0)								/*< RTC prescaler */
#define RTC_OVERFLOW_TIME				(uint16_t)512					/*< Time to RTC counter register overflow in seconds */
#define RTC_COUNTER_MAXVAL				(uint32_t)0x01000000//0x01000000
#define RTC_COUNTER_INT_SEC				(uint16_t)(NRF_RTC1->COUNTER >> 15)
#define	RTC_COUNTER_SUB_SEC				(uint16_t)(NRF_RTC1->COUNTER & 0x7FFF)
#define RTC_COUNTER_HALF_SECOND			(uint16_t)(0x3FFF)

#define RTC_PRIOR						(uint8_t)1

#define RTC_OP_OK						NRF_SUCCESS
#define RTC_OP_ERROR					(uint8_t)1

#define RTC_DELAY_COMPLETED             (uint8_t)0xFF					/*< RTC delay completed flag */


#define RTC_US_TO_TICKS(x)				(uint32_t)(x/30.517)			/*< Microseconds to ticks converter. Don't use with variables! */
#define RTC_MS_TO_TICKS(x)          	(uint32_t)(1000*x/30.517)		/*< Miliseconds to ticks converter. Don't use with variables! */

#if (RTC_FREQ == (32768)) 
	#define RTC_S_TO_TICKS(x)			(uint32_t)(x<<15)
#endif



typedef struct
{
	uint16_t day;
	uint8_t  hour;
	uint8_t  minute;
	uint8_t  second;
}date_t;

typedef struct
{
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint16_t msec;
}utc_time_t;

extern volatile uint8_t  timeout_flag;


/** \brief Performs RTC configuration */
void RTC_Config(void);

/** \brief Starts RTC */
void RTC_Start();

/** \brief Performs wait for given time
 *	\param time - Time to delay given in RTC ticks */
void RTC_Wait(uint32_t time);

/** \brief Starts timeout for given time
 *	\param time - Time to hit timeout given in RTC ticks */
void RTC_Timeout(uint32_t time);

extern void RTC_Cancel_Timeout();

/** \brief Sets RTC date
 *	\param UNIX_NewTime - New date as UNIX timestamp */
uint8_t RTC_Set_Timestamp(uint32_t timestamp);

/** \brief Gets RTC date
 *	\return Date in UNIX timestamp */
uint32_t RTC_Get_Timestamp();

/** \brief Gets raw RTC counter value
 *	\return RTC counter value */
uint32_t RTC_Get_Counter(void);

/**	\brief	This function calculates the period of time between the given two timestamps and saves the days, hours, minutes and seconds between them in the given  date structure
 *	\param earlier_timestamp	-	The older timestamp
 *	\param later_timestamp 		- 	The younger timestamp
 *	\param date 				-	Pointer to the structure where the date will be held */
void GetDiffBetweenTwoTimestamps(uint32_t earlier_timestamp, uint32_t later_timestamp, date_t* date);

/** \brief Calculates day difference between two dates given as timestamps
 *	\param earlier_timestamp - first timestamp
 *	\param later_timestamp - second timestamp
 *	\return Number of days */
uint16_t Get_Integer_Day_Number_Between_Timestamps(uint32_t earlier_timestamp, uint32_t later_timestamp);

/**	\brief This function calculates an absolute date (since year 1970) from the given timestamp
 *	\param timestamp 	- 	The given date
 *	\param date 		-	Pointer to the structure where the date will be held */
void TranslateTimestampToDate(uint32_t timestamp, date_t* date);


#endif /* RTC_H_INCLUDED */
