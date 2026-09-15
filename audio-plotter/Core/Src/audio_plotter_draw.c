/*
 * audio_plotter_draw.c
 *
 *  Created on: 19 Mar 2026
 *      Author: irinej
 */

#include "audio_plotter.h"

uint8_t toggle = 0;



void plot_segment_wave(const int16_t *seg, uint32_t seg_size, draw_window_t w)
{
    if (!seg || seg_size == 0 || w.width == 0 || w.height == 0) return;

    UTIL_LCD_FillRect(w.x, w.y, w.width, w.height, UTIL_LCD_COLOR_WHITE);

    const int32_t mid = (int32_t)w.y + (int32_t)w.height / 2;
    int32_t yp = mid;
    uint32_t xp = w.x;

    for (uint32_t px = 0; px < w.width; px++)
    {
        uint32_t i = (uint32_t)(((uint64_t)px * (seg_size - 1)) / (w.width - 1));

        int32_t s = seg[i];
        int32_t y = mid - (s * (int32_t)(w.height / 2)) / INT16_MAX;

        if (y < (int32_t)w.y) y = (int32_t)w.y;
        if (y > (int32_t)(w.y + w.height - 1)) y = (int32_t)(w.y + w.height - 1);

        uint32_t x = w.x + px;
        if (px == 0) {
            UTIL_LCD_SetPixel(x, (uint32_t)y, UTIL_LCD_COLOR_BLUE);
        } else {
            UTIL_LCD_DrawLine(xp, (uint32_t)yp, x, (uint32_t)y, UTIL_LCD_COLOR_BLUE);
        }
        xp = x;
        yp = y;
    }
}


void plot_segment_minmax(const int16_t *seg, uint32_t seg_size, draw_window_t w)
{
    if (!seg || (seg_size < 2) || (seg_size & 1u)) return;

    UTIL_LCD_FillRect(w.x, w.y, w.width, w.height, UTIL_LCD_COLOR_WHITE);

    const int32_t mid = (int32_t)w.y + (int32_t)w.height / 2;
    uint32_t n_pairs = seg_size / 2u;

    for (uint32_t px = 0; px < w.width; px+=2)
    {
        uint32_t g = (uint32_t)(((uint64_t)px * (n_pairs - 1u)) / (w.width - 1u));

        int32_t mn = seg[2*g];
        int32_t mx = seg[2*g + 1u];

        int32_t y1 = mid - (mn * (int32_t)(w.height/2)) / INT16_MAX;
        int32_t y2 = mid - (mx * (int32_t)(w.height/2)) / INT16_MAX;

        if (y1 < (int32_t)w.y) y1 = (int32_t)w.y;
        if (y1 > (int32_t)(w.y + w.height - 1)) y1 = (int32_t)(w.y + w.height - 1);
        if (y2 < (int32_t)w.y) y2 = (int32_t)w.y;
        if (y2 > (int32_t)(w.y + w.height - 1)) y2 = (int32_t)(w.y + w.height - 1);

        if (y1 > y2) { int32_t t=y1; y1=y2; y2=t; } // ensure y1 is top

        uint32_t x = w.x + px;
        UTIL_LCD_DrawLine(x, (uint32_t)y1, x, (uint32_t)y2, UTIL_LCD_COLOR_BLUE);
    }
}



// todo: verify if it plots the buffers interchangibly
void plot_buffer(audio_plotter_handle_t *h,int16_t *buff, uint32_t buffSize)
{
  h->plot.window.y      = h->main_window.y;
  h->plot.window.width  = h->main_window.width/2;
  h->plot.window.height = h->main_window.height;

  //sample(h, SAMPLE_DECIMATION);
  if (toggle)
  {
	 h->plot.window.x = h->main_window.x;
	 toggle = 0;
  }
  else
  {
    h->plot.window.x = h->main_window.x + h->main_window.width / 2;
    toggle = 1;
  }
  //plot_segment_wave(h->plot.buffer, h->plot.buffer_size, h->plot.window);
  plot_segment_minmax(h->plot.buffer, h->plot.buffer_size, h->plot.window);

}


void draw_segment(audio_plotter_handle_t *h)
{
    uint32_t segment_width = h->main_window.width / h->n_segments;

    h->plot.window.x = h->main_window.x + (h->current_segment * segment_width);
    h->plot.window.y = h->main_window.y;
    h->plot.window.width = segment_width;
    h->plot.window.height = h->main_window.height;

    plot_segment_minmax(h->plot.buffer, h->plot.buffer_size, h->plot.window);

    h->current_segment++;
    if (h->current_segment >= h->n_segments) {
    	h->current_segment = 0;
    }
}







