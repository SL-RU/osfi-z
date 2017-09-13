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
//#include "dsp_core.h"
#include "metadata.h"
//#include "tdspeed.h"
#include "platform.h"
#include "load_code.h"
#include "i2s.h"
#include "FreeRTOS.h"
#include "semphr.h"

extern char _plug_start[];
extern char _plug_end[];


void debugf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

void panicf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    exit (-1);
}

int find_first_set_bit(uint32_t value)
{
    if (value == 0)
        return 32;
    return __builtin_ctz(value);
}

off_t filesize(int fd)
{
    struct stat st;
    fstat(fd, &st);
    return st.st_size;
}



/***************** INTERNAL *****************/

static enum { MODE_PLAY, MODE_WRITE } mode;
static bool use_dsp = false;
static bool enable_loop = false;
static const char *config = "";

/* Volume control */
#define VOL_FRACBITS 31
#define VOL_FACTOR_UNITY (1u << VOL_FRACBITS)
static uint32_t playback_vol_factor = VOL_FACTOR_UNITY;

static int input_fd;
static enum codec_command_action codec_action;
static intptr_t codec_action_param = 0;
static unsigned long num_output_samples = 0;
static struct codec_api cic;
struct codec_api *ci = &cic;

xSemaphoreHandle xI2S_semaphore;
xSemaphoreHandle xI2S_semaphore_h;

static struct {
    intptr_t freq;
    intptr_t stereo_mode;
    intptr_t depth;
    int channels;
} format;


/***** MODE_PLAY *****/

/* MODE_PLAY uses a double buffer: one half is read by the playback thread and
 * the other half is written to by the main thread. When a thread is done with
 * its current half, it waits for the other thread and then switches. The main
 * advantage of this method is its simplicity; the main disadvantage is that it
 * has long latency. ALSA buffer underruns still occur sometimes, but this is
 * SDL's fault. */

#define PLAYBACK_BUFFER_SIZE 2048
static bool playback_running = false;
static uint16_t playback_buffer[2][PLAYBACK_BUFFER_SIZE];
static int playback_play_ind, playback_decode_ind;
static int playback_play_pos, playback_decode_pos;
#define DEC_BUFFER_MAX 30*1024
static uint8_t  __attribute__ ((section (".ccram"))) dec_buffer[DEC_BUFFER_MAX];
static uint32_t dec_id = 0;



static void playback_init(void)
{
    mode = MODE_PLAY;
    /* if (SDL_Init(SDL_INIT_AUDIO)) { */
    /*     printf("error: Can't initialize SDL: %s\n", SDL_GetError()); */
    /*     exit(1); */
    /* } */
    playback_play_ind = 0;
    playback_play_pos = 0;
    playback_decode_ind = 0;
    playback_decode_pos = 0;
    /* playback_play_sema = SDL_CreateSemaphore(0); */
    /* playback_decode_sema = SDL_CreateSemaphore(0); */
}


static void playback_set_volume(int volume)
{
    if (volume > 0)
        volume = 0;

    playback_vol_factor = pow(10, (double)volume / 20.0) * VOL_FACTOR_UNITY;
}

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    //printf("h\n");
    static BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xI2S_semaphore_h, &xHigherPriorityTaskWoken);
    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}
void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    //printf("c\n");
    static BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xI2S_semaphore, &xHigherPriorityTaskWoken);
    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}


static void playback_start(void)
{
    playback_running = true;
    /* SDL_AudioSpec spec = {0}; */
    
    HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t*)playback_buffer, PLAYBACK_BUFFER_SIZE*2);
    printf("buf trans\n");
    /* spec.format = AUDIO_S16SYS; */
    /* spec.channels = 2; */
    /* spec.samples = 0x400; */
    /* spec.callback = playback_callback; */
    /* spec.userdata = NULL; */
    /* if (SDL_OpenAudio(&spec, NULL)) { */
    /*     printf("error: Can't open SDL audio: %s\n", SDL_GetError()); */
    /*     exit(1); */
    /* } */
    /* SDL_PauseAudio(0); */
}

static void playback_quit(void)
{
    printf("Playback stop\n");
    if (!playback_running)
        playback_start();
    memset(playback_buffer[playback_decode_ind] + playback_decode_pos, 0,
           PLAYBACK_BUFFER_SIZE - playback_decode_pos);
    playback_running = false;
    HAL_I2S_DMAStop(&hi2s3);
    /* SDL_SemPost(playback_decode_sema); */
    /* SDL_SemWait(playback_play_sema); */
    /* SDL_SemWait(playback_play_sema); */
    /* SDL_Quit(); */
}


