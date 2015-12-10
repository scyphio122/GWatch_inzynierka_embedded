/*
 * fifo.h
 *
 *  Created on: 10 gru 2015
 *      Author: Konrad
 */

#ifndef LIBRARIES_FIFO_H_
#define LIBRARIES_FIFO_H_

void Fifo_Init(app_fifo_t* fifo, uint8_t* buf, uint16_t buf_size);
void Fifo_Clear(app_fifo_t* fifo);

#endif /* LIBRARIES_FIFO_H_ */
