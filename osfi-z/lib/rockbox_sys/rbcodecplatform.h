#ifndef RBCODECPLATFORM_H_INCLUDED
#define RBCODECPLATFORM_H_INCLUDED

/* assert */
#include <assert.h>

/* O_RDONLY, O_WRONLY, O_CREAT, O_APPEND */
#include <fcntl.h>

/* isdigit, islower, isprint, isspace, toupper */
#include <ctype.h>

/* memchr, memcmp, memcpy, memmove, memset, strcat, strchr, strcmp, strcpy,
 * strlen, strncmp, strrchr */
#include <string.h>

/* strcasecmp */
#include <strings.h>

/* abs, atoi, labs, rand */
#include <stdlib.h>

#include "system-arm.h"
/* swap16, swap32 */
//#include <byteswap.h>
#ifndef swap16
#define swap16(x) bswap_16(x)
#endif
#ifndef swap32
#define swap32(x) bswap_32(x)
#endif

/* hto{be,le}{16,32}, {be,le}toh{16,32}, ROCKBOX_{BIG,LITTLE}_ENDIAN */
#include "rbendian.h"
#ifndef betoh16
#define betoh16 be16toh
#endif
#ifndef betoh32
#define betoh32 be32toh
#endif
#ifndef letoh16
#define letoh16 le16toh
#endif
#ifndef letoh32
#define letoh32 le32toh
#endif
#if BYTE_ORDER == LITTLE_ENDIAN
#define ROCKBOX_LITTLE_ENDIAN 1
#else
#define ROCKBOX_BIG_ENDIAN 1
#endif

/* filesize */
off_t filesize(int fd);

/* snprintf */
#include <stdio.h>

/* debugf, logf */
/* #ifdef DEBUG */
/* #define debugf(...) fprintf(stderr, __VA_ARGS__) */
/* #ifndef logf */
/* #define logf(...) do { fprintf(stderr, __VA_ARGS__); \ */
/*                        putc('\n', stderr);           \ */
/*                   } while (0) */
/* #endif */
/* #ifndef panicf */
/* #define panicf(...) do { fprintf(stderr, __VA_ARGS__); \ */
/*                          putc('\n', stderr);           \ */
/*                          exit(-1);                     \ */
/*                     } while (0) */
/* #endif */
/* #endif */

/* static inline bool tdspeed_alloc_buffers(int32_t **buffers, */
/*     const int *buf_s, int nbuf) */
/* { */
/*     int i; */
/*     for (i = 0; i < nbuf; i++) */
/*     { */
/*         buffers[i] = malloc(buf_s[i]); */
/*         if (!buffers[i]) */
/*             return false; */
/*     } */
/*     return true; */
/* } */

/* static inline void tdspeed_free_buffers(int32_t **buffers, int nbuf) */
/* { */
/*     int i; */
/*     for (i = 0; i < nbuf; i++) */
/*     { */
/*         free(buffers[i]); */
/*     } */
/* } */

#endif

