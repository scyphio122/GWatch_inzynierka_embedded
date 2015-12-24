/*
 * hardware_settings.h
 *
 *  Created on: 10 gru 2015
 *      Author: Konrad
 */

#ifndef HARDWARE_HARDWARE_SETTINGS_H_
#define HARDWARE_HARDWARE_SETTINGS_H_

/** BUTTON MODULE **/
#define BUTTON_PIN			(uint8_t)20

/** UART MODULE **/
#define UART_TX_PIN			(uint8_t)30
#define UART_RX_PIN			(uint8_t)0

/** SPI0 MODULE **/
#define SPI0_MISO_PIN		(uint8_t)17
#define SPI0_MOSI_PIN		(uint8_t)16
#define SPI0_SCK_PIN		(uint8_t)15

/** DISPLAY SPI MODULE **/
//#define SPI1_USED			(uint8_t)1
#define DISP_MISO			(uint8_t)25
#define DISP_SCK			BUTTON_PIN//(uint8_t)24
#define DISP_MOSI			UART_TX_PIN//(uint8_t)23
#define DISP_CS				(uint8_t)22
#define DISP_TOGGLE_PIN		(uint8_t)21
#define DISP_LIGHT			(uint8_t)28

/** GPS MODULE **/
#define GPS_3D_FIX_PIN		(uint8_t)1
#define GPS_RESET_PIN		(uint8_t)2
#define GPS_ON_PIN			(uint8_t)3
#define GPS_RTCM_PIN		(uint8_t)4
#define GPS_1_PPS_PIN		(uint8_t)5

/** ADC MODULE **/
#define BAT_MEASURE_PIN		(uint8_t)6


#define ACCELER_MAG_ON_PIN	(uint8_t)7

/** ACCELEROMETER MODULE **/
#define ACCELER_CS_PIN		(uint8_t)8

/** MAGNETOMETER MODULE **/
#define MAG_IRQ_XL_PIN		(uint8_t)9
#define MAG_DATA_RDY_PIN	(uint8_t)10
#define MAG_CS_PIN			(uint8_t)11
#define MAG_IRQ_PIN			(uint8_t)12

/** EXTERNAL FLASH MODULE **/
#define EXT_FLASH_CS_PIN	(uint8_t)13
#define EXT_FLASH_RESET_PIN	(uint8_t)14
#define EXT_FLASH_WP_PIN	(uint8_t)18
#define EXT_FLASH_ON_PIN	(uint8_t)19





#endif /* HARDWARE_HARDWARE_SETTINGS_H_ */
