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
#include <inttypes.h>
#include "metadata.h"

#ifdef ROCKBOX_BIG_ENDIAN
#define IS_BIG_ENDIAN 1
#else
#define IS_BIG_ENDIAN 0
#endif

#define TAG_NAME_LENGTH             32
#define TAG_VALUE_LENGTH            128

#define FOURCC(a,b,c,d) (((a)<<24) | ((b) << 16) | ((c) << 8) | (d))

enum tagtype { TAGTYPE_APE = 1, TAGTYPE_VORBIS };

bool read_ape_tags(int fd, struct mp3entry* id3);
long read_vorbis_tags(int fd, struct mp3entry *id3,
    long tag_remaining);

bool skip_id3v2(int fd, struct mp3entry *id3);
long read_string(int fd, char* buf, long buf_size, int eos, long size);

int read_uint8(int fd, uint8_t* buf);
#ifdef ROCKBOX_BIG_ENDIAN
#define read_uint16be(fd,buf) read((fd), (buf), 2)
#define read_uint32be(fd,buf) read((fd), (buf), 4)
#define read_uint64be(fd,buf) read((fd), (buf), 8)
int read_uint16le(int fd, uint16_t* buf);
int read_uint32le(int fd, uint32_t* buf);
int read_uint64le(int fd, uint64_t* buf);
#else
int read_uint16be(int fd, uint16_t* buf);
int read_uint32be(int fd, uint32_t* buf);
int read_uint64be(int fd, uint64_t* buf);
#define read_uint16le(fd,buf) read((fd), (buf), 2)
#define read_uint32le(fd,buf) read((fd), (buf), 4)
#define read_uint64le(fd,buf) read((fd), (buf), 8)
#endif

uint64_t get_uint64_le(void* buf);
uint32_t get_long_le(void* buf);
uint16_t get_short_le(void* buf);
uint32_t get_long_be(void* buf);
uint16_t get_short_be(void* buf);
int32_t get_slong(void* buf);
uint32_t get_itunes_int32(char* value, int count);
long parse_tag(const char* name, char* value, struct mp3entry* id3,
    char* buf, long buf_remaining, enum tagtype type);
