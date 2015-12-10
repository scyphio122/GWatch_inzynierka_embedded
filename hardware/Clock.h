#ifndef CLOCK_H_INCLUDED
#define CLOCK_H_INCLUDED

#include "nrf51.h"
#include "nrf51_bitfields.h"

#define CPU_FREQ 16000000
#define LFCLK_FREQ  32768

void HFCLK_Clock_Configure(uint8_t HCLK_Freq);
void LFCLK_Clock_Configure(uint8_t LFCLK_Source);


#endif /* CLOCK_H_INCLUDED */
