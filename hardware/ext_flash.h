/*
 * ext_flash.h
 *
 *  Created on: 24 gru 2015
 *      Author: Konrad
 */

#ifndef HARDWARE_EXT_FLASH_H_
#define HARDWARE_EXT_FLASH_H_


#define EXT_FLASH_PAGE_SIZE										(uint16_t)256

#define EXT_FLASH_TURN_ON_DELAY_READ_US							(uint8_t)100
#define EXT_FLASH_TURN_ON_DELAY_PROGRAM_ERASE_US				(uint16_t)3100


/**			EXTERNAL FLASH COMMANDS			**/

/*	STATUS REG COMMANDS */
#define EXT_FLASH_STATUS_REG_READ								(uint8_t)0xD7
#define EXT_FLASH_STATUS_REG_WRITE								(uint8_t)

/*	PROGRAM COMMANDS */
#define EXT_FLASH_WRITE_BUFFER_1								(uint8_t)0x84
#define EXT_FLASH_WRITE_BUFFER_2								(uint8_t)0x87

#define EXT_FLASH_PAGE_PROG_FROM_BUF_W_PREERASE_BUF_1			(uint8_t)0x83
#define EXT_FLASH_PAGE_PROG_FROM_BUF_W_PREERASE_BUF_2			(uint8_t)0x86

#define EXT_FLASH_PAGE_PROG_FROM_BUF_WITHOUT_PREERASE_BUF_1		(uint8_t)0x88
#define EXT_FLASH_PAGE_PROG_FROM_BUF_WITHOUT_PREERASE_BUF_2		(uint8_t)0x89


typedef enum
{
	EXT_FLASH_READ_OP = 0,
	EXT_FLASH_PROGRAM_OP,
	EXT_FLASH_ERASE_OP
}ext_flash_operation_type_e;

typedef enum
{
	EXT_FLASH_BUFFER_1 = 0,
	EXT_FLASH_BUFFER_2
}ext_flash_buffer_number_e;

#pragma GCC push
#pragma GCC optimize("O0")
typedef struct
{
	uint8_t page_size_config 		:1;		/**< 0 - 264 bytes; 1 - 256 bytes */
	uint8_t	sector_protect_stat		:1;		/**< 0 - sector protection disabled; 1 - sector protection enabled */
	uint8_t	memory_size				:4;		/**< 1111 - 64 MBit */
	uint8_t compare_result			:1;		/**< 0 - main memory page matches buffer data; 1 - main memory page is differen than buffer data */
	uint8_t ready_or_busy			:1;		/**< 0 - device busy; 1 - device ready */

	uint8_t erase_suspend			:1;		/**< 0 - erase not suspended; 1 - erase op suspended */
	uint8_t program_suspend_buf_1	:1;     /**< 0 - the sector is not program suspended from buf 1; 1 - the sector is program suspended from buf 1 */
	uint8_t program_suspend_buf_2	:1;		/**< 0 - the sector is not program suspended from buf 2; 1 - the sector is program suspended from buf 2 */
	uint8_t sector_lockdown_enabled :1; 	/**< 0 - sector lockdown disabled; 1 - sector lockdown enabled */
	uint8_t xxx_1					:1;		/**< not used */
	uint8_t erase_or_program_error	:1;		/**< 0 - the program or erase operation was successful; 1 - en error during program or erase op occured */
	uint8_t xxx_2					:1;		/**< not used */
	uint8_t ready_or_busy_2			:1;		/**< 0 - device busy; 1 - device ready */

}ext_flash_status_reg_t;


#pragma GCC pop

#endif /* HARDWARE_EXT_FLASH_H_ */
