/*
 * audio_plotter_sampling.c
 *
 *  Created on: 19 Mar 2026
 *      Author: irinej
 */

#include "audio_plotter.h"




// simple downsample by picking every Nth sample, where N = in_size / out_size
void simple_downsample(const uint16_t *p_buff_in, uint32_t in_size,
  uint32_t dc_offset, int16_t *p_buff_out, uint32_t out_size)
{
  if (out_size == 0 || p_buff_in == NULL || p_buff_out == NULL)
  {
    error_handler(ERR_INVALID_PARAM);
    return;
  }

  uint16_t group_size = (uint16_t) (in_size / out_size);
  if (group_size == 0)
  {
    error_handler(ERR_OUT_OF_BOUNDS);
    return;
  }

  for(uint32_t i = 0; i < out_size; i++)
  {
      uint32_t idx = (i * group_size);
      if (idx < in_size)
      {
          uint16_t raw = p_buff_in[idx];
          p_buff_out[i] = (int16_t)((int32_t)raw - dc_offset);
      }
  }
}

// downsample by picking min and max from each group of samples, where group size = in_size / (out_size/2)
void peak_downsample(const uint16_t *p_buff_in, uint32_t in_size,
		uint32_t dc_offset, int16_t *p_buff_out, uint32_t out_size)
{
  if (!p_buff_in || !p_buff_out || in_size == 0 || out_size < 2u) return;
  if (out_size & 1u) return; // must be even


  uint32_t n_groups = out_size / 2u;
  uint32_t step = in_size / n_groups;
  if (step == 0u) step = 1u;

  for (uint32_t g = 0; g < n_groups; g++)
  {
    int16_t mn = INT16_MAX;
    int16_t mx = INT16_MIN;

    uint32_t base = g * step;

    for (uint32_t j = 0; j < step; j++)
    {
      int32_t s = (int32_t)p_buff_in[base + j] - dc_offset;

      // cap to range
      if (s < mn) mn = (int16_t)s;
      if (s > mx) mx = (int16_t)s;
    }
    p_buff_out[2u*g]     = mn;
    p_buff_out[2u*g + 1] = mx;
  }
}

// downsample by picking the average of each group of samples
void avg_downsample(const uint16_t *p_buff_in, uint32_t in_size,
		uint32_t dc_offset, int16_t *p_buff_out, uint32_t out_size)
{
  if (!p_buff_in || !p_buff_out || in_size == 0 || out_size == 0) return;

  uint32_t step = (uint16_t) (in_size / out_size);
  if (step == 0u) step = 1u;

  for (uint32_t i = 0; i < out_size; i++)
  {
    uint32_t base = i * step;

    if (base + step > in_size) break;

    int32_t sum = 0;
    for (uint32_t j = 0; j < step; j++)
    {
      sum += p_buff_in[base + j];
    }
    p_buff_out[i] = (int16_t)(sum / (int32_t)step) - dc_offset;
  }
}

void high_pass_filter(const int16_t *p_buff_in, int16_t *p_buff_out, uint32_t size, uint32_t window)
{
    // A simple Moving Average Subtraction filter
    for (uint32_t i = 0; i < size; i++)
    {
        int32_t sum = 0;
        uint32_t count = 0;

        // 1. Calculate the Low-Pass (Average) for the current window
        for (int32_t j = -(int32_t)window; j <= (int32_t)window; j++)
        {
            if ((int32_t)i + j >= 0 && (int32_t)i + j < size) {
                sum += p_buff_in[i + j];
                count++;
            }
        }
        int16_t local_average = (int16_t)(sum / count);

        // 2. Subtract the slow-moving average from the raw sample
        p_buff_out[i] = p_buff_in[i] - local_average;
    }
}


void upsample(const int16_t *p_buff_in, uint32_t in_size, int16_t *p_buff_out, uint32_t out_size)
{
  if (!p_buff_in || !p_buff_out || in_size == 0 || out_size == 0) return;

  uint32_t step = out_size / in_size;
  for (uint32_t i = 0; i < in_size; i++)
  {
	for (uint32_t s = 0; s < out_size; s += step)
	{
	  p_buff_out[s] = p_buff_in[i];
	}
  }
}

void sample(audio_plotter_handle_t *h)
{
	uint16_t *p_buff_in = h->decimate.buffer;
    uint32_t in_size = h->decimate.buffer_size;
    int16_t *p_buff_out = h->plot.buffer;
    uint32_t out_size = h->plot.buffer_size;
    uint32_t dc_offset = h->decimate.dc_offset;

    SCB_InvalidateDCache_by_Addr((uint16_t *)p_buff_in, in_size*sizeof(uint16_t));
    if ((uint16_t *)p_buff_in == NULL || in_size == 0) return;
    switch (h->decimate.sampling_method)
    {
        case SAMPLE_DECIMATION:
            simple_downsample(p_buff_in, in_size, dc_offset, p_buff_out, out_size);
            break;

        case PEAK_DECIMATION: // Combined into one logic using dc_offset
            peak_downsample(p_buff_in, in_size, dc_offset, p_buff_out, out_size);
            break;

        case AVG_DECIMATION:
            avg_downsample(p_buff_in, in_size, dc_offset, p_buff_out, out_size);
            break;

        default:
            error_handler(ERR_UNSUPPORTED);
            break;
    }
}






