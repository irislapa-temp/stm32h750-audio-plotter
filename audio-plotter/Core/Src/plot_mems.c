/**
  ******************************************************************************
  * @file    audio-plotter/Core/Src/plot_mems.c
  * @brief   Audio record demo using MEMS PDM.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>
#include "string.h"
#include "audio_plotter.h"
#include "plot_err.h"

/* Parameters ----------------------------------------------------------------*/
#define AUDIO_SAMPLE_RATE_HZ        16000U
#define AUDIO_VOLUME               40U
#define AUDIO_IN_PDM_BUFFER_SIZE   ((uint32_t)(128U * AUDIO_SAMPLE_RATE_HZ / 16000U * 2U))
#define AUDIO_PCM_BUFFER_SIZE 4096U
#define INVERT_CH(ch) (3U - (ch)) // universal solution for mono/stereo, returns 2 for mono and 1 for stereo

/* DMA buffer ---------------------------------------------------------------*/
#if defined ( __CC_ARM )
  ALIGN_32BYTES (uint16_t recordPDMBuff[AUDIO_IN_PDM_BUFFER_SIZE]) __attribute__((section(".RAM_D3")));
#elif defined ( __ICCARM__ )
#pragma location=0x38000000
ALIGN_32BYTES (uint16_t recordPDMBuff[AUDIO_IN_PDM_BUFFER_SIZE]);
#elif defined ( __GNUC__ )
  ALIGN_32BYTES (uint16_t recordPDMBuff[AUDIO_IN_PDM_BUFFER_SIZE]) __attribute__((section(".RAM_D3")));
#endif

ALIGN_32BYTES (uint16_t recordPCMBuff[AUDIO_PCM_BUFFER_SIZE]);
uint32_t pcmWritePtr = 0;

extern uint32_t channel_nbr;
extern __IO uint32_t ButtonState;
extern uint8_t CheckForUserInput(void);

static audio_plotter_handle_t hplot_mems;


/**
  * @brief  Audio Record demo using MEMS PDM.
  * @retval None
  */
void PlotMemsDemo(void)
{
  uint32_t x_size, y_size;
  BSP_AUDIO_Init_t audio_in;

  BSP_LCD_GetXSize(0, &x_size);
  BSP_LCD_GetYSize(0, &y_size);

  /* Clear the LCD */
  UTIL_LCD_Clear(UTIL_LCD_COLOR_WHITE);
  /* Header */
  UTIL_LCD_FillRect(0, 0, x_size, 90, UTIL_LCD_COLOR_BLUE);
  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
  UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_BLUE);
  UTIL_LCD_SetFont(&Font24);
  UTIL_LCD_DisplayStringAt(0, 0, (uint8_t *)"MEMS PDM RECORD PLOTTER", CENTER_MODE);
  UTIL_LCD_SetFont(&Font16);
  UTIL_LCD_DisplayStringAt(0, 40, (uint8_t *)"Press User button to stop", CENTER_MODE);

  /* Plot frame */
  UTIL_LCD_DrawRect(10, 100, x_size - 20, y_size - 110, UTIL_LCD_COLOR_BLUE);
  UTIL_LCD_DrawRect(11, 101, x_size - 22, y_size - 112, UTIL_LCD_COLOR_BLUE);

  audio_in.Device = AUDIO_IN_DEVICE_DIGITAL_MIC;
  audio_in.ChannelsNbr = channel_nbr;
  audio_in.SampleRate = AUDIO_SAMPLE_RATE_HZ;
  audio_in.BitsPerSample = AUDIO_RESOLUTION_16B;
  audio_in.Volume = AUDIO_VOLUME;

  BSP_AUDIO_IN_Init(1, &audio_in);

  UTIL_LCD_DisplayStringAt(0, 190, (uint8_t *)"Start Recording ", CENTER_MODE);
  BSP_AUDIO_IN_RecordPDM(1, (uint8_t*)&recordPDMBuff, 2U * AUDIO_IN_PDM_BUFFER_SIZE);


  hplot_mems.decimate.audio_source    = AUDIO_SOURCE_MEMS;
  hplot_mems.decimate.sampling_method = PEAK_DECIMATION;
  hplot_mems.decimate.buffer          = (int16_t *) recordPCMBuff;
  hplot_mems.decimate.buffer_size     = AUDIO_PCM_BUFFER_SIZE;
  hplot_mems.decimate.dma_wr          = pcmWritePtr;
  hplot_mems.main_window.x            = 12;
  hplot_mems.main_window.y            = 102;
  hplot_mems.main_window.width        = x_size - 24;
  hplot_mems.main_window.height       = y_size - 114;

  init_plotter(&hplot_mems);


  while (1)
  {
	plot_audio(&hplot_mems);
    if (CheckForUserInput() > 0)
    {
      ButtonState = 0;
      BSP_AUDIO_IN_Stop(1);
      BSP_AUDIO_IN_DeInit(1);
      return;
    }
  }
}


void Process_Input(uint32_t Instance, uint32_t pdm_offset)
{
	BSP_AUDIO_IN_PDMToPCM(Instance,
						(uint16_t*)&recordPDMBuff[pdm_offset],
						&recordPCMBuff[pcmWritePtr]);

	SCB_CleanDCache_by_Addr((uint32_t*)&recordPCMBuff[pcmWritePtr],
						  AUDIO_IN_PDM_BUFFER_SIZE/4);


	// todo: extract segment size from plot_t
	if (pcmWritePtr % hplot_mems.decimate.buffer_size == 0 && pcmWritePtr != 0)
	{
		uint32_t n_samples = hplot_mems.decimate.buffer_size/hplot_mems.n_segments;
		audio_segment_t seg = {
		.p_samples = (int16_t *)&recordPCMBuff[pcmWritePtr],
		.n_samples = hplot_mems.decimate.buffer_size
		};

	__disable_irq();
	plotter_queue_push(&hplot_mems, seg);
	__enable_irq();
	}
	pcmWritePtr += AUDIO_IN_PDM_BUFFER_SIZE/4/2/INVERT_CH(channel_nbr);
	if(pcmWritePtr >= AUDIO_PCM_BUFFER_SIZE)
	{
	  pcmWritePtr = 0;
	}
}

/**
  * @brief  DMA transfer complete callback for MEMS PDM.
  * @retval None
  */
void BSP_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance)
{
  if (Instance != 1) return;

  SCB_InvalidateDCache_by_Addr((uint32_t *)&recordPDMBuff[AUDIO_IN_PDM_BUFFER_SIZE/2],
                               AUDIO_IN_PDM_BUFFER_SIZE*2);
  Process_Input(Instance, AUDIO_IN_PDM_BUFFER_SIZE/2);
}

/**
  * @brief  DMA half transfer callback for MEMS PDM.
  * @retval None
  */
void BSP_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance)
{
  if (Instance != 1) return;

  SCB_InvalidateDCache_by_Addr((uint32_t *)&recordPDMBuff[0],
                               AUDIO_IN_PDM_BUFFER_SIZE*2);
  Process_Input(Instance, 0);
}
