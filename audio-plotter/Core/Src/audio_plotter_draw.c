/*
 * audio_plotter_draw.c
 *
 *  Created on: 19 Mar 2026
 *      Author: irinej
 */

#include "audio_plotter.h"



// idea from gemini, to use dma2d to quickly fill the buffer area with white before drawing the waveforms, instead of using a slow loop or LCD fill function. This allows for smoother updates and less flickering.
//todo: clean up the logic
extern DMA2D_HandleTypeDef hlcd_dma2d;
extern LTDC_HandleTypeDef  hlcd_ltdc;
volatile uint8_t dma2d_ready = 1;

uint8_t toggle = 0;
uint32_t front_buffer = BUFFER_PING; // Currently being read by LTDC and shown on LCD
uint32_t back_buffer  = BUFFER_PONG; // Hidden, currently being wiped/drawn by us

void Custom_DMA2D_CompleteCallback(DMA2D_HandleTypeDef *hdma2d)
{
    dma2d_ready = 1;
}

// (Optional) A custom error callback just to be safe
void Custom_DMA2D_ErrorCallback(DMA2D_HandleTypeDef *hdma2d)
{
    dma2d_ready = 1;
}

void swap_buffers(void)
{
	// Tell the LTDC to look at our freshly drawn back buffer for Layer 0
	HAL_LTDC_SetAddress_NoReload(&hlcd_ltdc, back_buffer, 0);

	// Request a reload during the next Vertical Blanking period to prevent tearing mid-screen
	HAL_LTDC_Reload(&hlcd_ltdc, LTDC_RELOAD_VERTICAL_BLANKING);

	// Swap our internal pointers so we draw on the other buffer next frame
	uint32_t temp = front_buffer;
	front_buffer = back_buffer;
	back_buffer = temp;
}

void init_draw_module(plot_t *plot_cfg)
{
    if (!plot_cfg) return;

    // Fetch the live LTDC configuration EXACTLY ONCE
    uint32_t p_format = hlcd_ltdc.LayerCfg[0].PixelFormat;

    // Cache it inside the struct
    plot_cfg->dma2d_cfg.pixel_format = p_format;
    plot_cfg->dma2d_cfg.bpp = (p_format == LTDC_PIXEL_FORMAT_ARGB8888) ? 4 : 2;

    dma2d_ready = 1;
}

void wipe_dma2d_buffer(dma2d_config_t *cfg, uint32_t target_buffer, uint16_t x, uint16_t y, uint32_t width, uint32_t height, uint32_t color)
{
    while (!dma2d_ready);
    dma2d_ready = 0;

    // USE CACHED VALUES
    uint32_t wipe_addr = target_buffer + (((y * LCD_WIDTH) + x) * cfg->bpp);

    hlcd_dma2d.Init.Mode = DMA2D_R2M;
    hlcd_dma2d.Init.ColorMode = cfg->pixel_format;
    hlcd_dma2d.Init.OutputOffset = LCD_WIDTH - width;

    if (HAL_DMA2D_Init(&hlcd_dma2d) != HAL_OK) { dma2d_ready = 1; return; }

    hlcd_dma2d.XferCpltCallback  = Custom_DMA2D_CompleteCallback;
    hlcd_dma2d.XferErrorCallback = Custom_DMA2D_ErrorCallback;

    if (HAL_DMA2D_Start_IT(&hlcd_dma2d, color, wipe_addr, width, height) != HAL_OK) {
        dma2d_ready = 1;
    }
}

void copy_dma2d_window(dma2d_config_t *cfg, uint32_t src_buffer, uint32_t dest_buffer, uint16_t x, uint16_t y, uint32_t width, uint32_t height)
{
    while (!dma2d_ready);
    dma2d_ready = 0;

    // USE CACHED VALUES
    uint32_t src_addr  = src_buffer  + (((y * LCD_WIDTH) + x) * cfg->bpp);
    uint32_t dest_addr = dest_buffer + (((y * LCD_WIDTH) + x) * cfg->bpp);

    hlcd_dma2d.Init.Mode         = DMA2D_M2M;
    hlcd_dma2d.Init.ColorMode    = cfg->pixel_format;
    hlcd_dma2d.Init.OutputOffset = LCD_WIDTH - width;

    hlcd_dma2d.LayerCfg[1].InputOffset    = LCD_WIDTH - width;
    hlcd_dma2d.LayerCfg[1].InputColorMode = cfg->pixel_format;
    hlcd_dma2d.LayerCfg[1].AlphaMode      = DMA2D_NO_MODIF_ALPHA;
    hlcd_dma2d.LayerCfg[1].InputAlpha     = 0xFF;

    if (HAL_DMA2D_Init(&hlcd_dma2d) != HAL_OK) { dma2d_ready = 1; return; }
    if (HAL_DMA2D_ConfigLayer(&hlcd_dma2d, 1) != HAL_OK) { dma2d_ready = 1; return; }

    hlcd_dma2d.XferCpltCallback  = Custom_DMA2D_CompleteCallback;
    hlcd_dma2d.XferErrorCallback = Custom_DMA2D_ErrorCallback;

    if (HAL_DMA2D_Start_IT(&hlcd_dma2d, src_addr, dest_addr, width, height) != HAL_OK) {
        dma2d_ready = 1;
    }
}

