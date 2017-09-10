/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2005 Dave Chapman
 * Copyright (C) 2009 Yoshihisa Uchida
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

#include "codeclib.h"
#include "codecs/libpcm/support_formats.h"

CODEC_HEADER

/* WAVE (RIFF) codec:
 * 
 *  For a good documentation on WAVE files, see:
 *  http://www.tsp.ece.mcgill.ca/MMSP/Documents/AudioFormats/WAVE/WAVE.html
 *  and
 *  http://www.sonicspot.com/guide/wavefiles.html
 *
 *  For sample WAV files, see:
 *  http://www.tsp.ece.mcgill.ca/MMSP/Documents/AudioFormats/WAVE/Samples.html
 *
 */

#define PCM_SAMPLE_SIZE (4096*2)

static int32_t samples[PCM_SAMPLE_SIZE] IBSS_ATTR;

/* This codec support WAVE files with the following formats: */
enum
{
    WAVE_FORMAT_UNKNOWN = 0x0000, /* Microsoft Unknown Wave Format */
    WAVE_FORMAT_PCM = 0x0001,   /* Microsoft PCM Format */
    WAVE_FORMAT_ADPCM = 0x0002, /* Microsoft ADPCM Format */
    WAVE_FORMAT_IEEE_FLOAT = 0x0003, /* IEEE Float */
    WAVE_FORMAT_ALAW = 0x0006,  /* Microsoft ALAW */
    WAVE_FORMAT_MULAW = 0x0007, /* Microsoft MULAW */
    WAVE_FORMAT_DVI_ADPCM = 0x0011, /* Intel's DVI ADPCM */
    WAVE_FORMAT_DIALOGIC_OKI_ADPCM = 0x0017, /* Dialogic OKI ADPCM */
    WAVE_FORMAT_YAMAHA_ADPCM = 0x0020, /* Yamaha ADPCM */
    WAVE_FORMAT_XBOX_ADPCM = 0x0069, /* XBOX ADPCM */
    IBM_FORMAT_MULAW = 0x0101,  /* same as WAVE_FORMAT_MULAW */
    IBM_FORMAT_ALAW = 0x0102,   /* same as WAVE_FORMAT_ALAW */
    WAVE_FORMAT_SWF_ADPCM = 0x5346, /* Adobe SWF ADPCM */
    WAVE_FORMAT_EXTENSIBLE = 0xFFFE
};

static const struct pcm_entry wave_codecs[] = {
    { WAVE_FORMAT_UNKNOWN,            0                            },
    { WAVE_FORMAT_PCM,                get_linear_pcm_codec         },
    { WAVE_FORMAT_ADPCM,              get_ms_adpcm_codec           },
    { WAVE_FORMAT_IEEE_FLOAT,         get_ieee_float_codec         },
    { WAVE_FORMAT_ALAW,               get_itut_g711_alaw_codec     },
    { WAVE_FORMAT_MULAW,              get_itut_g711_mulaw_codec    },
    { WAVE_FORMAT_DVI_ADPCM,          get_dvi_adpcm_codec          },
    { WAVE_FORMAT_DIALOGIC_OKI_ADPCM, get_dialogic_oki_adpcm_codec },
    { WAVE_FORMAT_YAMAHA_ADPCM,       get_yamaha_adpcm_codec       },
    { WAVE_FORMAT_XBOX_ADPCM,         get_dvi_adpcm_codec          },
    { IBM_FORMAT_MULAW,               get_itut_g711_mulaw_codec    },
    { IBM_FORMAT_ALAW,                get_itut_g711_alaw_codec     },
    { WAVE_FORMAT_SWF_ADPCM,          get_swf_adpcm_codec          },
};

#define NUM_FORMATS 13

static const struct pcm_codec *get_wave_codec(uint32_t formattag)
{
    int i;

    for (i = 0; i < NUM_FORMATS; i++)
    {
        if (wave_codecs[i].format_tag == formattag)
        {
            if (wave_codecs[i].get_codec)
                return wave_codecs[i].get_codec();
            return 0;
        }
    }
    return 0;
}

