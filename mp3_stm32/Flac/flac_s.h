#include "decoder.h"
#include <string.h>
#include <stdio.h>
#include "ff.h"

#define MAX_BLOCKSIZE 4608 
#define MAX_FRAMESIZE 20*1024  /* Maxsize in bytes of one compressed frame */
#define FLAC_OUTPUT_DEPTH 16   /* Provide samples left-shifted to 28 bits+sign */

void dump_headers(FLACContext *s);

void flac_open(FIL * fl, FLACContext * fc);

uint16_t* flac_getBuf(void);

void flac_swch(uint8_t half);

void flac_decode(void);
