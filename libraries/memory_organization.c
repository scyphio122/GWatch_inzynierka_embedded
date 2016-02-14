/*
 * memory_organization.c
 *
 *  Created on: 26 gru 2015
 *      Author: Konrad
 */


#include <ble_uart.h>
#include <GPS.h>
#include <libraries/memory_organization.h>
#include <nrf_error.h>
#include <nrf_soc.h>
#include <nrf51.h>
#include <RTC.h>
#include <stdbool.h>
#include <string.h>
#include "ext_flash.h"



static uint32_t 		mem_org_next_free_key_address = MEM_ORG_KEY_AREA_START_ADDRESS;
static uint32_t 	    mem_org_next_free_data_sample_address = MEM_ORG_DATA_AREA_START_ADDRESS;
static uint16_t			mem_org_tracks_stored = 0;
volatile uint8_t		mem_org_track_samples_storage_enabled = 0;
static uint16_t         mem_org_track_size;
uint32_t 				mem_org_gps_sample_storage_interval = 15;
/**
 * \brief This function finds the first free key address pn the page (if existing)
 *
 * \param 			page_address - address within the page to search
 * \param[out] 		key_address - the buffer for the key if found. If not found, NULL value is stored
 *
 * \return 	NRF_SUCCESS - the key was found
 *  		NRF_ERROR_NOT_FOUND - the key was not found
 */
static uint32_t Mem_Org_Key_Find_First_Free_On_Page(uint32_t page_address, uint32_t* key_address)
{
	uint32_t temp_key = 0;
#ifndef EXT_FLASH_AVAILABLE
	uint16_t flash_page_size = INTERNAL_FLASH_PAGE_SIZE;
#else
	Ext_Flash_Turn_On(EXT_FLASH_READ_OP);
	uint16_t flash_page_size = EXT_FLASH_PAGE_SIZE;
#endif
	///	Calculate the next page start address
	uint32_t next_page_address = page_address - (page_address % flash_page_size) + flash_page_size;
	///	Set the address to the first free uint32_t after page header
	page_address = page_address - (page_address % flash_page_size) + sizeof(mem_org_flash_page_header_t);

	do
	{
#ifndef EXT_FLASH_AVAILABLE
		memcpy(&temp_key, (uint32_t*)page_address, sizeof(uint32_t));
#else
		Ext_Flash_Read_Page(page_address, &temp_key, sizeof(uint32_t));
#endif
		if(temp_key == 0xFFFFFFFF)
		{
			*key_address = page_address;
#ifdef EXT_FLASH_AVAILABLE
		Ext_Flash_Turn_Off();
#endif
			return NRF_SUCCESS;
		}
		else
		{
			mem_org_tracks_stored = (temp_key >> MEM_ORG_KEY_TRACK_NUMBER_SHIFT) & 0x7FFF;
			page_address += 4;
		}
	}while(page_address < next_page_address);

	*key_address = 0;
#ifdef EXT_FLASH_AVAILABLE
		Ext_Flash_Turn_Off();
#endif
	return NRF_ERROR_NOT_FOUND;
}

/**
 * \brief This function returns the decoded data address which is held in the key
 *
 * \param key - the key to decode
 *
 * \return NRF_SUCCESS
 */
static inline uint32_t Mem_Org_Get_Key_Encoded_Address(uint32_t key)
{
	return (((key & 0xFFFF) << MEM_ORG_KEY_ADD_SHIFT) + MEM_ORG_DATA_AREA_START_ADDRESS);
}

/**
 * \brief This function returns the track number encoded in the given key
 *
 * \param key - the key to decode
 *
 * \return the track number
 */
static inline uint32_t Mem_Org_Get_Key_Encoded_Track_Number(uint32_t key)
{
	return (key & 0x7FFF0000) >> MEM_ORG_KEY_TRACK_NUMBER_SHIFT;
}


/**
 * \brief This function searches entire KEY_AREA in order to find the first free cell
 *
 * \return	NRF_SUCCESS - the key was found
 * 			NRF_ERROR_NOT_FOUND - the key wasn't found
 */
