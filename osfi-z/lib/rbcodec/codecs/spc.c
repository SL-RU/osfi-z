/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2007-2008 Michael Sevakis (jhMikeS)
 * Copyright (C) 2006-2007 Adam Gashlin (hcs)
 * Copyright (C) 2004-2007 Shay Green (blargg)
 * Copyright (C) 2002 Brad Martin
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/

/* lovingly ripped off from Game_Music_Emu 0.5.2. http://www.slack.net/~ant/ */
/* DSP Based on Brad Martin's OpenSPC DSP emulator */
/* tag reading from sexyspc by John Brawn (John_Brawn@yahoo.com) and others */
#include "codeclib.h"
#include "fracmul.h"
#include "libspc/spc_codec.h"
#include "libspc/spc_profiler.h"

CODEC_HEADER

/**************** ID666 parsing ****************/

static struct {
    unsigned char isBinary;
    char song[32];
    char game[32];
    char dumper[16];
    char comments[32];
    int day,month,year;
    unsigned long length;
    unsigned long fade;
    char artist[32];
    unsigned char muted;
    unsigned char emulator;
} ID666;

static int LoadID666(unsigned char *buf) {
    unsigned char *ib=buf;
    int isbinary = 1;
    int i;
  
    memcpy(ID666.song,ib,32);
    ID666.song[31]=0;
    ib+=32;

    memcpy(ID666.game,ib,32);
    ID666.game[31]=0;
    ib+=32;

    memcpy(ID666.dumper,ib,16);
    ID666.dumper[15]=0;
    ib+=16;

    memcpy(ID666.comments,ib,32);
    ID666.comments[31]=0;
    ib+=32;

    /* Ok, now comes the fun part. */
   
    /* Date check */
    if(ib[2] == '/'  && ib[5] == '/' )
        isbinary = 0;
  
    /* Reserved bytes check */
    if(ib[0xD2 - 0x2E - 112] >= '0' &&
       ib[0xD2 - 0x2E - 112] <= '9' &&
       ib[0xD3 - 0x2E - 112] == 0x00)
        isbinary = 0;
    
    /* is length & fade only digits? */
    for (i=0;i<8 && ( 
        (ib[0xA9 - 0x2E - 112+i]>='0'&&ib[0xA9 - 0x2E - 112+i]<='9') ||
        ib[0xA9 - 0x2E - 112+i]=='\0');
        i++);
    if (i==8) isbinary=0;
    
    ID666.isBinary = isbinary;
  
    if(isbinary) {
        DEBUGF("binary tag detected\n");
        ID666.year=*ib;
        ib++;
        ID666.year|=*ib<<8;
        ib++;
        ID666.month=*ib;
        ib++;    
        ID666.day=*ib;
        ib++;

        ib+=7;

        ID666.length=*ib;
        ib++;
    
        ID666.length|=*ib<<8;
        ib++;
    
        ID666.length|=*ib<<16;
        ID666.length*=1000;
        ib++;
    
        ID666.fade=*ib;
        ib++;
        ID666.fade|=*ib<<8; 
        ib++;
        ID666.fade|=*ib<<16;
        ib++;
        ID666.fade|=*ib<<24;
        ib++;

        memcpy(ID666.artist,ib,32);
        ID666.artist[31]=0;
        ib+=32;

        ID666.muted=*ib;
        ib++;

        ID666.emulator=*ib;
        ib++;    
    } else {
        unsigned long tmp;
        char buf[64];
        
        DEBUGF("text tag detected\n");
        
        memcpy(buf, ib, 2);
        buf[2] = 0; 
        tmp = 0;
        for (i=0;i<2 && buf[i]>='0' && buf[i]<='9';i++) tmp=tmp*10+buf[i]-'0';
        ID666.month = tmp;
        ib+=3;
        
        memcpy(buf, ib, 2);
        buf[2] = 0; 
        tmp = 0;
        for (i=0;i<2 && buf[i]>='0' && buf[i]<='9';i++) tmp=tmp*10+buf[i]-'0';
        ID666.day = tmp;
        ib+=3;
        
        memcpy(buf, ib, 4);
        buf[4] = 0; 
        tmp = 0;
        for (i=0;i<4 && buf[i]>='0' && buf[i]<='9';i++) tmp=tmp*10+buf[i]-'0';
        ID666.year = tmp;
        ib+=5;
    
        memcpy(buf, ib, 3);
        buf[3] = 0; 
        tmp = 0;
        for (i=0;i<3 && buf[i]>='0' && buf[i]<='9';i++) tmp=tmp*10+buf[i]-'0';
        ID666.length = tmp * 1000;
        ib+=3;

        memcpy(buf, ib, 5);
        buf[5] = 0;
        tmp = 0;
        for (i=0;i<5 && buf[i]>='0' && buf[i]<='9';i++) tmp=tmp*10+buf[i]-'0';
        ID666.fade = tmp;
        ib+=5;

        memcpy(ID666.artist,ib,32);
        ID666.artist[31]=0;
        ib+=32;

        /*I have no idea if this is right or not.*/
        ID666.muted=*ib;
        ib++;

        memcpy(buf, ib, 1);
        buf[1] = 0;
        tmp = 0;
        ib++;
    }
    return 1;
}

