#include "warble.h"
#include "fm.h"

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

#define PLAYBACK_BUFFER_SIZE 4096
static bool playback_running = false;
static uint16_t playback_buffer[2][PLAYBACK_BUFFER_SIZE];
static int playback_decode_ind;
static int playback_decode_pos;

//Buffer for codec's purposes
#define DEC_BUFFER_MAX 38*1024
static uint8_t
__attribute__ ((section (".ccmram")))
dec_buffer[DEC_BUFFER_MAX];
static uint32_t dec_id = 0;
#define DEC_INPUT_BUFFER_LEN 26*1024
static uint32_t
__attribute__ ((section (".ccmram")))
input_buffer[DEC_INPUT_BUFFER_LEN/4];




static void playback_init(void)
{
    mode = MODE_PLAY;

    playback_decode_ind = 0;
    playback_decode_pos = 0;

    dec_id = 0;
}


static void playback_set_volume(int volume)
{
    if (volume > 0)
        volume = 0;

    playback_vol_factor = pow(10, (double)volume / 20.0) * VOL_FACTOR_UNITY;
}

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
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
    printf("Playback start\n");
    playback_running = true;
    
    HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t*)playback_buffer, PLAYBACK_BUFFER_SIZE*2);
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
}


/***** ALL MODES *****/

static void *ci_codec_get_buffer(size_t *size)
{
    static char buffer[2 * 1024];
    char *ptr = buffer;
    printf("get_buffer\n");
    *size = sizeof(buffer);
    if ((intptr_t)ptr & (CACHEALIGN_SIZE - 1))
        ptr += CACHEALIGN_SIZE - ((intptr_t)ptr & (CACHEALIGN_SIZE - 1));
    return ptr;
}

static void ci_pcmbuf_insert(const void *ch1, const void *ch2, int count)
{
    num_output_samples += count;
    int i;
    if(format.stereo_mode == STEREO_INTERLEAVED)
    {
	count *= 2;
    }

    for (i = 0; i < count; i ++) {
	playback_buffer[playback_decode_ind][playback_decode_pos] =
	    (uint16_t)(((uint32_t*)ch1)[i] >> 13);
	playback_decode_pos ++;
	if(format.stereo_mode == STEREO_NONINTERLEAVED)
	{
	    playback_buffer[playback_decode_ind][playback_decode_pos] =
		(uint16_t)(((uint32_t*)ch2)[i] >> 13);
	    playback_decode_pos ++;
	}
	    
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

static void ci_set_elapsed(unsigned long value)
{
    //printf("Time elapsed: %lu\n", value);
}


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
//    if (!rbcodec_format_is_atomic(ci->id3->codectype))
    if(reqsize > DEC_INPUT_BUFFER_LEN)
	printf("WARNING: request buffer reqsize is too large %ld > %d\n", reqsize, DEC_INPUT_BUFFER_LEN); 
    reqsize = MIN(reqsize, DEC_INPUT_BUFFER_LEN);
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
	printf("ERROR: request dec buffer %ld %d\n", reqsize, DEC_BUFFER_MAX - dec_id);
	reqsize = *realsize = DEC_BUFFER_MAX - dec_id;
    }
    printf("req %d %ld ", dec_id, reqsize);
    uint8_t *b = dec_buffer + dec_id;
    *realsize = reqsize;
    dec_id += reqsize;
    while (dec_id % 4) //align(4) !!!
	dec_id ++;
    printf("%d\n", dec_id);
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
    off_t actual = lseek(input_fd, newpos, SEEK_SET);
    if (actual >= 0)
        ci->curpos = actual;
    return actual != -1;
}

static void ci_seek_complete(void)
{
}

static void ci_set_offset(size_t value)
{
    ci->id3->offset = value;
}

static void ci_configure(int setting, intptr_t value)
{
    printf("ci configure\n");
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

static enum codec_command_action ci_get_command(intptr_t *param)
{
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
};


static char alb[20] = "Album: ";
static char tit[20] = "Title: ";
static char art[20] = "Artist: ";

static void print_mp3entry(const struct mp3entry *id3)
{
    printf("Path: %s\n", id3->path);
    if (id3->title)
    {
	snprintf(tit, 20, "Title: %s", id3->title);
	printf("Title: %s\n", id3->title);
    }
    if (id3->artist)
    {
	snprintf(tit, 20, "Artist: %s", id3->artist);
	printf("Artist: %s\n", id3->artist);
    }
    if (id3->album)
    {
	
	printf("Album: %ld\n", id3->frequency);
    }
    if (id3->genre_string)
    {
	snprintf(tit, 20, "Genre: %s", id3->genre_string);
	printf("Genre: %s\n", id3->genre_string);
    }
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
    snprintf(alb, 20, "Freq: %ld Hz", id3->frequency);	
    if (id3->id3v2len) printf("ID3v2 length: %lu\n", id3->id3v2len);
    if (id3->id3v1len) printf("ID3v1 length: %lu\n", id3->id3v1len);
    if (id3->first_frame_offset) printf("First frame offset: %lu\n", id3->first_frame_offset);
    printf("File size without headers: %lu\n", id3->filesize);
    printf("Song length: %lu ms\n", id3->length);
    snprintf(tit, 20, "Len: %ld s", id3->length/1000);	
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

    fm_cre(art, tit, alb);
    

}

static void decode_file(const char *input_fn)
{    
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
    print_mp3entry(&id3);
    
    ci->filesize = filesize(input_fd);
    ci->id3 = &id3;

    /* Load codec */
    char str[MAX_PATH];
    snprintf(str, sizeof(str), "codecs/%s.codec", audio_formats[id3.codectype].codec_root_fn);
    debugf("Loading %s\n", str);

    /* Run the codec */
    uint8_t res;
    if(id3.codectype == 3)
	res = mpa_codec_main(CODEC_LOAD);
    else if(id3.codectype == 5)
	res = wav_codec_main(CODEC_LOAD);
    else if(id3.codectype == 4)
	res = aiff_codec_main(CODEC_LOAD);
    else if(id3.codectype == 7)
	res = flac_codec_main(CODEC_LOAD);
    else
    {
	printf("Codec wrong %d!\n", id3.codectype);
	exit(1);
    }
    
    printf("codec_main %d\n", res);
    
    if(res != CODEC_OK)
    {
        printf("error: codec returned error from codec_main\n");
        exit(1);
    }
    if(id3.codectype == 3)
	res = mpa_codec_run();
    if(id3.codectype == 5)
	res = wav_codec_run();
    else if(id3.codectype == 4)
	res = aiff_codec_run();
    else if(id3.codectype == 7)
	res = flac_codec_run();
    
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


char p[1024] = {0};
void w_set_file(char *file)
{
    strncpy(p, file, 1024);
}

int dmain()
{
    printf("playback init...\n");
    playback_init();
    
    printf("playback volume...\n");
    playback_set_volume(10);
    printf("playback decode...\n");
    decode_file(p);

    playback_quit();

    return 0;
}
