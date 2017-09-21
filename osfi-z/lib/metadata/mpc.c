/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2005 Thom Johansen 
 * Copyright (C) 2010 Andree Buschmann
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

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include "platform.h"
#include "metadata.h"
#include "metadata_common.h"
#include "metadata_parsers.h"
#include "logf.h"
#include "replaygain.h"
#include "fixedpoint.h"

/* Needed for replay gain and clipping prevention of SV8 files. */
#define SV8_TO_SV7_CONVERT_GAIN (6482)  /* 64.82 * 100, MPC_OLD_GAIN_REF */
#define SV8_TO_SV7_CONVERT_PEAK (23119) /* 256 * 20 * log10(32768) */

static int set_replaygain_sv7(struct mp3entry* id3, 
                              bool album, 
                              long value, 
                              long used)
{
    long gain = (int16_t) ((value >> 16) & 0xffff);
    long peak = (uint16_t) (value & 0xffff);
    
    /* We use a peak value of 0 to indicate a given gain type isn't used. */
    if (peak != 0) {
        /* Save the ReplayGain data to id3-structure for further processing. */
        parse_replaygain_int(album, gain * 512 / 100, peak << 9, id3);
    }
    
    return used;
}

static int set_replaygain_sv8(struct mp3entry* id3, 
                              bool album, 
                              long gain, 
                              long peak,
                              long used)
{
    gain = (long)(SV8_TO_SV7_CONVERT_GAIN - ((gain*100)/256));

    /* Transform SV8's logarithmic peak representation to the desired linear
     * representation: linear = pow(10, peak/256/20).
     *
     * FP_BITS   = 24 bits = desired fp representation for dsp routines
     * FRAC_BITS = 12 bits = resolution used for fp_bits
     * fp_factor(peak*(1<<FRAC_BITS)/256, FRAC_BITS) << (FP_BITS-FRAC_BITS)
     **/
    peak = (fp_factor((peak-SV8_TO_SV7_CONVERT_PEAK)*16, 12) << 12);

    /* We use a peak value of 0 to indicate a given gain type isn't used. */
    if (peak != 0) {
        /* Save the ReplayGain data to id3-structure for further processing. */
        parse_replaygain_int(album, gain * 512 / 100, peak, id3);
    }
    
    return used;
}

static int sv8_get_size(uint8_t *buffer, int index, uint64_t *p_size)
{
    unsigned char tmp;
    uint64_t size = 0;

    do {
        tmp = buffer[index++];
        size = (size << 7) | (tmp & 0x7F);
    } while((tmp & 0x80));

    *p_size = size;
    return index;
}

bool get_musepack_metadata(int fd, struct mp3entry *id3)
{
    static const int32_t sfreqs[4] = { 44100, 48000, 37800, 32000 };
    uint32_t header[8];
    uint64_t samples = 0;
    int i;
    
    if (!skip_id3v2(fd, id3))
        return false;
    if (read(fd, header, 4*8) != 4*8) return false;
    /* Musepack files are little endian, might need swapping */
    for (i = 1; i < 8; i++) 
       header[i] = letoh32(header[i]); 
    if (!memcmp(header, "MP+", 3)) { /* Compare to sig "MP+" */
        unsigned int streamversion;
        header[0] = letoh32(header[0]);
        streamversion = (header[0] >> 24) & 15;
        if (streamversion == 7) {
            unsigned int gapless = (header[5] >> 31) & 0x0001;
            unsigned int last_frame_samples = (header[5] >> 20) & 0x07ff;
            unsigned int bufused = 0;
            
            id3->frequency = sfreqs[(header[2] >> 16) & 0x0003];
            samples = (uint64_t)header[1]*1152; /* 1152 is mpc frame size */
            if (gapless)
                samples -= 1152 - last_frame_samples;
            else
                samples -= 481; /* Musepack subband synth filter delay */
           
            bufused = set_replaygain_sv7(id3, false, header[3], bufused);
            bufused = set_replaygain_sv7(id3, true , header[4], bufused);
            
            id3->codectype = AFMT_MPC_SV7;
        } else {
            return false; /* only SV7 is allowed within a "MP+" signature */
        }
    } else if (!memcmp(header, "MPCK", 4)) { /* Compare to sig "MPCK" */
        uint8_t sv8_header[32];
        /* 4 bytes 'MPCK' */
        lseek(fd, 4, SEEK_SET);
        if (read(fd, sv8_header, 2) != 2) return false; /* read frame ID */
        if (!memcmp(sv8_header, "SH", 2)) { /* Stream Header ID */
            int32_t k = 0;
            uint32_t streamversion;
            uint64_t size  = 0; /* tag size */
            uint64_t dummy = 0; /* used to dummy read data from header */

            /* 4 bytes 'MPCK' +  2 'SH' */
            lseek(fd, 6, SEEK_SET);
            if (read(fd, sv8_header, 32) != 32) return false;
            
            /* Read the size of 'SH'-tag */
            k = sv8_get_size(sv8_header, k, &size);
            
            /* Skip crc32 */
            k += 4;
        
            /* Read stream version */
            streamversion = sv8_header[k++];
            if (streamversion != 8) return false; /* Only SV8 is allowed. */
            
            /* Number of samples */
            k = sv8_get_size(sv8_header, k, &samples);
            
            /* Number of leading zero-samples */
            k = sv8_get_size(sv8_header, k, &dummy);
            
            /* Sampling frequency */
            id3->frequency = sfreqs[(sv8_header[k++] >> 5) & 0x0003];
            
            /* Number of channels */
            id3->channels = (sv8_header[k++] >> 4) + 1;

            /* Skip to next tag: k = size -2 */
            k = size - 2;

            if (!memcmp(sv8_header+k, "RG", 2)) { /* Replay Gain ID */
                long peak, gain; 
                int bufused = 0;
                
                k += 2; /* 2 bytes 'RG' */
                
                /* sv8_get_size must be called to skip the right amount of
                 * bits within the header data. */
                k = sv8_get_size(sv8_header, k, &size);
                
                /* Read and set replay gain */
                if (sv8_header[k++] == 1) {
                    /* Title's peak and gain */
                    gain = (int16_t) ((sv8_header[k]<<8) + sv8_header[k+1]); k += 2;
                    peak = (uint16_t)((sv8_header[k]<<8) + sv8_header[k+1]); k += 2;
                    bufused += set_replaygain_sv8(id3, false, gain, peak, bufused);

                    /* Album's peak and gain */
                    gain = (int16_t) ((sv8_header[k]<<8) + sv8_header[k+1]); k += 2;
                    peak = (uint16_t)((sv8_header[k]<<8) + sv8_header[k+1]); k += 2;
                    bufused += set_replaygain_sv8(id3, true , gain, peak, bufused);
                }
            }
            
            id3->codectype = AFMT_MPC_SV8;
        } else {
            /* No sv8 stream header found */
            return false;
        }
    } else {
        return false; /* SV4-6 is not supported anymore */
    }

    id3->vbr = true;
    /* Estimate bitrate, we should probably subtract the various header sizes
       here for super-accurate results */
    id3->length = ((int64_t) samples * 1000) / id3->frequency;

    if (id3->length <= 0)
    {
        logf("mpc length invalid!");
        return false;
    }

    id3->filesize = filesize(fd);
    id3->bitrate = id3->filesize * 8 / id3->length;

    read_ape_tags(fd, id3);
    return true;
}
