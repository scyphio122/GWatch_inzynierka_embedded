#ifndef WATCHDOG_H_INCLUDED
#define WATCHDOG_H_INCLUDED

#define DEBUG_WDT
#ifdef  DEBUG_WDT
	#define WATCHDOG_TIMEOUT       (uint32_t)(20)                        //  The WATCHDOG_TIMEOUT value is set in seconds
#else
	#define WATCHDOG_TIMEOUT       (uint32_t)(60)
#endif
#define WATCHDOG_CRV_TICKS      (uint32_t)(WATCHDOG_TIMEOUT*32768 - 1)
#define WATCHDOG_RELOAD_REQUEST (uint32_t)(0x6E524635)
#define WATCHDOG_RELOAD_TIME    (uint32_t)(WATCHDOG_TIMEOUT*RTC_FREQ*0.25)

void Watchdog_Config(void);
void Watchdog_Reload(void);

#endif /* WATCHDOG_H_INCLUDED */
