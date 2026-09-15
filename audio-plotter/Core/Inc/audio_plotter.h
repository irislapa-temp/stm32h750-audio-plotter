/*
 * signal_plotter.h
 *
 *  Created on: 19 Feb 2026
 *      Author: irine
 */

#ifndef INC_SIGNAL_PLOTTER_H_
#define INC_SIGNAL_PLOTTER_H_

#include <stdint.h>
#include <limits.h>
#include "main.h"
#include "plot_err.h"
#include "audio_plotter_config.h"

#define DMA_TRANSFER_HALF 0u
#define DMA_TRANSFER_COMPLETE 1u

#define AUDIO_SOURCE_ADC 0u
#define AUDIO_SOURCE_MEMS 1u

#define SIMPLE 0u
#define PEAK 1u
#define AVG 2u

#define SAMPLE_DECIMATION 0u
#define PEAK_DECIMATION 1u
#define AVG_DECIMATION 2u

typedef struct {
  int16_t *p_samples;
  uint32_t n_samples;
} audio_segment_t;

typedef struct {
  audio_segment_t buffer[AUDIO_QUEUE_SIZE];
  uint16_t size;
  uint16_t wr; //where to push
  uint16_t r;  //where to pop
#include "plot_err.h"
  uint16_t count;
} audio_queue_t;

typedef struct {
	uint8_t audio_source;
	uint16_t *buffer;
	uint16_t buffer_size;

	uint32_t dc_offset;
	uint32_t sampling_method;
	uint8_t plot_flag;
	uint32_t dma_wr;
	uint8_t dma_transfer;
} decimate_t;

typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
} draw_window_t;

typedef struct {
  int16_t *buffer;
  uint16_t buffer_size;
  uint16_t n_buffs;
  draw_window_t window;
} plot_t;

typedef struct {
  decimate_t decimate;
  plot_t plot;
  draw_window_t main_window;
  audio_queue_t queue;
  uint16_t n_segments;
  uint16_t current_segment;
} audio_plotter_handle_t;

void decimate(audio_plotter_handle_t *h, uint32_t decimation_method);

void plot_segment_wave(const int16_t *seg, uint32_t seg_size, draw_window_t w);
void plot_segment_minmax(const int16_t *seg, uint32_t seg_size, draw_window_t w);
void plot_buffer(audio_plotter_handle_t *h,int16_t *buff, uint32_t buffSize);

void init_plotter_adc(audio_plotter_handle_t *h, uint8_t audio_source);

//void get_snapshot(int16_t *srcBuffBase, uint32_t srcBuffSize, uint32_t *srcIdx, int16_t *snapshotBuff, uint32_t snapshotBuffSize);

#endif /* INC_SIGNAL_PLOTTER_H_ */
