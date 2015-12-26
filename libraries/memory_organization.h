/*
 * memory_organization.h
 *
 *  Created on: 26 gru 2015
 *      Author: Konrad
 */

#ifndef LIBRARIES_MEMORY_ORGANIZATION_H_
#define LIBRARIES_MEMORY_ORGANIZATION_H_

#include "stdint-gcc.h"
#include "int_flash.h"

typedef struct
{
	uint16_t entry_number;
	uint16_t entry_size_in_bytes;
	//uint32_t entry_timestamp;
}mem_org_flash_page_header_t;

typedef struct
{
	uint32_t timestamp;
	uint8_t  longtitude[10];
	uint8_t  latitude[10];
}mem_org_gps_sample_t;

#define	MEM_ORG_KEY_AREA_START_ADDRESS				(uint32_t)0x28800
#define MEM_ORG_KEY_AREA_END_ADDRESS				(uint32_t)0x29000

#define MEM_ORG_KEY_AREA_KEYS_ON_PAGE				(uint8_t)(INTERNAL_FLASH_PAGE_SIZE - sizeof(mem_org_flash_page_header_t))/4

#define MEM_ORG_DATA_AREA_START_ADDRESS				MEM_ORG_KEY_AREA_END_ADDRESS
#define MEM_ORG_DATA_AREA_END_ADDRESS				(uint32_t)0x3F800

#define MEM_ORG_KEY_ADD_SHIFT						(uint8_t)5
#define MEM_ORG_KEY_TRACK_NUMBER_SHIFT				(uint8_t)16



#endif /* LIBRARIES_MEMORY_ORGANIZATION_H_ */
