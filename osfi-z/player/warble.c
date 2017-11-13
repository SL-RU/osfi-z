#include "warble.h"


/***************** INTERNAL *****************/
static struct codec_api cic;
struct codec_api *ci = &cic;
static WPlayer player;

//Buffer for codec's purposes
static uint32_t
__attribute__ ((section (".ccmram")))
dec_buffer[DEC_BUFFER_MAX/4];
static uint32_t dec_id = 0;
static uint32_t
__attribute__ ((section (".ccmram")))
input_buffer[DEC_INPUT_BUFFER_LEN/4];

/***** MODE_PLAY *****/

#define WMUTEX_REQUEST warble_mutex_request_grant
#define WMUTEX_RELEASE warble_mutex_release_grant


static void playback_set_volume(int volume)
{
    if (volume > 0)
        volume = 0;

    WMUTEX_REQUEST(&player.mutex);
    player.playback_vol_factor = volume;//pow(10, (double)volume / 20.0) * VOL_FACTOR_UNITY;
    WMUTEX_RELEASE(&player.mutex);
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
    warble_hw_insert(ch1, ch2, count, player.format.stereo_mode);
}

static void ci_set_elapsed(unsigned long value)
{
    WMUTEX_REQUEST(&player.mutex);
    player.time_elapsed = value;
    if(player.handlers.ontimeelapsed != 0)
    	player.handlers.ontimeelapsed(&player.current_track, value);

    //printf("e%u\n", player.time_elapsed);
    WMUTEX_RELEASE(&player.mutex);
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
    WMUTEX_REQUEST(&player.mutex);
    ssize_t actual = read(player.current_track.descriptor,
			  ptr, size);
    WMUTEX_RELEASE(&player.mutex);
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
    WMUTEX_REQUEST(&player.mutex);
    if(reqsize > DEC_INPUT_BUFFER_LEN)
	printf("WARNING: request buffer reqsize is too large %ld > %d\n", reqsize, DEC_INPUT_BUFFER_LEN); 
    reqsize = MIN(reqsize, DEC_INPUT_BUFFER_LEN);

    *realsize = read(player.current_track.descriptor,
		     input_buffer, reqsize);
    if(*realsize == 0)
    {
	WMUTEX_RELEASE(&player.mutex);
	return NULL;
    }
    lseek(player.current_track.descriptor, -*realsize, SEEK_CUR);

    WMUTEX_RELEASE(&player.mutex);
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
    uint8_t *b = (uint8_t*)dec_buffer + dec_id;
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
    WMUTEX_REQUEST(&player.mutex);
    //printf("adv %ld\n", amount);
    lseek(player.current_track.descriptor, amount, SEEK_CUR);
    ci->curpos += amount;
    ci->id3->offset = ci->curpos;
    WMUTEX_RELEASE(&player.mutex);
}

/*
 * Seek to a position in the input file.
 *
 * This invalidates buffers returned by request_buffer.
 */
static bool ci_seek_buffer(size_t newpos)
{
    WMUTEX_REQUEST(&player.mutex);
    off_t actual = lseek(player.current_track.descriptor,
			 newpos, SEEK_SET);
    if (actual >= 0)
        ci->curpos = actual;

    //printf("sek %ld\n", newpos);
    WMUTEX_RELEASE(&player.mutex);
    return actual != -1;
}

static void ci_seek_complete(void)
{
    WMUTEX_REQUEST(&player.mutex);
    WMUTEX_RELEASE(&player.mutex);
}

static void ci_set_offset(size_t value)
{
    WMUTEX_REQUEST(&player.mutex);
    ci->id3->offset = value;
    WMUTEX_RELEASE(&player.mutex);
}

