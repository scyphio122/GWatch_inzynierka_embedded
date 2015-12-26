/*
 * memory_organization.c
 *
 *  Created on: 26 gru 2015
 *      Author: Konrad
 */


#include "memory_organization.h"
#include "int_flash.h"
#include "ext_flash.h"
#include "stdint-gcc.h"
#include "nrf_error.h"
#include "stdlib.h"
#include "RTC.h"

static uint32_t 		mem_org_next_free_key_address = MEM_ORG_KEY_AREA_START_ADDRESS;
static uint32_t 	    mem_org_next_free_data_sample_address = MEM_ORG_DATA_AREA_START_ADDRESS;
static uint16_t			mem_org_tracks_stored = 0;
static volatile uint8_t mem_org_track_samples_storage_enabled = 0;
static uint16_t         mem_org_track_size;
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
	///	Calculate the next page start address
	uint32_t next_page_address = page_address - (page_address % INTERNAL_FLASH_PAGE_SIZE) + INTERNAL_FLASH_PAGE_SIZE;
	///	Set the address to the first free uint32_t after page header
	page_address = page_address - (page_address % INTERNAL_FLASH_PAGE_SIZE) + sizeof(mem_org_flash_page_header_t);

	do
	{
		memcpy(&temp_key, page_address, sizeof(uint32_t));

		if(temp_key == 0xFFFFFFFF)
		{
			*key_address = page_address;
			return NRF_SUCCESS;
		}
		else
		{
			page_address += 4;
		}
	}while(page_address < next_page_address);

	*key_address = NULL;
	return NRF_ERROR_NOT_FOUND;
}

/**
 * \brief This function searches entire KEY_AREA in order to find the first free cell
 *
 * \return	NRF_SUCCESS - the key was found
 * 			NRF_ERROR_NOT_FOUND - the key wasn't found
 */
uint32_t Mem_Org_Key_Find_First_Free()
{
	uint32_t ret_val = 0;
	for(uint32_t i = MEM_ORG_KEY_AREA_START_ADDRESS; i< MEM_ORG_KEY_AREA_END_ADDRESS; i += 1024)
	{
		ret_val = Mem_Org_Key_Find_First_Free_On_Page(i, &mem_org_next_free_key_address);
		if(ret_val == NRF_SUCCESS)
			return NRF_SUCCESS;
	}

	return NRF_ERROR_NOT_FOUND;
}

uint32_t Mem_Org_Track_Address_Find_First_Free()
{

	return NRF_SUCCESS;
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
		///	Store the key
		err_code = Int_Flash_Store_Dword(key, mem_org_next_free_key_address);

		if(err_code == FLASH_OP_SUCCESS)
		{
			///	Clear the MSB
			key = key & 0x7FFFFFFF;
			///	Acknowledge the key
			err_code = Int_Flash_Store_Dword(key, mem_org_next_free_key_address);
			if(err_code == FLASH_OP_SUCCESS)
			{
				///	Increase the pointer to the next free address
				mem_org_next_free_key_address += 4;
				///	If we get to the next page start then set the pointer to the first address after the header
				if(mem_org_next_free_key_address % INTERNAL_FLASH_PAGE_SIZE == 0)
					mem_org_next_free_key_address += sizeof(mem_org_flash_page_header_t);
				return NRF_SUCCESS;
			}
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
	uint8_t page_number = track_number / MEM_ORG_KEY_AREA_KEYS_ON_PAGE;
	uint8_t key_on_page = track_number % MEM_ORG_KEY_AREA_KEYS_ON_PAGE;

	uint8_t key = 0;
	uint16_t key_index = 0;
	RTC_Timeout(RTC_S_TO_TICKS(2));
	do
	{
		///	Get the key
		memcpy(&key, MEM_ORG_KEY_AREA_START_ADDRESS + page_number*INTERNAL_FLASH_PAGE_SIZE + sizeof(mem_org_flash_page_header_t) + key_on_page*sizeof(uint32_t), sizeof(uint32_t));
		key_index = ((key >> MEM_ORG_KEY_TRACK_NUMBER_SHIFT) & 0x7FFF);
		///	If it is the requested key
		if(key_index == track_number)
		{
			///	If the key is valid
			if((key & 0x80000000) == 0)
			{
				*key_buf = key;
				RTC_Cancel_Timeout();
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

	return NRF_ERROR_INTERNAL;
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
	return (key << MEM_ORG_KEY_ADD_SHIFT) + MEM_ORG_DATA_AREA_START_ADDRESS;
}

uint32_t Mem_Org_Store_Sample(uint32_t timestamp)
{
	mem_org_gps_sample_t sample;
	uint8_t err_code = 0;

	sample.timestamp = timestamp;

	///	Copy the latitude
	memcpy(sample.latitude, &gga_message.latitude, 10);
	///	Copy the longtitude
	memcpy(sample.longtitude, &gga_message.longtitude, 10);

	if(mem_org_next_free_data_sample_address + sizeof(sample) >= (mem_org_next_free_data_sample_address - (mem_org_next_free_data_sample_address % INTERNAL_FLASH_PAGE_SIZE) + INTERNAL_FLASH_PAGE_SIZE))
			mem_org_next_free_data_sample_address = (mem_org_next_free_data_sample_address - (mem_org_next_free_data_sample_address % INTERNAL_FLASH_PAGE_SIZE) + INTERNAL_FLASH_PAGE_SIZE) + sizeof(sample);
	if(mem_org_next_free_data_sample_address + sizeof(sample) >= MEM_ORG_DATA_AREA_END_ADDRESS)
	{
		Mem_Org_Track_Stop_Storage();
		return NRF_ERROR_NO_MEM;
	}

	for(uint8_t i=0; i < sizeof(mem_org_gps_sample_t)/4; i+=1)
	{
		if(mem_org_next_free_data_sample_address % INTERNAL_FLASH_PAGE_SIZE == 0)
			mem_org_next_free_data_sample_address += sizeof(mem_org_flash_page_header_t);
		err_code = Int_Flash_Store_Dword(*((uint32_t*)&sample + i), mem_org_next_free_data_sample_address);
		mem_org_next_free_data_sample_address +=4;
	}

	mem_org_track_size += sizeof(sample);


	return NRF_SUCCESS;
}

uint32_t Mem_Org_Track_Start_Storage()
{
	///	Store the key
	uint32_t err_code = Mem_Org_Store_Key(mem_org_next_free_data_sample_address, mem_org_tracks_stored);

	mem_org_track_samples_storage_enabled = 1;

	return NRF_SUCCESS;
}

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
	err_code = Int_Flash_Store_Dword(*((uint32_t*)&header), track_start_address);

	///	Set the addres to the next page
	mem_org_next_free_data_sample_address = (mem_org_next_free_data_sample_address - (mem_org_next_free_data_sample_address % INTERNAL_FLASH_PAGE_SIZE)) + INTERNAL_FLASH_PAGE_SIZE;
	///	Increase the stored track counter
	mem_org_tracks_stored++;

	return NRF_SUCCESS;
}


