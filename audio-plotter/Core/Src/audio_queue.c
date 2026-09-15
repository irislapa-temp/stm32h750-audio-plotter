/*
 * plotter_queue.c
 *
 *  Created on: 10 Jul 2026
 *      Author: irine
 */

#include "audio_plotter.h"
#include "main.h"

void plotter_queue_push(audio_plotter_handle_t *h, audio_segment_t seg)
{
	if (!h) return;
    audio_queue_t *q = &h->queue;

    __disable_irq();
	if (q->count < q->size)
	{
		q->buffer[q->wr] = seg;
	    q->wr = (q->wr + 1) % q->size;
	    q->count++;
	}
	__enable_irq();
}

uint32_t plotter_queue_pop(audio_plotter_handle_t *h, audio_segment_t *seg)
{
	if (!h)
		return 0;

	audio_queue_t *q = &h->queue;
	*seg = (audio_segment_t) {NULL, 0};

	__disable_irq();
	if (q->count > 0)
	{
		uint16_t i = q->r;
		*seg = (audio_segment_t) q->buffer[i];
		q->r = (q->r + 1) % q->size;
		q->count--;
	}
	else {
		__enable_irq();
		return 0; // Queue is empty
	}
	__enable_irq();
	return 1;
}
