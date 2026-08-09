/**
  ******************************************************************************
  * @file    BSP/Src/audio_record.c
  * @author  MCD Application Team
  * @brief   This example describes how to use DFSDM HAL API to realize
  *          audio recording.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
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
/** @addtogroup STM32H7xx_HAL_Examples
  * @{
  */

/** @addtogroup BSP
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
typedef enum
{
  BUFFER_OFFSET_NONE = 0,
  BUFFER_OFFSET_HALF = 1,
  BUFFER_OFFSET_FULL = 2,
}BUFFER_StateTypeDef;


/* Private define ------------------------------------------------------------*/
#define AUDIO_FREQUENCY            16000U
//#define AUDIO_FREQUENCY            16000U
#define AUDIO_IN_PDM_BUFFER_SIZE  (uint32_t)(128*AUDIO_FREQUENCY/16000*2)
#define AUDIO_BUFF_SIZE  4096
#define AUDIO_NB_BLOCKS    ((uint32_t)4)

#define ADC_DMA_BUFF_SIZE 1024
#define FS_HZ            16000u
#define REC_SEC          5u

#define REC_SAMPLES      (FS_HZ * REC_SEC)   // 320000 samples

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#if defined ( __CC_ARM )  /* !< ARM Compiler */
  ALIGN_32BYTES (uint16_t recordPDMBuf[AUDIO_IN_PDM_BUFFER_SIZE]) __attribute__((section(".RAM_D3")));

#elif defined ( __ICCARM__ )  /* !< ICCARM Compiler */
#pragma location=0x38000000
ALIGN_32BYTES (uint16_t recordPDMBuf[AUDIO_IN_PDM_BUFFER_SIZE]);
#elif defined ( __GNUC__ )  /* !< GNU Compiler */
  ALIGN_32BYTES (uint16_t recordPDMBuf[AUDIO_IN_PDM_BUFFER_SIZE]) __attribute__((section(".RAM_D3")));
#endif

ALIGN_32BYTES (uint16_t  RecPlayback[AUDIO_BUFF_SIZE]);
ALIGN_32BYTES (uint16_t  PlaybackBuffer[2*AUDIO_BUFF_SIZE]);
ALIGN_32BYTES (uint16_t  recordADCBuff[ADC_DMA_BUFF_SIZE]);


ALIGN_32BYTES (uint16_t copyPCMBuf[REC_SAMPLES]);

/* Pointer to record_data */
uint32_t playbackPtr;

uint32_t  InState = 0;
uint32_t  OutState = 0;

uint32_t AudioBufferOffset;
uint32_t PlaybackStarted = 0;
BSP_AUDIO_Init_t  AudioInInit;
BSP_AUDIO_Init_t  AudioOutInit;
BSP_AUDIO_Init_t AnalogInInit;

static uint32_t AudioFreq[9] = {8000 ,11025, 16000, 22050, 32000, 44100, 48000, 96000, 192000};
uint32_t *AudioFreq_ptr;

uint32_t VolumeLevel = 40, AudioOutState, AudioInState, MuteState;
char text[256];
char* Audio_Out_State[4] = {
  " AUDIO_OUT_STATE_RESET  ",
  " AUDIO_OUT_STATE_PLAYING",
  " AUDIO_OUT_STATE_STOP   ",
  " AUDIO_OUT_STATE_PAUSE  "
};

char* Audio_In_State[4] = {
  "AUDIO_IN_STATE_RESET    ",
  "AUDIO_IN_STATE_RECORDING",
  "AUDIO_IN_STATE_STOP     ",
  "AUDIO_IN_STATE_PAUSE    "
};

char* Mute_State[2] = {
  " BSP_AUDIO_MUTE_DISABLED",
  " BSP_AUDIO_MUTE_ENABLED ",
};


extern uint8_t CheckForUserInput(void);



/* Private user variables ----------------------------------------------------*/
extern channel_nbr;
// larger recording buffer in ram
static int16_t g_rec_pcm[REC_SAMPLES];
static volatile uint32_t g_rec_wr = 0;
static volatile uint8_t  g_rec_done  = 0;

//draw_wave_t hdraw_pdm;
//draw_wave_t hdraw_adc;
signal_plotter_handle_t hplotterADC;
uint32_t adc_dma_write_idx = 0;
/* Private function prototypes -----------------------------------------------*/
//static void rec_copy_chunk(const int16_t *src, uint32_t n_samples);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Audio Record and Playback multi-buffer mode test
  *         Record:
  *          - Audio IN instance: 1 (DFSDM)
  *          - Audio IN Device  : digital MIC1 and MIC2
  *          - Audio IN number of channel  : 2
  *         Playback:
  *          - Audio OUT instance: 0 (SAI)
  *          - Audio OUT Device  : HDMI
  * @retval None
  */
