/*
 * fifo.c
 *
 *  Created on: 10 gru 2015
 *      Author: Konrad
 */
#include "app_fifo.h"
#include "fifo.h"

/**
 * This function initializes the fifo - sets the buffer, it's size and initializes the read and write inices
 *
 * \param fifo 		- the fifo to be configured
 * \param buf  		- the buffer which will be attached to the fifo
 * \param buf_size	- the size of the buffer which will be attached to the fifo
 */
void Fifo_Init(app_fifo_t* fifo, uint8_t* buf, uint16_t buf_size)
{
	app_fifo_init(fifo, buf, buf_size);
}

/**
 * This function clear the fifo (it sets the read and write indices to zero)
 *
 * \param fifo - fifo to clear
 */
inline void Fifo_Clear(app_fifo_t* fifo)
{
	fifo->read_pos = 0;
	fifo->write_pos = 0;
}
