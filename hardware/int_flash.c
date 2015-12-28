/*
 * int_flash.c
 *
 *  Created on: 26 gru 2015
 *      Author: Konrad
 */

#ifndef HARDWARE_INT_FLASH_C_
#define HARDWARE_INT_FLASH_C_

#include "nrf51.h"
#include "core_cm0.h"
#include "core_cmInstr.h"
#include <int_flash.h>
#include <nrf_error.h>
#include <nrf_soc.h>
#include <stdint-gcc.h>
#include <string.h>

static uint8_t								flash_write_or_erase_flag 			= 0;
static volatile uint8_t						flash_operation_completed_flag		= TRUE;
static uint32_t 							last_flashed_data;
static uint32_t*							last_flashed_data_pointer;

static void Flash_Wait_Till_Op_Done()
{
	while(flash_operation_completed_flag == FALSE)
	{
		__WFI();
		//Watchdog_Reload();
	}
}

/**
*	\brief This function is an equivalent of the pstorage_update function.
*
*	IMPORTANT: the end address of the updated data must lie on the same page as the start
*	\param[in] p_data	-	pointer to the data which user requires to save
*	\param[in] data_len	-	the size of the data in bytes
*	\param[in] address	-	the flash address where we require to store the data
*
*	\return 	FLASH_OP_SUCCESS		- 	When everything went fine
*				NRF_ERROR_DATA_SIZE		- 	The end address of the updated data doesn't lie on the same page as the start
*				FLASH_OP_WRITE_ERROR	-	If the data couldn't been correctly flashed
*
*/
uint32_t Int_Flash_Update_Page(uint8_t* p_data, uint32_t data_len, uint32_t* address)
{
	uint32_t* 			temp_pointer = address;		/*< Temp pointer needed to check if there is enough blank space for copy the data and to update the data */
	dword_to_byte_u 	data;						/*< Internal buffer for data. The size is equal to uint32_t */
	uint32_t 			empty_bytes_counter = 0;    /**< This variable calculates the number of the free (0xFFFFFFFF) flash cells in the area starting with \var address and size of \var data_len */
	uint32_t 			err_code;					/*<	Error code to check if Int_Flash_Erase_Page and Int_Flash_Store_Dword went successfully */
	int16_t 			temp_swap_index;			/*< This variable gets an index of the current processed byte in uint32_t unit */
	///	Get flash page start address
	uint32_t* 			page_start_address = (uint32_t*)((uint32_t)address - ((uint32_t)address % INTERNAL_FLASH_PAGE_SIZE));	/*< Pointer to the start of the page we want to update data on */
	uint8_t 			flash_operation_counter = 0;	/*< Counter which prevents infinite flashing of data */

	///	Check if an end address of the data lies on the same page as the start
	if(address + data_len/4 > page_start_address + INTERNAL_FLASH_PAGE_SIZE_UINT32)
		return NRF_ERROR_DATA_SIZE;

	/** CHECK IF BY CHANCE THE ENTIRE PLACE WHERE WE WANT TO STORE THE DATA ISN'T EMPTY */
	for(uint32_t i=0; i<data_len; i+=4)
	{
		///	Check if the cell we are trying to save the data in is empty
		memcpy(&data.doubleword, temp_pointer, sizeof(uint32_t));
		///	If the flash cell is empty
		if(data.doubleword == 0xFFFFFFFF)
			empty_bytes_counter += 4;
		///	Increase the pointer to the next uint32_t
		temp_pointer++;
	}

	/** COPY THE DATA TO THE SWAP PAGE IF NEEDED */
	///	If there is some data in the place where we want to store the data
	if(empty_bytes_counter <= data_len)
	{
		do
		{
			flash_operation_counter++;
			temp_swap_index = 0;
			///	Erase swap page
			err_code = Int_Flash_Erase_Page(INTERNAL_FLASH_SWAP_PAGE_ADDRESS);
			///	Copy the flash page to the swap page from the beginning to the address we want to update
			while((page_start_address + temp_swap_index) != address)
			{
				///	Get the byte from the page...
				memcpy(&data.doubleword, page_start_address+temp_swap_index, sizeof(uint32_t));
				if(data.doubleword == 0xFFFFFFFF)
				{
					/// Increase the swap byte index to indicate the next uint32_t address we want to store
					temp_swap_index++;
					///	Skip flashing the 0xFFFFFFFF because it has no sense
					continue;
				}
				///	...and flash it in the swap page
				err_code = Int_Flash_Store_Dword(data.doubleword, INTERNAL_FLASH_SWAP_PAGE_ADDRESS + temp_swap_index);
				if(err_code != FLASH_OP_SUCCESS)
				{
					break;
				}
				/// Increase the swap byte index to indicate the next uint32_t address we want to store
				temp_swap_index++;
			}
			if(err_code != FLASH_OP_SUCCESS)
			{
				continue;
			}
			///	Get the index of the first uint32_t behind the place we want to store
			temp_swap_index = ((uint32_t)temp_pointer % INTERNAL_FLASH_PAGE_SIZE)>>2;
			///	Copy the data which lies behind the block we want to update
			while((page_start_address + temp_swap_index) != page_start_address + INTERNAL_FLASH_PAGE_SIZE_UINT32)
			{
				///	Get the byte from the page...
				memcpy(&data.doubleword, page_start_address+temp_swap_index, sizeof(uint32_t));
				///	...and flash it in the swap page
				err_code = Int_Flash_Store_Dword(data.doubleword, INTERNAL_FLASH_SWAP_PAGE_ADDRESS + temp_swap_index);
				if(err_code != FLASH_OP_SUCCESS)
				{
					break;
				}
				if(data.doubleword == 0xFFFFFFFF)
				{
					/// Increase the swap byte index to indicate the next uint32_t address we want to store
					temp_swap_index++;
					///	Skip flashing the 0xFFFFFFFF because it has no sense
					continue;
				}
				/// Increase the swap byte index to indicate the next uint32_t address we want to store
				temp_swap_index++;
			}
		}while(err_code != FLASH_OP_SUCCESS && flash_operation_counter <= MAX_FLASH_OPERATION_ERROR_COUNTER);
		///	Check if there was an error after three times reflashing

		if(flash_operation_counter > MAX_FLASH_OPERATION_ERROR_COUNTER)
		{
			return FLASH_OP_WRITE_ERROR;
		}
		flash_operation_counter = 0;
		do
		{
			flash_operation_counter++;
			/*< ERASE THE PAGE */
			///	Erase the page where we want to store the data
			Int_Flash_Erase_Page(page_start_address);

			/*< COPY THE NOT AFFECTED DATA BACK TO THE PAGE */

			///	Set the swap index to the last uint32_t of the working page
			if(temp_swap_index == 256)
				temp_swap_index--;
			///	While we don't get to the end of the message
			while((page_start_address + temp_swap_index) >= (address + (data_len >> 2)))
			{
				///	Get the byte from the swap page...
				memcpy(&data.doubleword, INTERNAL_FLASH_SWAP_PAGE_ADDRESS+temp_swap_index, sizeof(uint32_t));
				if(data.doubleword == 0xFFFFFFFF)
				{
					/// Increase the swap byte index to indicate the next uint32_t address we want to store
					temp_swap_index--;
					///	Skip flashing the 0xFFFFFFFF because it has no sense
					continue;
				}
				///	...and flash it in the page
				err_code = Int_Flash_Store_Dword(data.doubleword, page_start_address + temp_swap_index); // bug - dodawanie liczby do adresu?
				if(err_code != FLASH_OP_SUCCESS)
				{
					break;
				}
				/// Increase the swap byte index to indicate the next uint32_t address we want to store
				temp_swap_index--;
			}
			if(err_code != FLASH_OP_SUCCESS)
			{
				continue;
			}
			///	Set the temp_swap index to the first uint32_t before the update area
			temp_swap_index = (((uint32_t)address - (uint32_t)page_start_address)>>2) - 1;

			///	While we don't get to the beginning of the page
			while((page_start_address + temp_swap_index) >= page_start_address)
			{
				///	Get the byte from the swap page...
				memcpy(&data.doubleword, INTERNAL_FLASH_SWAP_PAGE_ADDRESS+temp_swap_index, sizeof(uint32_t));
				if(data.doubleword == 0xFFFFFFFF)
				{
					/// Increase the swap byte index to indicate the next uint32_t address we want to store
					temp_swap_index--;
					///	Skip flashing the 0xFFFFFFFF because it has no sense
					continue;
				}
				///	...and flash it in the page
				err_code = Int_Flash_Store_Dword(data.doubleword, page_start_address + temp_swap_index);
				if(err_code != FLASH_OP_SUCCESS)
				{
					break;
				}
				/// Increase the swap byte index to indicate the next uint32_t address we want to store
				temp_swap_index--;
			}
		}while(err_code != FLASH_OP_SUCCESS && flash_operation_counter <= MAX_FLASH_OPERATION_ERROR_COUNTER);

	}
	///	Check if there was an error after three times reflashing
	if(flash_operation_counter > MAX_FLASH_OPERATION_ERROR_COUNTER)
	{
		return FLASH_OP_WRITE_ERROR;
	}
	flash_operation_counter = 0;

	do
	{
		flash_operation_counter++;
		/*< UPDATE THE DATA */
		///	Go back with the temp_pointer to the given address
		temp_pointer = address;
		///	Store the data we wanted to update
		for(uint16_t i=0; i< data_len; i += 4)
		{
			///	Copy the data in the internal buffer
			memcpy(data.byte, &p_data[i], sizeof(uint32_t));
			///	Flash the internal buffer
			err_code = Int_Flash_Store_Dword(data.doubleword, temp_pointer);
			if(err_code != FLASH_OP_SUCCESS)
			{
				break;
			}
			temp_pointer++;
		}
	}while(err_code != FLASH_OP_SUCCESS && flash_operation_counter <= MAX_FLASH_OPERATION_ERROR_COUNTER);
	///	Check if there was an error after three times reflashing
	if(flash_operation_counter > MAX_FLASH_OPERATION_ERROR_COUNTER)
	{
		return FLASH_OP_WRITE_ERROR;
	}

	return FLASH_OP_SUCCESS;
}

