/*
 * ext_flash.c
 *
 *  Created on: 24 gru 2015
 *      Author: Konrad
 */

#include <ext_flash.h>
#include <hardware_settings.h>
#include <nrf_error.h>
#include <nrf51.h>
#include <RTC.h>
#include <sys/_stdint.h>



static uint8_t	 				ext_flash_on = 0;
static uint8_t*	 				ext_flash_data_ptr = NULL;
static uint16_t  				ext_flash_data_size;
static ext_flash_status_reg_t 	ext_flash_status_reg = {0};

static void SPI_0_Init()
{
	nrf_gpio_cfg_input(SPI0_MISO_PIN);
	nrf_gpio_cfg_output(SPI0_MOSI_PIN);
	nrf_gpio_cfg_output(SPI0_SCK_PIN);

	NRF_SPI1->PSELSCK = SPI0_SCK_PIN;
	NRF_SPI1->PSELMOSI = SPI0_MOSI_PIN;
	NRF_SPI0->PSELMISO = SPI0_MISO_PIN;

	///	Enable the SPI Interrupt
	//NRF_SPI0->INTENSET = SPI_INTENSET_READY_Msk;

	///	Clear configuration
	NRF_SPI0->CONFIG = 0;

	///	Set the CPHA0 and CPOL0 and MSB bit first
	NRF_SPI0->CONFIG = (SPI_CONFIG_CPHA_Leading << SPI_CONFIG_CPHA_Pos) | (SPI_CONFIG_CPOL_ActiveHigh << SPI_CONFIG_CPOL_Pos) | (SPI_CONFIG_ORDER_MsbFirst << SPI_CONFIG_ORDER_Pos);

	///	Set the Display SPI CLK freqency to 8 MHz
	NRF_SPI0->FREQUENCY = SPI_FREQUENCY_FREQUENCY_M8;
}

uint32_t Ext_Flash_Init()
{
	SPI_0_Init();

	nrf_gpio_cfg_output(EXT_FLASH_ON_PIN);
	NRF_GPIO->OUTSET = 1 << EXT_FLASH_ON_PIN;

	nrf_gpio_cfg_output(EXT_FLASH_WP_PIN);
	NRF_GPIO->OUTSET = 1 << EXT_FLASH_WP_PIN;

	nrf_gpio_cfg_output(EXT_FLASH_CS_PIN);
	NRF_GPIO->OUTSET = 1 << EXT_FLASH_CS_PIN;

	nrf_gpio_cfg_output(EXT_FLASH_RESET_PIN);
	NRF_GPIO->OUTSET = 1 << EXT_FLASH_RESET_PIN;

	return NRF_SUCCESS;
}
/**
 * \brief This function turns on the External Flash module. It blocks program execution untill the Read or Program/Erase operation can be done.
 *
 * \param read_or_erase - the param which tells what kind of operation will be done next. It is needed, because the time from power-up to read command or program/erase command is different
 *
 */
uint32_t Ext_Flash_Turn_On(ext_flash_operation_type_e read_or_erase)
{
	if(ext_flash_on == 0)
	{
		NRF_GPIO->OUTCLR = 1 << EXT_FLASH_ON_PIN;
		ext_flash_on = 1;
		if(read_or_erase == EXT_FLASH_READ_OP)
			RTC_Wait(RTC_US_TO_TICKS(EXT_FLASH_TURN_ON_DELAY_READ_US));
		else
			RTC_Wait(RTC_US_TO_TICKS(EXT_FLASH_TURN_ON_DELAY_PROGRAM_ERASE_US));
		return NRF_SUCCESS;
	}

	return NRF_ERROR_INVALID_STATE;
}

/**
 * \brief This function turns off the external flash module.
 */
uint32_t Ext_Flash_Turn_Off()
{
	if(ext_flash_on == 1)
	{
		NRF_GPIO->OUTSET = 1 << EXT_FLASH_ON_PIN;
		ext_flash_on = 0;

		return NRF_SUCCESS;
	}

	return NRF_ERROR_INVALID_STATE;
}


/**
 * \brief This function reads the single status register
 *
 * \return 	NRF_SUCCESS - everything went fine
 * 			NRF_ERROR_INVALID_STATE - the flash module is powered down
 */
static uint32_t Ext_Flash_Read_Status_Reg()
{
	if(ext_flash_on == 0)
		return NRF_ERROR_INVALID_STATE;

	uint8_t command_code = EXT_FLASH_STATUS_REG_READ;

	SPI_Transfer_Blocking(NRF_SPI0, &command_code, sizeof(command_code), &ext_flash_status_reg, sizeof(ext_flash_status_reg));

	return NRF_SUCCESS;
}

/**
 * \brief This function blocks the program execution until the external flash is ready to do another operation
 *
 * \return 	NRF_SUCCESS - everything went fine
 * 			NRF_ERROR_INVALID_STATE - the flash module is powered down
 */
static uint32_t Ext_Flash_Wait_Till_Ready()
{
	if(ext_flash_on == 0)
		return NRF_ERROR_INVALID_STATE;
	do
	{
		Ext_Flash_Read_Status_Reg();
		RTC_Wait(RTC_US_TO_TICKS(50));
	}while(ext_flash_status_reg.ready_or_busy != 1);

	return NRF_SUCCESS;
}

/**
 * \brief This function checks the value of the erase_or_program_error bit in the ext_flash_status_reg structure.
 *			If the bit is set high after the last Status Register Read operation, an error ocurred, otherwise no error was discovered
 *
 * \return 		NRF_SUCCESS - no error ocurred
 * 				NRF_ERROR_INTERNAL - error occurred
 *
 */
