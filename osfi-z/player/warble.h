#ifndef WARBLE_H
#define WARBLE_H
#include <sys/types.h>
#include <fcntl.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "core_alloc.h"
#include "codecs.h"
#include "metadata.h"
#include "platform.h"
#include "i2s.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "dsp_core.h"

typedef struct {
    int descriptor;
    struct mp3entry *id3;
} WTrack;

typedef struct {
    WTrack current_track;
    WTrack next_track;
    
    enum codec_command_action codec_action;
    intptr_t codec_action_param;
    uint32_t num_output_samples;
    
    uint32_t time_elapsed;
} WPlayer;

int dmain();
#endif
