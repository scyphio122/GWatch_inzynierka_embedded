/*
 * ble_uart.c
 *
 *  Created on: 19 gru 2015
 *      Author: Konrad
 */


/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
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

/* Attention!
*  To maintain compliance with Nordic Semiconductor ASAï¿½s Bluetooth profile
*  qualification listings, this section of source code must not be modified.
*/


#include <ble_gap.h>
#include <ble_gatt.h>
#include <ble_gwatch.h>
#include <ble_types.h>
#include <ble_uart.h>
#include "nrf51.h"
#include "core_cm0.h"
#include "core_cmInstr.h"
#include <nordic_common.h>
#include <nrf_error.h>
#include <RTC.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_stdint.h>

/**@brief Buffer to hold data to send via TX. */
static volatile bool		ble_tx_packet_in_progress = false;		/**< Mutex on single packet HVX transmission */
volatile uint8_t 			ble_tx_in_progress;						/**< Mutex on entire message HVX transmission */
static uint8_t				ble_uart_tx_buffer[20];					/**< Buffer for the single packet message which is sent from the device to the central */
static uint8_t 				ble_uart_rx_buffer[20];					/**< Buffer for incomming from central data */
static volatile uint16_t	ble_uart_tx_data_size;					/**< Size of data which are to be send */
static uint8_t*				ble_data_ptr;							/**< Pointer where the data will be stored. It will be dynamically allocated buffer */
ble_uart_t					m_ble_uart;

/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_uart       Blood Pressure Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_uart_t * p_uart, ble_evt_t * p_ble_evt)
{
    p_uart->conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
}


/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_uart       Blood Pressure Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_uart_t * p_uart, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_uart->conn_handle = BLE_CONN_HANDLE_INVALID;
}


/**@brief Function for handling the CCCD write events to TX characteristic.
 *
 * @param[in]   p_uart         Data Transfer Service structure.
 * @param[in]   p_evt_write   Write event received from the BLE stack.
 */
static void on_tx_cccd_write(ble_uart_t * p_uart, ble_gatts_evt_write_t * p_evt_write)
{
    if (p_evt_write->len == 2)
    {
        // CCCD written, update indication state
        if (p_uart->evt_handler != NULL)
        {
            ble_uart_evt_t evt;

            if (ble_srv_is_indication_enabled(p_evt_write->data))
            {
                evt.evt_type = BLE_UART_EVT_INDICATION_ENABLED;
            }
            else
            {
                evt.evt_type = BLE_UART_EVT_INDICATION_DISABLED;
            }

            p_uart->evt_handler(p_uart, &evt, p_evt_write->data, 0);
        }
    }
}
/**@brief Function for handling the CCCD write events to Device Events characteristic.
 *
 * @param[in]   p_uart         Data Transfer Service structure.
 * @param[in]   p_evt_write   Write event received from the BLE stack.
 */
static void on_dev_events_cccd_write(ble_uart_t * p_uart, ble_gatts_evt_write_t * p_evt_write)
{
    if (p_evt_write->len == 2)
    {
        // CCCD written, update indication state
        if (p_uart->evt_handler != NULL)
        {
            ble_uart_evt_t evt;

            if (ble_srv_is_notification_enabled(p_evt_write->data))
            {
                evt.evt_type = BLE_UART_EVT_NOTIFICATION_ENABLED;
            }
            else
            {
                evt.evt_type = BLE_UART_EVT_NOTIFICATION_DISABLED;
            }

            p_uart->evt_handler(p_uart, &evt, p_evt_write->data, 0);
        }
    }
}

/**@brief Function for handling the value write events to RX characteristic.
 *
 * @param[in]   p_uart         Data Transfer Service structure.
 * @param[in]   p_evt_write   Write event received from the BLE stack.
 */