void AudioRecord_demo(void)
{

  uint32_t x_size, y_size;

  BSP_LCD_GetXSize(0, &x_size);
  BSP_LCD_GetYSize(0, &y_size);

  /* Clear the LCD */
  UTIL_LCD_Clear(UTIL_LCD_COLOR_WHITE);
  /* Set Audio Demo description */
  UTIL_LCD_FillRect(0, 0, x_size, 90, UTIL_LCD_COLOR_BLUE);
  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
  UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_BLUE);
  UTIL_LCD_SetFont(&Font24);
  UTIL_LCD_DisplayStringAt(0, 0, (uint8_t *)"AUDIO RECORD SAI PDM EXAMPLE", CENTER_MODE);
  UTIL_LCD_SetFont(&Font16);
  UTIL_LCD_DisplayStringAt(0, 24, (uint8_t *)"Make sure the SW2 is in position PDM ", CENTER_MODE);
  UTIL_LCD_DisplayStringAt(0, 40,  (uint8_t *)"Press User button for next menu", CENTER_MODE);
  /* Set the LCD Text Color */
  UTIL_LCD_DrawRect(10, 100, x_size - 20, y_size - 110, UTIL_LCD_COLOR_BLUE);
  UTIL_LCD_DrawRect(11, 101, x_size - 22, y_size - 112, UTIL_LCD_COLOR_BLUE);

  //displayDimensions(12, 102, x_size - 24, y_size - 114);

  AudioFreq_ptr = AudioFreq+4; /* AUDIO_FREQUENCY_16K; */

  AudioOutInit.Device = AUDIO_OUT_DEVICE_HEADPHONE;
  AudioOutInit.ChannelsNbr = channel_nbr;
  AudioOutInit.SampleRate = *AudioFreq_ptr;
  AudioOutInit.BitsPerSample = AUDIO_RESOLUTION_16B;
  AudioOutInit.Volume = VolumeLevel;

  AudioInInit.Device = AUDIO_IN_DEVICE_DIGITAL_MIC;
  AudioInInit.ChannelsNbr = channel_nbr;
  AudioInInit.SampleRate = *AudioFreq_ptr;
  AudioInInit.BitsPerSample = AUDIO_RESOLUTION_16B;
  AudioInInit.Volume = VolumeLevel;

  /* Initialize Audio Recorder with 2 channels to be used */
  BSP_AUDIO_IN_Init(1, &AudioInInit);
  BSP_AUDIO_IN_GetState(1, &InState);

  BSP_AUDIO_OUT_Init(0, &AudioOutInit);

  //init_DrawWavePDM(&hdraw_pdm, (int16_t *) RecPlayback[0],  12, 102, x_size - 24, y_size - 114);
  /* Start Recording */
  UTIL_LCD_DisplayStringAt(0, 190, (uint8_t *)"Start Recording ", CENTER_MODE);
  BSP_AUDIO_IN_RecordPDM(1, (uint8_t*)&recordPDMBuf, 2*AUDIO_IN_PDM_BUFFER_SIZE);

  /* Play the recorded buffer*/
  UTIL_LCD_DisplayStringAt(0, 220, (uint8_t *)"Play the recorded buffer... ", CENTER_MODE);
  BSP_AUDIO_OUT_Play(0, (uint8_t*)&RecPlayback[0], 2*AUDIO_BUFF_SIZE);

  MX_DMA_Init();
  MX_TIM6_Init();
  MX_ADC3_Init();
  HAL_TIM_Base_Start(&htim6);

  HAL_ADC_Start_DMA(&hadc3, (uint32_t*)recordADCBuff, ADC_DMA_BUFF_SIZE);

  hplotterADC.input_buffer = (int16_t*)recordADCBuff;
  hplotterADC.input_buffer_size = ADC_DMA_BUFF_SIZE;
  hplotterADC.window.x = 12;
  hplotterADC.window.y = 102;
  hplotterADC.window.width = x_size - 24;
  hplotterADC.window.height = y_size - 114;
  hplotterADC.p_dma_write_idx = &adc_dma_write_idx;

  init_plotter(&hplotterADC, (void *) NULL);

  while (1)
  {
    signal_plotter_plot(&hplotterADC);

    if (CheckForUserInput() > 0)
    {
      ButtonState = 0;
      BSP_AUDIO_IN_Stop(1);
      BSP_AUDIO_IN_DeInit(1);
      BSP_AUDIO_OUT_Stop(0);
      BSP_AUDIO_OUT_DeInit(0);
      return;
    }
  }
}

