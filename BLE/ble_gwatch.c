/*
 * ble.c
 *
 *  Created on: 18 gru 2015
 *      Author: Konrad
 */

#include <ble.h>
#include <ble_debug_assert_handler.h>
#include <ble_gap.h>
#include <ble_gatt.h>
#include <ble_gatts.h>
#include <ble_gwatch.h>
#include <ble_hci.h>
#include <ble_stack_handler_types.h>
#include <ble_types.h>
#include <nrf_sdm.h>
#include <nrf_soc.h>
#include <nrf51.h>
#include <pstorage_platform.h>
#include <softdevice_handler.h>
#include <stdbool.h>
#include <string.h>
#include <sys/_stdint.h>
#include "ble_advdata.h"
#include "ble_uart.h"

uint16_t 							m_conn_handle = BLE_CONN_HANDLE_INVALID;		/*< This variable holds an information whether the device is in BLE connection or not **/
static ble_gap_adv_params_t			m_adv_params;
ble_advdata_t 						m_advdata;


/**@brief Function for error handling, which is called when an error has occurred.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of error.
 *
 * @param[in] error_code  Error code supplied to the handler.
 * @param[in] line_num    Line number where the handler is called.
 * @param[in] p_file_name Pointer to the file name.
 */
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
     /*This call can be used for debug purposes during application development.
     @note CAUTION: Activating this code will write the stack to flash on an error.
                    This function should NOT be used in a final product.
                    It is intended STRICTLY for development/debugging purposes.
                    The flash write will happen EVEN if the radio is active, thus interrupting
                    any communication.
                    Use with care. Un-comment the line below to use.*/
    ble_debug_assert_handler(error_code, line_num, p_file_name);

    // On assert, the system can only recover with a reset.
    //NVIC_SystemReset();
}

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init()
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)ADVERTISING_DEVICE_NAME,
                                          strlen(ADVERTISING_DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing services that will be used by the application.
 */
static void services_init()
{
    uint32_t 						err_code;
	ble_uart_init_t					uart_init;

	//handler assignement
	uart_init.evt_handler = Ble_Uart_Handler;

	err_code = ble_uart_init(&m_ble_uart, &uart_init);
	APP_ERROR_CHECK(err_code);

}


/**@brief Function for initializing security parameters.
 */
static void sec_params_init()
{
    /*m_sec_params.bond         = SEC_PARAM_BOND;
    m_sec_params.mitm         = SEC_PARAM_MITM;
    m_sec_params.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    m_sec_params.oob          = SEC_PARAM_OOB;
    m_sec_params.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    m_sec_params.max_key_size = SEC_PARAM_MAX_KEY_SIZE;*/
}

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t                         err_code;
    static ble_gap_evt_auth_status_t m_auth_status;
    ble_gap_enc_info_t *             p_enc_info;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            sd_nvic_DisableIRQ(SWI1_IRQn);

//            Advertising_Pause_Data_Packet(); // If swapping was enabled, it's better to pause it to avoid pointless processor use during connection
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            sd_nvic_EnableIRQ(SWI1_IRQn);

			Advertising_Start();
            break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
					/***
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle,
                                                   BLE_GAP_SEC_STATUS_SUCCESS,
                                                   &m_sec_params);
            APP_ERROR_CHECK(err_code);
				***/
            break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
					/***
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0);
            APP_ERROR_CHECK(err_code);
				***/
            break;

        case BLE_GAP_EVT_AUTH_STATUS:
            m_auth_status = p_ble_evt->evt.gap_evt.params.auth_status;
            break;

        case BLE_GAP_EVT_SEC_INFO_REQUEST:
					/***		TODO: Security settings
				p_enc_info = &m_auth_status.periph_keys.enc_info;
            if (p_enc_info->div == p_ble_evt->evt.gap_evt.params.sec_info_request.div)
            {
                err_code = sd_ble_gap_sec_info_reply(m_conn_handle, p_enc_info, NULL);
                APP_ERROR_CHECK(err_code);
            }
            else
            {
                // No keys found for this device
                err_code = sd_ble_gap_sec_info_reply(m_conn_handle, NULL, NULL);
                APP_ERROR_CHECK(err_code);
            }
				***/
            break;

        case BLE_GAP_EVT_TIMEOUT:
            if (p_ble_evt->evt.gatts_evt.params.timeout.src == BLE_GATT_TIMEOUT_SRC_PROTOCOL)
            {
                err_code = sd_ble_gap_disconnect(m_conn_handle,
                                                 BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
                APP_ERROR_CHECK(err_code);
            }
			if (p_ble_evt->evt.gap_evt.params.timeout.src == BLE_GAP_TIMEOUT_SRC_ADVERTISING)
			{
				//advertising_start();
			}
            break;

        default:
            // No implementation needed.
            break;
    }
}



/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the scheduler in the main loop after a BLE stack
 *          event has been received.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
	//ble_advertising_on_ble_evt(p_ble_evt);
	on_ble_evt(p_ble_evt);
	ble_uart_on_ble_evt(&m_ble_uart ,p_ble_evt);
}


/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in]   sys_evt   System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
	pstorage_sys_event_handler(sys_evt);
	//SD_flash_operation_callback(sys_evt);
}

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init()
{
    uint32_t err_code;

    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, false);

    // Enable BLE stack

    ble_enable_params_t ble_enable_params;
    memset(&ble_enable_params, 0, sizeof(ble_enable_params));
    ble_enable_params.gatts_enable_params.service_changed = 0;
    err_code = sd_ble_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for system events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);

    APP_ERROR_CHECK(err_code);
}


void Advertising_Init(void)
{
	uint32_t      err_code;

	// Build and set advertising data
	memset(&m_advdata, 0, sizeof(m_advdata));

	m_advdata.name_type              	= BLE_ADVDATA_FULL_NAME;
	m_advdata.include_appearance     	= false;
	m_advdata.flags              		= BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
	m_adv_params.type 					= BLE_GAP_ADV_TYPE_ADV_IND;
	m_adv_params.p_peer_addr 			= NULL;
	m_adv_params.fp 					= BLE_GAP_ADV_FP_ANY;
	m_adv_params.p_whitelist 			= NULL;
	m_adv_params.interval 				= ADVERTISING_INTERVAL;
	m_adv_params.timeout 				= ADVERTISING_TIMEOUT;

	err_code = ble_advdata_set(&m_advdata, NULL);
	if(err_code != NRF_SUCCESS)
		APP_ERROR_CHECK(err_code);
}

void Advertising_Start()
{
	uint32_t err_code = 0;//sd_ble_gap_adv_data_set(NULL, 0,NULL, 0);
	if(err_code != NRF_SUCCESS)		//	//
	{
		APP_ERROR_CHECK(err_code);
	}

	err_code = sd_ble_gap_adv_start(&m_adv_params);
	if(err_code != NRF_SUCCESS)		//	//
	{
		APP_ERROR_CHECK(err_code);
	}
}

void BLE_Init()
{
	ble_stack_init();
	gap_params_init();
	services_init();
	sec_params_init();
}