/**
*	@brief This function tries to flash the given data at the given pointer. IF the data wasn't programmed (the cell still is empty) it reflashes it
*			If any error occures, it returns COMPLETED_WRITE_ERROR or FLASH_OPERATION_ERROR if data wasn't stored. Otherwise it returns FLASH_OP_SUCCESS. It checks automatically if the data was successfully stored
*	@param data_to_flash 	- 	the given uint32_t which we want to store
*	@param pointer 			-	the pointer where we try to store te data_to_flash
*	@return flag which informs us if the data was successfully stored or not:
*			COMPLETED_WRITE_ERROR	-	data was stored but corrupted
*			FLASH_OP_SUCCESS		-	storage OK
*
**/
uint32_t  Int_Flash_Store_Dword(uint32_t data_to_flash, uint32_t* pointer)
{
	flash_write_or_erase_flag = FLASH_WRITE_OPERATION;
	static uint8_t flash_counter = 0;
	do
	{
		//	Clear the flag which informs whether the flash write was completed and was corrupted
		flash_operation_completed_flag = FALSE;
		//	Fill the global variable with the data we want to store
		last_flashed_data = data_to_flash;
		//	Set the global pointer where we try to store the given uint32_t
		last_flashed_data_pointer = pointer;
		//	Try to write the data at the given address
		sd_flash_write(pointer, &data_to_flash, 1);
		//	Wait till the flag is set indicating whethet the flash store was completed and has ended with an error
		Flash_Wait_Till_Op_Done();
		flash_counter++;

		//	If it was FLASH_OPERATION_ERROR and the memory was not programmed, try to flash it again. If it was FLASH_OP_SUCCESS or FLASH_OP_WRITE_ERROR then return from this function with a proper value
	}while(flash_operation_completed_flag == FLASH_OPERATION_ERROR && flash_counter < MAX_FLASH_OPERATION_ERROR_COUNTER);

	///	If after MAX_FLASH_OPERATION_ERROR_COUNTER tries it still isn't programmed, then return FLASH_OP_WRITE_ERROR
	if(flash_operation_completed_flag == FLASH_OPERATION_ERROR)
		flash_operation_completed_flag = FLASH_OP_WRITE_ERROR;
	flash_counter = 0;

	//	If there was an flash write error, log it
	if(flash_operation_completed_flag != FLASH_OP_SUCCESS)
	{

	}
	//	return callback flag
	return flash_operation_completed_flag;
}