#define PCM_SAMPLES_PER_CB (AUDIO_IN_PDM_BUFFER_SIZE/8)

static inline void rec_copy_chunk(const int16_t *src, uint32_t n)
{
  uint32_t remaining = REC_SAMPLES - g_rec_wr;
  if (n > remaining)
  {
	  n = remaining;
  }
  for (uint32_t i = 0; i < n; i++)
  {
	  copyPCMBuf[g_rec_wr + i] = src[i];
  }

  g_rec_wr += n;

  if (g_rec_wr >= REC_SAMPLES)
  {
	  g_rec_done = 1;
  }
}

uint32_t get_g_rec_wr(void)
{
  return g_rec_wr;
}


/**
  * @brief Calculates the remaining file size and new position of the pointer.
  * @retval None
  */
void BSP_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance)
{
  if(Instance == 1U)
  {
        /* Invalidate Data Cache to get the updated content of the SRAM*/
    SCB_InvalidateDCache_by_Addr((uint32_t *)&recordPDMBuf[AUDIO_IN_PDM_BUFFER_SIZE/2], AUDIO_IN_PDM_BUFFER_SIZE*2);

    BSP_AUDIO_IN_PDMToPCM(Instance, (uint16_t*)&recordPDMBuf[AUDIO_IN_PDM_BUFFER_SIZE/2], &RecPlayback[playbackPtr]);

    /* Clean Data Cache to update the content of the SRAM */
    SCB_CleanDCache_by_Addr((uint32_t*)&RecPlayback[playbackPtr], AUDIO_IN_PDM_BUFFER_SIZE/4);
/*
    int16_t *pcm_chunk = (int16_t*)&RecPlayback[playbackPtr];

    audio_chunk_t draw_chunk = {
	  .data = pcm_chunk,
	  .n_samples = PCM_SAMPLES_PER_CB
	};

    __disable_irq();
    //rec_copy_chunk(pcm_chunk, PCM_SAMPLES_PER_CB);
    draw_q_push(&hdraw_pdm, draw_chunk);

    playbackPtr += AUDIO_IN_PDM_BUFFER_SIZE/4/2;
    if(playbackPtr >= AUDIO_BUFF_SIZE)
    {
    	playbackPtr = 0;
    }
    __enable_irq();
  */
  }
  else
  {
    AudioBufferOffset = BUFFER_OFFSET_FULL;
  }
}

/**
  * @brief  Manages the DMA Half Transfer complete interrupt.
  * @retval None
  */
void BSP_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance)
{
  if(Instance == 1U)
  {
        /* Invalidate Data Cache to get the updated content of the SRAM*/
    SCB_InvalidateDCache_by_Addr((uint32_t *)&recordPDMBuf[0], AUDIO_IN_PDM_BUFFER_SIZE*2);

    BSP_AUDIO_IN_PDMToPCM(Instance, (uint16_t*)&recordPDMBuf[0], &RecPlayback[playbackPtr]);

    /* Clean Data Cache to update the content of the SRAM */
    SCB_CleanDCache_by_Addr((uint32_t*)&RecPlayback[playbackPtr], AUDIO_IN_PDM_BUFFER_SIZE/4);

    int16_t *pcm_chunk = (int16_t*)&RecPlayback[playbackPtr];

    audio_chunk_t draw_chunk = {
	  .data = pcm_chunk,
	  .n_samples = PCM_SAMPLES_PER_CB
	};
    __disable_irq();
    //rec_copy_chunk(pcm_chunk, PCM_SAMPLES_PER_CB);
    draw_q_push(&hdraw_pdm, draw_chunk);

    playbackPtr += AUDIO_IN_PDM_BUFFER_SIZE/4/2;
    if(playbackPtr >= AUDIO_BUFF_SIZE)
    {
      playbackPtr = 0;
    }
    __enable_irq();


  }
  else
  {
    AudioBufferOffset = BUFFER_OFFSET_HALF;
  }
}

/**
  * @brief  Conversion complete callback in non-blocking mode.
  * @param hadc ADC handle
  * @retval None
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC3)
    {
    	adc_dma_write_idx = ADC_DMA_BUFF_SIZE;
      
    }
}

/**
  * @brief  Conversion DMA half-transfer callback in non-blocking mode.
  * @param hadc ADC handle
  * @retval None
  */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC3)
    {
    	adc_dma_write_idx = ADC_DMA_BUFF_SIZE/2;
    }
}


/**
  * @}
  */

/**
  * @}
  */

