#include "stm32f4xx_hal.h"
#include <math.h>
#include "ff.h"
#include "mp3dec.h"
#include <string.h>

#define MP3_FILEBUFF_SIZE	4096
// размер файлового буфера
#define DAC_BUFFER_SIZE		(1152*2)

void mp3_init(void);

void mp3_fopen(FIL* f);

uint16_t * mp3_getBuf(void);

void mp3_swch(uint8_t half);

void mp3_decode(void);