/**
*	@brief	This function should be called in order to erase the flash page. IF the erasing won't succeed for 3 times, then this function returns an FLASH_OP_ERASE_ERROR
*	@param  flash_page_beginning_address - the beginning address of the page we want to erase.
*	@param	The actually erased flash page beginning address
*				FLASH_OP_ERASE_ERROR	-	page wasn't erased
*				FLASH_OP_SUCCESS		-	page erased
**/
uint32_t Int_Flash_Erase_Page(uint32_t* flash_page_beginning_address)
{
	uint8_t erase_counter = 0;
	flash_write_or_erase_flag = FLASH_ERASE_OPERATION;
	flash_operation_completed_flag = FALSE;
	uint32_t temp;
	uint32_t ret_val = 0;
	do
	{
		ret_val = sd_flash_page_erase((uint32_t)flash_page_beginning_address/1024);

		Flash_Wait_Till_Op_Done();

		//	If the first cell on the page is cleared
		memcpy(&temp, flash_page_beginning_address, 4);
		if(temp == 0xFFFFFFFF)
		{
			//	And the second cell on the page also  is cleared
			memcpy(&temp, flash_page_beginning_address+1, 4);
			if(temp == 0xFFFFFFFF)
			{
				return FLASH_OP_SUCCESS;
			}
			else

				erase_counter++;
		}
		else
			erase_counter++;

	}while(erase_counter<MAX_FLASH_OPERATION_ERROR_COUNTER);


	return FLASH_OP_ERASE_ERROR;
}

