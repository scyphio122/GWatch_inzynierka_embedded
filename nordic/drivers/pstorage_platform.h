/* Copyright (c)  2013 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

 /** @cond To make doxygen skip this file */

/** @file
 *  This header contains defines with respect persistent storage that are specific to
 *  persistent storage implementation and application use case.
 */
#ifndef PSTORAGE_PL_H__
#define PSTORAGE_PL_H__

#include <stdint.h>
#include "nrf.h"

static __INLINE uint16_t pstorage_FLASH_PAGE_SIZE_BYTES()
{
  return (uint16_t)NRF_FICR->CODEPAGESIZE;
}

#define PSTORAGE_FLASH_PAGE_SIZE_BYTES     pstorage_FLASH_PAGE_SIZE_BYTES()          /**< Size of one flash page. */
#define PSTORAGE_FLASH_EMPTY_MASK    0xFFFFFFFF                          /**< Bit mask that defines an empty address in flash. */

static __INLINE uint32_t pstorage_flash_page_end()
{
   uint32_t bootloader_addr = NRF_UICR->BOOTLOADERADDR;
  
   return ((bootloader_addr != PSTORAGE_FLASH_EMPTY_MASK) ?
           (bootloader_addr/ PSTORAGE_FLASH_PAGE_SIZE_BYTES) : NRF_FICR->CODESIZE);
}

#define PSTORAGE_FLASH_PAGE_END     pstorage_flash_page_end()

#define PSTORAGE_MAX_APPLICATIONS   10                                                      /**< Maximum number of applications that can be registered with the module, configurable based on system requirements. */
#define PSTORAGE_MIN_BLOCK_SIZE     0x0004                                                      /**< Minimum size of block that can be registered with the module. Should be configured based on system requirements, recommendation is not have this value to be at least size of word. */



#define PSTORAGE_DATA_START_ADDR   0x28000// 0x1EC00//((PSTORAGE_FLASH_PAGE_END - PSTORAGE_MAX_APPLICATIONS - 1) \
                                    * PSTORAGE_FLASH_PAGE_SIZE_BYTES)                                 /**< Start address for persistent data, configurable according to system requirements. */
#define PSTORAGE_DATA_END_ADDR      0x28800//0x1F400//((PSTORAGE_FLASH_PAGE_END - 1) * PSTORAGE_FLASH_PAGE_SIZE_BYTES)  /**< End address for persistent data, configurable according to system requirements. */


#define PSTORAGE_SWAP_ADDR          PSTORAGE_DATA_END_ADDR                                      /**< Top-most page is used as swap area for clear and update. */

#define PSTORAGE_MAX_BLOCK_SIZE     PSTORAGE_FLASH_PAGE_SIZE_BYTES                                    /**< Maximum size of block that can be registered with the module. Should be configured based on system requirements. And should be greater than or equal to the minimum size. */
#define PSTORAGE_CMD_QUEUE_SIZE     10                                                          /**< Maximum number of flash access commands that can be maintained by the module for all applications. Configurable. */


/** Abstracts persistently memory block identifier. */
typedef uint32_t pstorage_block_t;

typedef struct
{
    uint32_t            module_id;      /**< Module ID.*/
    pstorage_block_t    block_id;       /**< Block ID.*/
} pstorage_handle_t;

typedef uint16_t pstorage_size_t;      /** Size of length and offset fields. */

/**@brief Handles Flash Access Result Events. To be called in the system event dispatcher of the application. */
void pstorage_sys_event_handler (uint32_t sys_evt);

#endif // PSTORAGE_PL_H__

/** @} */
/** @endcond */
