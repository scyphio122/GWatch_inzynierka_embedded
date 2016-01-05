/*
 * scheduler.h
 *
 *  Created on: 28 gru 2015
 *      Author: Konrad
 */

#ifndef LIBRARIES_SCHEDULER_H_
#define LIBRARIES_SCHEDULER_H_

#include "app_fifo.h"

#define BLE_SCHEDULER_QUEUE_SIZE	(uint8_t)8

extern app_fifo_t	   ble_task_fifo;

uint32_t Scheduler_Init();
uint32_t Scheduler_Schedule_Task(app_fifo_t *fifo, uint8_t state_op_code);
uint32_t Scheduler_Get_Task(app_fifo_t* fifo, uint8_t* buffer);
uint32_t Scheduler_Empty(app_fifo_t* fifo);

#endif /* LIBRARIES_SCHEDULER_H_ */