static void on_rx_write(ble_uart_t * p_uart, ble_gatts_evt_write_t * p_evt_write)
{
    if (p_evt_write->len)
    {
        // CCCD written, update indication state
        if (p_uart->evt_handler != NULL)
        {
            ble_uart_evt_t evt;
						evt.evt_type = BLE_UART_EVT_RX_DATA_RECEIVED;
            p_uart->evt_handler(p_uart, &evt, p_evt_write->data, p_evt_write->len);
        }
    }
}


/**@brief Function for handling the Write event
 *
 * @param[in]   p_uart       Blood Pressure Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_write(ble_uart_t * p_uart, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if (p_evt_write->handle == p_uart->tx_handles.cccd_handle) //Handle CCCD write to TX characteristic
    {
        on_tx_cccd_write(p_uart, p_evt_write);
    }
    if (p_evt_write->handle == p_uart->dev_events_handles.cccd_handle) //Handle CCCD write to Device Events characteristic
    {
    	on_dev_events_cccd_write(p_uart, p_evt_write);
    }
	if (p_evt_write->handle == p_uart->rx_handles.value_handle) //Handle VALUE write to RX characteristic
	{
			on_rx_write(p_uart, p_evt_write);
	}
}


/**@brief Function for handling the HVC event.
 *
 * @details Handles HVC events from the BLE stack.
 *
 * @param[in]   p_uart       Blood Pressure Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_hvc(ble_uart_t * p_uart, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_hvc_t * p_hvc = &p_ble_evt->evt.gatts_evt.params.hvc;

    if (p_hvc->handle == p_uart->tx_handles.value_handle)
    {
        ble_uart_evt_t evt;

        evt.evt_type = BLE_UART_EVT_INDICATION_CONFIRMED;
        p_uart->evt_handler(p_uart, &evt, 0, 0);
    }
}

static void on_notify_completed(ble_uart_t * p_uart, ble_evt_t * p_ble_evt)
{
	//ble_gatts_evt_hvc_t * p_hvc = &p_ble_evt->evt.gatts_evt.params.hvc;

    //if (p_hvc->handle == p_uart->dev_events_handles.value_handle)
   // {
        ble_uart_evt_t evt;

        evt.evt_type = BLE_UART_EVT_NOTIFICATION_TRANSMITTED;
        p_uart->evt_handler(p_uart, &evt, 0, 0);
    //}
}
/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the BLE Uart Service.
 *
 * @param[in]   p_uart      Data Transfer Service structure.
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 */
void ble_uart_on_ble_evt(ble_uart_t * p_uart, ble_evt_t * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_uart, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_uart, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_uart, p_ble_evt);
            break;

        case BLE_GATTS_EVT_HVC:			/**< Event for indication ACK from central**/
            on_hvc(p_uart, p_ble_evt);
            break;
        case BLE_EVT_TX_COMPLETE:
        {
        	on_notify_completed(p_uart, p_ble_evt);
        	break;
        }
        default:
            // No implementation needed.
            break;
    }
}



/**
 * \brief This function blocks program execution until the single BLE packet is transmitted
 */
static void Ble_Uart_Wait_Till_Packet_Transmission_In_Progress()
{
	while(ble_tx_packet_in_progress)
	{
		__WFE();
	}

	return;
}

/***
 * \brief This function blocks program execution until entire message (even more than single packet) is transmitted
 */
void Ble_Uart_Wait_For_Transmission_End()
{
	while(ble_tx_in_progress)
	{
		__WFE();
	}
}


/**
 * This function is called when the write event from central is generated. Depending on the central's request code the appropriate action is triggered
 *
 * \param p_data - pointer to the data sent from central to the device
 * \param data_size - size of data sent from central to the device
 */
