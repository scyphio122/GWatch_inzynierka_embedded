#include "scheduler.h"
#include "fifo.h"
#include "nrf_error.h"

static uint8_t  ble_task_buffer[BLE_SCHEDULER_QUEUE_SIZE];

app_fifo_t	   ble_task_fifo;


uint32_t Scheduler_Init()
{
	Fifo_Init(&ble_task_fifo, ble_task_buffer, sizeof(ble_task_buffer));

	return NRF_SUCCESS;
}

inline uint32_t Scheduler_Schedule_Task(app_fifo_t *fifo, uint8_t state_op_code)
{
	Fifo_Put(fifo, state_op_code);
	return NRF_SUCCESS;
}

inline uint32_t Scheduler_Get_Task(app_fifo_t* fifo, uint8_t* buffer)
{
	Fifo_Get(fifo, buffer);
	return NRF_SUCCESS;
}

inline uint32_t Scheduler_Empty(app_fifo_t* fifo)
{
	return Fifo_Is_Empty(fifo);
}