void wipe_dma2d_buffer_fast(dma2d_config_t *cfg, uint32_t target_buffer, uint16_t x, uint16_t y, uint32_t width, uint32_t height, uint32_t color)
{
    uint32_t wipe_addr = target_buffer + (((y * LCD_WIDTH) + x) * cfg->bpp);

    // Wait for any previous DMA2D transfer to complete
    while ((DMA2D->CR & DMA2D_CR_START) != 0);

    // Configure DMA2D registers directly
    DMA2D->CR = DMA2D_R2M;                 // Register-to-Memory mode
    DMA2D->OCOLR = color;                  // Output color
    DMA2D->OMAR = wipe_addr;               // Output memory address
    DMA2D->OOR = LCD_WIDTH - width;        // Output line offset
    DMA2D->OPFCCR = cfg->pixel_format;     // Output pixel format
    DMA2D->NLR = (width << 16) | height;   // Pixels per line and Number of lines

    // Start transfer
    DMA2D->CR |= DMA2D_CR_START;

    // Poll for Transfer Complete Interrupt Flag (TCIF)
    while ((DMA2D->ISR & DMA2D_ISR_TCIF) == 0);
    DMA2D->IFCR = DMA2D_IFCR_CTCIF;        // Clear the flag
}

void copy_dma2d_window_fast(dma2d_config_t *cfg, uint32_t src_buffer, uint32_t dest_buffer, uint16_t x, uint16_t y, uint32_t width, uint32_t height)
{
    uint32_t src_addr  = src_buffer  + (((y * LCD_WIDTH) + x) * cfg->bpp);
    uint32_t dest_addr = dest_buffer + (((y * LCD_WIDTH) + x) * cfg->bpp);

    // Wait for any previous DMA2D transfer to complete
    while ((DMA2D->CR & DMA2D_CR_START) != 0);

    // Configure DMA2D registers directly
    DMA2D->CR = DMA2D_M2M;                 // Memory-to-Memory mode

    DMA2D->FGMAR = src_addr;               // Foreground memory address
    DMA2D->FGOR = LCD_WIDTH - width;       // Foreground line offset
    DMA2D->FGPFCCR = cfg->pixel_format;    // Foreground pixel format

    DMA2D->OMAR = dest_addr;               // Output memory address
    DMA2D->OOR = LCD_WIDTH - width;        // Output line offset
    DMA2D->OPFCCR = cfg->pixel_format;     // Output pixel format

    DMA2D->NLR = (width << 16) | height;   // Pixels per line and Number of lines

    // Start transfer
    DMA2D->CR |= DMA2D_CR_START;

    // Poll for Transfer Complete Interrupt Flag (TCIF)
    while ((DMA2D->ISR & DMA2D_ISR_TCIF) == 0);
    DMA2D->IFCR = DMA2D_IFCR_CTCIF;        // Clear the flag
}


/**
 * @brief Draws a continuous line between any two arbitrary points directly to SDRAM.
 * Uses Bresenham's algorithm. Safe for any X/Y jumps.
 */
void bresnhams_line(uint32_t target_buffer, int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint8_t bpp, uint32_t color)
{
    int32_t dx = abs(x1 - x0);
    int32_t sx = x0 < x1 ? 1 : -1;
    int32_t dy = -abs(y1 - y0);
    int32_t sy = y0 < y1 ? 1 : -1;
    int32_t err = dx + dy, e2;

    while (1)
    {
    	// if in bounds, then plot
    	if (x0 >= 0 && x0 < LCD_WIDTH && y0 >= 0 && y0 < LCD_HEIGHT)
        {
    		if (bpp == 2) {
				uint16_t *pixel_ptr = (uint16_t *)(target_buffer + (((y0 * LCD_WIDTH) + x0) * 2));
				*pixel_ptr = (uint16_t) color;
			} else {
				uint32_t *pixel_ptr = (uint32_t *)(target_buffer + (((y0 * LCD_WIDTH) + x0) * 4));
				*pixel_ptr = color;
			}
        }

    	e2 = 2 * err;

    	if (e2 >= dy)
    	{
    		if (x0 == x1) break;
    		err += dy;
    		x0  += sx;
    	}
    	if (e2 <= dx)
    	{
    		if (y0 == y1) break;
    		err += dx;
    		y0  += sy;
    	}
    }
}


