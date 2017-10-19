/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2006 Thom Johansen
 * Copyright (C) 2012 Michael Sevakis
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
#include "rbcodecconfig.h"
#include "audiohw.h"
#include "fixedpoint.h"
#include "fracmul.h"
#include "dsp_proc_entry.h"
#include "channel_mode.h"
#include <string.h>

#if 0
/* SOUND_CHAN_STEREO mode is a noop so has no function - just outline one for
 * completeness. */
void channel_mode_proc_stereo(struct dsp_proc_entry *this,
                              struct dsp_buffer **buf_p);
#endif
void channel_mode_proc_mono(struct dsp_proc_entry *this,
                            struct dsp_buffer **buf_p);
void channel_mode_proc_mono_left(struct dsp_proc_entry *this,
                                 struct dsp_buffer **buf_p);
void channel_mode_proc_mono_right(struct dsp_proc_entry *this,
                                  struct dsp_buffer **buf_p);
void channel_mode_proc_custom(struct dsp_proc_entry *this,
                              struct dsp_buffer **buf_p);
void channel_mode_proc_karaoke(struct dsp_proc_entry *this,
                               struct dsp_buffer **buf_p);

static struct channel_mode_data
{
    long  sw_gain;   /* 00h: for mode: custom */
    long  sw_cross;  /* 04h: for mode: custom */
    int   mode;
} channel_mode_data =
{
    .sw_gain = 0,
    .sw_cross = 0,
    .mode = SOUND_CHAN_STEREO
};

#if 0
/* SOUND_CHAN_STEREO mode is a noop so has no function - just outline one for
 * completeness. */
void channel_mode_proc_stereo(struct dsp_proc_entry *this,
                              struct dsp_buffer **buf_p)
{
    /* The channels are each just themselves */
    (void)this; (void)buf_p;
}
#endif

//#if !defined(CPU_COLDFIRE) && !defined(CPU_ARM)
/* Unoptimized routines */
void channel_mode_proc_mono(struct dsp_proc_entry *this,
                            struct dsp_buffer **buf_p)
{
    struct dsp_buffer *buf = *buf_p;
    int32_t *sl = buf->p32[0];
    int32_t *sr = buf->p32[1];
    int count = buf->remcount;

    do
    {
        int32_t lr = *sl / 2 + *sr / 2;
        *sl++ = lr;
        *sr++ = lr;
    }
    while (--count > 0);

    (void)this;
}

void channel_mode_proc_custom(struct dsp_proc_entry *this,
                              struct dsp_buffer **buf_p)
{
    struct channel_mode_data *data = (void *)this->data;
    struct dsp_buffer *buf = *buf_p;

    int32_t *sl = buf->p32[0];
    int32_t *sr = buf->p32[1];
    int count = buf->remcount;

    const int32_t gain  = data->sw_gain;
    const int32_t cross = data->sw_cross;

    do
    {
        int32_t l = *sl;
        int32_t r = *sr;
        *sl++ = FRACMUL(l, gain) + FRACMUL(r, cross);
        *sr++ = FRACMUL(r, gain) + FRACMUL(l, cross);
    }
    while (--count > 0);
}

void channel_mode_proc_karaoke(struct dsp_proc_entry *this,
                               struct dsp_buffer **buf_p)
{
    struct dsp_buffer *buf = *buf_p;
    int32_t *sl = buf->p32[0];
    int32_t *sr = buf->p32[1];
    int count = buf->remcount;

    do
    {
        int32_t ch = *sl / 2 - *sr / 2;
        *sl++ = ch;
        *sr++ = -ch;
    }
    while (--count > 0);

    (void)this;
}
//#endif /* CPU */

void channel_mode_proc_mono_left(struct dsp_proc_entry *this,
                                 struct dsp_buffer **buf_p)
{
    /* Just copy over the other channel */
    struct dsp_buffer *buf = *buf_p;
    memcpy(buf->p32[1], buf->p32[0], buf->remcount * sizeof (int32_t));
    (void)this;
}

void channel_mode_proc_mono_right(struct dsp_proc_entry *this,
                                  struct dsp_buffer **buf_p)
{
    /* Just copy over the other channel */
    struct dsp_buffer *buf = *buf_p;
    memcpy(buf->p32[0], buf->p32[1], buf->remcount * sizeof (int32_t));
    (void)this;
}

void channel_mode_set_config(int value)
{
    if (value < 0 || value >= SOUND_CHAN_NUM_MODES)
        value = SOUND_CHAN_STEREO; /* Out of range */

    if (value == channel_mode_data.mode)
        return;

    channel_mode_data.mode = value;
    dsp_proc_enable(dsp_get_config(CODEC_IDX_AUDIO), DSP_PROC_CHANNEL_MODE,
                    value != SOUND_CHAN_STEREO);
}

void channel_mode_custom_set_width(int value)
{
    long width, straight, cross;

    width = value * 0x7fffff / 100;

    if (value <= 100)
    {
        straight = (0x7fffff + width) / 2;
        cross = straight - width;
    }
    else
    {
        /* straight = (1 + width) / (2 * width) */
        straight = fp_div(0x7fffff + width, width, 22);
        cross = straight - 0x7fffff;
    }

    channel_mode_data.sw_gain  = straight << 8;
    channel_mode_data.sw_cross = cross << 8;
}

static void update_process_fn(struct dsp_proc_entry *this)
{
    static const dsp_proc_fn_type fns[SOUND_CHAN_NUM_MODES] =
    {
        [SOUND_CHAN_STEREO]     = NULL,
        [SOUND_CHAN_MONO]       = channel_mode_proc_mono,
        [SOUND_CHAN_CUSTOM]     = channel_mode_proc_custom,
        [SOUND_CHAN_MONO_LEFT]  = channel_mode_proc_mono_left,
        [SOUND_CHAN_MONO_RIGHT] = channel_mode_proc_mono_right,
        [SOUND_CHAN_KARAOKE]    = channel_mode_proc_karaoke,
    };

    this->process = fns[((struct channel_mode_data *)this->data)->mode];
}

/* Handle format changes and verify the format compatibility */
static intptr_t channel_mode_new_format(struct dsp_proc_entry *this,
                                        struct dsp_config *dsp,
                                        struct sample_format *format)
{
    DSP_PRINT_FORMAT(DSP_PROC_CHANNEL_MODE, format);

    bool active = format->num_channels >= 2;
    dsp_proc_activate(dsp, DSP_PROC_CHANNEL_MODE, active);

    if (active)
        return PROC_NEW_FORMAT_OK;

    /* Can't do this. Sleep until next change. */
    DEBUGF("  DSP_PROC_CHANNEL_MODE- deactivated\n");
    return PROC_NEW_FORMAT_DEACTIVATED;

    (void)this;
}

/* DSP message hook */
static intptr_t channel_mode_configure(struct dsp_proc_entry *this,
                                       struct dsp_config *dsp,
                                       unsigned int setting,
                                       intptr_t value)
{
    intptr_t retval = 0;

    switch (setting)
    {
    case DSP_PROC_INIT:
        if (value == 0)
            this->data = (intptr_t)&channel_mode_data;

        update_process_fn(this);
        break;

    case DSP_PROC_NEW_FORMAT:
        retval = channel_mode_new_format(this, dsp,
                                         (struct sample_format *)value);
        break;
    }

    return retval;
}

/* Database entry */
DSP_PROC_DB_ENTRY(
    CHANNEL_MODE,
    channel_mode_configure);
