#ifndef WARBLE_HW_H
#define WARBLE_HW_H
#include "i2s.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "dsp_core.h"

#define PLAYBACK_BUFFER_SIZE 4096

uint8_t warble_hw_init();

uint8_t warble_hw_start();
uint8_t warble_hw_stop();

uint8_t warble_hw_insert(const void *ch1, const void *ch2,
			 int count, uint8_t stereo_format);


#endif