/***** ALL MODES *****/

static void perform_config(void)
{
    /* TODO: equalizer, etc. */
    while (config) {
        const char *name = config;
        const char *eq = strchr(config, '=');
        if (!eq)
            break;
        const char *val = eq + 1;
        const char *end = val + strcspn(val, ": \t\n");

        if (!strncmp(name, "wait=", 5)) {
            if (atoi(val) > num_output_samples)
                return;
        } else if (!strncmp(name, "dither=", 7)) {
            //dsp_dither_enable(atoi(val) ? true : false);
        } else if (!strncmp(name, "halt=", 5)) {
            if (atoi(val))
                codec_action = CODEC_ACTION_HALT;
        } else if (!strncmp(name, "loop=", 5)) {
            enable_loop = atoi(val) != 0;
        } else if (!strncmp(name, "offset=", 7)) {
            ci->id3->offset = atoi(val);
        } else if (!strncmp(name, "rate=", 5)) {
            //dsp_set_pitch(atof(val) * PITCH_SPEED_100);
        } else if (!strncmp(name, "seek=", 5)) {
            codec_action = CODEC_ACTION_SEEK_TIME;
            codec_action_param = atoi(val);
        } else if (!strncmp(name, "tempo=", 6)) {
            //dsp_set_timestretch(atof(val) * PITCH_SPEED_100);
        } else if (!strncmp(name, "vol=", 4)) {
            playback_set_volume(atoi(val));
        } else {
            printf("error: unrecognized config \"%.*s\"\n",
		   (int)(eq - name), name);
            exit(1);
        }

        if (*end)
            config = end + 1;
        else
            config = NULL;
    }
}

static void *ci_codec_get_buffer(size_t *size)
{
    static char buffer[2 * 1024];
    printf("get buffer\n", size);
    char *ptr = buffer;
    *size = sizeof(buffer);
    if ((intptr_t)ptr & (CACHEALIGN_SIZE - 1))
        ptr += CACHEALIGN_SIZE - ((intptr_t)ptr & (CACHEALIGN_SIZE - 1));
    return ptr;
}

static void ci_pcmbuf_insert(const void *ch1, const void *ch2, int count)
{
    //printf("insert\n");
    num_output_samples += count;
    if (use_dsp) {

    } else {
        //Convert to 32-bit interleaved.
        //count *= format.channels;
        int i;
	//count;

	for (i = 0; i < count; i ++) {

	    playback_buffer[playback_decode_ind][playback_decode_pos] =
		(uint16_t)(((uint32_t*)ch1)[i] >> 13);
            playback_decode_pos ++;
	    playback_buffer[playback_decode_ind][playback_decode_pos] =
	    	(uint16_t)(((uint32_t*)ch2)[i] >> 13);
            playback_decode_pos ++;

	    
	    if(playback_decode_pos >= PLAYBACK_BUFFER_SIZE)
	    {
		if (!playback_running && playback_decode_ind)
		    playback_start();
		if (playback_running && playback_decode_ind)
		    xSemaphoreTake(xI2S_semaphore_h, portMAX_DELAY);
		if (playback_running && !playback_decode_ind)
		    xSemaphoreTake(xI2S_semaphore, portMAX_DELAY);

		
		playback_decode_pos = 0;
		playback_decode_ind = !playback_decode_ind;
	    }
	    
	}

    }

    perform_config();
}

static void ci_set_elapsed(unsigned long value)
{
    //debugf("Time elapsed: %lu\n", value);
}

static char __attribute__ ((section (".ccram"))) input_buffer[6*1024];

/*
 * Read part of the input file into a provided buffer.
 *
 * The entire size requested will be provided except at the end of the file.
 * The current file position will be moved, just like with advance_buffer, but
 * the offset is not updated. This invalidates buffers returned by
 * request_buffer.
 */
static size_t ci_read_filebuf(void *ptr, size_t size)
{
    //free(input_buffer);
    //input_buffer = NULL;
    //printf("read file buf\n");

    ssize_t actual = read(input_fd, ptr, size);
    if (actual < 0)
        actual = 0;
    ci->curpos += actual;
    return actual;
}

/*
 * Request a buffer containing part of the input file.
 *
 * The size provided will be the requested size, or the remaining size of the
 * file, whichever is smaller. Packet audio has an additional maximum of 32
 * KiB. The returned buffer remains valid until the next time read_filebuf,
 * request_buffer, advance_buffer, or seek_buffer is called.
 */
