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

#define LCD_WIDTH 480u
#define LCD_HEIGHT 272u

#define DEFAULT_MAIN_WINDOW_X 16
#define DEFAULT_MAIN_WINDOW_Y 102
#define DEFAULT_MAIN_WINDOW_WIDTH 448
#define DEFAULT_MAIN_WINDOW_HEIGHT 148


#define DEFAULT_PLOT_BUFF_SIZE DEFAULT_MAIN_WINDOW_WIDTH

#define DEFAULT_SAMPLE_SIZE 1024u // should be divisible by 2
#define DEFAULT_MAX_SEGMENTS 64u
#define DEFAULT_SEGMENT_SIZE 512u // should be divisible by 2

#define AUDIO_QUEUE_SIZE DEFAULT_MAX_SEGMENTS

// --- Double Buffering Memory Map ---
// STM32H750 Discovery SDRAM starts at 0xD0000000, apparently
//TODO: place this in a more readable location
#define BUFFER_PING 0xD0000000
// Offset by ~522KB (480x272x4) to give plenty of room for the next frame
#define BUFFER_PONG 0xD0100000


typedef struct {
    uint32_t pixel_format;
    uint8_t  bpp;
} dma2d_config_t;

typedef struct {
  int16_t *p_samples;
  uint32_t n_samples;
} audio_segment_t;

typedef struct {
  audio_segment_t buffer[AUDIO_QUEUE_SIZE];
  uint16_t size;
  uint16_t wr; //where to push
  uint16_t r;  //where to pop
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

  draw_window_t window;
  dma2d_config_t dma2d_cfg;
} plot_t;

typedef struct {
  decimate_t decimate;
  plot_t plot;
  draw_window_t main_window;
  audio_queue_t queue;
  uint16_t n_segments;
  uint16_t current_segment;
} audio_plotter_handle_t;




void plotter_queue_push(audio_plotter_handle_t *h, audio_segment_t seg);
uint32_t plotter_queue_pop(audio_plotter_handle_t *h, audio_segment_t *seg);
void sample(audio_plotter_handle_t *h);

void draw_wave(plot_t *plot_cfg, int16_t *buffer, uint32_t buffer_size);
void draw_minmax(int16_t *buffer, uint32_t buffer_size, draw_window_t w);
void draw_buffer(audio_plotter_handle_t *h, int16_t *buff, uint32_t buffSize);
void draw_segment(audio_plotter_handle_t *h);

void init_plotter(audio_plotter_handle_t *h);

void plot_mems(audio_plotter_handle_t *h);
void plot_audio(audio_plotter_handle_t *h);

//void get_snapshot(int16_t *srcBuffBase, uint32_t srcBuffSize, uint32_t *srcIdx, int16_t *snapshotBuff, uint32_t snapshotBuffSize);

#endif /* INC_SIGNAL_PLOTTER_H_ */