static struct pcm_format format;
static uint32_t bytesdone;

static bool set_msadpcm_coeffs(const uint8_t *buf)
{
    int i;
    int num;
    int size;

    buf += 4; /* skip 'fmt ' */
    size = buf[0] | (buf[1] << 8) | (buf[1] << 16) | (buf[1] << 24);
    if (size < 50)
    {
        DEBUGF("CODEC_ERROR: microsoft adpcm 'fmt ' chunk size=%lu < 50\n",
                             (unsigned long)size);
        return false;
    }

    /* get nNumCoef */
    buf += 24;
    num = buf[0] | (buf[1] << 8);

    /*
     * In many case, nNumCoef is 7.
     * Depending upon the encoder, as for this value there is a possibility of
     * increasing more.
     * If you found the file where this value exceeds 7, please report.
     */
    if (num != MSADPCM_NUM_COEFF)
    {
        DEBUGF("CODEC_ERROR: microsoft adpcm nNumCoef=%d != 7\n", num);
        return false;
    }

    /* get aCoeffs */
    buf += 2;
    for (i = 0; i < MSADPCM_NUM_COEFF; i++)
    {
        format.coeffs[i][0] = buf[0] | (SE(buf[1]) << 8);
        format.coeffs[i][1] = buf[2] | (SE(buf[3]) << 8);
        buf += 4;
    }

    return true;
}

static uint8_t *read_buffer(size_t *realsize)
{
    uint8_t *buffer = (uint8_t *)ci->request_buffer(realsize, format.chunksize);
    if (bytesdone + (*realsize) > format.numbytes)
        *realsize = format.numbytes - bytesdone;
    bytesdone += *realsize;
    ci->advance_buffer(*realsize);
    return buffer;
}

/* this is the codec entry point */
enum codec_status codec_main(enum codec_entry_call_reason reason)
{
    if (reason == CODEC_LOAD) {
        /* Generic codec initialisation */
        ci->configure(DSP_SET_SAMPLE_DEPTH, PCM_OUTPUT_DEPTH-1);
    }

    return CODEC_OK;
}

/* this is called for each file to process */
enum codec_status codec_run(void)
{
    uint32_t decodedsamples;
    size_t n;
    int bufcount;
    int endofstream;
    unsigned char *buf;
    uint8_t *wavbuf;
    off_t firstblockposn;     /* position of the first block in file */
    const struct pcm_codec *codec;
    uint32_t size;
    intptr_t param;

    if (codec_init()) {
        DEBUGF("codec_init() error\n");
        return CODEC_ERROR;
    }

    codec_set_replaygain(ci->id3);
    
    /* Need to save resume for later use (cleared indirectly by advance_buffer) */
    param = ci->id3->elapsed;
    bytesdone = ci->id3->offset;

    /* get RIFF chunk header */
    ci->seek_buffer(0);
    buf = ci->request_buffer(&n, 12);
    if (n < 12) {
        DEBUGF("request_buffer error\n");
        return CODEC_ERROR;
    }
    if ((memcmp(buf, "RIFF", 4) != 0) || (memcmp(&buf[8], "WAVE", 4) != 0)) {
        DEBUGF("CODEC_ERROR: missing riff header\n");
        return CODEC_ERROR;
    }

    /* advance to first WAVE chunk */
    ci->advance_buffer(12);

    firstblockposn = 12;
    ci->memset(&format, 0, sizeof(struct pcm_format));
    format.is_signed = true;
    format.is_little_endian = true;

    decodedsamples = 0;
    codec = 0;