static void *ci_request_buffer(size_t *realsize, size_t reqsize)
{
    //free(input_buffer);
    if (!rbcodec_format_is_atomic(ci->id3->codectype))
        reqsize = MIN(reqsize, 6 * 1024);
    //printf("Request buffer size: %lu\n", reqsize);
    //input_buffer = malloc(reqsize);
    *realsize = read(input_fd, input_buffer, reqsize);
    if (*realsize < 0)
        *realsize = 0;
    lseek(input_fd, -*realsize, SEEK_CUR);
    return input_buffer;
}
void* request_dec_buffer(size_t *realsize, size_t reqsize)
{
    if(dec_id + reqsize > DEC_BUFFER_MAX)
    {
	printf("ERROR: request dec buffer %d %d\n", reqsize, DEC_BUFFER_MAX - dec_id);
	reqsize = *realsize = DEC_BUFFER_MAX - dec_id;
    }
    printf("req %d %ld", dec_id, reqsize);
    uint8_t *b = &dec_buffer[dec_id];
    dec_id += reqsize;
    return b;
}
/*
 * Advance the current position in the input file.
 *
 * This automatically updates the current offset. This invalidates buffers
 * returned by request_buffer.
 */
static void ci_advance_buffer(size_t amount)
{
    //free(input_buffer);
    //input_buffer = NULL;
    //printf("Advance buffer %d\n", amount);

    lseek(input_fd, amount, SEEK_CUR);
    ci->curpos += amount;
    ci->id3->offset = ci->curpos;
}

/*
 * Seek to a position in the input file.
 *
 * This invalidates buffers returned by request_buffer.
 */
static bool ci_seek_buffer(size_t newpos)
{
    /* free(input_buffer); */
    /* input_buffer = NULL; */
    //printf("seek buffer\n");

    off_t actual = lseek(input_fd, newpos, SEEK_SET);
    if (actual >= 0)
        ci->curpos = actual;
    return actual != -1;
}

static void ci_seek_complete(void)
{
    //printf("ci seek completed\n");
}

static void ci_set_offset(size_t value)
{
    //printf("ci set offset\n");
    ci->id3->offset = value;
}

static void ci_configure(int setting, intptr_t value)
{
    printf("ci configure\n");
    if (use_dsp) {
        //dsp_configure(ci->dsp, setting, value);
    } else {
        if (setting == DSP_SET_FREQUENCY
	    || setting == DSP_SET_FREQUENCY)
	{
            format.freq = value;
	    printf("dsp set freq %d\n", value);
	}
        else if (setting == DSP_SET_SAMPLE_DEPTH)
	{
	    printf("dsp set depth %d\n", value);
	    format.depth = value;
	}
        else if (setting == DSP_SET_STEREO_MODE) {
	    printf("dsp set stereo %d\n", (value == STEREO_MONO) ? 1 : 2);
            format.stereo_mode = value;
            format.channels = (value == STEREO_MONO) ? 1 : 2;
        }
    }
}

static enum codec_command_action ci_get_command(intptr_t *param)
{
    //printf("ci get command\n");
    enum codec_command_action ret = codec_action;
    *param = codec_action_param;
    codec_action = CODEC_ACTION_NULL;
    return ret;
}

static bool ci_should_loop(void)
{
    printf("ci should loop\n");
    return enable_loop;
}

static unsigned ci_sleep(unsigned ticks)
{
    printf("ci sleep\n");
    return 0;
}

static void ci_debugf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

#ifdef ROCKBOX_HAS_LOGF
static void ci_logf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    putc('\n', stderr);
    va_end(ap);
}
#endif

static void stub_void_void(void) { }

static struct codec_api cic = {

    0,                   /* filesize */
    0,                   /* curpos */
    NULL,                /* id3 */
    -1,                  /* audio_hid */
    NULL,                /* struct dsp_config *dsp */
    ci_codec_get_buffer,
    ci_pcmbuf_insert,
    ci_set_elapsed,
    ci_read_filebuf,
    ci_request_buffer,
    request_dec_buffer,
    ci_advance_buffer,
    ci_seek_buffer,
    ci_seek_complete,
    ci_set_offset,
    ci_configure,
    ci_get_command,
    ci_should_loop,

    ci_sleep,
    stub_void_void, /* yield */

#if NUM_CORES > 1
    ci_create_thread,
    ci_thread_thaw,
    ci_thread_wait,
    ci_semaphore_init,
    ci_semaphore_wait,
    ci_semaphore_release,
#endif

