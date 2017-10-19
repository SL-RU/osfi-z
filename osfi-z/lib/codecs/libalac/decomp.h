#ifndef __ALAC__DECOMP_H
#define __ALAC__DECOMP_H

#ifndef ICODE_ATTR_ALAC
#define ICODE_ATTR_ALAC ICODE_ATTR
#endif

/* Always output samples shifted to 28 bits + sign*/
#define ALAC_OUTPUT_DEPTH 29
#define SCALE16 (ALAC_OUTPUT_DEPTH - 16)
#define SCALE24 (ALAC_OUTPUT_DEPTH - 24)
#define ALAC_MAX_CHANNELS 2
#define ALAC_BLOCKSIZE 4096  /* Number of samples per channel per block */

typedef struct
{
    unsigned char *input_buffer;
    int input_buffer_bitaccumulator; /* used so we can do arbitary
                                        bit reads */

    /* rockbox: not used
    int samplesize;
    int numchannels;
    int bytespersample; */

    int bytes_consumed;

    /* stuff from setinfo */
    uint32_t setinfo_max_samples_per_frame; /* 0x1000 = 4096 */    /* max samples per frame? */
    uint8_t setinfo_7a; /* 0x00 */
    uint8_t setinfo_sample_size; /* 0x10 */
    uint8_t setinfo_rice_historymult; /* 0x28 */
    uint8_t setinfo_rice_initialhistory; /* 0x0a */
    uint8_t setinfo_rice_kmodifier; /* 0x0e */
    uint8_t setinfo_7f; /* 0x02 */
    uint16_t setinfo_80; /* 0x00ff */
    uint32_t setinfo_82; /* 0x000020e7 */
    uint32_t setinfo_86; /* 0x00069fe4 */
    uint32_t setinfo_8a_rate; /* 0x0000ac44 */
    /* end setinfo stuff */
} alac_file;

/* rockbox: not used
void create_alac(int samplesize, int numchannels, alac_file* alac)
    ICODE_ATTR_ALAC; */

int alac_decode_frame(alac_file *alac,
                      unsigned char *inbuffer,
                      int32_t outputbuffer[ALAC_MAX_CHANNELS][ALAC_BLOCKSIZE],
                      void (*yield)(void)) ICODE_ATTR_ALAC;
void alac_set_info(alac_file *alac, char *inputbuffer) ICODE_ATTR_ALAC;

#endif /* __ALAC__DECOMP_H */

