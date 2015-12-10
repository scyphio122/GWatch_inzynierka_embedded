#include "Clock.h"
#include "nrf51.h"
#include "nrf51_bitfields.h"


/** \brief This function configures the External HFCLK
 *
 * \param  uint8_t HCLK_Freq - The Frequency of the oscillator MACRO
 * \param
    \note   External oscillator uses less current than built-in RC module
* \return
 *
 */

inline void HFCLK_Clock_Configure(uint8_t HCLK_Freq)
{
    //  Write the value of External High Speed Oscillator
    NRF_CLOCK-> XTALFREQ = HCLK_Freq;
    //  Enable Interrupt triggered by start of HFCLK - may be useful for waking up from Low Power Modes
    NRF_CLOCK-> INTENSET |= CLOCK_INTENSET_HFCLKSTARTED_Enabled << CLOCK_INTENSET_HFCLKSTARTED_Pos;
    //  Start the HFCLK
    NRF_CLOCK-> TASKS_HFCLKSTART |= 1;
    //  Wait until the clock is started and stable
    while(NRF_CLOCK->HFCLKSTAT != ((CLOCK_HFCLKSTAT_SRC_Xtal << CLOCK_HFCLKSTAT_SRC_Pos) | (CLOCK_HFCLKSTAT_STATE_Running << CLOCK_HFCLKSTAT_STATE_Pos)))
    {
    }
}


/** \brief This function configures the LFCLK and enables the LFCLK_Start Interrupt
 *
 * \param uint8_t LFCLK_Source - The source of LFCLK
                * 0 - Internal RC Oscillator
                * 1 - External 32768Hz Oscillator
                * 2 - LFCLK synthesized from HFCLK
 * \param
     \note   External oscillator uses less current than built-in RC module
 * \return
 *
 */

inline void LFCLK_Clock_Configure(uint8_t LFCLK_Source)
{
    //  Enable the Interrupt triggered by the start of LFCLK
    NRF_CLOCK-> INTENSET |= CLOCK_INTENSET_LFCLKSTARTED_Enabled << CLOCK_INTENSET_LFCLKSTARTED_Pos;

    //  Select the source of 32768Hz Clock
    NRF_CLOCK-> LFCLKSRC = LFCLK_Source;
    //  Start the LFCLK
    NRF_CLOCK-> TASKS_LFCLKSTART = 1;
    //  Wait until the LFCLK is started and stable
    while(NRF_CLOCK->LFCLKSTAT != ((CLOCK_LFCLKSTAT_SRC_Xtal << CLOCK_LFCLKSTAT_SRC_Pos) | (CLOCK_LFCLKSTAT_STATE_Running << CLOCK_LFCLKSTAT_STATE_Pos)))
    {
    }

}