    stub_void_void, /* commit_dcache */
    stub_void_void, /* commit_discard_dcache */
    stub_void_void, /* commit_discard_idcache */

    /* strings and memory */
    strcpy,
    strlen,
    strcmp,
    strcat,
    memset,
    memcpy,
    memmove,
    memcmp,
    memchr,
#if defined(DEBUG) || defined(SIMULATOR)
    ci_debugf,
#endif
#ifdef ROCKBOX_HAS_LOGF
    ci_logf,
#endif

    qsort,

#ifdef HAVE_RECORDING
    ci_enc_get_inputs,
    ci_enc_set_parameters,
    ci_enc_get_chunk,
    ci_enc_finish_chunk,
    ci_enc_get_pcm_data,
    ci_enc_unget_pcm_data,

    /* file */
    open,
    close,
    read,
    lseek,
    write,
    ci_round_value_to_list32,

#endif /* HAVE_RECORDING */
};

static void print_mp3entry(const struct mp3entry *id3, FILE *f)
{
    printf("Path: %s\n", id3->path);
    if (id3->title) printf("Title: %s\n", id3->title);
    if (id3->artist) printf("Artist: %s\n", id3->artist);
    if (id3->album) printf("Album: %s\n", id3->album);
    if (id3->genre_string) printf("Genre: %s\n", id3->genre_string);
    if (id3->disc_string || id3->discnum) printf("Disc: %s (%d)\n", id3->disc_string, id3->discnum);
    if (id3->track_string || id3->tracknum) printf("Track: %s (%d)\n", id3->track_string, id3->tracknum);
    if (id3->year_string || id3->year) printf("Year: %s (%d)\n", id3->year_string, id3->year);
    if (id3->composer) printf("Composer: %s\n", id3->composer);
    if (id3->comment) printf("Comment: %s\n", id3->comment);
    if (id3->albumartist) printf("Album artist: %s\n", id3->albumartist);
    if (id3->grouping) printf("Grouping: %s\n", id3->grouping);
    if (id3->layer) printf("Layer: %d\n", id3->layer);
    if (id3->id3version) printf("ID3 version: %u\n", (int)id3->id3version);
    printf("Codec: %s\n", audio_formats[id3->codectype].label);
    printf("Bitrate: %d kb/s\n", id3->bitrate);
    printf("Frequency: %lu Hz\n", id3->frequency);
    if (id3->id3v2len) printf("ID3v2 length: %lu\n", id3->id3v2len);
    if (id3->id3v1len) printf("ID3v1 length: %lu\n", id3->id3v1len);
    if (id3->first_frame_offset) printf("First frame offset: %lu\n", id3->first_frame_offset);
    printf("File size without headers: %lu\n", id3->filesize);
    printf("Song length: %lu ms\n", id3->length);
    if (id3->lead_trim > 0 || id3->tail_trim > 0) printf("Trim: %d/%d\n", id3->lead_trim, id3->tail_trim);
    if (id3->samples) printf("Number of samples: %lu\n", id3->samples);
    if (id3->frame_count) printf("Number of frames: %lu\n", id3->frame_count);
    if (id3->bytesperframe) printf("Bytes per frame: %lu\n", id3->bytesperframe);
    if (id3->vbr) printf("VBR: true\n");
    if (id3->has_toc) printf("Has TOC: true\n");
    if (id3->channels) printf("Number of channels: %u\n", id3->channels);
    if (id3->extradata_size) printf("Size of extra data: %u\n", id3->extradata_size);
    if (id3->needs_upsampling_correction) printf("Needs upsampling correction: true\n");
    /* TODO: replaygain; albumart; cuesheet */
    if (id3->mb_track_id) printf("Musicbrainz track ID: %s\n", id3->mb_track_id);
}

