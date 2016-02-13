#include "nrf51.h"
#include "nrf51_bitfields.h"
#include "spi.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include "nrf_soc.h"
#include "RTC.h"
//#include "hardware_settings.h"

#define SPI1_USED

volatile uint8_t  spi_0_transfer_ongoing_flag;			/*< SPI 1 transfer in progress flag */

uint8_t  spi_0_cs_pin;
uint8_t* spi_0_rx_buff;						/*< Rx buffer pointer */
uint8_t* spi_0_tx_buff;						/*< Tx buffer pointer */
uint16_t spi_0_rx_index;					/*< Rx buffer index */
uint16_t spi_0_tx_index;					/*< Tx buffer index */
uint16_t spi_0_tx_buff_size;				/*< Tx buffer size */
uint16_t spi_0_rx_buff_size;				/*< Rx buffer size */
uint8_t	 spi_0_dynamically_allocated_buf = false;


#ifdef SPI1_USED
uint8_t  spi_1_cs_pin;
uint8_t* spi_1_rx_buff;						/*< Rx buffer pointer */
uint8_t* spi_1_tx_buff;						/*< Tx buffer pointer */
uint16_t spi_1_rx_index;					/*< Rx buffer index */
uint16_t spi_1_tx_index;					/*< Tx buffer index */
uint16_t spi_1_tx_buff_size;				/*< Tx buffer size */
uint16_t spi_1_rx_buff_size;				/*< Rx buffer size */
uint8_t	 spi_1_dynamically_allocated_buf = false;
volatile uint8_t  spi_1_transfer_ongoing_flag;			/*< SPI 1 transfer in progress flag */
#endif

/**
 * \brief This function starts the transaction using SPI module
 *
 * \param SPI - the SPI module to use
 */
__attribute__((optimize("O0")))
static void  SPI_Execute_Transaction(NRF_SPI_Type* SPI)
{
	uint8_t* rx_buff;
	uint8_t* tx_buff;
	uint16_t rx_index;
	uint16_t tx_index;
	uint16_t rx_buff_size;
	uint16_t tx_buff_size;

	if(SPI == NRF_SPI0)
	{
		rx_buff = spi_0_rx_buff;
		tx_buff = spi_0_tx_buff;
		rx_index = spi_0_rx_index;
		tx_index = spi_0_tx_index;
		tx_buff_size = spi_0_tx_buff_size;
		rx_buff_size = spi_0_rx_buff_size;
	}
#ifdef SPI1_USED
	if(SPI == NRF_SPI1)
	{
		rx_buff = spi_1_rx_buff;
		tx_buff = spi_1_tx_buff;
		rx_index = spi_1_rx_index;
		tx_index = spi_1_tx_index;
		tx_buff_size = spi_1_tx_buff_size;
		rx_buff_size = spi_1_rx_buff_size;
	}
#endif
	//wyslany jedne znak, jedno miesce w buforze
	volatile char dummy_byte;

	if(tx_buff_size != 0)
	{
			if(tx_buff_size == 1)
			{
				SPI->TXD = tx_buff[tx_index++];
				while(!SPI->EVENTS_READY){}
				dummy_byte =  SPI->RXD;
				SPI->EVENTS_READY = 0;
			}
			else
			{
				if((tx_buff_size & 1) == 0)
				{
					while(tx_index < tx_buff_size)
					{
						SPI->TXD = tx_buff[tx_index++];
						while(!SPI->EVENTS_READY){}
						SPI->TXD = tx_buff[tx_index++];
						while(!SPI->EVENTS_READY){}
					}
				}
				else
				{
					while(tx_index < (tx_buff_size - 1))
					{
						
						SPI->TXD = tx_buff[tx_index++];
						while(!SPI->EVENTS_READY){}
						SPI->TXD = tx_buff[tx_index++];
						while(!SPI->EVENTS_READY){}
					}
					SPI->TXD = tx_buff[tx_index++];
				}
				while(!SPI->EVENTS_READY){}
					
				while(SPI->EVENTS_READY == 1)
				{
					dummy_byte =  SPI->RXD;
					dummy_byte =  SPI->RXD;
					SPI->EVENTS_READY = 0;
				}
				
			}
	}
	if(rx_buff_size != 0)
	{
		if(rx_buff_size == 1)
		{
			SPI->TXD = 0x00;
			while(!SPI->EVENTS_READY){}
			rx_buff[rx_index] = SPI->RXD;
			rx_index++;
			SPI->EVENTS_READY = 0;
		}
		else
		{
			if((rx_buff_size & 1) == 0)
			{
				while(rx_index < rx_buff_size)
				{
					SPI->TXD = 0x00;
					SPI->TXD = 0x00;
					while(!SPI->EVENTS_READY);
					SPI->EVENTS_READY = 0;
					rx_buff[rx_index++] = SPI->RXD;
					while(!SPI->EVENTS_READY);
					rx_buff[rx_index++] = SPI->RXD;
					SPI->EVENTS_READY = 0;
				}
			}
			else
			{
				while(rx_index < (rx_buff_size - 1))
				{
					SPI->TXD = 0x00;
					SPI->TXD = 0x00;
					while(!SPI->EVENTS_READY);
					SPI->EVENTS_READY = 0;
					rx_buff[rx_index++] = SPI->RXD;
					while(!SPI->EVENTS_READY);
					rx_buff[rx_index++] = SPI->RXD;
					SPI->EVENTS_READY = 0;
				}
				SPI->TXD = 0x00;
				while(!SPI->EVENTS_READY);
				rx_buff[rx_index++] = SPI->RXD;
				SPI->EVENTS_READY = 0;
			}
		}
	}
}