/**************** Codec ****************/
enum {SAMPLE_RATE = 32000};
static struct Spc_Emu spc_emu IBSS_ATTR_SPC CACHEALIGN_ATTR;

#if SPC_DUAL_CORE
/** Implementations for pipelined dual-core operation **/
static int spc_emu_thread_stack[DEFAULT_STACK_SIZE/sizeof(int)]
    CACHEALIGN_ATTR;

static const unsigned char * const spc_emu_thread_name = "spc emu";
static unsigned int emu_thread_id = 0;

enum
{
    SPC_EMU_AUDIO = 0,
    SPC_EMU_LOAD,
    SPC_EMU_QUIT,
};

struct spc_load
{
    uint8_t *buf;
    size_t size;
};

/* sample queue */
#define WAV_NUM_CHUNKS 2
#define WAV_CHUNK_MASK (WAV_NUM_CHUNKS-1)
struct sample_queue_chunk
{
    long id;
    union
    {
        intptr_t data;
        int32_t audio[WAV_CHUNK_SIZE*2];
    };
};

static struct
{
    int head, tail;
    struct semaphore emu_sem_head;
    struct semaphore emu_sem_tail;
    struct semaphore emu_evt_reply;
    intptr_t retval;
    struct sample_queue_chunk wav_chunk[WAV_NUM_CHUNKS];
} sample_queue SHAREDBSS_ATTR;

static inline void samples_release_wrbuf(void)
{
    sample_queue.tail++;
    ci->semaphore_release(&sample_queue.emu_sem_head);
}

static inline struct sample_queue_chunk * samples_get_wrbuf(void)
{
    ci->semaphore_wait(&sample_queue.emu_sem_tail, TIMEOUT_BLOCK);
    return &sample_queue.wav_chunk[sample_queue.tail & WAV_CHUNK_MASK];
}

static inline void samples_release_rdbuf(void)
{
    if (sample_queue.head != sample_queue.tail) {
        sample_queue.head++;
    }

    ci->semaphore_release(&sample_queue.emu_sem_tail);
}

static inline int32_t * samples_get_rdbuf(void)
{
    ci->semaphore_wait(&sample_queue.emu_sem_head, TIMEOUT_BLOCK);
    return sample_queue.wav_chunk[sample_queue.head & WAV_CHUNK_MASK].audio;
}

static intptr_t emu_thread_send_msg(long id, intptr_t data)
{
    struct sample_queue_chunk *chunk;
    /* Grab an audio output buffer */
    ci->semaphore_wait(&sample_queue.emu_sem_head, TIMEOUT_BLOCK);
    chunk = &sample_queue.wav_chunk[sample_queue.head & WAV_CHUNK_MASK];
    /* Place a message in it instead of audio */
    chunk->id = id;
    chunk->data = data;
    /* Release it to the emu thread */
    samples_release_rdbuf();

    if (id != SPC_EMU_QUIT) {
        /* Wait for a response */
        ci->semaphore_wait(&sample_queue.emu_evt_reply, TIMEOUT_BLOCK);
    }

    return sample_queue.retval;    
}

/* thread function */
static bool emu_thread_process_msg(struct sample_queue_chunk *chunk)
{
    long id = chunk->id;
    bool ret = id != SPC_EMU_QUIT;

    chunk->id = SPC_EMU_AUDIO; /* Reset chunk type to audio */
    sample_queue.retval = 0;

    if (id == SPC_EMU_LOAD)
    {
        struct spc_load *ld = (struct spc_load *)chunk->data;
        ci->commit_discard_dcache();
        SPC_Init(&spc_emu);
        sample_queue.retval = SPC_load_spc(&spc_emu, ld->buf, ld->size);

        /* Empty the audio queue */
        ci->semaphore_release(&sample_queue.emu_sem_tail);
        ci->semaphore_release(&sample_queue.emu_sem_tail);
        ci->semaphore_wait(&sample_queue.emu_sem_head, TIMEOUT_NOBLOCK);
        ci->semaphore_wait(&sample_queue.emu_sem_head, TIMEOUT_NOBLOCK);
        sample_queue.head = sample_queue.tail = 0;
    }

    if (id != SPC_EMU_QUIT) {
        ci->semaphore_release(&sample_queue.emu_evt_reply);
    }

    return ret;
}

