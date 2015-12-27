/*
 * ble_uart.h
 *
 *  Created on: 19 gru 2015
 *      Author: Konrad
 */

#ifndef BLE_BLE_UART_H_
#define BLE_BLE_UART_H_

/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 */

/** @file
 *
 * @defgroup ble_sdk_srv_uart Data Transfer Service
 * @{
 * @ingroup ble_sdk_srv
 * @brief Data Transfer Service module.
 *
 * @details This module implements the BLE UART Service.
 *
 *          If an event handler is supplied by the application, the BLE UART
 *          Service will generate BLE UART Service events to the application.
 *
 * @note The application must propagate BLE stack events to the BLE UART Service
 *       module by calling ble_uart_on_ble_evt() from the @ref softdevice_handler function.
 *
 * @note Attention!
 *  To maintain compliance with Nordic Semiconductor ASA Bluetooth profile
 *  qualification listings, this section of source code must not be modified.
 */

#include <ble.h>
#include <ble_gatts.h>
#include <ble_srv_common.h>
#include <stdbool.h>


#define BLE_UART_SERVICE_UUID 		(0x0001)
#define RX_CHAR_UUID 				(0x0002)
#define TX_CHAR_UUID				(0x0003)
#define DEV_EVENTS_UUID				(0x0004)

/*** SETTINGS ***/
#ifdef AES_ENABLED_COMMUNICATION
	#define BLE_uart_PACKET_SIZE		(16)
	#define BLE_uart_HEADER_SIZE		(sizeof(uint8_t))
	#define BLE_uart_CHECKSUM_SIZE	(sizeof(uint16_t))
#else
	#define BLE_uart_PACKET_SIZE		(20)
	#define BLE_uart_HEADER_SIZE		(sizeof(uint16_t))
	#define BLE_uart_CHECKSUM_SIZE		(0)
#endif

#define BLE_uart_PAYLOAD_SIZE		(BLE_uart_PACKET_SIZE - BLE_uart_HEADER_SIZE - BLE_uart_CHECKSUM_SIZE)
/**@brief Data Transfer Service event type. */
typedef enum
{
    BLE_UART_EVT_INDICATION_ENABLED,                                         /**< BLE UART value indication enabled event. */
    BLE_UART_EVT_INDICATION_DISABLED,                                        /**< BLE UART value indication disabled event. */
	BLE_UART_EVT_NOTIFICATION_ENABLED,
	BLE_UART_EVT_NOTIFICATION_DISABLED,
	BLE_UART_EVT_NOTIFICATION_TRANSMITTED,
    BLE_UART_EVT_INDICATION_CONFIRMED,                                       /**< Confirmation of a BLE UART indication has been received. */
	BLE_UART_EVT_RX_DATA_RECEIVED
} ble_uart_evt_type_t;

/**@brief BLE UART Service event. */
typedef struct
{
    ble_uart_evt_type_t evt_type;                                            /**< Type of event. */
} ble_uart_evt_t;

// Forward declaration of the ble_uart_t type.
typedef struct ble_uart_s ble_uart_t;

/**@brief UART BLE Service data typedef. This is a Data Transfer
 *        data packet. */

typedef uint8_t* ble_uart_data_t;

/**@brief BLE UART Service event handler type. */
typedef void (*ble_uart_evt_handler_t) (ble_uart_t * p_uart, ble_uart_evt_t * p_evt, ble_uart_data_t p_data, uint8_t data_size);


/**@brief BLE UART Service init structure. This contains all options and data
 *        needed for initialization of the service. */
typedef struct
{
    ble_uart_evt_handler_t       evt_handler;                               /**< Event handler to be called for handling events in the Data Transfer Service. */
    ble_srv_cccd_security_mode_t uart_meas_attr_md;                          /**< Initial security level for Data Transfer measurement attribute */
    ble_srv_security_mode_t      uart_feature_attr_md;                       /**< Initial security level for Data Transfer feature attribute */
    uint16_t                     feature;                                  /**< Initial value for Data Transfer feature */
} ble_uart_init_t;

/**@brief Data Transfer Service structure. This contains various status information for
 *        the service. */
struct ble_uart_s
{
    ble_uart_evt_handler_t       evt_handler;                               /**< Event handler to be called for handling events in the Data Transfer Service. */
    uint16_t                     service_handle;                            /**< Handle of Data Transfer Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t     tx_handles;                              /**< Handles related to the Data Transfer TX characteristic. */
    ble_gatts_char_handles_t     rx_handles;                           /**< Handles related to the Data Transfer RX characteristic. */
    ble_gatts_char_handles_t	 dev_events_handles;							/**< Handles of Alarms characteristics */
    uint8_t 					 uuid_type;
	uint16_t                     conn_handle;                               /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
    uint16_t                     feature;                                   /**< Value of Data Transfer feature. */
    uint32_t 					 alarm_flags;				/**< Error alarm flags */
};

typedef uint8_t ble_command_t ;

/**@brief Data Transfer data typedef. This is a Data Transfer
 *        data struct. */
typedef struct
{
	ble_command_t command_code;
	uint16_t number;
	uint8_t* data_pointer;
}ble_uart_data_to_send_t;


typedef enum
{
	BLE_TIMESTAMP_SET_CMD				 	= (uint8_t)0,
	BLE_TIMESTAMP_GET_CMD 					= (uint8_t)1,
	BLE_GET_GPS_POS_CMD 					= (uint8_t)2,
	BLE_GET_GPS_VELOCITY 					= (uint8_t)3,
	BLE_INDICATE_GPS_FIX 					= (uint8_t)4,
	BLE_GET_SATTELITES_USED					= (uint8_t)12,
	BLE_GET_HISTORY_TRACK 					= (uint8_t)5,
	BLE_GET_AVAILABLE_TRACKS 				= (uint8_t)6,
	BLE_GET_BAT_VOLT 						= (uint8_t)7,
	BLE_TRANSMISSION_TEST 					= (uint8_t)8,
	BLE_ENABLE_GPS_SAMPLES_STORAGE 			= (uint8_t)9,
	BLE_DISABLE_GPS_SAMPLES_STORAGE 		= (uint8_t)10,
	BLE_CLEAR_TRACK_MEMORY					= (uint8_t)11,

}ble_uart_commands_e;

///**@brief Buffer to hold TX values */
//extern uint8_t* ble_uart_buffer;

extern ble_uart_t							m_ble_uart;

uint32_t 	ble_uart_init(ble_uart_t * p_uart, const ble_uart_init_t * p_uart_init);

void 		ble_uart_on_ble_evt(ble_uart_t * p_uart, ble_evt_t * p_ble_evt);

uint32_t 	ble_uart_is_indication_enabled(ble_uart_t * p_uart, bool * p_indication_enabled);

uint32_t 	Ble_Uart_Data_Send(uint8_t command_code, uint8_t* data, uint16_t actual_data_size, uint8_t data_buf_dynamically_allocated);

void		Ble_Uart_Wait_For_Transmission_End();

void 		Ble_Uart_Handler(ble_uart_t * p_uart, ble_uart_evt_t * p_evt, ble_uart_data_t p_data, uint8_t data_size);

uint32_t 	Ble_Uart_Notify_Central(uint8_t command_code, uint8_t* data, uint16_t actual_data_size, uint8_t data_buf_dynamically_allocated);

uint32_t 	ble_uart_is_notification_enabled(ble_uart_t * p_uart, bool * p_indication_enabled);
/** @} */



#endif /* BLE_BLE_UART_H_ */