static uint32_t Mem_Org_Key_Find_First_Free()
{
	uint32_t ret_val = 0;
	for(uint32_t i = MEM_ORG_KEY_AREA_START_ADDRESS; i< MEM_ORG_KEY_AREA_END_ADDRESS; i += 1024)
	{
		ret_val = Mem_Org_Key_Find_First_Free_On_Page(i, &mem_org_next_free_key_address);
		if(ret_val == NRF_SUCCESS)
		{
			return NRF_SUCCESS;
		}
	}

	return NRF_ERROR_NOT_FOUND;
}

/**
 * \brief This function finds the first free page where the track can be stored
 *
 * \return	NRF_SUCCESS - everything went fine
 * 			NRF_ERROR_INTERNAL - timeout event occured
 */
static uint32_t Mem_Org_Track_Address_Find_First_Free()
{
	uint32_t err_code = 0;
	uint32_t key;

#ifdef EXT_FLASH_AVAILABLE
		Ext_Flash_Turn_On(EXT_FLASH_READ_OP);
#endif
	///	Get the last key
	err_code = Mem_Org_Find_Key(mem_org_tracks_stored, &key);
	if(err_code == NRF_ERROR_NOT_FOUND)
	{
		mem_org_next_free_data_sample_address = MEM_ORG_DATA_AREA_START_ADDRESS;
	}
	else
	{
		uint32_t data_address = Mem_Org_Get_Key_Encoded_Address(key);
		mem_org_flash_page_header_t header;
#ifndef EXT_FLASH_AVAILABLE
		uint16_t flash_page_size = INTERNAL_FLASH_PAGE_SIZE;
		memcpy((uint32_t*)&header, (uint32_t*)data_address, sizeof(uint32_t));
#else
		uint16_t flash_page_size = EXT_FLASH_PAGE_SIZE;
		Ext_Flash_Read_Page(data_address, &header, sizeof(uint32_t));
#endif

		if(header.entry_size_in_bytes != 0xFFFF)
		{
			uint8_t flash_pages_used = header.entry_size_in_bytes/(MEM_ORG_DATA_SAMPLES_ON_INT_FLASH_PAGE*sizeof(mem_org_gps_sample_t));

			mem_org_next_free_data_sample_address = data_address + (flash_pages_used + 1)*flash_page_size;
		}
		else
		{
			uint32_t temp_add = data_address;
			uint32_t temp = 0;
			RTC_Timeout(RTC_S_TO_TICKS(3));
			do
			{
#ifndef EXT_FLASH_AVAILABLE
				memcpy(&temp, temp_add + 4, sizeof(uint32_t));
#else
				Ext_Flash_Read_Page(temp_add+4, &temp, sizeof(uint32_t));
#endif
				if(temp == 0xFFFFFFFF)
				{
					mem_org_next_free_data_sample_address = data_address;
					timeout_flag = 0;
					RTC_Cancel_Timeout();
#ifdef EXT_FLASH_AVAILABLE
		Ext_Flash_Turn_Off();
#endif
					return NRF_SUCCESS;
				}
				else
					temp_add += flash_page_size;
			}while(!timeout_flag);
		}
	}
	timeout_flag = 0;
	RTC_Cancel_Timeout();
#ifdef EXT_FLASH_AVAILABLE
		Ext_Flash_Turn_Off();
#endif
	return NRF_ERROR_INTERNAL;
}

/**
 *  \brief This function initializes important pointers for the  memory organization module. It should be called t the beginning of the program
 *
 *  \return NRF_SUCCESS - everything went fine
 *  		NRF_ERROR_INTERNAL - timeout occured
 */
uint32_t Mem_Org_Init()
{
	uint32_t err_code = 0;
	err_code = Mem_Org_Key_Find_First_Free();
	if(err_code == NRF_SUCCESS)
	{
		err_code = Mem_Org_Track_Address_Find_First_Free();
		return NRF_SUCCESS;
	}

	return NRF_ERROR_INTERNAL;

}

