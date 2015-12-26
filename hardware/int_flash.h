/*
 * int_flash.h
 *
 *  Created on: 26 gru 2015
 *      Author: Konrad
 */

#ifndef HARDWARE_INT_FLASH_H_
#define HARDWARE_INT_FLASH_H_

#include <stdint-gcc.h>

#define TRUE										(uint8_t)1
#define FALSE										(uint8_t)0

#define MAX_FLASH_OPERATION_ERROR_COUNTER			(uint8_t)3
/**	ADDRESSES				*/
#define INTERNAL_FLASH_SWAP_PAGE_ADDRESS			(uint32_t*)0x28400

#define INTERNAL_FLASH_PAGE_SIZE_UINT32				(uint16_t)256
#define INTERNAL_FLASH_PAGE_SIZE					(uint16_t)1024

/*< FLASH OPERATIONS MACROS */
/// These macros are used to define which operation was executed last
#define FLASH_ERASE_OPERATION						(uint8_t)0xF0
#define FLASH_WRITE_OPERATION						(uint8_t)0x0F
#define FLASH_OP_SUCCESS						   	(uint8_t)0xFF
#define FLASH_OP_WRITE_ERROR					   	(uint8_t)0x0f
#define FLASH_OP_ERASE_ERROR					  	(uint8_t)0xf0
#define DATA_SIZE_ERROR								(uint8_t)0x0A
#define FLASH_OPERATION_ERROR					   	(uint8_t)0xA0


typedef union
{
	uint8_t 	byte[4];
	uint32_t 	doubleword;
}dword_to_byte_u;

uint32_t 	Int_Flash_Update_Page(uint8_t* p_data, uint32_t data_len, uint32_t* address);
uint32_t 	Int_Flash_Store_Dword(uint32_t data_to_flash, uint32_t* pointer);
uint32_t  	Int_Flash_Erase_Page(uint32_t* flash_page_beginning_address);
void 		SD_flash_operation_callback(uint32_t sys_evt);
#endif /* HARDWARE_INT_FLASH_H_ */