/**
*	@brief This function is a callback which is called in sys_evt_dispatch() function, after some data is stored or the page erased using sd_flash_xxx() function. It gives us a feedback when the operation is completed and if the data was stored successfully
*	@param uint32_t sys_evt - the number of event which is given by SWIO from softdevice
**/
void SD_flash_operation_callback(uint32_t sys_evt)
{
	switch(sys_evt)
	{
		case NRF_EVT_FLASH_OPERATION_SUCCESS:
		{
			switch(flash_write_or_erase_flag)
			{
				case FLASH_WRITE_OPERATION:
				{
					uint32_t temp;
					//	read what was just flashed
					memcpy(&temp, last_flashed_data_pointer, sizeof(uint32_t));
					//	Check if data just read is identical as the last_flashed_data
					if(last_flashed_data == temp)
					{
						flash_operation_completed_flag = FLASH_OP_SUCCESS;	///	data ok
					}
					else
					{
						flash_operation_completed_flag = FLASH_OP_WRITE_ERROR;	///	data corrupted
					}
					break;
				}
				case FLASH_ERASE_OPERATION:
					flash_operation_completed_flag = FLASH_OP_SUCCESS;
					break;
				default:	//	If we do not want to check the stored data
					flash_operation_completed_flag = FLASH_OP_SUCCESS;
					break;
			}
			break;
		}
		case NRF_EVT_FLASH_OPERATION_ERROR:
			switch(flash_write_or_erase_flag)
			{
				case FLASH_WRITE_OPERATION:
					{
						uint32_t temp;
						//	read what was just flashed
						memcpy(&temp, last_flashed_data_pointer, sizeof(uint32_t));
						//	If the data wasn't even stored
						if(temp == 0xFFFFFFFF)
						{
							flash_operation_completed_flag = FLASH_OPERATION_ERROR;
						}
						else//	Check if data just read is identical as the last_flashed_data, if not then call an error
						if(temp != last_flashed_data)
						{
							flash_operation_completed_flag = FLASH_OP_WRITE_ERROR;
						}
					}
					break;
				default:
					flash_operation_completed_flag = FLASH_OP_ERASE_ERROR;
					break;
			}
			break;
	}
	flash_write_or_erase_flag = 0;	//	clear the flag
}

#endif /* HARDWARE_INT_FLASH_C_ */
