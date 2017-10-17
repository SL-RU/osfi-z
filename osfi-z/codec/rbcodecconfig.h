#ifndef RBCODECCONFIG_H_INCLUDED
#define RBCODECCONFIG_H_INCLUDED

#define CONFIG_CODEC SWCODEC
//#define HAVE_PITCHCONTROL
//#define HAVE_SW_TONE_CONTROLS

#define NUM_CORES 1
/* All the same unless a configuration option is added to warble */
#define DSP_OUT_MIN_HZ     48000
#define DSP_OUT_DEFAULT_HZ 48000
#define DSP_OUT_MAX_HZ     48000
//#ifndef __ASSEMBLER__

/* {,u}int{8,16,32,64}_t, {,U}INT{8,16,32,64}_{MIN,MAX}, intptr_t, uintptr_t */
#include <inttypes.h>

/* bool, true, false */
#include <stdbool.h>

/* NULL, offsetof, size_t */
#include <stddef.h>

/* ssize_t, off_t, open, close, read, lseek, SEEK_SET, SEEK_CUR, SEEK_END */
//#include <unistd.h>
//#define lseek(
//#include "filesystem-native.h"

/* MAX_PATH, {UCHAR,USHRT,UINT,ULONG,SCHAR,SHRT,INT,LONG}_{MIN,MAX} */
#include <limits.h>
// FIXME changing MAX_PATH is broken for now
//#define MAX_PATH PATH_MAX
// set same as rb to avoid dragons
#define MAX_PATH 260
//#endif

#endif
