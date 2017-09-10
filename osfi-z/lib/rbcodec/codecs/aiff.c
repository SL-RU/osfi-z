/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (c) 2005 Jvo Studer
 * Copyright (c) 2009 Yoshihisa Uchida
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

#define FOURCC(c1, c2, c3, c4) \
((((uint32_t)c1)<<24)|(((uint32_t)c2)<<16)|(((uint32_t)c3)<<8)|((uint32_t)c4))

/* This codec supports the following AIFC compressionType formats */
enum {
    AIFC_FORMAT_PCM          = FOURCC('N', 'O', 'N', 'E'), /* AIFC PCM Format (big endian) */
    AIFC_FORMAT_ALAW         = FOURCC('a', 'l', 'a', 'w'), /* AIFC ALaw compressed */
    AIFC_FORMAT_MULAW        = FOURCC('u', 'l', 'a', 'w'), /* AIFC uLaw compressed */
    AIFC_FORMAT_IEEE_FLOAT32 = FOURCC('f', 'l', '3', '2'), /* AIFC IEEE float 32 bit */
    AIFC_FORMAT_IEEE_FLOAT64 = FOURCC('f', 'l', '6', '4'), /* AIFC IEEE float 64 bit */
    AIFC_FORMAT_QT_IMA_ADPCM = FOURCC('i', 'm', 'a', '4'), /* AIFC QuickTime IMA ADPCM */
};

static const struct pcm_entry pcm_codecs[] = {
    { AIFC_FORMAT_PCM,          get_linear_pcm_codec      },
    { AIFC_FORMAT_ALAW,         get_itut_g711_alaw_codec  },
    { AIFC_FORMAT_MULAW,        get_itut_g711_mulaw_codec },
    { AIFC_FORMAT_IEEE_FLOAT32, get_ieee_float_codec      },
    { AIFC_FORMAT_IEEE_FLOAT64, get_ieee_float_codec      },
    { AIFC_FORMAT_QT_IMA_ADPCM, get_qt_ima_adpcm_codec    },
};

#define PCM_SAMPLE_SIZE (1024*2)

static int32_t samples[PCM_SAMPLE_SIZE] IBSS_ATTR;