static uint32_t Ble_Uart_Rx_Handler(uint8_t* p_data, uint8_t data_size)
{
	uint8_t request_code = p_data[0];

	switch(request_code)
	{
		case BLE_TIMESTAMP_SET_CMD:
		{
			uint32_t* timestamp_to_set = (uint32_t*)&p_data[1];
			RTC_Set_Timestamp(*timestamp_to_set);
			break;
		}
		case BLE_TIMESTAMP_GET_CMD:
		{
			break;
		}
		case BLE_GET_GPS_POS_CMD:
		{
			break;
		}
		case BLE_GET_GPS_VELOCITY:
		{
			break;
		}
		case BLE_GET_HISTORY_TRACK:
		{
			break;
		}
		case BLE_GET_AVAILABLE_TRACKS:
		{
			break;
		}
		case BLE_GET_BAT_VOLT:
		{
			break;
		}
		case BLE_TRANSMISSION_TEST:
		{
			const uint8_t text[] = "Natenczas Wojski chwyci³ na taœmie przypiêty Swoj rog bawoli, dlugi, cetkowany, krety\n\rJak waz boa, oburacz do ust go przycisnal,\n\rWzdal policzki jak banie, w oczach krwia zablysnal,\n\rZasunal wpol powieki, wci¹gnal w glab pol brzucha\n\rI do pluc wyslal z niego caly zapas ducha,\n\rI zagral: rog jak wicher, wirowatym dechem\n\rNiesie w puszcze muzyke i podwaja echem.\n\rUmilkli strzelcy, stali szczwacze zadziwieni\n\rMoca, czystoscia, dziwna harmonija pieni.\n\rStarzec caly kunszt, ktorym niegdys w lasach slynal,\n\rJeszcze raz przed uszami mysliwcow rozwinal;\n\rNapelnil wnet, ozywil knieje i dabrowy,\n\rJakby psiarniê w nie wpuscil i rozpocz¹l lowy.\n\rBo w graniu byla lowow historyja krotka:\n\rrazu odzew dzwiêcz¹cy, rzeski: to pobudka;\n\rPotem jeki po jekach skomla: to psow granie;\n\rA gdzieniegdzie ton twardszy jak grzmot: to strzelanie...\n\r";
			uint8_t* test_data = malloc(sizeof(text));
			memcpy(test_data, text, sizeof(text));
			//Ble_Uart_Data_Send(8, test_data, sizeof(text));
			Ble_Uart_Notify_Central(8, test_data, sizeof(text));
			//Ble_Uart_Wait_Till_Transmission_In_Progress();
			//RTC_Wait(1);
		}
	}
}



/**
 *  \brief This function sends single packet (up to 20 bytes) of data with BLE
 *
 *  \param p_uart - pointer to the uart service structure, which will be used to send data
 *  \param data - pointer to the data buffer
 *  \param data_size - size of data in the packet
 */
static uint32_t Ble_Uart_Send_Single_Packet(ble_uart_t* p_uart, uint8_t* data, uint8_t actual_data_size)
{
	if(p_uart->conn_handle != BLE_CONN_HANDLE_INVALID)
	    {
        	uint16_t               	hvx_len;
	        ble_gatts_hvx_params_t 	hvx_params;
			ble_gatts_value_t	   	value_params;
			uint32_t 				err_code = 0;

	        memset(&hvx_params, 0, sizeof(hvx_params));
			memset(&value_params,0,sizeof(value_params));

			/// Copy the message to the buffer
			memcpy(ble_uart_tx_buffer+1, data, actual_data_size);

			//Fill structure with data size. This will avoid sending empty bytes when sending <20 bytes
			value_params.len = actual_data_size + 1;
			value_params.offset = 0;
			value_params.p_value = NULL;

			err_code = sd_ble_gatts_value_set(p_uart->conn_handle, p_uart->tx_handles.value_handle, &value_params);

			hvx_len = actual_data_size + 1;
	        hvx_params.handle = p_uart->tx_handles.value_handle;
	        hvx_params.type   = BLE_GATT_HVX_INDICATION;
	        hvx_params.offset = 0;
	        hvx_params.p_len  = &hvx_len;
	        hvx_params.p_data = ble_uart_tx_buffer;

	        Ble_Uart_Wait_Till_Packet_Transmission_In_Progress();
	    	///	Set the ble transmission flag high to indicate ongoing transmission
	    	ble_tx_packet_in_progress = true;
	    	///	Send the data
	        err_code = sd_ble_gatts_hvx(p_uart->conn_handle, &hvx_params);

	        ble_uart_tx_data_size -= actual_data_size;

	        return NRF_SUCCESS;
	    }

	return NRF_ERROR_INVALID_STATE;
}


