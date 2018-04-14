#ifndef WARBLE_H
#define WARBLE_H
#include "warble_hw.h"
#include <sys/types.h>
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
#include "stdio_fatfs.h"

#define DEC_INPUT_BUFFER_LEN 26*1024
#define DEC_BUFFER_MAX 38*1024

#define VOL_FRACBITS 31
#define VOL_FACTOR_UNITY (1u << VOL_FRACBITS)

typedef enum WResult {
    WOK = 0,
    WERR
} WResult;

typedef struct {
    TCHAR path[MAX_PATH];
    int descriptor;
    struct mp3entry id3;
} WTrack;

typedef struct {
    WTrack current_track;

    struct {
	enum codec_command_action type;
	int64_t param;
    } action;
    
    //audio format of current track
    struct {
	intptr_t freq;
	intptr_t stereo_mode;
	intptr_t depth;
	int channels;
    } format;
    
    bool enable_loop;

    uint32_t volume; /* From 0 to 99*/
    uint32_t playback_vol_factor;
    
    uint32_t time_elapsed; //elapsed time of current track

    struct {
	void (*onend)(WTrack *track);
	void (*onstart)(WTrack *track);
	void (*gotmetadata)(WTrack *track);
	void (*ontimeelapsed)(WTrack *track, uint32_t time);
    } handlers; //handlers of specific events

    W_MUTEX_t mutex; //main player mutex
} WPlayer;

/**
 * Init playback(hardware, mutexes and etc)
 * @return 
 */
int warble_init();
/**
 * Start playback. Run thread and etc
 *
 * @param file relative path of file
 * @return 0 if OK
 */
void warble_play_file(TCHAR *file);
/**
 * Get player structure
 * @return pointer to WPlayer structure
 */
WPlayer* warble_get_player();

/**
 * DO NOT USE. Interface only for warble_hw.c. Main decode function for the thread.
 * @return pointer to WPlayer structure
 */
void warble_decode_file();

/**
 * Set handler of onend event. It will called when track was over
 * @param onend pointer to function
 * @return 
 */
void warble_set_onend(void (*onend)(WTrack *track));
/**
 * Set handler of onstart event. It will called when player was initialized and before starting playback.
 * @param onstart pointer to function
 * @return 
 */
void warble_set_onstart(void (*onstart)(WTrack *track));
/**
 * Set handler of gotmetadata event. It will called when metadata of current track was calculated
 * @param gotmetadata pointer to function
 * @return 
 */
void warble_set_gotmetadata(void (*gotmetadata)(WTrack *track));
/**
 * Set handler of ontimeelapsed event. It will called when playback time was changed
 * @param ontimeelapsed pointer to function
 * @return 
 */
void warble_set_ontimeelapsed(void (*ontimeelapsed)(WTrack *track, uint32_t time));



/**
 * Halt playback
 * @return 
 */
void warble_stop();

/**
 * Pause/continue playback
 * @return 
 */
void warble_pause();
/**
 * Seek time through track
 * @return 
 */
void warble_seek(int32_t time);

#endif