/**
 * \brief This function stores the key which encodes the pointer to the data.
 *
 * \param address_to_data - address where the data will be stored
 * \param track_number - the track number which will describe the data encoded by the key
 *
 * \return	NRF_SUCCESS - everything went fine
 * 			NRF_ERROR_INTERNAL - the key couldn't been stored for three times
 */
uint32_t Mem_Org_Store_Key(uint32_t address_to_data, uint16_t track_number)
{
	uint32_t key = 0x80000000;
	uint32_t err_code = 0;
	uint8_t safety_counter = 0;

	///	Set the address
	key |= (address_to_data - MEM_ORG_DATA_AREA_START_ADDRESS) >> MEM_ORG_KEY_ADD_SHIFT;
	///	Set the track number
	key |= (track_number) << MEM_ORG_KEY_TRACK_NUMBER_SHIFT;

	do
	{
#ifndef EXT_FLASH_AVAILABLE
		uint16_t flash_page_size = INTERNAL_FLASH_PAGE_SIZE;
		///	Store the key
		err_code = Int_Flash_Store_Dword(key, (uint32_t*)mem_org_next_free_key_address);
		if(err_code == FLASH_OP_SUCCESS)
		{
#else
		uint16_t flash_page_size = EXT_FLASH_PAGE_SIZE;
		err_code = Ext_Flash_Program_Page_Through_Buffer_Without_Preerase(mem_org_next_free_key_address, &key, sizeof(key));
		if(err_code == NRF_SUCCESS)
		{
#endif
			uint32_t temp = key & (uint32_t)0x7FFFFFFF;
			///	Clear the MSB
			key = temp;
			///	Acknowledge the key
#ifndef 	EXT_FLASH_AVAILABLE
			///	Store the key
			err_code = Int_Flash_Store_Dword(key, (uint32_t*)mem_org_next_free_key_address);
			if(err_code == FLASH_OP_SUCCESS)
			{
				///	Increase the pointer to the next free address
				mem_org_next_free_key_address += 4;
				///	If we get to the next page start then set the pointer to the first address after the header
				if(mem_org_next_free_key_address % flash_page_size == 0)
					mem_org_next_free_key_address += sizeof(mem_org_flash_page_header_t);
				return NRF_SUCCESS;
			}
#else
			err_code = Ext_Flash_Program_Page_Through_Buffer_Without_Preerase(mem_org_next_free_key_address, &key, sizeof(key));
			if(err_code == NRF_SUCCESS)
			{
				///	Increase the pointer to the next free address
				mem_org_next_free_key_address += 4;
				///	If we get to the next page start then set the pointer to the first address after the header
				if(mem_org_next_free_key_address % flash_page_size == 0)
					mem_org_next_free_key_address += sizeof(mem_org_flash_page_header_t);
				return NRF_SUCCESS;
			}
#endif
		}
		else
		{
			///	Increase the pointer to the next free address
			mem_org_next_free_key_address += 4;
			continue;
		}
	}while(++safety_counter < MAX_FLASH_OPERATION_ERROR_COUNTER);

	return NRF_ERROR_INTERNAL;
}

/**
 * \brief This function finds the key which encodes the requested track number
 *
 * \param track_number - the requested track_number
 * \param[out] key_buf - buffer for key
 *
 * \return 	NRF_SUCCESS - key was found
 * 			NRF_ERROR_INVALID - timeout occured
 */
uint32_t Mem_Org_Find_Key(uint16_t track_number, uint32_t* key_buf)
{
	uint8_t page_number = (track_number - 1) / MEM_ORG_KEY_AREA_KEYS_ON_PAGE;
	uint8_t key_on_page = (track_number - 1) % MEM_ORG_KEY_AREA_KEYS_ON_PAGE;

	uint32_t key = 0;
	uint16_t key_index = 0;

#ifdef EXT_FLASH_AVAILABLE
	Ext_Flash_Turn_On(EXT_FLASH_READ_OP);
#endif

	RTC_Timeout(RTC_S_TO_TICKS(2));
	do
	{
#ifndef EXT_FLASH_AVAILABLE
		uint16_t flash_page_size = INTERNAL_FLASH_PAGE_SIZE;
		///	Get the key
		memcpy(&key, MEM_ORG_KEY_AREA_START_ADDRESS + page_number*INTERNAL_FLASH_PAGE_SIZE + sizeof(mem_org_flash_page_header_t) + key_on_page*sizeof(uint32_t), sizeof(uint32_t));
#else
		uint16_t flash_page_size = EXT_FLASH_PAGE_SIZE;
		uint32_t add = MEM_ORG_KEY_AREA_START_ADDRESS + page_number*flash_page_size + sizeof(mem_org_flash_page_header_t) + key_on_page*sizeof(uint32_t);
		Ext_Flash_Read_Page(add, &key, sizeof(uint32_t));
#endif
		if(key == 0xFFFFFFFF)
		{
#ifdef EXT_FLASH_AVAILABLE
			Ext_Flash_Turn_Off();
#endif
			return NRF_ERROR_NOT_FOUND;
		}

		key_index = ((key >> MEM_ORG_KEY_TRACK_NUMBER_SHIFT) & 0x7FFF);
		///	If it is the requested key
		if(key_index == track_number)
		{
			///	If the key is valid
			if((key & 0x80000000) == 0)
			{
				*key_buf = key;
				RTC_Cancel_Timeout();
#ifdef EXT_FLASH_AVAILABLE
				Ext_Flash_Turn_Off();
#endif
				return NRF_SUCCESS;
			}
			else
			{
				key_on_page++;
				continue;
			}

		}
		else
			if(key_index > track_number)
			{
				key_on_page--;
			}
			else
			{
				key_on_page++;
			}
	}while (!timeout_flag);

	timeout_flag = 0;
	RTC_Cancel_Timeout();

#ifdef EXT_FLASH_AVAILABLE
	Ext_Flash_Turn_Off();
#endif
	return NRF_ERROR_INTERNAL;
}

/**
 * \brief This function stores the gps sample in the memory
 *
 * \param timestamp - the timestamp of the sample
 *
 * \return 	NRF_ERROR_NO_MEM - no more available memory
 * 			NRF_SUCCESS - sample stored without any error
 */
uint32_t Mem_Org_Store_Sample(uint32_t timestamp)
{
	mem_org_gps_sample_t sample;
	uint8_t err_code = 0;
#ifndef EXT_FLASH_AVAILABLE
	uint16_t flash_page_size = INTERNAL_FLASH_PAGE_SIZE
#else
	uint16_t flash_page_size = EXT_FLASH_PAGE_SIZE;
#endif

	sample.timestamp = timestamp;

#ifdef EXT_FLASH_AVAILABLE
	Ext_Flash_Turn_On(EXT_FLASH_ERASE_OP);
#endif
	///	Copy the latitude
	memcpy(sample.latitude, &gga_message.latitude, 10);
	///	Copy the longtitude
	memcpy(sample.longtitude, &gga_message.longtitude, 10);



	///	If no more memory is available - stop sampling and return error
	if(mem_org_next_free_data_sample_address + sizeof(sample) >= MEM_ORG_DATA_AREA_END_ADDRESS)
	{
		Mem_Org_Track_Stop_Storage();

#ifdef EXT_FLASH_AVAILABLE
		Ext_Flash_Turn_Off();
#endif
		return NRF_ERROR_NO_MEM;
	}
#ifndef EXT_FLASH_AVAILABLE
	for(uint8_t i=0; i < sizeof(mem_org_gps_sample_t)/4; i+=1)
	{
		if(mem_org_next_free_data_sample_address % flash_page_size == 0)
			mem_org_next_free_data_sample_address += sizeof(mem_org_flash_page_header_t);

		err_code = Int_Flash_Store_Dword(*((uint32_t*)&sample + i), (uint32_t*)mem_org_next_free_data_sample_address);

		mem_org_next_free_data_sample_address += 4;
	}
#else
		if(mem_org_next_free_data_sample_address % flash_page_size == 0)
			mem_org_next_free_data_sample_address += sizeof(mem_org_flash_page_header_t);


		err_code = Ext_Flash_Program_Page_Through_Buffer_Without_Preerase(mem_org_next_free_data_sample_address, &sample, sizeof(sample));

		mem_org_next_free_data_sample_address += sizeof(sample);
		///	If the sample would be stored in between flash pages than go to the next flash page
		if(mem_org_next_free_data_sample_address + sizeof(sample) >= (mem_org_next_free_data_sample_address - (mem_org_next_free_data_sample_address % flash_page_size) + flash_page_size))
			mem_org_next_free_data_sample_address = (mem_org_next_free_data_sample_address - (mem_org_next_free_data_sample_address % flash_page_size) + flash_page_size) + sizeof(mem_org_flash_page_header_t);

#endif
	mem_org_track_size += sizeof(sample);

#ifdef EXT_FLASH_AVAILABLE
	Ext_Flash_Turn_Off();
#endif
	return NRF_SUCCESS;
}

/**
 * \brief This function prepares the mem org module to store next track - it creates the key for it
 *
 * \return NRF_SUCCESS
 */
uint32_t Mem_Org_Track_Start_Storage()
{
	mem_org_tracks_stored++;
	Ext_Flash_Turn_On(EXT_FLASH_ERASE_OP);
	///	Store the key
	uint32_t err_code = Mem_Org_Store_Key(mem_org_next_free_data_sample_address, mem_org_tracks_stored);
	Ext_Flash_Turn_Off();
	mem_org_track_samples_storage_enabled = 1;

	return NRF_SUCCESS;
}

/**
 * \brief This function writes end data for the stored track
 *
 * \return NRF_SUCCESS
 */
uint32_t Mem_Org_Track_Stop_Storage()
{

	uint32_t key = 0;
	///	Clear the flag
	mem_org_track_samples_storage_enabled = 0;
	///	Get the key
	uint32_t err_code = Mem_Org_Find_Key(mem_org_tracks_stored, &key);
	uint32_t track_start_address = Mem_Org_Get_Key_Encoded_Address(key);
	///	Store the data size
	mem_org_flash_page_header_t header;
	header.entry_number = mem_org_tracks_stored;
	header.entry_size_in_bytes = mem_org_track_size;
#ifndef EXT_FLASH_AVAILABLE
	uint16_t flash_page_size = INTERNAL_FLASH_PAGE_SIZE;
	err_code = Int_Flash_Store_Dword(*((uint32_t*)&header), (uint32_t*)track_start_address);
#else

	uint16_t flash_page_size = EXT_FLASH_PAGE_SIZE;
	Ext_Flash_Turn_On(EXT_FLASH_ERASE_OP);
	err_code = Ext_Flash_Program_Page_Through_Buffer_Without_Preerase(track_start_address, &header, sizeof(header));
	Ext_Flash_Turn_Off();
#endif
	///	Set the addres to the next page
	mem_org_next_free_data_sample_address = (mem_org_next_free_data_sample_address - (mem_org_next_free_data_sample_address % flash_page_size)) + flash_page_size;
	///	Clear the track size variable
	mem_org_track_size = 0;
	return NRF_SUCCESS;
}

/**
 * \brief This function clear entire KEY and DATA areas
 */
uint32_t Mem_Org_Clear_Tracks_Memory()
{
#ifndef EXT_FLASH_AVAILABLE
	for(uint32_t i=MEM_ORG_
	sd_nvic_DisableIRQ(UART0_IRQn);
KEY_AREA_START_ADDRESS; i<MEM_ORG_KEY_AREA_END_ADDRESS; i += INTERNAL_FLASH_PAGE_SIZE)
	{
		Int_Flash_Erase_Page(i);
	}

	for(uint32_t i=MEM_ORG_DATA_AREA_START_ADDRESS; i < MEM_ORG_DATA_AREA_END_ADDRESS; i += INTERNAL_FLASH_PAGE_SIZE)
	{
		Int_Flash_Erase_Page(i);
	}
	mem_org_tracks_stored = 0;

	sd_nvic_EnableIRQ(UART0_IRQn);
#else
	sd_nvic_DisableIRQ(UART0_IRQn);
	Ext_Flash_Turn_On(EXT_FLASH_ERASE_OP);
	Ext_Flash_Erase_Chip();
	Ext_Flash_Turn_Off();
	mem_org_tracks_stored = 0;
	sd_nvic_EnableIRQ(UART0_IRQn);
#endif
	return NRF_SUCCESS;
}

/**
 * \brief This function sends via BLE list of all available tracks with with their timestamp. It sends first the total number of available tracks. Then it sends track number and track start timestamp in one packet.
 *  Each packet corresponds to the single track entry.
 *
 *  \return 	NRF_SUCCESS - no error occurred
 *  			NRF_ERROR_NOT_FOUNT - an empty key was reached
 *  			NRF_ERROR_INTERNAL - timeout was triggered
 */
uint32_t Mem_Org_List_Tracks_Through_BLE()
{

	uint32_t temp_address = MEM_ORG_KEY_AREA_START_ADDRESS;
	uint32_t temp_key = 0;
	uint16_t temp_track_number = 0;
	uint8_t  data_buffer[5];
	uint32_t encoded_address = 0;
	RTC_Timeout(RTC_S_TO_TICKS(2));

#ifdef EXT_FLASH_AVAILABLE
	uint16_t flash_page_size = EXT_FLASH_PAGE_SIZE;
	Ext_Flash_Turn_On(EXT_FLASH_READ_OP);
#else
	uint16_t flash_page_size = INTERNAL_FLASH_PAGE_SIZE;
#endif


	///	Send the packet informing how many available tracks there is
	Ble_Uart_Data_Send(BLE_GET_AVAILABLE_TRACKS, &mem_org_tracks_stored, sizeof(mem_org_tracks_stored), false);
	Ble_Uart_Wait_For_Transmission_End();
	do
	{
		if(temp_address % flash_page_size == 0)
			temp_address += sizeof(mem_org_flash_page_header_t);
#ifndef EXT_FLASH_AVAILABLE
		memcpy(&temp_key, (uint32_t*)temp_address, sizeof(uint32_t));
#else
		Ext_Flash_Read_Page(temp_address, &temp_key, sizeof(uint32_t));
#endif

		if(temp_key != 0xFFFFFFFF)
		{
			temp_track_number = Mem_Org_Get_Key_Encoded_Track_Number(temp_key);
			if(temp_track_number <= mem_org_tracks_stored)
			{
				///	Put the track number in the buffer
				memcpy(&data_buffer[0], &temp_track_number, sizeof(temp_track_number));
				///	Get the track timestamp
				encoded_address = Mem_Org_Get_Key_Encoded_Address(temp_key);
#ifndef EXT_FLASH_AVAILABLE
				///	Put the timestamp in the buffer
				memcpy(&data_buffer[1], (uint32_t*)(encoded_address + sizeof(mem_org_flash_page_header_t)), sizeof(uint32_t));
#else
				Ext_Flash_Read_Page(encoded_address + sizeof(mem_org_flash_page_header_t), &data_buffer[1], sizeof(uint32_t));
#endif
				///	Send the track number and track timestamp
				Ble_Uart_Data_Send(BLE_GET_AVAILABLE_TRACKS, data_buffer, sizeof(data_buffer), false);
				Ble_Uart_Wait_For_Transmission_End();
				///	Increase the address
				temp_address += 4;

			}
			else
			{
				RTC_Cancel_Timeout();

			#ifdef EXT_FLASH_AVAILABLE
				Ext_Flash_Turn_Off();
			#endif
				return NRF_SUCCESS;
			}
		}
		else
		{
			RTC_Cancel_Timeout();

		#ifdef EXT_FLASH_AVAILABLE
			Ext_Flash_Turn_Off();
		#endif
			return NRF_ERROR_NOT_FOUND;
		}
	}while(!timeout_flag);
	RTC_Cancel_Timeout();

#ifdef EXT_FLASH_AVAILABLE
	Ext_Flash_Turn_Off();
#endif
	timeout_flag = 0;
	return NRF_ERROR_INTERNAL;
}


/**
 * \brief This function sends via BLE the track with the given key.
 * 			Data is send in order:  timestamp (4bytes), longitude(10bytes)  - first packet
 * 			 						latitude(10bytes) 						- second packet
 *
 * \param key - key which encodes the track
 *
 * \return  NRF_SUCCESS - everything went fine
 * 			ERROR_TIMEOUT - the timeouut error occured
 * 			NRF_ERROR_NO_MEM - end of memory has been reached
 */
uint32_t Mem_Org_Send_Track_Via_BLE(uint32_t key)
{
	uint32_t address = Mem_Org_Get_Key_Encoded_Address(key);
	uint32_t byte_counter = 0;
	uint32_t bytes_to_send_cnt = 0;
	mem_org_gps_sample_t sample = {0};

#ifdef EXT_FLASH_AVAILABLE
	Ext_Flash_Turn_On(EXT_FLASH_READ_OP);
#endif

#ifndef EXT_FLASH_AVAILABLE
	uint16_t flash_page_size = INTERNAL_FLASH_PAGE_SIZE;
	///	Get the number of data bytes in the track
	memcpy(&bytes_to_send_cnt, address + 2, sizeof(uint16_t));
#else
	uint16_t flash_page_size = EXT_FLASH_PAGE_SIZE;
	Ext_Flash_Read_Page(address + 2, &bytes_to_send_cnt, sizeof(uint16_t));
#endif


	RTC_Timeout(RTC_S_TO_TICKS(5));

	///	Send the number of bytes in the track
	Ble_Uart_Data_Send(BLE_GET_HISTORY_TRACK, &bytes_to_send_cnt, sizeof(bytes_to_send_cnt), false);
	Ble_Uart_Wait_For_Transmission_End();
	do
	{
		///	Check if we are not out of the data memory
		if(address + sizeof(mem_org_gps_sample_t) >= MEM_ORG_DATA_AREA_END_ADDRESS)
			return NRF_ERROR_NO_MEM;

		///	If we are trying to read the sample from in between flash pages
		if(address + sizeof(mem_org_gps_sample_t) >= address - (address % flash_page_size) + flash_page_size)
		{
			address = address - (address % flash_page_size) + flash_page_size;
		}

		///	If it's the flash page beginning, set the address just after the flash page header
		if(address % flash_page_size == 0)
		{
			address += sizeof(mem_org_flash_page_header_t);
		}

#ifndef EXT_FLASH_AVAILABLE
		///	Fill in the sample
		memcpy(&sample.timestamp, address, sizeof(sample));
#else
		Ext_Flash_Read_Page(address, &sample.timestamp, sizeof(sample));
#endif
		///	Send the first sample part - timestamp and longtitude
		Ble_Uart_Data_Send(BLE_GET_HISTORY_TRACK, &sample, sizeof(sample.timestamp) + sizeof(sample.longtitude), false);
		///	Wait till the packet is sent
		Ble_Uart_Wait_For_Transmission_End();
		///	Send next packet with latitude
		Ble_Uart_Data_Send(BLE_GET_HISTORY_TRACK, &sample.latitude, sizeof(sample.latitude), false);
		///	Wait till the packet is sent
		Ble_Uart_Wait_For_Transmission_End();

		///	Increase the send bytes counter
		byte_counter += sizeof(sample);
		address += sizeof(sample);

	}while((byte_counter < bytes_to_send_cnt) && !timeout_flag);

	if(timeout_flag)
	{
		timeout_flag = 0;
		RTC_Cancel_Timeout();

	#ifdef EXT_FLASH_AVAILABLE
		Ext_Flash_Turn_Off();
	#endif
		return ERROR_TIMEOUT;
	}


#ifdef EXT_FLASH_AVAILABLE
	Ext_Flash_Turn_Off();
#endif
	return NRF_SUCCESS;

}