uint32_t t_wipe;
uint32_t t_dma2d_ready;
uint32_t t_line;
uint32_t t_clean;
uint32_t t_draw_wave;

void draw_wave(plot_t *plot_cfg, int16_t *buffer, uint32_t buffer_size)
{
	draw_window_t w = plot_cfg->window;
    if (!buffer || buffer_size == 0 || w.width == 0 || w.height == 0 || buffer_size != w.width) return;

    uint32_t t0 = DWT_GetCycles();
    //copy_dma2d_window_fast(&plot_cfg->dma2d_cfg, front_buffer, back_buffer, w.x, w.y, w.width, w.height);
    //while(!dma2d_ready); // Wait for DMA2D to finish copying
    //UTIL_LCD_FillRect(w.x, w.y, w.width, w.height, UTIL_LCD_COLOR_WHITE);
    wipe_dma2d_buffer_fast(&plot_cfg->dma2d_cfg, back_buffer, w.x, w.y, w.width, w.height, UTIL_LCD_COLOR_BLACK); // Clear the buffer area using DMA2D
    t_wipe = DWT_GetCycles() - t0;
    //while(!dma2d_ready); // Wait for DMA2D to finish clearing
    t_dma2d_ready = DWT_GetCycles() - t_wipe - t0;

    int32_t mid = (int32_t)w.y + (int32_t)w.height / 2;
    int32_t yp = mid;
    uint32_t xp = w.x;
    uint32_t y_max;
    uint32_t y_min;

    for (uint32_t i = 0; i < w.width; i++)
    {
        int32_t s = buffer[i];
        int32_t y = mid - (s * (int32_t)(w.height / 2)) / INT16_MAX;
        y_max = w.y;
        y_min = w.y + w.height - 1;

        if (y < (int32_t)w.y) y = (int32_t)w.y;
        if (y > (int32_t)(w.y + w.height - 1)) y = (int32_t)(w.y + w.height - 1);

        uint32_t x = w.x + i;
        if (i > 0) {
            //TIL_LCD_DrawLine(xp, (uint32_t)yp, x, (uint32_t)y, UTIL_LCD_COLOR_WHITE);
        	uint32_t lcd_buffer = hlcd_ltdc.LayerCfg[0].FBStartAdress; // Base address of the framebuffer
        	bresnhams_line(back_buffer, xp, yp, x, y, plot_cfg->dma2d_cfg.bpp, UTIL_LCD_COLOR_WHITE); // Draw line directly to SDRAM
        }
        xp = x;
        yp = y;
    }

    t_line = DWT_GetCycles() - t_dma2d_ready - t_wipe - t0;
    uint32_t start_addr = back_buffer + (y_min * LCD_WIDTH * plot_cfg->dma2d_cfg.bpp);
    uint32_t total_bytes = (y_max - y_min + 1) * LCD_WIDTH * plot_cfg->dma2d_cfg.bpp;

    //SCB_CleanDCache_by_Addr((uint32_t*)start_addr, total_bytes);
    SCB_CleanDCache_by_Addr((uint32_t*)back_buffer, (LCD_WIDTH * LCD_HEIGHT * plot_cfg->dma2d_cfg.bpp));
    t_clean = DWT_GetCycles() - t_line - t_dma2d_ready - t_wipe - t0;
    swap_buffers();
}


