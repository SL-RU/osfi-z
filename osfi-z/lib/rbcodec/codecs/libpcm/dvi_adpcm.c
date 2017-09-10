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
#include "ima_adpcm_common.h"
#include "support_formats.h"

/*
 * Intel DVI ADPCM (IMA ADPCM)
 *
 * References
 * [1] The IMA Digital Audio Focus and Technical Working Groups,
 *     Recommended Practices for Enhancing Digital Audio Compatibility
 *     in Multimedia Systems Revision 3.00, 1992
 * [2] Microsoft Corporation, New Multimedia Data Types and Data Techniques,
 *     Revision:3.0, 1994
 * [3] ffmpeg source code, libavcodec/adpcm.c
 */

static struct pcm_format *fmt;

static bool set_format(struct pcm_format *format)
{
    fmt = format;

    if (fmt->bitspersample < 2 || fmt->bitspersample > 5)
    {
        DEBUGF("CODEC_ERROR: dvi adpcm must be 2, 3, 4 or 5 bitspersample: %d\n",
                             fmt->bitspersample);
        return false;
    }

    fmt->chunksize = fmt->blockalign;

    init_ima_adpcm_decoder(fmt->bitspersample, NULL);
    return true;
}

static struct pcm_pos *get_seek_pos(uint32_t seek_val, int seek_mode,
                                    uint8_t *(*read_buffer)(size_t *realsize))
{
    static struct pcm_pos newpos;
    uint32_t newblock = (seek_mode == PCM_SEEK_TIME) ?
                        ((uint64_t)seek_val * ci->id3->frequency / 1000LL)
                                            / fmt->samplesperblock :
                        seek_val / fmt->blockalign;

    (void)read_buffer;
    newpos.pos     = newblock * fmt->blockalign;
    newpos.samples = newblock * fmt->samplesperblock;
    return &newpos;
}

static inline void decode_2bit(const uint8_t **inbuf, size_t inbufsize,
                               int32_t **outbuf, int *outbufcount)
{
    int ch;
    int i;
    int32_t *pcmbuf;
    int samples;

    samples = inbufsize / (4 * fmt->channels) - 1;
    *outbufcount += (samples << 4);
    while (samples-- > 0)
    {
        for (ch = 0; ch < fmt->channels; ch++)
        {
            pcmbuf = *outbuf + ch;
            for (i = 0; i < 4; i++)
            {
                *pcmbuf = create_pcmdata(ch, **inbuf     ) << IMA_ADPCM_INC_DEPTH;
                pcmbuf += fmt->channels;
                *pcmbuf = create_pcmdata(ch, **inbuf >> 2) << IMA_ADPCM_INC_DEPTH;
                pcmbuf += fmt->channels;
                *pcmbuf = create_pcmdata(ch, **inbuf >> 4) << IMA_ADPCM_INC_DEPTH;
                pcmbuf += fmt->channels;
                *pcmbuf = create_pcmdata(ch, **inbuf >> 6) << IMA_ADPCM_INC_DEPTH;
                pcmbuf += fmt->channels;
                (*inbuf)++;
            }
        }
        *outbuf += 16 * fmt->channels;
    }
}

static inline void decode_3bit(const uint8_t **inbuf, size_t inbufsize,
                               int32_t **outbuf, int *outbufcount)
{
    const uint8_t *adpcmbuf;
    uint32_t adpcms;
    int ch;
    int i;
    int32_t *pcmbuf;
    int samples;

    samples = (inbufsize - 4 * fmt->channels) / (12 * fmt->channels);
    *outbufcount += (samples << 5);
    while (samples--)
    {
        for (ch = 0; ch < fmt->channels; ch++)
        {
            adpcmbuf = *inbuf + ch * 4;
            pcmbuf = *outbuf + ch;
            adpcms = *adpcmbuf++;
            adpcms |= (*adpcmbuf++) << 8;
            adpcms |= (*adpcmbuf++) << 16;
            for (i = 0; i < 8; i++)
            {
                *pcmbuf = create_pcmdata(ch, adpcms >> (3 * i)) << IMA_ADPCM_INC_DEPTH;
                pcmbuf += fmt->channels;
            }
            adpcms = *adpcmbuf++;
            adpcmbuf += (fmt->channels - 1) * 4;
            adpcms |= (*adpcmbuf++) << 8;
            adpcms |= (*adpcmbuf++) << 16;
            for (i = 0; i < 8; i++)
            {
                *pcmbuf = create_pcmdata(ch, adpcms >> (3 * i)) << IMA_ADPCM_INC_DEPTH;
                pcmbuf += fmt->channels;
            }
            adpcms = *adpcmbuf++;
            adpcms |= (*adpcmbuf++) << 8;
            adpcmbuf += (fmt->channels - 1) * 4;
            adpcms |= (*adpcmbuf++) << 16;
            for (i = 0; i < 8; i++)
            {
                *pcmbuf = create_pcmdata(ch, adpcms >> (3 * i)) << IMA_ADPCM_INC_DEPTH;
                pcmbuf += fmt->channels;
            }
            adpcms = *adpcmbuf++;
            adpcms |= (*adpcmbuf++) << 8;
            adpcms |= (*adpcmbuf++) << 16;
            for (i = 0; i < 8; i++)
            {
                *pcmbuf = create_pcmdata(ch, adpcms >> (3 * i)) << IMA_ADPCM_INC_DEPTH;
                pcmbuf += fmt->channels;
            }
        }
        *outbuf += 32 * fmt->channels;
        *inbuf += 12 * fmt->channels;
    }
}