/**
 * \brief This function is the API for the data transmission over BLE. It should be called every time user wants to send something through BLE.
 *  		If the data is larger than 19 bytes it cuts them into packets and sends one at a time.
 *
 *  \param p_uart - pointer to the BLE UART service structure
 *  \param command_code - the 1 byte of command code which will recognize the type of data which are being sent. It is always the first byte of each packet
 *  \param data - pointer to the buffer with data which are to be sent. It should exist until the transmission ends. IT MUST BE DYNAMICALLY ALLOCATED
 *  \param data_size - size of data which are to be sent
 */
uint32_t Ble_Uart_Data_Send(uint8_t command_code, uint8_t* data, uint16_t data_size)
{
	///	Set the flag to indicate that message is going to be sent
	ble_tx_in_progress = 1;
	///	Set the size of data which are to be sent
	ble_uart_tx_data_size = data_size;
	///	Set the pointer to the data
	ble_data_ptr = data;
	///	Set the command code in the buffer
	ble_uart_tx_buffer[0] = command_code;

	///	If there is more than one message to send
	if(data_size > 19)
		Ble_Uart_Send_Single_Packet(&m_ble_uart, data, 19);	///	Send the first packet (19 bytes, because the first one is command code)
	else
		Ble_Uart_Send_Single_Packet(&m_ble_uart, data, data_size);	///	If there is only 1 message to send

	return NRF_SUCCESS;
}

/**
 * \brief This function should be called after receiving the acknowledgement from the central on the receiving the first packet of data.
 * 			If there is more data, this function sends another packet
 *
 * 			\param pointer to the uart service structure
 *
 * 			\return NRF_SUCCESS - if the packet was successfully send
 * 					NRF_ERROR_INVALID_STATE - if the device is not in the BLE connection
 */
static uint32_t Ble_Uart_Send_Next_Packet(ble_uart_t* p_uart)
{
	uint32_t err_code = 0;
	if(ble_uart_tx_data_size > 19)
		err_code = Ble_Uart_Send_Single_Packet(p_uart, &ble_data_ptr[ble_uart_tx_data_size], 19);
	else
		err_code = Ble_Uart_Send_Single_Packet(p_uart, &ble_data_ptr[ble_uart_tx_data_size], ble_uart_tx_data_size);

	return err_code;
}

static uint32_t Ble_Uart_Notification_Single_Packet_Send(ble_uart_t* p_uart, uint8_t* data, uint8_t data_size)
{
	if(p_uart->conn_handle != BLE_CONN_HANDLE_INVALID)
	    {
        	uint16_t               	hvx_len;
	        ble_gatts_hvx_params_t 	hvx_params;
			ble_gatts_value_t	   	value_params;
			uint32_t 				err_code = 0;

	        memset(&hvx_params, 0, sizeof(hvx_params));
			memset(&value_params,0,sizeof(value_params));

			/// Copy the message to the buffer
			memcpy(ble_uart_tx_buffer+1, data, data_size);

			//Fill structure with data size. This will avoid sending empty bytes when sending <20 bytes
			value_params.len = data_size + 1;
			value_params.offset = 0;
			value_params.p_value = NULL;

			err_code = sd_ble_gatts_value_set(p_uart->conn_handle, p_uart->dev_events_handles.value_handle, &value_params);

			hvx_len = data_size + 1;
	        hvx_params.handle = p_uart->dev_events_handles.value_handle;
	        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
	        hvx_params.offset = 0;
	        hvx_params.p_len  = &hvx_len;
	        hvx_params.p_data = ble_uart_tx_buffer;

	        Ble_Uart_Wait_Till_Packet_Transmission_In_Progress();
	    	///	Set the ble transmission flag high to indicate ongoing transmission
	    	ble_tx_packet_in_progress = true;
	    	///	Send the data
	        err_code = sd_ble_gatts_hvx(p_uart->conn_handle, &hvx_params);


	        ble_uart_tx_data_size -= data_size;

	        return NRF_SUCCESS;
	    }

	return NRF_ERROR_INVALID_STATE;
}

