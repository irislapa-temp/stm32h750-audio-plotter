/*
 * plot_err.h
 *
 *  Created on: 19 Mar 2026
 *      Author: irine
 */

#ifndef INC_PLOT_ERR_H_
#define INC_PLOT_ERR_H_

#include <stdint.h>

#define ERR_NONE                0x00
#define ERR_NULL_PTR            0x01
#define ERR_UNSUPPORTED         0x02

// Plot Errors
#define ERR_INVALID_WINDOW      0x10  // Width or Height is 0
#define ERR_INVALID_BUFFER_SIZE 0x11  // Buffer size is 0 or too small
#define ERR_WINDOW_OUT_OF_BOUNDS 0x12 // Window exceeds physical screen

// Decimation Errors
#define ERR_DECIMATE_OVERFLOW   0x20  // dma_idx exceeds buffer size
#define ERR_INVALID_METHOD      0x21  // Unknown decimation method
#define ERR_DC_OFFSET_INVALID   0x22  // Offset too large for 16-bit logic

// Data Integrity Errors
#define ERR_MINMAX_MISMATCH     0x30  // Peak decimate called with odd out_size
#define ERR_QUEUE_OVERFLOW      0x31  // Pushing to a full queue
#define ERR_QUEUE_UNDERFLOW     0x32  // Popping from an empty queue
#define ERR_OUT_OF_BOUNDS	      0x33

// Parameter errors
#define ERR_INVALID_PARAM       0x40

void error_handler(uint32_t error_code);

#endif /* INC_PLOT_ERR_H_ */