    /* iterate over WAVE chunks until the 'data' chunk, which should be after the 'fmt ' chunk */
    while (true) {
        /* get WAVE chunk header */
        buf = ci->request_buffer(&n, 1024);
        if (n < 8) {
            DEBUGF("data chunk request_buffer error\n");
            /* no more chunks, 'data' chunk must not have been found */
            return CODEC_ERROR;
        }

        /* chunkSize */
        size = (buf[4]|(buf[5]<<8)|(buf[6]<<16)|(buf[7]<<24));
        if (memcmp(buf, "fmt ", 4) == 0) {
            if (size < 16) {
                DEBUGF("CODEC_ERROR: 'fmt ' chunk size=%lu < 16\n",
                       (unsigned long)size);
                return CODEC_ERROR;
            }
            /* wFormatTag */
            format.formattag=buf[8]|(buf[9]<<8);
            /* wChannels */
            format.channels=buf[10]|(buf[11]<<8);
            /* skipping dwSamplesPerSec */
            /* skipping dwAvgBytesPerSec */
            /* wBlockAlign */
            format.blockalign=buf[20]|(buf[21]<<8);
            /* wBitsPerSample */
            format.bitspersample=buf[22]|(buf[23]<<8);
            if (format.formattag != WAVE_FORMAT_PCM) {
                if (size < 18) {
                    /* this is not a fatal error with some formats,
                     * we'll see later if we can't decode it */
                    DEBUGF("CODEC_WARNING: non-PCM WAVE (formattag=0x%x) "
                           "doesn't have ext. fmt descr (chunksize=%d<18).\n",
                           (unsigned int)format.formattag, (int)size);
                }
                else
                {
                    if (format.formattag != WAVE_FORMAT_EXTENSIBLE)
                        format.samplesperblock = buf[26]|(buf[27]<<8);
                    else
                    {
                        format.size = buf[24]|(buf[25]<<8);
                        if (format.size < 22) {
                            DEBUGF("CODEC_ERROR: WAVE_FORMAT_EXTENSIBLE is "
                                   "missing extension\n");
                            return CODEC_ERROR;
                        }
                        /* wValidBitsPerSample */
                        format.bitspersample = buf[26]|(buf[27]<<8);
                        /* skipping dwChannelMask (4bytes) */
                        /* SubFormat (only get the first two bytes) */
                        format.formattag = buf[32]|(buf[33]<<8);
                    }
                }
            }

            /* msadpcm specific */
            if (format.formattag == WAVE_FORMAT_ADPCM)
            {
                if (!set_msadpcm_coeffs(buf))
                {
                    return CODEC_ERROR;
                }
            }

            /* get codec */
            codec = get_wave_codec(format.formattag);
            if (!codec)
            {
                DEBUGF("CODEC_ERROR: unsupported wave format 0x%x\n", 
                    (unsigned int) format.formattag);
                return CODEC_ERROR;
            }

            /* riff 8bit linear pcm is unsigned */
            if (format.formattag == WAVE_FORMAT_PCM && format.bitspersample == 8)
                format.is_signed = false;

            /* set format, parse codec specific tag, check format, and calculate chunk size */
            if (!codec->set_format(&format))
            {
                return CODEC_ERROR;
            }
        } else if (memcmp(buf, "data", 4) == 0) {
            format.numbytes = size;
            /* advance to start of data */
            ci->advance_buffer(8);
            firstblockposn += 8;
            break;
        } else if (memcmp(buf, "fact", 4) == 0) {
            /* dwSampleLength */
            if (size >= 4)
                format.totalsamples =
                             (buf[8]|(buf[9]<<8)|(buf[10]<<16)|(buf[11]<<24));
        } else {
            DEBUGF("unknown WAVE chunk: '%c%c%c%c', size=%lu\n",
                   buf[0], buf[1], buf[2], buf[3], (unsigned long)size);
        }

        /* go to next chunk (even chunk sizes must be padded) */
        size += 8 + (size & 0x01);

        ci->advance_buffer(size);
        firstblockposn += size;
    }

    if (!codec)
    {
        DEBUGF("CODEC_ERROR: 'fmt ' chunk not found\n");
        return CODEC_ERROR;
    }

