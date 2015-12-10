#include "nrf51.h"
#include "nrf51_bitfields.h"
#include "WATCHDOG.h"


inline void Watchdog_Config()
{
	//  Enable the Watchdog during processor's sleep mode
    NRF_WDT-> CONFIG |= WDT_CONFIG_SLEEP_Msk;

    //  Set the Counter Reload Register
    NRF_WDT-> CRV = WATCHDOG_CRV_TICKS;
    //  Enable the RR[0] register used to request reload
    NRF_WDT-> RREN |= WDT_RREN_RR0_Msk;

    //  Enable TIMEOUT interrupt
    NRF_WDT-> INTENSET |= WDT_INTENSET_TIMEOUT_Msk;


    //  Start watchdog
    NRF_WDT-> TASKS_START = 1;

}

inline void Watchdog_Reload()
{
    NRF_WDT-> RR[0] = WATCHDOG_RELOAD_REQUEST;
}