#ifdef SPI1_USED
__attribute__((optimize("O0")))
void SPI1_TWI1_IRQHandler()
{

	///	Clear the interrupt flag
	//NRF_SPI1->EVENTS_READY = 0;
	///	If we reached end of receive or transmission
	if((spi_1_tx_index >= spi_1_tx_buff_size))
	{


		NRF_SPI1->INTENCLR = SPI_INTENCLR_READY_Msk;

		///	Deassert the cs pin
		//SPI_Assert_CS(spi_1_cs_pin);
		//RTC_Wait(RTC_US_TO_TICKS(62));


		if(spi_1_dynamically_allocated_buf == true)
		{
			free(spi_1_tx_buff);
			spi_1_dynamically_allocated_buf = false;
		}

		spi_1_transfer_ongoing_flag = 0;

		if(spi_1_cs_pin != SPI_CS_MANUALLY_CHANGED)
			NRF_GPIO->OUTCLR = 1 << spi_1_cs_pin;
		/*while(!NRF_SPI1->EVENTS_READY){}
		///	Disable the peripheral
		NRF_SPI1->ENABLE = 0;
		NRF_SPI1->EVENTS_READY = 0;*/
	}
	else
	///	If we want to transmit
	if(spi_1_tx_buff_size != 0)
	{
		NRF_SPI1->TXD = spi_1_tx_buff[spi_1_tx_index++];
		uint8_t dummy_byte = NRF_SPI1->RXD;
	}

	///	If we want to receive...
	if(spi_1_rx_buff_size != 0)
	{
		NRF_SPI1->TXD = 0;
		spi_1_rx_buff[spi_1_rx_index++] = NRF_SPI1->RXD;
	}





}
#endif

uint32_t Spi_Init(spi_config_t * init, uint8_t cs_pin)
{
	NRF_SPI0->PSELMISO = init->SPI_Pin_MISO;
	NRF_SPI0->PSELMOSI = init->SPI_Pin_MOSI;
	NRF_SPI0->PSELSCK = init->SPI_Pin_SCK;
	NRF_SPI0->FREQUENCY = init->SPI_Freq;

	NRF_SPI0->CONFIG = 0;
	NRF_SPI0->CONFIG |= init->SPI_CONFIG_CPHA |
						init->SPI_CONFIG_CPOL |
						init->SPI_CONFIG_ORDER;


	cs_pin = init->SPI_Pin_SS;

	nrf_gpio_cfg_input(init->SPI_Pin_MISO,NRF_GPIO_PIN_PULLUP);

	nrf_gpio_cfg_output(init->SPI_Pin_MOSI);
	nrf_gpio_pin_clear(init->SPI_Pin_MOSI);

	nrf_gpio_cfg_output(init->SPI_Pin_SCK);
	nrf_gpio_pin_clear(init->SPI_Pin_SCK);
	nrf_gpio_cfg_output(cs_pin);
	nrf_gpio_pin_set(cs_pin);


	return NRF_SUCCESS;
}

