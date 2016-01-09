/*
 * ble.h
 *
 *  Created on: 18 gru 2015
 *      Author: Konrad
 */

#ifndef BLE_BLE_GWATCH_H_
#define BLE_BLE_GWATCH_H_

#include "app_util.h"

#define ADVERTISING_INTERVAL			1600										/**< Advertising interval in units of 0.625 ms */
#define ADVERTISING_TIMEOUT				0											/**< Advertising timeout. Zero for infinite */


#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(500, UNIT_1_25_MS)            /**< Minimum acceptable connection interval (0.5 seconds). */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(1000, UNIT_1_25_MS)           /**< Maximum acceptable connection interval (1 second). */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(3000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds). */ //4000
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER)  /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER) 	/**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                          	/**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_TIMEOUT               30                                          /**< Timeout for Pairing Request or Security Request (in seconds). */
#define SEC_PARAM_BOND                  1                                           /**< Perform bonding. */
#define SEC_PARAM_MITM                  0                                           /**< Man In The Middle protection not required. */
#define SEC_PARAM_IO_CAPABILITIES       BLE_GAP_IO_CAPS_NONE                        /**< No I/O capabilities. */
#define SEC_PARAM_OOB                   0                                           /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE          7                                           /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE          16                                          /**< Maximum encryption key size. */

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */


#define ADVERTISING_DEVICE_NAME			"GWatch"

#define GWATCH_UUID_BASE				{(uint8_t)0xb0, (uint8_t)0x07, (uint8_t)0x20, (uint8_t)0xac, (uint8_t)0xca, (uint8_t)0x16, (uint8_t)0x20, (uint8_t)0x3c, (uint8_t)0xb9, (uint8_t)0xe7, (uint8_t)0x14, (uint8_t)0x72, (uint8_t)0x00, (uint8_t)0x00, (uint8_t)0x04, (uint8_t)0xac}

extern uint16_t                         m_conn_handle;

void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name);
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name);

/**@brief Function to initialize all BLE stuff */
void BLE_Init();
void Advertising_Init(void);
void Advertising_Start();


#endif /* BLE_BLE_GWATCH_H_ */