static void ci_configure(int setting, intptr_t value)
{
    WMUTEX_REQUEST(&player.mutex);
    printf("ci configure\n");
    if (setting == DSP_SET_FREQUENCY
	|| setting == DSP_SET_FREQUENCY)
    {
	player.format.freq = value;
	printf("dsp set freq %d\n", value);
    }
    else if (setting == DSP_SET_SAMPLE_DEPTH)
    {
	printf("dsp set depth %d\n", value);
	player.format.depth = value;
    }
    else if (setting == DSP_SET_STEREO_MODE) {
	printf("dsp set stereo %d\n", (value == STEREO_MONO) ? 1 : 2);
	player.format.stereo_mode = value;
	player.format.channels = (value == STEREO_MONO) ? 1 : 2;
    }
    WMUTEX_RELEASE(&player.mutex);
}

static enum codec_command_action ci_get_command(intptr_t *param)
{
    WMUTEX_REQUEST(&player.mutex);
    enum codec_command_action ret = player.codec_action;
    *param = player.codec_action_param;
    player.codec_action = CODEC_ACTION_NULL;
    player.codec_action_param = 0;
    WMUTEX_RELEASE(&player.mutex);

    if(ret == CODEC_ACTION_PAUSE)
    {
	warble_hw_stop();
	uint32_t was = 0;
	while (!was) {
	    WMUTEX_REQUEST(&player.mutex);
	    ret = player.codec_action;
	    *param = player.codec_action_param;
	    player.codec_action = CODEC_ACTION_NULL;
	    player.codec_action_param = 0;
	    WMUTEX_RELEASE(&player.mutex);
	    if(ret == CODEC_ACTION_HALT ||
	       ret == CODEC_ACTION_PAUSE)
		break;
	    osDelay(50);
	}
	if(ret != CODEC_ACTION_HALT)
	    ret = CODEC_ACTION_NULL;
    }
    return ret;
}

