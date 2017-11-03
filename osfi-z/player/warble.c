#include "warble.h"
#include "fm.h"


/***************** INTERNAL *****************/
static WPlayer player;

static struct codec_api cic;
struct codec_api *ci = &cic;

//Buffer for codec's purposes
static uint8_t
    __attribute__ ((section (".ccmram")))
    dec_buffer[DEC_BUFFER_MAX];
static uint32_t dec_id = 0;
static uint32_t
    __attribute__ ((section (".ccmram")))
    input_buffer[DEC_INPUT_BUFFER_LEN/4];

/***** MODE_PLAY *****/




static void playback_init(void)
{
    warble_hw_init();
}


static void playback_set_volume(int volume)
{
    if (volume > 0)
        volume = 0;

    player.playback_vol_factor = pow(10, (double)volume / 20.0) * VOL_FACTOR_UNITY;
}



static void playback_quit(void)
{
    warble_hw_stop();
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

    ssize_t actual = read(player.current_track.descriptor,
			  ptr, size);
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
    *realsize = read(player.current_track.descriptor,
		     input_buffer, reqsize);
    //if ((int32_t)(*realsize) < 0)
    //    *realsize = 0;
    lseek(player.current_track.descriptor, -*realsize, SEEK_CUR);
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
    lseek(player.current_track.descriptor, amount, SEEK_CUR);
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
    off_t actual = lseek(player.current_track.descriptor,
			 newpos, SEEK_SET);
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
}

static enum codec_command_action ci_get_command(intptr_t *param)
{
    enum codec_command_action ret = player.codec_action;
    *param = player.codec_action_param;
    player.codec_action = CODEC_ACTION_NULL;
    return ret;
}

static bool ci_should_loop(void)
{
    printf("ci should loop\n");
    return player.enable_loop;
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

static void decode_file(const char *input_fn)
{    
    /* Open file */
    printf("open file\n");
    player.current_track.descriptor = open(input_fn, O_RDONLY);
    if (player.current_track.descriptor == -1) {
	printf(input_fn);
    }

    /* Set up ci */
    printf("mp3entry\n");
    struct mp3entry id3;
    if (!get_metadata(&id3, player.current_track.descriptor,
		      input_fn))
    {
        printf("error: metadata parsing failed\n");
        return;
    }
    print_mp3entry(&id3);
    
    ci->filesize = filesize(player.current_track.descriptor);
    ci->id3 = &id3;

    /* Load codec */
    char str[MAX_PATH];
    snprintf(str, sizeof(str), "codecs/%s.codec", audio_formats[id3.codectype].codec_root_fn);
    ci_debugf("Loading %s\n", str);

    /* Run the codec */
    uint8_t res;
    if(id3.codectype == AFMT_MPA_L3)
	res = mpa_codec_main(CODEC_LOAD);
    else if(id3.codectype == AFMT_PCM_WAV)
	res = wav_codec_main(CODEC_LOAD);
    else if(id3.codectype == AFMT_AIFF)
	res = aiff_codec_main(CODEC_LOAD);
    else if(id3.codectype == AFMT_FLAC)
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
    if(id3.codectype == AFMT_MPA_L3)
	res = mpa_codec_run();
    if(id3.codectype == AFMT_PCM_WAV)
	res = wav_codec_run();
    else if(id3.codectype == AFMT_AIFF)
	res = aiff_codec_run();
    else if(id3.codectype == AFMT_FLAC)
	res = flac_codec_run();
    
    printf("proc %d\n", res);
    if (res != CODEC_OK) {
        printf("error: codec error\n");
    }
    //c_hdr->entry_point(CODEC_UNLOAD);

    /* Close */
    //dlclose(dlcodec);
    if (player.current_track.descriptor != STDIN_FILENO)
        close(player.current_track.descriptor);
}

int dmain()
{
    printf("playback volume...\n");
    playback_set_volume(10);
    printf("playback decode...\n");
    decode_file(p);

    playback_quit();

    return 0;
}

int warble_init()
{
    warble_mutex_create(player.mutex);
    playback_init();
    playback_set_volume(10);
    return 0;
}
