#include "nrf51.h"
#include "nrf51_bitfields.h"
#include "spi.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include "nrf_soc.h"

static uint8_t cs_pin;				/*< Number of CS pin */
unsigned char* rx_buff;				/*< Rx buffer pointer */
unsigned char* tx_buff;				/*< Tx buffer pointer */
uint16_t rx_index;					/*< Rx buffer index */
uint16_t tx_index;					/*< Tx buffer index */
uint16_t tx_buff_size;				/*< Tx buffer size */
uint16_t rx_buff_size;				/*< Rx buffer size */
uint8_t spi_transfer_ongoing_flag;	/*< SPI transfer in progress flag */


uint32_t Spi_Init(spi_config_t * init)
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

	NRF_SPI0->INTENSET = SPI_INTENSET_READY_Enabled<<SPI_INTENSET_READY_Pos;
	sd_nvic_DisableIRQ(SPI0_TWI0_IRQn);
	return NRF_SUCCESS;	
}

inline void SPI_Assert_CS()
{
	NRF_GPIO->OUTCLR = 1 << cs_pin;
}

inline void SPI_Deassert_CS()
{
	NRF_GPIO->OUTSET = 1 << cs_pin;
}

__attribute__((optimize("O2")))
uint32_t SPI_Transfer(unsigned char* data_to_send, uint16_t data_size, unsigned char* rx_buffer, uint16_t rx_size)
{
	tx_buff = data_to_send;
	rx_buff = rx_buffer;
	tx_buff_size = data_size;
	rx_buff_size = rx_size;
	tx_index = 0;
	rx_index = 0;
	/// Enable peripheral
	NRF_SPI0->ENABLE = 1;
	///	Clear (assert) the Chip Select
	NRF_GPIO->OUTCLR = ((uint32_t)1 << cs_pin);

	spi_transfer_ongoing_flag = 1;
	spi_send_message();
	///	Disable the SPI module
	NRF_SPI0->ENABLE = 0;
	spi_transfer_ongoing_flag = 0;
	///	Deassert (set in high state) CS pin
	NRF_GPIO->OUTSET = 1 << cs_pin;

	return NRF_SUCCESS;
}
#ifdef ARMCC


#pragma push
#pragma O0
#pragma pop
#endif
__attribute__((optimize("O1")))
void  spi_send_message()
{
	//wyslany jedne znak, jedno miesce w buforze
	volatile char dummy_byte;

	if(tx_buff_size != 0)
	{
			if(tx_buff_size == 1)
			{
				NRF_SPI0->TXD = tx_buff[tx_index++];
				while(!NRF_SPI0->EVENTS_READY){}
				dummy_byte =  NRF_SPI0->RXD;
				NRF_SPI0->EVENTS_READY = 0;
			}
			else
			{
				if((tx_buff_size & 1) == 0)
				{
					while(tx_index < tx_buff_size)
					{
						NRF_SPI0->TXD = tx_buff[tx_index++];
						while(!NRF_SPI0->EVENTS_READY){}
						NRF_SPI0->TXD = tx_buff[tx_index++];
						while(!NRF_SPI0->EVENTS_READY){}
					}
				}
				else
				{
					while(tx_index < (tx_buff_size - 1))
					{
						
						NRF_SPI0->TXD = tx_buff[tx_index++];
						while(!NRF_SPI0->EVENTS_READY){}
						NRF_SPI0->TXD = tx_buff[tx_index++];
						while(!NRF_SPI0->EVENTS_READY){}
					}
					NRF_SPI0->TXD = tx_buff[tx_index++];
				}
				while(!NRF_SPI0->EVENTS_READY){}
					
				while(NRF_SPI0->EVENTS_READY == 1)
				{
				dummy_byte =  NRF_SPI0->RXD;
				dummy_byte =  NRF_SPI0->RXD;
				NRF_SPI0->EVENTS_READY = 0;
				}
				
			}
	}
	if(rx_buff_size != 0)
	{
		if(rx_buff_size == 1)
		{
			NRF_SPI0->TXD = 0x00;
			while(!NRF_SPI0->EVENTS_READY){}
			rx_buff[rx_index] = NRF_SPI0->RXD;
			rx_index++;
			NRF_SPI0->EVENTS_READY = 0;
		}
		else
		{
			if((rx_buff_size & 1) == 0)
			{
				while(rx_index < rx_buff_size)
				{
					NRF_SPI0->TXD = 0x00;
					NRF_SPI0->TXD = 0x00;
					while(!NRF_SPI0->EVENTS_READY);
					NRF_SPI0->EVENTS_READY = 0;
					rx_buff[rx_index++] = NRF_SPI0->RXD;
					while(!NRF_SPI0->EVENTS_READY);
					rx_buff[rx_index++] = NRF_SPI0->RXD;
					NRF_SPI0->EVENTS_READY = 0;
				}
			}
			else
			{
				while(rx_index < (rx_buff_size - 1))
				{
					NRF_SPI0->TXD = 0x00;
					NRF_SPI0->TXD = 0x00;
					while(!NRF_SPI0->EVENTS_READY);
					NRF_SPI0->EVENTS_READY = 0;
					rx_buff[rx_index++] = NRF_SPI0->RXD;
					while(!NRF_SPI0->EVENTS_READY);
					rx_buff[rx_index++] = NRF_SPI0->RXD;
					NRF_SPI0->EVENTS_READY = 0;
				}
				NRF_SPI0->TXD = 0x00;
				while(!NRF_SPI0->EVENTS_READY);
				rx_buff[rx_index++] = NRF_SPI0->RXD;
				NRF_SPI0->EVENTS_READY = 0;
			}
		}
	}
}

