/*
 * signal_plotter.c
 *
 *  Created on: 19 Feb 2026
 *      Author: irine
 */
#include "audio_plotter.h"
#include "measureCycles.h"
#include "plot_err.h"


#ifndef PLOT_BUFF_SIZE
#define PLOT_BUFF_SIZE 228
#endif

int16_t plot_buff[DEFAULT_PLOT_BUFF_SIZE];


void segment_config(audio_plotter_handle_t *h, uint16_t n_segments)
{
	if (!h) {
		error_handler(ERR_NULL_PTR);
		return;
	}
	if (n_segments == 0 || n_segments > DEFAULT_MAX_SEGMENTS)
	{
		error_handler(ERR_INVALID_PARAM);
		return;
	}
	h->n_segments = n_segments;
	h->current_segment = 0;
	h->decimate.buffer_size = DEFAULT_SAMPLE_SIZE / n_segments;
	h->plot.buffer_size = DEFAULT_PLOT_BUFF_SIZE / n_segments;
}


// todo: define default setup
void init_plotter(audio_plotter_handle_t *h)
{
	if (!h) {
		error_handler(ERR_NULL_PTR);
		return;
	}
	if (h->decimate.audio_source == AUDIO_SOURCE_ADC)
	{
		h->decimate.dma_wr = 0;
		h->plot.buffer = &plot_buff;
		h->plot.buffer_size = DEFAULT_PLOT_BUFF_SIZE;
	}
	else if (h->decimate.audio_source == AUDIO_SOURCE_MEMS)
	{
		h->decimate.dma_wr = 0;
		h->plot.buffer = &plot_buff;
		h->plot.buffer_size = DEFAULT_PLOT_BUFF_SIZE;
		h->queue.size = AUDIO_QUEUE_SIZE;
		h->queue.r = 0;
		h->queue.wr = 0;

	}
	else
	{
		error_handler(ERR_UNSUPPORTED);
		return;
	}
	segment_config(h, 2);
	init_draw_module(&h->plot);
}




//todo: what am i plotting? n_buffers, half-half buffer, or am i polling n samples
void plot_adc(audio_plotter_handle_t *h)
{
	draw_buffer(h, h->plot.buffer, h->plot.buffer_size);
}


void plot_mems(audio_plotter_handle_t *h)
{
	//draw_buffer(h, h->plot.buffer, h->plot.buffer_size);

}



// bottom of the callstack
uint32_t core_clock;
uint32_t t_pop;
uint32_t t_sample;
uint32_t t_draw;
void plot_audio(audio_plotter_handle_t *h)
{
	DWT_Init();
	core_clock = get_system_clock();
	uint32_t t0 = DWT_GetCycles();

  //SCB_InvalidateDCache_by_Addr((uint32_t *)h->decimate.buffer, h->decimate.buffer_size * sizeof(uint16_t));
  audio_segment_t seg;
  if (!plotter_queue_pop(h, &seg))
	  return;
  t_pop = DWT_GetCycles() - t0;
  h->decimate.buffer = (uint16_t *)seg.p_samples;
  h->decimate.buffer_size = seg.n_samples;
  sample(h);
  t_sample = DWT_GetCycles() - t0 - t_pop;
  //draw_buffer(h, h->plot.buffer, h->plot.buffer_size);
  draw_segment(h);
  t_draw = DWT_GetCycles() - t0 - t_pop - t_sample;
  uint32_t t1 = DWT_GetCycles();
  n_cycles = t1 - t0;
}


/*=========== error handler for fatal errors in plotter code ============ */
void error_handler(uint32_t error_code)
{
    __disable_irq();
    __BKPT(0);   // debugger stops here if attached
    uint32_t code = error_code;
    while (1)
    {
        // stay here for debugger inspection
    }
}




