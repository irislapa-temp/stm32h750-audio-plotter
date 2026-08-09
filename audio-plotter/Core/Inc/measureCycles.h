/*
 * measureCycles.h
 *
 *  Created on: 23 Feb 2026
 *      Author: irine
 */

#ifndef INC_MEASURECYCLES_H_
#define INC_MEASURECYCLES_H_

#include "main.h"
#include <stdint.h>
#include "core_cm7.h"

extern uint32_t n_cycles;
void EnableTiming(void);
void Delay(uint32_t cycles);
void TimeStart(void);
void TimeEnd(void);

void DWT_Init(void);
uint32_t DWT_GetCycles(void);
uint32_t get_system_clock(void);

#endif /* INC_MEASURECYCLES_H_ */