static void decode_file(const char *input_fn)
{
    /* Initialize DSP before any sort of interaction */
    printf("dsp init\n");
    //dsp_init();

    /* /\* Set up global settings *\/ */
    /* memset(&global_settings, 0, sizeof(global_settings)); */
    /* globael_settings.timestretch_enabled = true; */
    printf("dsp timestretch\n");
    //dsp_timestretch_enable(true);

    xI2S_semaphore = xSemaphoreCreateCounting(100, 0);
    xI2S_semaphore_h = xSemaphoreCreateCounting(100, 0);
    
    /* Open file */
    printf("open file\n");
    input_fd = open(input_fn, O_RDONLY);
    if (input_fd == -1) {
	printf(input_fn);
    }

    /* Set up ci */
    printf("mp3entry\n");
    struct mp3entry id3;
    if (!get_metadata(&id3, input_fd, input_fn)) {
        printf("error: metadata parsing failed\n");
        //exit(1);
    }
    print_mp3entry(&id3, stderr);
    ci->filesize = filesize(input_fd);
    ci->id3 = &id3;
    if (use_dsp) {
        /* ci->dsp = dsp_get_config(CODEC_IDX_AUDIO); */
        /* dsp_configure(ci->dsp, DSP_SET_OUT_FREQUENCY, DSP_OUT_DEFAULT_HZ); */
        /* dsp_configure(ci->dsp, DSP_RESET, 0); */
        //dsp_dither_enable(false);
    }
    perform_config();

    /* Load codec */
    char str[MAX_PATH];
    snprintf(str, sizeof(str), "codecs/%s.codec", audio_formats[id3.codectype].codec_root_fn);
    debugf("Loading %s\n", str);
    //void *dlcodec = dlopen(str, RTLD_NOW);
    /* if (!dlcodec) { */
    /*     printf("error: dlopen failed: %s\n", dlerror()); */
    /*     exit(1); */
    /* } */
    
    //struct codec_header *c_hdr = lc_open(str, _plug_start, _plug_end - _plug_start);
    /* if(c_hdr == 1) */
    /* { */
    /* 	printf("Binary doesn't fit into memory\n"); */
    /* 	exit(1); */
    /* } else if(c_hdr == 2) */
    /* { */
    /* 	printf("Could not open file\n"); */
    /* 	exit(1); */
    /* } else if(c_hdr == 3) */
    /* { */
    
    /* 	printf("Could not read from file\n"); */
    /* 	exit(1); */
    /* } */
    /* //c_hdr = dlsym(dlcodec, "__header"); */
    /* if (c_hdr->lc_hdr.magic != CODEC_MAGIC) { */
    /*     printf("error: %s invalid: incorrect magic\n", str); */
    /*     exit(1); */
    /* } */
    /* if (c_hdr->lc_hdr.target_id != TARGET_ID) { */
    /*     printf("error: %s invalid: incorrect target id\n", str); */
    /*     exit(1); */
    /* } */
    /* if (c_hdr->lc_hdr.api_version != CODEC_API_VERSION) { */
    /*     printf("error: %s invalid: incorrect API version\n", str); */
    /*     exit(1); */
    /* } */

    /* Run the codec */
    //*c_hdr->api = &ci;
    uint8_t res;
    //res = flac_codec_main(CODEC_LOAD);
    res = mpa_codec_main(CODEC_LOAD);
    //res = wav_codec_main(CODEC_LOAD);
    printf("codec_main %d\n", res);
    
    if(res != CODEC_OK)
    {
        printf("error: codec returned error from codec_main\n");
        exit(1);
    }
    res = mpa_codec_run();
    //res = flac_codec_run();
    //res = wav_codec_run();
    
    printf("proc %d\n", res);
    if (res != CODEC_OK) {
        printf("error: codec error\n");
    }
    //c_hdr->entry_point(CODEC_UNLOAD);

    /* Close */
    //dlclose(dlcodec);
    if (input_fd != STDIN_FILENO)
        close(input_fd);
}


int dmain()
{
    int opt;
    /* while ((opt = getopt(argc, argv, "c:fhr")) != -1) { */
    /*     switch (opt) { */
    /*     case 'c': */
    /*         config = optarg; */
    /*         break; */
    /*     case 'f': */
    /*         use_dsp = false; */
    /*         break; */
    /*     case 'r': */
    /*         use_dsp = false; */
    /*         write_raw = true; */
    /*         break; */
    /*     case 'h': /\* fallthrough *\/ */
    /*     default: */
    /*         print_help(argv[0]); */
    /*         exit(1); */
    /*     } */
    /* } */

    /* if (argc == optind + 2) { */
    /*     write_init(argv[optind + 1]); */
    /* } else if (argc == optind + 1) { */
    /*     if (!use_dsp) { */
    /*         printf("error: -r can't be used for playback\n"); */
    /*         print_help(argv[0]); */
    /*         exit(1); */
    /*     } */
    /*     //core_allocator_init(); */
    printf("playback init...\n");
    playback_init();
    /* } else { */
    /*     if (argc > 1) */
    /*         printf("error: wrong number of arguments\n"); */
    /*     print_help(argv[0]); */
    /*     exit(1); */
    /* } */
    printf("playback volume...\n");
    playback_set_volume(10);
    printf("playback decode...\n");
    decode_file("/m.mp3");

    playback_quit();

    return 0;
}