static inline  uint32_t Ext_Flash_Check_Program_Erase_Error()
{
	if(ext_flash_status_reg.erase_or_program_error == 1)
		return NRF_ERROR_INTERNAL;

	return NRF_SUCCESS;
}

/**
 * \brief This function stores the data in the specified Ext Flash SRAM buffer.
 * 			It then can be flashed using the Ext_Flash_Program_Page_With_Preerase() or Ext_Flash_Program_Page_Without_Preerase() functions
 *
 * \param buffer_number - number of the buffer where the data is to be stored
 * \param buffer_address - the start address, where the data is to be stored inside the buffer
 * \param data - pointer to the data buffer
 * \param data_size - size of data which is to be stored in the buffer
 *
 * \return 	NRF_SUCCESS - everything went fine
 * 			NRF_ERROR_INVALID_STATE - the flash module is powered down
 * 			NRF_ERROR_DATA_SIZE - the data exceeds outside the buffer size
 */
uint32_t Ext_Flash_Write_Buffer(ext_flash_buffer_number_e buffer_number, uint8_t buffer_address, uint8_t* data, uint16_t data_size)
{
	if(ext_flash_on == 0)
		return NRF_ERROR_INVALID_STATE;

	if((buffer_address + data_size) > (buffer_address + EXT_FLASH_PAGE_SIZE))
		return NRF_ERROR_DATA_SIZE;

	uint8_t* data_ptr = malloc(data_size + 4);

	///	Set the buffer address
	if(buffer_number == EXT_FLASH_BUFFER_1)
		data_ptr[0] = EXT_FLASH_WRITE_BUFFER_1;
	else
		data_ptr[0] = EXT_FLASH_WRITE_BUFFER_2;

	///	Set the 2 dummy bytes and buffer address
	data_ptr[1] = 0;
	data_ptr[2] = 0;
	data_ptr[3] = buffer_address;

	///	Copy the data
	memcpy(&data_ptr[4], data, data_size);

	///	Wait until the flash module is ready
	Ext_Flash_Read_Status_Reg();
	///	Send the data
	SPI_Transfer_Blocking(NRF_SPI0, data_ptr, data_size, NULL, 0, EXT_FLASH_CS_PIN);

	free(data_ptr);

	return NRF_SUCCESS;
}

/**
 * \brief This function flashes the data which is stored in the ext_flash module's SRAM buffer on the page with the given address.
 *
 * 		IT PRE_ERASES THE FLASH PAGE BEFORE DATA FLASHING
 *
 * NOTE: The flash page address must be aligned to the flash page start address (lower 8 bits are treated as dummy bits)
 *
 * \param buf_number - number of the buffer from which the data is to be flashed
 * \param address - the address of the flash page where the data is to be stored. It must be aligned to the flash page start address
 *
 * \return 	NRF_ERROR_INVALID_STATE - the ext_flash_module is turned off
 * 			NRF_SUCCESS - everything went fine
 */
uint32_t Ext_Flash_Program_Page_With_Preerase(ext_flash_buffer_number_e buf_number, uint32_t address)
{
	if(ext_flash_on == 0)
		return NRF_ERROR_INVALID_STATE;

	uint8_t spi_msg [4];

	if(buf_number == EXT_FLASH_BUFFER_1)
		spi_msg[0] = EXT_FLASH_PAGE_PROG_FROM_BUF_W_PREERASE_BUF_1;
	else
		spi_msg[0] = EXT_FLASH_PAGE_PROG_FROM_BUF_W_PREERASE_BUF_2;

	msmcpy(&spi_msg[1], &address, 3);

	///	Wait until the flash module is ready
	Ext_Flash_Read_Status_Reg();
	///	Program the page
	SPI_Transfer_Blocking(NRF_SPI0, spi_msg, sizeof(spi_msg), NULL, 0, EXT_FLASH_CS_PIN);
}

/**
 * \brief This function flashes the data which is stored in the ext_flash module's SRAM buffer on the page with the given address.
 *
 * 		IT DOES NOT ERASE THE FLASH PAGE BEFORE DATA FLASHING
 *
 * NOTE: The flash page address must be aligned to the flash page start address (lower 8 bits are treated as dummy bits)
 *
 * \param buf_number - number of the buffer from which the data is to be flashed
 * \param address - the address of the flash page where the data is to be stored. It must be aligned to the flash page start address
 *
 * \return 	NRF_ERROR_INVALID_STATE - the ext_flash_module is turned off
 * 			NRF_SUCCESS - everything went fine
 */
uint32_t Ext_Flash_Program_Page_Without_Preerase(ext_flash_buffer_number_e buf_number, uint32_t address)
{
	if(ext_flash_on == 0)
		return NRF_ERROR_INVALID_STATE;

	uint8_t spi_msg [4];

	if(buf_number == EXT_FLASH_BUFFER_1)
		spi_msg[0] = EXT_FLASH_PAGE_PROG_FROM_BUF_WITHOUT_PREERASE_BUF_1;
	else
		spi_msg[0] = EXT_FLASH_PAGE_PROG_FROM_BUF_WITHOUT_PREERASE_BUF_2;

	msmcpy(&spi_msg[1], &address, 3);
	///	Wait until the flash module is ready
	Ext_Flash_Read_Status_Reg();
	///	Start programming the page
	SPI_Transfer_Blocking(NRF_SPI0, spi_msg, sizeof(spi_msg), NULL, 0, EXT_FLASH_CS_PIN);
}


