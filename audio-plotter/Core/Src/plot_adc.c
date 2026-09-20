/**
  ******************************************************************************
  * @file    audio-plotter/Core/Src/plot_adc.c
  * @brief   Audio record demo using ADC + DMA.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>
#include "string.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "audio_plotter.h"

/* Parameters ----------------------------------------------------------------*/
#define ADC_DMA_BUFFER_SIZE MAX_ADC_DMA_BUFF_SIZE
uint32_t current_adcDmaBuffer_size = ADC_DMA_BUFFER_SIZE;

/* DMA buffers ---------------------------------------------------------------*/
ALIGN_32BYTES (uint16_t adcDmaBuffer[MAX_ADC_DMA_BUFF_SIZE]);

static audio_plotter_handle_t hplot_adc;

extern ADC_HandleTypeDef hadc3;
extern DMA_HandleTypeDef hdma_adc3;
extern void init_plotter(audio_plotter_handle_t *h);
extern void plot_audio(audio_plotter_handle_t *h);

extern uint32_t channel_nbr;
extern __IO uint32_t ButtonState;
extern uint8_t CheckForUserInput(void);

/**
  * @brief  ADC record demo with waveform plotter.
  * @retval None
  */
void PlotAdcDemo(void)
{
  uint32_t x_size, y_size;

  BSP_LCD_GetXSize(0, &x_size);
  BSP_LCD_GetYSize(0, &y_size);

  /* Clear the LCD */
  UTIL_LCD_Clear(UTIL_LCD_COLOR_WHITE);
  /* Header */
  UTIL_LCD_FillRect(0, 0, x_size, 90, UTIL_LCD_COLOR_BLUE);
  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
  UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_BLUE);
  UTIL_LCD_SetFont(&Font24);
  UTIL_LCD_DisplayStringAt(0, 0, (uint8_t *)"ADC RECORD PLOTTER", CENTER_MODE);
  UTIL_LCD_SetFont(&Font16);
  UTIL_LCD_DisplayStringAt(0, 24, (uint8_t *)"Press User button to stop", CENTER_MODE);

  /* Plot frame */
  UTIL_LCD_DrawRect(10, 100, x_size - 20, y_size - 110, UTIL_LCD_COLOR_BLUE);
  UTIL_LCD_DrawRect(11, 101, x_size - 22, y_size - 112, UTIL_LCD_COLOR_BLUE);

  MX_DMA_Init();
  MX_TIM6_Init();
  MX_ADC3_Init();
  HAL_TIM_Base_Start(&htim6);

  hplot_adc.decimate.audio_source    = AUDIO_SOURCE_ADC;
  hplot_adc.decimate.sampling_method = PEAK_DECIMATION;
  hplot_adc.decimate.buffer          = (uint16_t*)adcDmaBuffer;
  hplot_adc.decimate.buffer_size     = ADC_DMA_BUFFER_SIZE;
  hplot_adc.main_window.x            = 12;
  hplot_adc.main_window.y            = 102;
  hplot_adc.main_window.width        = x_size - 24;
  hplot_adc.main_window.height       = y_size - 114;

  init_plotter(&hplot_adc);

  HAL_ADC_Start_DMA(&hadc3, (uint32_t*)adcDmaBuffer, ADC_DMA_BUFFER_SIZE);

  while (1)
  {
    plot_audio(&hplot_adc, MINMAX);

    if (CheckForUserInput() > 0)
    {
      ButtonState = 0;
      HAL_ADC_Stop_DMA(&hadc3);
      HAL_TIM_Base_Stop(&htim6);
      return;
    }
  }
}

HAL_StatusTypeDef set_DMA_buff_size(ADC_HandleTypeDef *hadc,
							uint32_t buff_size)
{
	if (buff_size == 0 || buff_size > MAX_ADC_DMA_BUFF_SIZE)
	{
		return HAL_ERROR;
	}
	if (HAL_ADC_Stop_DMA(hadc) != HAL_OK)
	{
		return HAL_ERROR;
	}
	current_adcDmaBuffer_size = buff_size;
    return HAL_ADC_Start_DMA(hadc, (uint32_t *)adcDmaBuffer, buff_size);
}



//todo: what is the segment size, what is the pointer?
/**
  * @brief  Conversion complete callback in non-blocking mode.
  * @param  hadc ADC handle
  * @retval None
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
   if (hadc->Instance != ADC3) return;

   SCB_InvalidateDCache_by_Addr((uint32_t*)&adcDmaBuffer[current_adcDmaBuffer_size/2],
		   current_adcDmaBuffer_size/2 * sizeof(uint16_t));

   audio_segment_t seg = {
	 .p_samples = (int16_t*)&adcDmaBuffer[current_adcDmaBuffer_size/2],
	 .n_samples = current_adcDmaBuffer_size/2
   };

   __disable_irq();
   queue_push(&hplot_adc, seg);
   __enable_irq();
}

/**
  * @brief  Conversion DMA half-transfer callback in non-blocking mode.
  * @param  hadc ADC handle
  * @retval None
  */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
   if (hadc->Instance != ADC3) return;

   SCB_InvalidateDCache_by_Addr((uint32_t*)&adcDmaBuffer[0],
		   current_adcDmaBuffer_size/2 * sizeof(uint16_t));

   audio_segment_t seg = {
	 .p_samples = (int16_t*)&adcDmaBuffer[0],
	 .n_samples = current_adcDmaBuffer_size/2
   };

   __disable_irq();
   //queue_push(&hplot_adc, seg);
   __enable_irq();
}
