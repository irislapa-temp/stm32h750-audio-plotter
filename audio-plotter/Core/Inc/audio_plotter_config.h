/*
 * audio_plotter_config.h
 *
 *  Created on: Sep 14, 2026
 *      Author: irinej
 */

#ifndef INC_AUDIO_PLOTTER_CONFIG_H_
#define INC_AUDIO_PLOTTER_CONFIG_H_



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
#define AUDIO_QUEUE_SIZE 16u

#define AUDIO_QUEUE_SIZE DEFAULT_MAX_SEGMENTS

#endif /* INC_AUDIO_PLOTTER_CONFIG_H_ */
