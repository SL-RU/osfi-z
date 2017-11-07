#ifndef WARBLE_H
#define WARBLE_H
#include <sys/types.h>
#include <fcntl.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codecs.h"
#include "metadata.h"
#include "platform.h"
#include "dsp_core.h"
#include "makise_config.h"
#include "warble_hw.h"

#define DEC_INPUT_BUFFER_LEN 26*1024
#define DEC_BUFFER_MAX 38*1024

#define VOL_FRACBITS 31
#define VOL_FACTOR_UNITY (1u << VOL_FRACBITS)

typedef struct {
    char path[MAX_PATH];
    int descriptor;
    struct mp3entry id3;
} WTrack;

typedef struct {
    WTrack current_track;
    
    enum codec_command_action codec_action;
    intptr_t codec_action_param;
    uint32_t num_output_samples;

    struct {
	intptr_t freq;
	intptr_t stereo_mode;
	intptr_t depth;
	int channels;
    } format;
    
    bool enable_loop;

    uint32_t volume; /* From 0 to 99*/
    uint32_t playback_vol_factor;
    
    uint32_t time_elapsed;

    MAKISE_MUTEX_t mutex;
    
} WPlayer;

int warble_init();
void warble_play_file(char *file);
int warble_set_track(WTrack current, WTrack next);
WPlayer * warble_get_player();

#endif
