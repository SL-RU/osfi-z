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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <inttypes.h>
#include "platform.h"

#include "metadata.h"
#include "metadata_common.h"
#include "metadata_parsers.h"
#include "logf.h"

bool get_flac_metadata(int fd, struct mp3entry* id3)
{
    /* A simple parser to read vital metadata from a FLAC file - length,
     * frequency, bitrate etc. This code should either be moved to a 
     * seperate file, or discarded in favour of the libFLAC code.
     * The FLAC stream specification can be found at 
     * http://flac.sourceforge.net/format.html#stream
     */

    /* Use the trackname part of the id3 structure as a temporary buffer */
    unsigned char* buf = (unsigned char *)id3->path;
    bool last_metadata = false;
    bool rc = false;

    if (!skip_id3v2(fd, id3) || (read(fd, buf, 4) < 4))
    {
        return rc;
    }
    
    if (memcmp(buf, "fLaC", 4) != 0) 
    {
        return rc;
    }

    while (!last_metadata) 
    {
        unsigned long i;
        int type;
        
        if (read(fd, buf, 4) < 0)
        {
            return rc;
        }
        
        last_metadata = buf[0] & 0x80;
        type = buf[0] & 0x7f;
        /* The length of the block */
        i = (buf[1] << 16) | (buf[2] << 8) | buf[3];

        if (type == 0)       /* 0 is the STREAMINFO block */
        {
            unsigned long totalsamples;
            
            if (i >= sizeof(id3->path) || read(fd, buf, i) < 0)
            {
                return rc;
            }
          
            id3->vbr = true;   /* All FLAC files are VBR */
            id3->filesize = filesize(fd);
            id3->frequency = (buf[10] << 12) | (buf[11] << 4) 
                | ((buf[12] & 0xf0) >> 4);
            rc = true;  /* Got vital metadata */

            /* totalsamples is a 36-bit field, but we assume <= 32 bits are used */
            totalsamples = get_long_be(&buf[14]);
            
            if(totalsamples > 0)
            {
                /* Calculate track length (in ms) and estimate the bitrate (in kbit/s) */        
                id3->length = ((int64_t) totalsamples * 1000) / id3->frequency;
                id3->bitrate = (id3->filesize * 8) / id3->length;   
            } 
            else if (totalsamples == 0)
            {
                id3->length = 0;
                id3->bitrate = 0;
            }
            else 
            {
                logf("flac length invalid!");
                return false;
            }

        } 
        else if (type == 4)  /* 4 is the VORBIS_COMMENT block */
        {
            /* The next i bytes of the file contain the VORBIS COMMENTS. */
            if (read_vorbis_tags(fd, id3, i) == 0)
            {
                return rc;
            }
        } 
#ifdef HAVE_ALBUMART
        else if (type == 6) /* 6 is the PICTURE block */
        {
            if(!id3->has_embedded_albumart) /* only use the first PICTURE */
            {
                unsigned int buf_size = MIN(sizeof(id3->path), i);
                int picframe_pos = 4; /* skip picture type */
                int mime_length, description_length;

                id3->albumart.pos = lseek(fd, 0, SEEK_CUR);

                int bytes_read = read(fd, buf, buf_size);
                i -= bytes_read;

                mime_length = get_long_be(&buf[picframe_pos]);

                char *mime = buf + picframe_pos + 4;
                picframe_pos +=  4 + mime_length;

                id3->albumart.type = AA_TYPE_UNKNOWN;
                if (memcmp(mime, "image/", 6) == 0)
                {
                    mime += 6;
                    if (strcmp(mime, "jpeg") == 0 || strcmp(mime, "jpg") == 0){
                        id3->albumart.type = AA_TYPE_JPG;
                    }else if (strcmp(mime, "png") == 0)
                        id3->albumart.type = AA_TYPE_PNG;
                }

                description_length  = get_long_be(&buf[picframe_pos]);

                /* 16 = skip picture width,height,color-depth,color-used */
                picframe_pos += 4 + description_length + 16;

                /* if we support the format and image length is in the buffer */
                if(id3->albumart.type != AA_TYPE_UNKNOWN
                   && (picframe_pos + 4) - buf_size > 0)
                {
                    id3->has_embedded_albumart = true;
                    id3->albumart.size = get_long_be(&buf[picframe_pos]);
                    id3->albumart.pos += picframe_pos + 4;
                }
            }

            if (lseek(fd, i, SEEK_CUR) < 0)
            {
                return rc;
            }
        }
#endif
        else if (!last_metadata)
        {
            /* Skip to next metadata block */
            if (lseek(fd, i, SEEK_CUR) < 0)
            {
                return rc;
            }
        }
    }
    
    return true;
}