static bool ci_should_loop(void)
{
    WMUTEX_REQUEST(&player.mutex);
    printf("ci should loop\n");
    bool l = player.enable_loop;
    WMUTEX_RELEASE(&player.mutex);
    return l;
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
static void print_mp3entry(const struct mp3entry *id3)
{
    printf("Path: %s\n", id3->path);
    if (id3->title)
    {
	printf("Title: %s\n", id3->title);
    }
    if (id3->artist)
    {
	printf("Artist: %s\n", id3->artist);
    }
    if (id3->album)
    {
	
	printf("Album: %ld\n", id3->frequency);
    }
    if (id3->genre_string)
    {
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

void warble_decode_file()
{
    WMUTEX_REQUEST(&player.mutex);
    playback_set_volume(10);
    dec_id = 0;
    player.codec_action = CODEC_ACTION_NULL;
    
    /* Open file */
    printf("open file\n");
    player.current_track.descriptor = open(player.current_track.path, O_RDONLY);
    fseek_init(player.current_track.descriptor);
    if (player.current_track.descriptor == -1) {
	printf("error: open %s\n", player.current_track.path);
    }

    /* Set up ci */
    printf("mp3entry\n");

    if (!get_metadata(&player.current_track.id3, player.current_track.descriptor,
		      player.current_track.path))
    {
        printf("error: metadata parsing failed\n");
	WMUTEX_RELEASE(&player.mutex);
        return;
    }
    print_mp3entry(&player.current_track.id3);
    
    
    ci->filesize = filesize(player.current_track.descriptor);
    ci->id3 = &player.current_track.id3;

    /* Load codec */
    char str[MAX_PATH];
    snprintf(str, sizeof(str), "codecs/%s.codec", audio_formats[player.current_track.id3.codectype].codec_root_fn);
    ci_debugf("Loading %s\n", str);

    /* Run the codec */
    uint8_t res;

    
    if(player.handlers.gotmetadata != 0)
    	player.handlers.gotmetadata(&player.current_track);

    uint32_t ct = player.current_track.id3.codectype;
    WMUTEX_RELEASE(&player.mutex);
    
    if(ct == AFMT_MPA_L3)
	res = mpa_codec_main(CODEC_LOAD);
    else if(ct == AFMT_PCM_WAV)
	res = wav_codec_main(CODEC_LOAD);
    else if(ct == AFMT_AIFF)
	res = aiff_codec_main(CODEC_LOAD);
    else if(ct == AFMT_FLAC)
	res = flac_codec_main(CODEC_LOAD);
    else
    {
	printf("Codec wrong %d!\n", ct);
	exit(1);
    }
    
    printf("codec_main %d\n", res);

    WMUTEX_REQUEST(&player.mutex);
    if(player.handlers.onstart != 0)
    	player.handlers.onstart(&player.current_track);
    WMUTEX_RELEASE(&player.mutex);
    
    if(res != CODEC_OK)
    {
        printf("error: codec returned error from codec_main\n");
        exit(1);
    }
    if(ct == AFMT_MPA_L3)
	res = mpa_codec_run();
    if(ct == AFMT_PCM_WAV)
	res = wav_codec_run();
    else if(ct == AFMT_AIFF)
	res = aiff_codec_run();
    else if(ct == AFMT_FLAC)
	res = flac_codec_run();
    
    printf("proc %d\n", res);
    if (res != CODEC_OK) {
        printf("error: codec error\n");
    }
    
    WMUTEX_REQUEST(&player.mutex);

    warble_hw_stop();
    //if (player.current_track.descriptor != STDIN_FILENO)
    close(player.current_track.descriptor);
    if(player.handlers.onend != 0)
	player.handlers.onend(&player.current_track);    

    WMUTEX_RELEASE(&player.mutex);
}

void warble_play_file(char *file)
{
    WMUTEX_REQUEST(&player.mutex);
    strncpy(player.current_track.path, file, MAX_PATH);
    player.codec_action = CODEC_ACTION_NULL;
    WMUTEX_RELEASE(&player.mutex);
    
    warble_hw_start_thread();
}


int warble_init()
{
    warble_mutex_create(&player.mutex);

    player.handlers.ontimeelapsed = 0;
    player.handlers.gotmetadata = 0;
    player.handlers.onstart = 0;
    player.handlers.onend = 0;
    
    warble_hw_init();
    playback_set_volume(10);
    return 0;
}

WPlayer * warble_get_player()
{
    return &player;
}

void warble_set_onend(void (*onend)(WTrack *track))
{
    WMUTEX_REQUEST(&player.mutex);
    player.handlers.onend = onend;
    WMUTEX_RELEASE(&player.mutex);
}
void warble_set_onstart(void (*onstart)(WTrack *track))
{
    WMUTEX_REQUEST(&player.mutex);
    player.handlers.onstart = onstart;
    WMUTEX_RELEASE(&player.mutex);
}
void warble_set_gotmetadata(void (*gotmetadata)(WTrack *track))
{
    WMUTEX_REQUEST(&player.mutex);
    player.handlers.gotmetadata = gotmetadata;
    WMUTEX_RELEASE(&player.mutex);
}
void warble_set_ontimeelapsed(void (*ontimeelapsed)(WTrack *track, uint32_t time))
{
    WMUTEX_REQUEST(&player.mutex);
    player.handlers.ontimeelapsed = ontimeelapsed;
    WMUTEX_RELEASE(&player.mutex);
}

void warble_stop()
{
    WMUTEX_REQUEST(&player.mutex);
    player.codec_action = CODEC_ACTION_HALT;
    WMUTEX_RELEASE(&player.mutex);
}
void warble_seek(int32_t time)
{
    WMUTEX_REQUEST(&player.mutex);
    player.codec_action = CODEC_ACTION_SEEK_TIME;
    if((int32_t)player.time_elapsed + time < 0)
	player.codec_action_param = 0;
    else
	player.codec_action_param = player.time_elapsed + time;

    WMUTEX_RELEASE(&player.mutex);
}
void warble_pause()
{
    WMUTEX_REQUEST(&player.mutex);
    player.codec_action = CODEC_ACTION_PAUSE;
    WMUTEX_RELEASE(&player.mutex);
}
