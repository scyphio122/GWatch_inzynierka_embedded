#ifndef SPI_H_INCLUDED
#define SPI_H_INCLUDED
#include "nrf51.h"
#include "nrf51_bitfields.h"
#include "hardware_settings.h"

extern uint8_t  spi_transfer_ongoing_flag;			/*< SPI transfer in progress flag */

extern uint8_t* spi_0_rx_buff;						/*< Rx buffer pointer */
extern uint8_t* spi_0_tx_buff;						/*< Tx buffer pointer */
extern uint16_t spi_0_rx_index;						/*< Rx buffer index */
extern uint16_t spi_0_tx_index;						/*< Tx buffer index */
extern uint16_t spi_0_tx_buff_size;					/*< Tx buffer size */
extern uint16_t spi_0_rx_buff_size;					/*< Rx buffer size */


#ifdef SPI1_USED
extern uint8_t* spi_1_rx_buff;						/*< Rx buffer pointer */
extern uint8_t* spi_1_tx_buff;						/*< Tx buffer pointer */
extern uint16_t spi_1_rx_index;						/*< Rx buffer index */
extern uint16_t spi_1_tx_index;						/*< Tx buffer index */
extern uint16_t spi_1_tx_buff_size;					/*< Tx buffer size */
extern uint16_t spi_1_rx_buff_size;					/*< Rx buffer size */

#endif

/**@brief A structure containing configuration parameters of the SPI master driver. */
typedef struct
{
    uint32_t SPI_Freq;          /**< SPI master frequency */
    uint32_t SPI_Pin_SCK;       /**< SCK pin number. */
    uint32_t SPI_Pin_MISO;      /**< MISO pin number. */
    uint32_t SPI_Pin_MOSI;      /**< MOSI pin number .*/
    uint32_t SPI_Pin_SS;        /**< Slave select pin number. */
    uint32_t SPI_PriorityIRQ;   /**< SPI master interrupt priority. */
    uint8_t SPI_CONFIG_ORDER;   /**< Bytes order LSB or MSB shifted out first. */
    uint8_t SPI_CONFIG_CPOL;    /**< Serial clock polarity ACTIVEHIGH or ACTIVELOW. */
    uint8_t SPI_CONFIG_CPHA;    /**< Serial clock phase LEADING or TRAILING. */
    uint8_t SPI_DisableAllIRQ;  /**< Disable all IRQs in critical section. */
} spi_config_t;


/** \brief Function to init spi peripherial 
 * 	\param init - pointer to spi_config_t struct
 *	\return NRF_SUCCESS or error_code */
uint32_t Spi_Init(spi_config_t * init, uint8_t cs_pin);

void SPI_Assert_CS(uint8_t cs_pin);
void SPI_Deassert_CS(uint8_t cs_pin);

uint32_t SPI_Transfer_Blocking(NRF_SPI_Type* SPI,unsigned char* data_to_send, uint16_t data_size, unsigned char* rx_buffer, uint16_t rx_size, uint8_t cs_pin);
uint32_t SPI_Transfer_Non_Blocking(NRF_SPI_Type* SPI, uint8_t* data_to_send, uint16_t data_size, uint8_t* rx_buffer, uint16_t rx_size, uint8_t cs_pin, uint8_t dynamically_allcated_buf);
void  spi_send_message();

void SPI_Wait_For_Transmission_End(NRF_SPI_Type* SPI);
#endif /* SPI_H_INCLUDED */
