#ifndef PLAY_H
#define PLAY_H

#include "esp_log.h"
#include "esp_err.h"
#include "hal_i2c.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hal_i2s.h"
#include <sys/stat.h>
#include "wm8978.h"
#include "hal_i2c.h"
#include "hal_i2s.h"

#include "freertos/queue.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"


#define GPIO_OUTPUT_IO_0    5
#define GPIO_OUTPUT_PIN_SEL  ((1<<GPIO_OUTPUT_IO_0))


typedef struct 
{
    char rld[4];
    int  rLen;
    char wld[4];
    char fld[4];
 
    int fLen;
 
    short wFormatTag;
    short wChannels;
    int   nSamplesPersec;
    int   nAvgBitsPerSample;
    short wBlockAlign;
    short wBitsPerSample;

    char dld[4];
    int  wSampleLength;
 }WAV_HEADER;

void aplay_wav(char* filename);
void aplay_mp3(char* filename);
void WM8978_config();
void audio_task(void *parameter);
 #endif