    /* common format check */
    if (format.channels == 0) {
        DEBUGF("CODEC_ERROR: 'fmt ' chunk not found or 0-channels file\n");
        return CODEC_ERROR;
    }
    if (format.samplesperblock == 0) {
        DEBUGF("CODEC_ERROR: 'fmt ' chunk not found or 0-wSamplesPerBlock file\n");
        return CODEC_ERROR;
    }
    if (format.blockalign == 0)
    {
        DEBUGF("CODEC_ERROR: 'fmt ' chunk not found or 0-blockalign file\n");
        return CODEC_ERROR;
    }
    if (format.numbytes == 0) {
        DEBUGF("CODEC_ERROR: 'data' chunk not found or has zero-length\n");
        return CODEC_ERROR;
    }

    /* check chunksize */
    if ((format.chunksize / format.blockalign) * format.samplesperblock * format.channels
           > PCM_SAMPLE_SIZE)
        format.chunksize = (PCM_SAMPLE_SIZE / format.blockalign) * format.blockalign;
    if (format.chunksize == 0)
    {
        DEBUGF("CODEC_ERROR: chunksize is 0\n");
        return CODEC_ERROR;
    }

    ci->configure(DSP_SET_FREQUENCY, ci->id3->frequency);
    if (format.channels == 2) {
        ci->configure(DSP_SET_STEREO_MODE, STEREO_INTERLEAVED);
    } else if (format.channels == 1) {
        ci->configure(DSP_SET_STEREO_MODE, STEREO_MONO);
    } else {
        DEBUGF("CODEC_ERROR: more than 2 channels\n");
        return CODEC_ERROR;
    }

    /* make sure we're at the correct offset */
    if (bytesdone > (uint32_t) firstblockposn || param) {
        uint32_t seek_val;
        int seek_mode;

        if (bytesdone) {
            seek_val = bytesdone - MIN((uint32_t) firstblockposn, bytesdone);
            seek_mode = PCM_SEEK_POS;
        } else {
            seek_val = param;
            seek_mode = PCM_SEEK_TIME;
        }

        /* Round down to previous block */
        struct pcm_pos *newpos = codec->get_seek_pos(seek_val, seek_mode,
                                                     &read_buffer);

        if (newpos->pos > format.numbytes)
            return CODEC_OK;
        if (ci->seek_buffer(firstblockposn + newpos->pos))
        {
            bytesdone      = newpos->pos;
            decodedsamples = newpos->samples;
        }
    } else {
        /* already where we need to be */
        bytesdone = 0;
    }

    ci->set_elapsed(decodedsamples*1000LL/ci->id3->frequency);

    /* The main decoder loop */
    endofstream = 0;

    while (!endofstream) {
        enum codec_command_action action = ci->get_command(&param);

        if (action == CODEC_ACTION_HALT)
            break;

        if (action == CODEC_ACTION_SEEK_TIME) {
            struct pcm_pos *newpos = codec->get_seek_pos(param, PCM_SEEK_TIME,
                                                         &read_buffer);
            if (newpos->pos > format.numbytes)
            {
                ci->set_elapsed(ci->id3->length);
                ci->seek_complete();
                break;
            }

            if (ci->seek_buffer(firstblockposn + newpos->pos))
            {
                bytesdone      = newpos->pos;
                decodedsamples = newpos->samples;
            }

            ci->set_elapsed(decodedsamples*1000LL/ci->id3->frequency);
            ci->seek_complete();
        }

        wavbuf = (uint8_t *)ci->request_buffer(&n, format.chunksize);
        if (n == 0)
            break; /* End of stream */
        if (bytesdone + n > format.numbytes) {
            n = format.numbytes - bytesdone;
            endofstream = 1;
        }

        if (codec->decode(wavbuf, n, samples, &bufcount) == CODEC_ERROR)
        {
            DEBUGF("codec error\n");
            return CODEC_ERROR;
        }

        ci->pcmbuf_insert(samples, NULL, bufcount);
        ci->advance_buffer(n);
        bytesdone += n;
        decodedsamples += bufcount;

        if (bytesdone >= format.numbytes)
            endofstream = 1;
        ci->set_elapsed(decodedsamples*1000LL/ci->id3->frequency);
    }

    return CODEC_OK;
}