void draw_minmax(int16_t *buffer, uint32_t buffer_size, draw_window_t w)
{
    if (!buffer || (buffer_size < 2) || (buffer_size & 1u)) return;

    UTIL_LCD_FillRect(w.x, w.y, w.width, w.height, UTIL_LCD_COLOR_WHITE);

    uint32_t mid = (int32_t)w.y + (int32_t)w.height / 2;
    uint32_t n_groups = buffer_size / 2u;

    for (uint32_t px = 0; px < w.width; px+=2)
    {
        uint32_t g = (uint32_t)(((uint64_t)px * (n_groups - 1u)) / (w.width - 1u));

        int32_t mn = buffer[2*g];
        int32_t mx = buffer[2*g + 1u];

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

/**
 * @brief should use plot_segment to plot first half and second half of the buffer interchangibly
 *
 * @param sampleBuffer
 * @param window
 */
// todo: verify if it plots the buffers interchangibly
void draw_buffer(audio_plotter_handle_t *h, int16_t *buff, uint32_t buffSize)
{
	h->plot.window.y = h->main_window.y;
	h->plot.window.width = h->main_window.width / 2;
	h->plot.window.height = h->main_window.height;

	draw_window_t other_half = h->plot.window;

	if (toggle)
	{
		h->plot.window.x = h->main_window.x;
		other_half.x = h->main_window.x + h->plot.window.width; // The half we AREN'T drawing
		toggle = 0;
	}
	else
	{
		h->plot.window.x = h->main_window.x + h->main_window.width / 2;
		other_half.x = h->main_window.x; // The half we AREN'T drawing
		toggle = 1;
	}

	// 1. Sync the OTHER half from front to back so it isn't lost
	copy_dma2d_window_fast(&h->plot.dma2d_cfg, front_buffer, back_buffer,
					  other_half.x, other_half.y, other_half.width, other_half.height);
	while(!dma2d_ready);

	// 2. Draw the new half
	draw_wave(&h->plot, buff, buffSize);
  //plot_segment_minmax(h->plot.buffer, h->plot.buffer_size, h->plot.window);
}

uint32_t t_cpydma2d;
uint32_t t_dma2d_ready_seg;
uint32_t t0seg;
void draw_segment(audio_plotter_handle_t *h)
{
    uint32_t segment_width = h->main_window.width / h->n_segments;

    h->plot.window.x = h->main_window.x + (h->current_segment * segment_width);
    h->plot.window.y = h->main_window.y;
    h->plot.window.width = segment_width;
    h->plot.window.height = h->main_window.height;
    t0seg = DWT_GetCycles();
    copy_dma2d_window_fast(&h->plot.dma2d_cfg, front_buffer, back_buffer,
                      h->main_window.x, h->main_window.y,
                      h->main_window.width, h->main_window.height);

    t_cpydma2d = DWT_GetCycles() - t0seg;
    //while(!dma2d_ready); // Wait for DMA2D copy to finish
    t_dma2d_ready_seg = DWT_GetCycles() - t_cpydma2d - t0seg;
    // 3. Draw the new segment (draw_wave will handle the wipe, draw, and cache clean)

    __HAL_LTDC_DISABLE(&hlcd_ltdc);
    draw_wave(&h->plot, h->plot.buffer, h->plot.buffer_size);
    __HAL_LTDC_ENABLE(&hlcd_ltdc);

    t_draw_wave = DWT_GetCycles() - t_dma2d_ready_seg - t_cpydma2d - t0seg;

    // 4. Increment and wrap your segment counter for the next pass
    h->current_segment++;
    if (h->current_segment >= h->n_segments) {
    	h->current_segment = 0;
    }
}


/**
 * @brief Clones the static UI from Buffer A to Buffer B using DMA2D
 */
void copy_bspUI_to_backbuff(plot_t *plot_cfg)
{
    while (!dma2d_ready);
    dma2d_ready = 0;

    dma2d_config_t *cfg = &plot_cfg->dma2d_cfg;

    hlcd_dma2d.Init.Mode         = DMA2D_M2M;
    hlcd_dma2d.Init.ColorMode    = cfg->pixel_format; // <-- NO MORE HARDCODED RGB565!
    hlcd_dma2d.Init.OutputOffset = 0;

    hlcd_dma2d.LayerCfg[1].InputOffset = 0;
    hlcd_dma2d.LayerCfg[1].InputColorMode = cfg->pixel_format; // <-- NO MORE HARDCODED RGB565!
    hlcd_dma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    hlcd_dma2d.LayerCfg[1].InputAlpha = 0xFF;

    if (HAL_DMA2D_Init(&hlcd_dma2d) != HAL_OK) { dma2d_ready = 1; return; }
    if (HAL_DMA2D_ConfigLayer(&hlcd_dma2d, 1) != HAL_OK) { dma2d_ready = 1; return; }

    hlcd_dma2d.XferCpltCallback  = Custom_DMA2D_CompleteCallback;
    hlcd_dma2d.XferErrorCallback = Custom_DMA2D_ErrorCallback;

    if (HAL_DMA2D_Start_IT(&hlcd_dma2d, BUFFER_PING, BUFFER_PONG, LCD_WIDTH, LCD_HEIGHT) != HAL_OK) {
        dma2d_ready = 1;
    }
    while (!dma2d_ready);
}