static uint32_t Ble_Uart_Notify_Send_Next_Packet(ble_uart_t* p_uart)
{
	uint32_t err_code = 0;
	if(ble_uart_tx_data_size > 19)
		err_code = Ble_Uart_Notification_Single_Packet_Send(p_uart, &ble_data_ptr[ble_uart_tx_data_size], 19);
	else
		err_code = Ble_Uart_Notification_Single_Packet_Send(p_uart, &ble_data_ptr[ble_uart_tx_data_size], ble_uart_tx_data_size);

	return err_code;
}

uint32_t Ble_Uart_Notify_Central(uint8_t command_code, uint8_t* data, uint16_t actual_data_size)
{
	///	Set the flag to indicate that message is going to be sent
	ble_tx_in_progress = 1;
	///	Set the size of data which are to be sent
	ble_uart_tx_data_size = actual_data_size;
	///	Set the pointer to the data
	ble_data_ptr = data;
	///	Set the command code in the buffer
	ble_uart_tx_buffer[0] = command_code;

	///	If there is more than one message to send
	if(actual_data_size > 19)
		Ble_Uart_Notification_Single_Packet_Send(&m_ble_uart, data, 19);	///	Send the first packet (19 bytes, because the first one is command code)
	else
		Ble_Uart_Notification_Single_Packet_Send(&m_ble_uart, data, actual_data_size);	///	If there is only 1 message to send

	return NRF_SUCCESS;
}

/**
 * \brief This is the Handler assigned to the BLE Uart Service. It should be attached in services_init function.
 */