inline void SPI_Assert_CS(uint8_t cs_pin)
{
	NRF_GPIO->OUTCLR = 1 << cs_pin;
}

inline void SPI_Deassert_CS(uint8_t cs_pin)
{
	NRF_GPIO->OUTSET = 1 << cs_pin;
}

__attribute__((optimize("O0")))
uint32_t SPI_Transfer_Blocking(NRF_SPI_Type* SPI, unsigned char* data_to_send, uint16_t data_size, unsigned char* rx_buffer, uint16_t rx_size, uint8_t cs_pin)
{
	if(SPI == NRF_SPI0)
	{
		spi_0_tx_buff = data_to_send;
		spi_0_rx_buff = rx_buffer;
		spi_0_tx_buff_size = data_size;
		spi_0_rx_buff_size = rx_size;
		spi_0_tx_index = 0;
		spi_0_rx_index = 0;
	}
#ifdef SPI1_USED
	if(SPI == NRF_SPI1)
	{
		spi_1_tx_buff = data_to_send;
		spi_1_rx_buff = rx_buffer;
		spi_1_tx_buff_size = data_size;
		spi_1_rx_buff_size = rx_size;
		spi_1_tx_index = 0;
		spi_1_rx_index = 0;
	}
#endif
	//NRF_SPI0->INTENSET = SPI_INTENSET_READY_Enabled<<SPI_INTENSET_READY_Pos;
	/// Enable peripheral
	SPI->ENABLE = 1;
	///	Clear (assert) the Chip Select
	SPI_Assert_CS(cs_pin);

	spi_0_transfer_ongoing_flag = 1;
	SPI_Execute_Transaction(SPI);

	spi_0_transfer_ongoing_flag = 0;
	///	Deassert (set in high state) CS pin
	SPI_Deassert_CS(cs_pin);

	///	Disable the SPI module
	SPI->ENABLE = 0;

	return NRF_SUCCESS;
}

uint32_t SPI_Transfer_Non_Blocking(NRF_SPI_Type* SPI, uint8_t* data_to_send, uint16_t data_size, uint8_t* rx_buffer, uint16_t rx_size, uint8_t cs_pin, uint8_t dynamically_allcated_buf)
{
	if(SPI == NRF_SPI0)
	{
		spi_0_tx_buff = data_to_send;
		spi_0_rx_buff = rx_buffer;
		spi_0_tx_buff_size = data_size;
		spi_0_rx_buff_size = rx_size;
		spi_0_tx_index = 0;
		spi_0_rx_index = 0;
		spi_0_cs_pin = cs_pin;
		spi_0_transfer_ongoing_flag = 1;
		spi_0_dynamically_allocated_buf = dynamically_allcated_buf;

	}
#ifdef SPI1_USED
	if(SPI == NRF_SPI1)
	{
		spi_1_tx_buff = data_to_send;
		spi_1_rx_buff = rx_buffer;
		spi_1_tx_buff_size = data_size;
		spi_1_rx_buff_size = rx_size;
		spi_1_tx_index = 0;
		spi_1_rx_index = 0;
		spi_1_cs_pin = cs_pin;
		spi_1_transfer_ongoing_flag = 1;
		spi_1_dynamically_allocated_buf = dynamically_allcated_buf;

	}
#endif


	if(cs_pin != SPI_CS_MANUALLY_CHANGED)
		NRF_GPIO->OUTSET = 1 << cs_pin;
	//Timer_Delay(TIMER_US_TO_TICKS(31));
	//RTC_Wait(RTC_US_TO_TICKS(62));
	///	Enable the SPI peripheral
	SPI->ENABLE = 1;

	SPI->INTENSET = SPI_INTENSET_READY_Msk;

	//SPI->TXD = *data_to_send;
	sd_nvic_SetPendingIRQ(SPI1_TWI1_IRQn);

	return NRF_SUCCESS;
}

volatile void SPI_Wait_For_Transmission_End(NRF_SPI_Type* SPI)
{
	volatile uint8_t* flag = NULL;
	if(SPI == NRF_SPI0)
		flag = &spi_0_transfer_ongoing_flag;
	else
		flag = &spi_1_transfer_ongoing_flag;

	while(!flag)
	{
		__WFE();
	}
}

