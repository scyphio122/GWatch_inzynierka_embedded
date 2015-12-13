/*
 * UART.h
 *
 *  Created on: 10 gru 2015
 *      Author: Konrad
 */

#ifndef HARDWARE_UART_H_
#define HARDWARE_UART_H_

#include <app_fifo.h>
#include <sys/_stdint.h>

#define UART_FIFO_SIZE (uint8_t)128

extern app_fifo_t 				uart_rx_fifo;
extern uint8_t 					uart_fifo_buffer[UART_FIFO_SIZE];

void UART_Init();
void UART_Enable();
void UART_Start_Tx();
void UART_Stop_Tx();
void UART_Start_Rx();
void UART_Stop_Rx();
void UART_Disable();
void UART_Send_String(uint8_t* data_to_send, uint16_t data_size);
void UART_Wait_For_Transmission_End();
void UART_Change_Baudrate(uint32_t baudrate);
#endif /* HARDWARE_UART_H_ */
