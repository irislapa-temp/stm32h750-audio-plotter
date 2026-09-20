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
extern current_adcDmaBuffer_size;

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
	h->n_segments           = n_segments;
	h->current_segment      = 0;
	h->plot.buffer_size     = DEFAULT_PLOT_BUFF_SIZE / n_segments;
	h->decimate.buffer_size = (DEFAULT_PLOT_BUFF_SIZE/n_segments)*2;
}


// todo: define default setup
void init_plotter(audio_plotter_handle_t *h)
{
	if (!h) {
		error_handler(ERR_NULL_PTR);
		return;
	}
	if (h->decimate.audio_source != AUDIO_SOURCE_ADC &&
	        h->decimate.audio_source != AUDIO_SOURCE_MEMS)
	{
		error_handler(ERR_UNSUPPORTED);
		return;
	}
    h->plot.buffer      = plot_buff;
    h->plot.buffer_size = DEFAULT_PLOT_BUFF_SIZE;

    h->queue.size  = MAX_AUDIO_QUEUE_SIZE;
    h->queue.r     = 0;
    h->queue.wr    = 0;
    h->queue.count = 0;
    h->decimate.dma_wr      = 0;
    if (h->decimate.audio_source == AUDIO_SOURCE_ADC)
    {
    	h->decimate.dc_offset = INT16_MAX;
    }

	segment_config(h, 1);
	//init_draw_module(&h->plot);
}

// bottom of the callstack

audio_segment_t curr_seg;
void plot_audio(audio_plotter_handle_t *h, uint8_t ploting_method)
{

  if (!queue_pop(h, &curr_seg))
	  return;

  h->decimate.buffer = (uint16_t *)curr_seg.p_samples;
  h->decimate.buffer_size = curr_seg.n_samples;

  sample(h);

  draw_segment(h);

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