static void spc_emu_thread(void)
{
    CPU_Init(&spc_emu);

    while (1) {
        /* get a buffer for output */
        struct sample_queue_chunk *chunk = samples_get_wrbuf();

        if (chunk->id != SPC_EMU_AUDIO) {
            /* This chunk doesn't contain audio but a command */
            if (!emu_thread_process_msg(chunk))
                break;
            /* Have to re-get this pointer to keep semaphore counts correct */
            continue;
        }

        ENTER_TIMER(render);
        /* fill samples buffer */
        if ( SPC_play(&spc_emu, WAV_CHUNK_SIZE*2, chunk->audio) )
            assert( false );
        EXIT_TIMER(render);

        /* done so release it to output */
        samples_release_wrbuf();
        ci->yield();
    }
}

static bool spc_emu_start(void)
{
    emu_thread_id = ci->create_thread(spc_emu_thread, spc_emu_thread_stack,
                           sizeof(spc_emu_thread_stack), CREATE_THREAD_FROZEN,
                           spc_emu_thread_name IF_PRIO(, PRIORITY_PLAYBACK), COP);

    if (emu_thread_id == 0)
        return false;

    /* Initialize audio queue as full to prevent emu thread from trying to run the
       emulator before loading something */
    ci->semaphore_init(&sample_queue.emu_evt_reply, 1, 0);
    ci->semaphore_init(&sample_queue.emu_sem_tail, 2, 0);
    ci->semaphore_init(&sample_queue.emu_sem_head, 2, 2);
    sample_queue.head = 0;
    sample_queue.tail = 2;

    /* Start it running */
    ci->thread_thaw(emu_thread_id);
    return true;
}

/* load a new program on the emu thread */
static inline int load_spc_buffer(uint8_t *buf, size_t size)
{
    struct spc_load ld = { buf, size };
    ci->commit_dcache();
    return emu_thread_send_msg(SPC_EMU_LOAD, (intptr_t)&ld);
}

static inline void spc_emu_quit(void)
{
    if (emu_thread_id != 0) {
        emu_thread_send_msg(SPC_EMU_QUIT, 0);
        /* Wait for emu thread to be killed */
        ci->thread_wait(emu_thread_id);
        ci->commit_discard_dcache();
    }
}

static inline int32_t * spc_play_get_samples(void)
{
    /* obtain filled samples buffer */
    return samples_get_rdbuf();
}

static inline void spc_play_send_samples(int32_t *samples)
{
    ci->pcmbuf_insert(samples, samples+WAV_CHUNK_SIZE, WAV_CHUNK_SIZE);
    /* done with chunk so release it to emu thread */
    samples_release_rdbuf();
}

#else /* !SPC_DUAL_CORE */
/** Implementations for single-core operation **/
static int32_t wav_chunk[WAV_CHUNK_SIZE*2] IBSS_ATTR;

/* load a new program into emu */
static inline int load_spc_buffer(uint8_t *buf, size_t size)
{
    SPC_Init(&spc_emu);
    return SPC_load_spc(&spc_emu, buf, size);
}

static inline bool spc_emu_start(void)
{
#ifdef CPU_COLDFIRE
    /* signed integer mode with saturation */
    coldfire_set_macsr(EMAC_SATURATE);
#endif
    CPU_Init(&spc_emu);
    return true;
}

static inline void spc_play_send_samples(int32_t *samples)
{
    ci->pcmbuf_insert(samples, samples+WAV_CHUNK_SIZE, WAV_CHUNK_SIZE);
}

#define spc_emu_quit()
#define samples_release_rdbuf()

static inline int32_t * spc_play_get_samples(void)
{
    ENTER_TIMER(render);
    /* fill samples buffer */
    if ( SPC_play(&spc_emu,WAV_CHUNK_SIZE*2,wav_chunk) )
        assert( false );
    EXIT_TIMER(render);
    return wav_chunk;
}
#endif /* SPC_DUAL_CORE */