static const struct pcm_codec *get_codec(uint32_t formattag)
{
    unsigned i;
    for (i = 0; i < sizeof(pcm_codecs)/sizeof(pcm_codecs[0]); i++)
        if (pcm_codecs[i].format_tag == formattag)
            return pcm_codecs[i].get_codec();

    return NULL;
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
    struct pcm_format format;
    uint32_t bytesdone, decodedsamples;
    /* rockbox: comment 'set but unused' variables
    uint32_t num_sample_frames = 0;
    */
    size_t n;
    int bufcount;
    int endofstream;
    unsigned char *buf;
    uint8_t *aifbuf;
    uint32_t offset2snd = 0;
    off_t firstblockposn;     /* position of the first block in file */
    bool is_aifc = false;
    const struct pcm_codec *codec;
    uint32_t size;
    intptr_t param;

    if (codec_init()) {
        return CODEC_ERROR;
    }

    codec_set_replaygain(ci->id3);
    
    /* Need to save offset for later use (cleared indirectly by advance_buffer) */
    param = ci->id3->elapsed;
    bytesdone = ci->id3->offset;

    /* assume the AIFF header is less than 1024 bytes */
    ci->seek_buffer(0);
    buf = ci->request_buffer(&n, 1024);
    if (n < 54) {
        return CODEC_ERROR;
    }

    if (memcmp(buf, "FORM", 4) != 0)
    {
        DEBUGF("CODEC_ERROR: does not aiff format %4.4s\n", (char*)&buf[0]);
        return CODEC_ERROR;
    }
    if (memcmp(&buf[8], "AIFF", 4) == 0)
        is_aifc = false;
    else if (memcmp(&buf[8], "AIFC", 4) == 0)
        is_aifc = true;
    else
    {
        DEBUGF("CODEC_ERROR: does not aiff format %4.4s\n", (char*)&buf[8]);
        return CODEC_ERROR;
    }

    buf += 12;
    n -= 12;

    ci->memset(&format, 0, sizeof(struct pcm_format));
    format.is_signed = true;
    format.is_little_endian = false;

    decodedsamples = 0;
    codec = 0;

    /* read until 'SSND' chunk, which typically is last */
    while (format.numbytes == 0 && n >= 8)
    {
        /* chunkSize */
        size = ((buf[4]<<24)|(buf[5]<<16)|(buf[6]<<8)|buf[7]);
        if (memcmp(buf, "COMM", 4) == 0) {
            if ((!is_aifc && size < 18) || (is_aifc && size < 22))
            {
                DEBUGF("CODEC_ERROR: 'COMM' chunk size=%lu < %d\n",
                       (unsigned long)size, (is_aifc)?22:18);
                return CODEC_ERROR;
            }
            /* num_channels */
            format.channels = ((buf[8]<<8)|buf[9]);
            /* num_sample_frames */
            /*
            num_sample_frames = ((buf[10]<<24)|(buf[11]<<16)|(buf[12]<<8)
                                |buf[13]);
            */
            
            /* sample_size */
            format.bitspersample = ((buf[14]<<8)|buf[15]);
            /* sample_rate (don't use last 4 bytes, only integer fs) */
            if (buf[16] != 0x40) {
                DEBUGF("CODEC_ERROR: weird sampling rate (no @)\n");
                return CODEC_ERROR;
            }
            format.samplespersec = ((buf[18]<<24)|(buf[19]<<16)|(buf[20]<<8)|buf[21])+1;
            format.samplespersec >>= (16 + 14 - buf[17]);
            /* compressionType (AIFC only) */
            if (is_aifc)
            {
                format.formattag = (buf[26]<<24)|(buf[27]<<16)|(buf[28]<<8)|buf[29];

                /*
                 * aiff's sample_size is uncompressed sound data size.
                 * But format.bitspersample is compressed sound data size.
                 */
                if (format.formattag == AIFC_FORMAT_ALAW ||
                    format.formattag == AIFC_FORMAT_MULAW)
                    format.bitspersample = 8;
                else if (format.formattag == AIFC_FORMAT_QT_IMA_ADPCM)
                    format.bitspersample = 4;
            }
            else
                format.formattag = AIFC_FORMAT_PCM;
            /* calc average bytes per second */
            format.avgbytespersec = format.samplespersec*format.channels*format.bitspersample/8;
        } else if (memcmp(buf, "SSND", 4)==0) {
            if (format.bitspersample == 0) {
                DEBUGF("CODEC_ERROR: unsupported chunk order\n");
                return CODEC_ERROR;
            }
            /* offset2snd */
            offset2snd = (buf[8]<<24)|(buf[9]<<16)|(buf[10]<<8)|buf[11];
            /* block_size */
            format.blockalign = ((buf[12]<<24)|(buf[13]<<16)|(buf[14]<<8)|buf[15]) >> 3;
            if (format.blockalign == 0)
                format.blockalign = format.channels * format.bitspersample >> 3;
            format.numbytes = size - 8 - offset2snd;
            size = 8 + offset2snd; /* advance to the beginning of data */
        } else if (is_aifc && (memcmp(buf, "FVER", 4)==0)) {
            /* Format Version Chunk (AIFC only chunk) */
            /* skip this chunk */
        } else {
            DEBUGF("unsupported AIFF chunk: '%c%c%c%c', size=%lu\n",
                   buf[0], buf[1], buf[2], buf[3], (unsigned long)size);
        }

        size += 8 + (size & 0x01); /* odd chunk sizes must be padded */

        buf += size;
        if (n < size) {
            DEBUGF("CODEC_ERROR: AIFF header size > 1024\n");
            return CODEC_ERROR;
        }
        n -= size;
    } /* while 'SSND' */

    if (format.channels == 0) {
        DEBUGF("CODEC_ERROR: 'COMM' chunk not found or 0-channels file\n");
        return CODEC_ERROR;
    }
    if (format.numbytes == 0) {
        DEBUGF("CODEC_ERROR: 'SSND' chunk not found or has zero length\n");
        return CODEC_ERROR;
    }

    codec = get_codec(format.formattag);
    if (codec == 0)
    {
        DEBUGF("CODEC_ERROR: AIFC does not support compressionType: 0x%x\n", 
            (unsigned int)format.formattag);
        return CODEC_ERROR;
    }

    if (!codec->set_format(&format))
    {
        return CODEC_ERROR;
    }

    ci->configure(DSP_SET_FREQUENCY, ci->id3->frequency);

    if (format.channels == 2) {
        ci->configure(DSP_SET_STEREO_MODE, STEREO_INTERLEAVED);
    } else if (format.channels == 1) {
        ci->configure(DSP_SET_STEREO_MODE, STEREO_MONO);
    } else {
        DEBUGF("CODEC_ERROR: more than 2 channels unsupported\n");
        return CODEC_ERROR;
    }

    if (format.samplesperblock == 0)
    {
        DEBUGF("CODEC_ERROR: samplesperblock is 0\n");
        return CODEC_ERROR;
    }
    if (format.blockalign == 0)
    {
        DEBUGF("CODEC_ERROR: blockalign is 0\n");
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

    firstblockposn = 1024 - n;
    ci->advance_buffer(firstblockposn);

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
        struct pcm_pos *newpos = codec->get_seek_pos(seek_val, seek_mode, NULL);

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
            /* 3rd args(read_buffer) is unnecessary in the format which AIFF supports. */
            struct pcm_pos *newpos = codec->get_seek_pos(param, PCM_SEEK_TIME, NULL);

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
        aifbuf = (uint8_t *)ci->request_buffer(&n, format.chunksize);

        if (n == 0)
            break; /* End of stream */

        if (bytesdone + n > format.numbytes) {
            n = format.numbytes - bytesdone;
            endofstream = 1;
        }

        if (codec->decode(aifbuf, n, samples, &bufcount) == CODEC_ERROR)
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
