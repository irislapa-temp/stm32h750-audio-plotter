/*
 * measureCycles.c
 *
 *  Created on: 23 Feb 2026
 *      Author: irine
 */

#include "measureCycles.h"

//**************** Debug time measurement ******************

volatile unsigned int *DWT_CYCCNT   = (volatile unsigned int *)0xE0001004;
volatile unsigned int *DWT_CONTROL  = (volatile unsigned int *)0xE0001000;
volatile unsigned int *DWT_LAR      = (volatile unsigned int *)0xE0001FB0;
volatile unsigned int *SCB_DHCSR    = (volatile unsigned int *)0xE000EDF0;
volatile unsigned int *SCB_DEMCR    = (volatile unsigned int *)0xE000EDFC;
volatile unsigned int *ITM_TER      = (volatile unsigned int *)0xE0000E00;
volatile unsigned int *ITM_TCR      = (volatile unsigned int *)0xE0000E80;
static volatile int Debug_ITMDebug = 0;

static uint32_t start_time = 0;
uint32_t n_cycles = 0;

uint32_t get_system_clock(void)
{
  return HAL_RCC_GetHCLKFreq();
}

void EnableTiming(void)
{
  if ((*SCB_DHCSR & 1) && (*ITM_TER & 1)) // Enabled?
    Debug_ITMDebug = 1;

  *SCB_DEMCR |= 0x01000000;
  *DWT_LAR = 0xC5ACCE55; // enable access
  *DWT_CYCCNT = 0; // reset the counter
  *DWT_CONTROL |= 1 ; // enable the counter
}

//****************************************************************************
void DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // enable trace
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;            // enable cycle counter
}

uint32_t DWT_GetCycles(void)
{
    return DWT->CYCCNT;
}

void Delay(uint32_t cycles)
{
  uint32_t start = *DWT_CYCCNT;

  while(1)
  {
	  if ((*DWT_CYCCNT - start) >= cycles)
	  {
		  break;
	  }
  }
}

void TimeStart(void)
{
  if (Debug_ITMDebug)
  {
	  start_time = *DWT_CYCCNT;
  }
}

void TimeEnd(void)
{
  if (Debug_ITMDebug)
  {
	  n_cycles = *DWT_CYCCNT-start_time;
  }
}