/* The main decoder loop */
static int play_track( void )
{
    int sampleswritten=0;
    
    unsigned long fadestartsample = ID666.length*(long long) SAMPLE_RATE/1000;
    unsigned long fadeendsample = (ID666.length+ID666.fade)*(long long) SAMPLE_RATE/1000;
    int fadedec = 0;
    int fadevol = 0x7fffffffl;
    intptr_t param;
    
    if (fadeendsample>fadestartsample)
        fadedec=0x7fffffffl/(fadeendsample-fadestartsample)+1;
        
    ENTER_TIMER(total);

    while ( 1 )
    {
        enum codec_command_action action = ci->get_command(&param);

        if (action == CODEC_ACTION_HALT)
            break;

        if (action == CODEC_ACTION_SEEK_TIME) {
            int curtime = sampleswritten*1000LL/SAMPLE_RATE;
            DEBUGF("seek to %ld\ncurrently at %d\n", (long)param, curtime);
            if (param < curtime) {
                DEBUGF("seek backwards = reset\n");
                ci->set_elapsed(0);
                ci->seek_complete();
                return 1;
            }

            ci->set_elapsed(curtime);
            ci->seek_complete();
        }

        int32_t *samples = spc_play_get_samples();

        sampleswritten += WAV_CHUNK_SIZE;

        /* is track timed? */
        if (!ci->loop_track() && ci->id3->length) {
            unsigned long curtime = sampleswritten*1000LL/SAMPLE_RATE;
            unsigned long lasttimesample = (sampleswritten-WAV_CHUNK_SIZE);

            /* fade? */
            if (curtime>ID666.length)
            {
            #ifdef CPU_COLDFIRE
                /* Have to switch modes to do this */
                long macsr = coldfire_get_macsr();
                coldfire_set_macsr(EMAC_SATURATE | EMAC_FRACTIONAL | EMAC_ROUND);
            #endif
                int i;
                for (i=0;i<WAV_CHUNK_SIZE;i++) {
                    if (lasttimesample+i>fadestartsample) {
                        if (fadevol>0) {
                            samples[i] = FRACMUL(samples[i], fadevol);
                            samples[i+WAV_CHUNK_SIZE] = FRACMUL(samples[i+WAV_CHUNK_SIZE], fadevol);
                        } else samples[i]=samples[i+WAV_CHUNK_SIZE]=0;
                        fadevol-=fadedec;
                    }
                }
            #ifdef CPU_COLDFIRE
               coldfire_set_macsr(macsr);
            #endif
            }
            /* end? */
            if (lasttimesample>=fadeendsample)
            {
                samples_release_rdbuf();
                break;
            }
        }

        spc_play_send_samples(samples);

        if (ci->loop_track())
            ci->set_elapsed(0);
        else
            ci->set_elapsed(sampleswritten*1000LL/SAMPLE_RATE);
    }
    
    EXIT_TIMER(total);
    return 0;
}

/* this is the codec entry point */
enum codec_status codec_main(enum codec_entry_call_reason reason)
{
    if (reason == CODEC_LOAD) {
        if (!spc_emu_start())
            return CODEC_ERROR;

        ci->configure(DSP_SET_SAMPLE_DEPTH, 24);
        ci->configure(DSP_SET_FREQUENCY, SAMPLE_RATE);
        ci->configure(DSP_SET_STEREO_MODE, STEREO_NONINTERLEAVED);
    }
    else if (reason == CODEC_UNLOAD) {
        spc_emu_quit();
    }

    return CODEC_OK;
}

/* this is called for each file to process */
enum codec_status codec_run(void)
{
    DEBUGF("SPC: next_track\n");
    if (codec_init())
        return CODEC_ERROR;
    DEBUGF("SPC: after init\n");

    codec_set_replaygain(ci->id3);

    /* Read the entire file */
    DEBUGF("SPC: request initial buffer\n");
    ci->seek_buffer(0);
    size_t buffersize;
    uint8_t* buffer = ci->request_buffer(&buffersize, ci->filesize);
    if (!buffer)
        return CODEC_ERROR;

    DEBUGF("SPC: read size = 0x%lx\n",(unsigned long)buffersize);
    ci->set_elapsed(0);

    do
    {
        if (load_spc_buffer(buffer, buffersize)) {
            DEBUGF("SPC load failure\n");
            return CODEC_ERROR;
        }

        LoadID666(buffer+0x2e);

        if (!ci->loop_track() && ID666.length==0) {
            ID666.length=3*60*1000; /* 3 minutes */
            ID666.fade=5*1000; /* 5 seconds */
        }

        reset_profile_timers();
    }
    while ( play_track() );

    print_timers(ci->id3->path);

    return CODEC_OK;
}
