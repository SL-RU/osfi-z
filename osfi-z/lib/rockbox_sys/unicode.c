/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (c) 2004,2005 by Marcoen Hirschberg
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
/*   Some conversion functions for handling UTF-8
 *
 *   I got all the info from:
 *   http://www.cl.cam.ac.uk/~mgk25/unicode.html#utf-8
 *   and
 *   http://en.wikipedia.org/wiki/Unicode
 */

#include <stdio.h>
#include "config.h"
#include "system.h"
//#include "thread.h"
#include "debug.h"
#include "rbunicode.h"
#include "rbpaths.h"
#include "pathfuncs.h"


#ifndef O_BINARY
#define O_BINARY 0
#endif
#ifndef O_NOISODECODE
#define O_NOISODECODE 0
#endif

#define getle16(p) (p[0] | (p[1] << 8))
#define getbe16(p) ((p[0] << 8) | p[1])

/* #if !defined (__PCTOOL__) && (CONFIG_PLATFORM & PLATFORM_NATIVE) */
/* /\* Because file scanning uses the default CP table when matching entries, */
/*    on-demand loading is not feasible; we also must use the filesystem lock *\/ */
/* #include "file_internal.h" */
/* #else /\* APPLICATION *\/ */
/* #ifdef __PCTOOL__ */
/* #define yield() */
/* #define DEFAULT_CP_STATIC_ALLOC */
/* #endif */
/* #define open_noiso_internal open */
/* #endif /\* !APPLICATION *\/ */

static const unsigned char utf8comp[6] =
{
    0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC
};


/* Recode an iso encoded string to UTF-8 */
unsigned char* iso_decode(const unsigned char *iso, unsigned char *utf8,
                          int cp, int count)
{
    return 0;
}

/* Recode a UTF-16 string with little-endian byte ordering to UTF-8 */
unsigned char* utf16LEdecode(const unsigned char *utf16, unsigned char *utf8,
        int count)
{
    return 0;
}

/* Recode a UTF-16 string with big-endian byte ordering to UTF-8 */
unsigned char* utf16BEdecode(const unsigned char *utf16, unsigned char *utf8,
        int count)
{
    return 0;
}

#if 0 /* currently unused */
/* Recode any UTF-16 string to UTF-8 */
unsigned char* utf16decode(const unsigned char *utf16, unsigned char *utf8,
        unsigned int count)
{
    uAnsigned long ucs;

    ucs = *(utf16++) << 8;
    ucs |= *(utf16++);

    if (ucs == 0xFEFF) /* Check for BOM */
        return utf16BEdecode(utf16, utf8, count-1);
    else if (ucs == 0xFFFE)
        return utf16LEdecode(utf16, utf8, count-1);
    else { /* ADDME: Should default be LE or BE? */
        utf16 -= 2;
        return utf16BEdecode(utf16, utf8, count);
    }
}
#endif

/* Return the number of UTF-8 chars in a string */
unsigned long utf8length(const unsigned char *utf8)
{
    unsigned long l = 0;

    while (*utf8 != 0)
        if ((*utf8++ & MASK) != COMP)
            l++;

    return l;
}

/* Decode 1 UTF-8 char and return a pointer to the next char. */
const unsigned char* utf8decode(const unsigned char *utf8, unsigned short *ucs)
{
    unsigned char c = *utf8++;
    unsigned long code;
    int tail = 0;

    if ((c <= 0x7f) || (c >= 0xc2)) {
        /* Start of new character. */
        if (c < 0x80) {        /* U-00000000 - U-0000007F, 1 byte */
            code = c;
        } else if (c < 0xe0) { /* U-00000080 - U-000007FF, 2 bytes */
            tail = 1;
            code = c & 0x1f;
        } else if (c < 0xf0) { /* U-00000800 - U-0000FFFF, 3 bytes */
            tail = 2;
            code = c & 0x0f;
        } else if (c < 0xf5) { /* U-00010000 - U-001FFFFF, 4 bytes */
            tail = 3;
            code = c & 0x07;
        } else {
            /* Invalid size. */
            code = 0xfffd;
        }

        while (tail-- && ((c = *utf8++) != 0)) {
            if ((c & 0xc0) == 0x80) {
                /* Valid continuation character. */
                code = (code << 6) | (c & 0x3f);

            } else {
                /* Invalid continuation char */
                code = 0xfffd;
                utf8--;
                break;
            }
        }
    } else {
        /* Invalid UTF-8 char */
        code = 0xfffd;
    }
    /* currently we don't support chars above U-FFFF */
    *ucs = (code < 0x10000) ? code : 0xfffd;
    return utf8;
}
unsigned char* utf8encode(unsigned long ucs, unsigned char *utf8)
{
    int tail = 0;

    if (ucs > 0x7F)
        while (ucs >> (5*tail + 6))
            tail++;

    *utf8++ = (ucs >> (6*tail)) | utf8comp[tail];
    while (tail--)
        *utf8++ = ((ucs >> (6*tail)) & (MASK ^ 0xFF)) | COMP;

    return utf8;
}