static inline void decode_4bit(const uint8_t **inbuf, size_t inbufsize,
                               int32_t **outbuf, int *outbufcount)
{
    int ch;
    int i;
    int32_t *pcmbuf;
    int samples;

    samples = inbufsize / (4 * fmt->channels) - 1;
    *outbufcount += (samples << 3);
    while (samples-- > 0)
    {
        for (ch = 0; ch < fmt->channels; ch++)
        {
            pcmbuf = *outbuf + ch;
            for (i = 0; i < 4; i++)
            {
                *pcmbuf = create_pcmdata_size4(ch, **inbuf     ) << IMA_ADPCM_INC_DEPTH;
                pcmbuf += fmt->channels;
                *pcmbuf = create_pcmdata_size4(ch, **inbuf >> 4) << IMA_ADPCM_INC_DEPTH;
                pcmbuf += fmt->channels;
                (*inbuf)++;
            }
        }
        *outbuf += 8 * fmt->channels;
    }
}

static inline void decode_5bit(const uint8_t **inbuf, size_t inbufsize,
                               int32_t **outbuf, int *outbufcount)
{
    const uint8_t *adpcmbuf;
    uint64_t adpcms;
    int ch;
    int i;
    int32_t *pcmbuf;
    int samples;

    samples = (inbufsize - 4 * fmt->channels) / (20 * fmt->channels);
    *outbufcount += (samples << 5);
    while (samples--)
    {
        for (ch = 0; ch < fmt->channels; ch++)
        {
            adpcmbuf = *inbuf + ch * 4;
            pcmbuf = *outbuf + ch;
            adpcms = *adpcmbuf++;
            adpcms |= (*adpcmbuf++) << 8;
            adpcms |= (*adpcmbuf++) << 16;
            adpcms |= (uint64_t)(*adpcmbuf++) << 24;
            adpcmbuf += (fmt->channels - 1) * 4;
            adpcms |= (uint64_t)(*adpcmbuf++) << 32;
            for (i = 0; i < 8; i++)
            {
                *pcmbuf = create_pcmdata(ch, adpcms >> (5 * i)) << IMA_ADPCM_INC_DEPTH;
                pcmbuf += fmt->channels;
            }
            adpcms = *adpcmbuf++;
            adpcms |= (*adpcmbuf++) << 8;
            adpcms |= (*adpcmbuf++) << 16;
            adpcmbuf += (fmt->channels - 1) * 4;
            adpcms |= (uint64_t)(*adpcmbuf++) << 24;
            adpcms |= (uint64_t)(*adpcmbuf++) << 32;
            for (i = 0; i < 8; i++)
            {
                *pcmbuf = create_pcmdata(ch, adpcms >> (5 * i)) << IMA_ADPCM_INC_DEPTH;
                pcmbuf += fmt->channels;
            }
            adpcms = *adpcmbuf++;
            adpcms |= (*adpcmbuf++) << 8;
            adpcmbuf += (fmt->channels - 1) * 4;
            adpcms |= (*adpcmbuf++) << 16;
            adpcms |= (uint64_t)(*adpcmbuf++) << 24;
            adpcms |= (uint64_t)(*adpcmbuf++) << 32;
            for (i = 0; i < 8; i++)
            {
                *pcmbuf = create_pcmdata(ch, adpcms >> (5 * i)) << IMA_ADPCM_INC_DEPTH;
                pcmbuf += fmt->channels;
            }
            adpcms = *adpcmbuf++;
            adpcmbuf += (fmt->channels - 1) * 4;
            adpcms |= (*adpcmbuf++) << 8;
            adpcms |= (*adpcmbuf++) << 16;
            adpcms |= (uint64_t)(*adpcmbuf++) << 24;
            adpcms |= (uint64_t)(*adpcmbuf++) << 32;
            for (i = 0; i < 8; i++)
            {
                *pcmbuf = create_pcmdata(ch, adpcms >> (5 * i)) << IMA_ADPCM_INC_DEPTH;
                pcmbuf += fmt->channels;
            }
        }
        *outbuf += 32 * fmt->channels;
        *inbuf += 20 * fmt->channels;
    }
}

static int decode(const uint8_t *inbuf, size_t inbufsize,
                  int32_t *outbuf, int *outbufcount)
{
    int ch;
    int32_t init_pcmdata[2];
    int8_t init_index[2];

    *outbufcount = 0;
    for (ch = 0; ch < fmt->channels; ch++)
    {
        init_pcmdata[ch] = inbuf[0] | (inbuf[1] << 8);
        if (init_pcmdata[ch] > 32767)
            init_pcmdata[ch] -= 65536;

        init_index[ch] = inbuf[2];
        if (init_index[ch] > 88 || init_index[ch] < 0)
        {
            DEBUGF("CODEC_ERROR: dvi adpcm illegal step index=%d > 88\n",
                                 init_index[ch]);
            return CODEC_ERROR;
        }
        inbuf += 4;

        *outbuf++ = init_pcmdata[ch] << IMA_ADPCM_INC_DEPTH;
    }

    *outbufcount += 1;
    set_decode_parameters(fmt->channels, init_pcmdata, init_index);

    if (fmt->bitspersample == 4)
        decode_4bit(&inbuf, inbufsize, &outbuf, outbufcount);
    else if (fmt->bitspersample == 3)
        decode_3bit(&inbuf, inbufsize, &outbuf, outbufcount);
    else if (fmt->bitspersample == 5)
        decode_5bit(&inbuf, inbufsize, &outbuf, outbufcount);
    else /* fmt->bitspersample == 2 */
        decode_2bit(&inbuf, inbufsize, &outbuf, outbufcount);

    return CODEC_OK;
}

static const struct pcm_codec codec = {
                                          set_format,
                                          get_seek_pos,
                                          decode,
                                      };

const struct pcm_codec *get_dvi_adpcm_codec(void)
{
    return &codec;
}