void Ble_Uart_Handler(ble_uart_t * p_uart, ble_uart_evt_t * p_evt, ble_uart_data_t p_data, uint8_t data_size)
{
    switch (p_evt->evt_type)
    {
        case BLE_UART_EVT_INDICATION_ENABLED:
            break;

        case BLE_UART_EVT_INDICATION_CONFIRMED:
        {
     	   ///	Clear the flag to indicate that transmission has ended
     	   ble_tx_packet_in_progress = false;
        	///	If there is more data to be send, send next packet
           if(ble_uart_tx_data_size > 0)
        	   Ble_Uart_Send_Next_Packet(p_uart);
           else
           {
        	   ///	Free the data resources
        	   free(ble_data_ptr);
        	   ble_tx_in_progress = false;
           }
            break;
        }
        case BLE_UART_EVT_NOTIFICATION_ENABLED:
        	break;
        case BLE_UART_EVT_NOTIFICATION_TRANSMITTED:
        {
      	   ///	Clear the flag to indicate that transmission has ended
      	   ble_tx_packet_in_progress = false;
         	///	If there is more data to be send, send next packet
            if(ble_uart_tx_data_size > 0)
         	   Ble_Uart_Notify_Send_Next_Packet(p_uart);
            else
            {
         	   ///	Free the data resources
         	   free(ble_data_ptr);
         	   ble_tx_in_progress = false;
            }
        	break;
        }
		case BLE_UART_EVT_RX_DATA_RECEIVED:
			Ble_Uart_Rx_Handler(p_data, data_size);
			break;
        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for adding Tx characteristics.
 *
 * @param[in]   p_uart        Ble UART Service structure.
 * @param[in]   p_uart_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t uart_tx_char_add(ble_uart_t * p_uart, const ble_uart_init_t * p_uart_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    // Clear char_md structure
    memset(&char_md, 0, sizeof(char_md));

		//	Set indicate avaliability
    char_md.char_props.indicate  = 1;

		//	Setup descriptor
#ifdef LIGHT_CHARACTERISTICS
	char_md.p_char_user_desc = NULL;
#else
	// Human-friendly descriptor to debug
	char_md.p_char_user_desc = (uint8_t*)"Tx";
	char_md.char_user_desc_size = 2;
	char_md.char_user_desc_max_size = 20;
#endif
	char_md.p_char_pf        = NULL;
    char_md.p_user_desc_md   = NULL;
    char_md.p_cccd_md        = NULL;
    char_md.p_sccd_md        = NULL;

	//	Inform the characteristic that it will have a custom vendor UUID
	ble_uuid.type = p_uart->uuid_type;
	//	Set the 16-bit UUID for the characteristic
	ble_uuid.uuid = TX_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc       = BLE_GATTS_VLOC_USER;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(ble_uart_tx_buffer);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = sizeof(ble_uart_tx_buffer);
    attr_char_value.p_value      = ble_uart_tx_buffer;


    return sd_ble_gatts_characteristic_add(p_uart->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_uart->tx_handles);
}


/**@brief Function for adding Rx characteristics.
 *
 * @param[in]   p_uart        Ble UART Service structure.
 * @param[in]   p_uart_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t uart_rx_char_add(ble_uart_t * p_uart, const ble_uart_init_t * p_uart_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

		// clear the fields of char_md structure
    memset(&char_md, 0, sizeof(char_md));

	//	Set the read availability
	char_md.char_props.write 	= 1;


	char_md.p_char_user_desc  = NULL;
	char_md.char_user_desc_max_size = 20;
	char_md.char_user_desc_size = 0;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL;
    char_md.p_sccd_md         = NULL;

	//	Inform the characteristic that it will have a custom vendor UUID
	ble_uuid.type = p_uart->uuid_type;
	//	Set the 16-bit UUID for the characteristic
	ble_uuid.uuid = RX_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc       = BLE_GATTS_VLOC_USER;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(ble_uart_rx_buffer);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(ble_uart_rx_buffer);
    attr_char_value.p_value   = ble_uart_rx_buffer;

    return sd_ble_gatts_characteristic_add(p_uart->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_uart->rx_handles);
}


/**@brief Function for adding Tx characteristics.
 *
 * @param[in]   p_uart        Data Transfer Service structure.
 * @param[in]   p_uart_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t uart_device_events_char_add(ble_uart_t * p_uart, const ble_uart_init_t * p_uart_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    // Clear char_md structure
    memset(&char_md, 0, sizeof(char_md));

		//	Set indicate avaliability
    char_md.char_props.notify  = 1;

		//	Setup descriptor
#ifdef LIGHT_CHARACTERISTICS
	char_md.p_char_user_desc = NULL;
#else
	// Human-friendly descriptor to debug
	char_md.p_char_user_desc = (uint8_t*)"Events";
	char_md.char_user_desc_size = sizeof("Events");
	char_md.char_user_desc_max_size = 20;
#endif
	char_md.p_char_pf        = NULL;
    char_md.p_user_desc_md   = NULL;
    char_md.p_cccd_md        = NULL;
    char_md.p_sccd_md        = NULL;

	//	Inform the characteristic that it will have a custom vendor UUID
	ble_uuid.type = p_uart->uuid_type;
	//	Set the 16-bit UUID for the characteristic
	ble_uuid.uuid = DEV_EVENTS_UUID;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc       = BLE_GATTS_VLOC_USER;					/*< Set the characteristics buffer on the user's side of RAM **/
    attr_md.rd_auth    = 0;										/*< Read authentication */
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 1;										/*< Various length **/

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = 20;
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = 20;
    attr_char_value.p_value      = ble_uart_tx_buffer;


    return sd_ble_gatts_characteristic_add(p_uart->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_uart->dev_events_handles);
}

/**@brief Function for initializing the Data Transfer Service.
 *
 * @param[out]  p_uart       Data Transfer Service structure. This structure will have to
 *                          be supplied by the application. It will be initialized by this function,
 *                          and will later be used to identify this particular service instance.
 * @param[in]   p_uart_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
uint32_t ble_uart_init(ble_uart_t * p_uart, const ble_uart_init_t * p_uart_init)
{
    uint32_t   err_code;
	ble_uuid128_t base_uuid = GWATCH_UUID_BASE;

    // Initialize service structure
    p_uart->evt_handler = p_uart_init->evt_handler;
    p_uart->conn_handle = BLE_CONN_HANDLE_INVALID;

	// Assign base UUID
	err_code = sd_ble_uuid_vs_add(&base_uuid, &(p_uart->uuid_type));
	if(err_code != NRF_SUCCESS)
	{
			return err_code;
	}


	ble_uuid_t ble_uuid;
	//	Set the vendor specific UUID to the newly created service (?)
	ble_uuid.type = p_uart->uuid_type;
	//  Set the 16-bit UUID
	ble_uuid.uuid = BLE_UART_SERVICE_UUID;
	//	Add the service and create a service_handle to it
	err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &(p_uart->service_handle));
	if(err_code != NRF_SUCCESS)
	{
			return err_code;
	}


    // Add measurement characteristic
    err_code = uart_rx_char_add(p_uart, p_uart_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add feature characteristic
    err_code = uart_tx_char_add(p_uart, p_uart_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = uart_device_events_char_add(p_uart, p_uart_init);


    return NRF_SUCCESS;
}



/**@brief Function for checking if indication of Data Transfer Measurement is currently enabled.
 *
 * @param[in]   p_uart                  Data Transfer Service structure.
 * @param[out]  p_indication_enabled   TRUE if indication is enabled, FALSE otherwise.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_uart_is_indication_enabled(ble_uart_t * p_uart, bool * p_indication_enabled)
{
    uint32_t err_code;
    uint8_t  cccd_value_buf[BLE_CCCD_VALUE_LEN];
    ble_gatts_value_t gatts_value;

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len     = BLE_CCCD_VALUE_LEN;
    gatts_value.offset  = 0;
    gatts_value.p_value = cccd_value_buf;

    err_code = sd_ble_gatts_value_get(p_uart->conn_handle,
                                      p_uart->tx_handles.cccd_handle,
                                      &gatts_value);
    if (err_code == NRF_SUCCESS)
    {
        *p_indication_enabled = ble_srv_is_indication_enabled(cccd_value_buf);
    }
    return err_code;
}

/**@brief Function for checking if indication of Data Transfer Measurement is currently enabled.
 *
 * @param[in]   p_uart                  Data Transfer Service structure.
 * @param[out]  p_indication_enabled   TRUE if indication is enabled, FALSE otherwise.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_uart_is_notification_enabled(ble_uart_t * p_uart, bool * p_indication_enabled)
{
    uint32_t err_code;
    uint8_t  cccd_value_buf[BLE_CCCD_VALUE_LEN];
    ble_gatts_value_t gatts_value;

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len     = BLE_CCCD_VALUE_LEN;
    gatts_value.offset  = 0;
    gatts_value.p_value = cccd_value_buf;

    err_code = sd_ble_gatts_value_get(p_uart->conn_handle,
                                      p_uart->dev_events_handles.cccd_handle,
                                      &gatts_value);
    if (err_code == NRF_SUCCESS)
    {
        *p_indication_enabled = ble_srv_is_notification_enabled(cccd_value_buf);
    }
    return err_code;
}
