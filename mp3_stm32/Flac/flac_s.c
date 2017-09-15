#include "flac_s.h"

void dump_headers(FLACContext *s)
{
    printf("\n\r  Blocksize: %d .. %d\n\r", s->min_blocksize, 
	   s->max_blocksize);
    printf("  Framesize: %d .. %d\n\r", s->min_framesize, 
	   s->max_framesize);
    printf("  Samplerate: %d\n\r", s->samplerate);
    printf("  Channels: %d\n\r", s->channels);
    printf("  Bits per sample: %d\n\r", s->bps);
    printf("  Metadata length: %d\n\r", s->metadatalength);
    printf("  Total Samples: %lu\n\r",s->totalsamples);
    printf("  Duration: %d ms\n\r",s->length);
    printf("  Bitrate: %d kbps\n\r",s->bitrate);
}


FLACContext * Flc;
uint8_t * Flac_fb;
int FLAC_bytesleft;
int FLAC_consumed;
int FLAC_i;
FIL * Flac_fil;

int8_t PCM_buffer[4 * MAX_BLOCKSIZE * 2];
int8_t * PCM_buffer_cur = PCM_buffer;  
int8_t * PCM_buffer0 = PCM_buffer + 0;  
int8_t * PCM_buffer1 = PCM_buffer + 4 * MAX_BLOCKSIZE;	 
int8_t temp_buffer[4 * MAX_BLOCKSIZE ];


void flac_open(FIL * fl, FLACContext * fc)
{
    uint8_t buf[255];
//    struct stat statbuf;
    uint8_t found_streaminfo=0;
    int16_t endofmetadata=0;
    int16_t blocklength;
    uint32_t* p;
    uint32_t seekpoint_lo,seekpoint_hi;
    uint32_t offset_lo,offset_hi;
    UINT br;


    Flac_fil = fl;
    Flc = fc;
    
    f_lseek(fl, 0);

    
    printf("rd %d\n", f_read(fl, buf, 4, &br)); 

    if(memcmp(buf, "fLaC", 4) == 0)
    {
	printf("flac\n");
    }
    fc->metadatalength = 4;

    while (!endofmetadata) 
    {
        if (f_read(fl, buf, 4, &br) != FR_OK)
        {
            //return false;
        }

        endofmetadata=(buf[0]&0x80);
        blocklength = (buf[1] << 16) | (buf[2] << 8) | buf[3];
        fc->metadatalength+=blocklength+4;

        if ((buf[0] & 0x7f) == 0)       /* 0 is the STREAMINFO block */
        {
            /* FIXME: Don't trust the value of blocklength */
            if (f_read(fl, buf, blocklength, &br) != FR_OK)
            {
                //return false;
            }
          

            fc->filesize = f_size(fl);
	    
            fc->min_blocksize = (buf[0] << 8) | buf[1];
            fc->max_blocksize = (buf[2] << 8) | buf[3];
            fc->min_framesize = (buf[4] << 16) | (buf[5] << 8) | buf[6];
            fc->max_framesize = (buf[7] << 16) | (buf[8] << 8) | buf[9];
            fc->samplerate = (buf[10] << 12) | (buf[11] << 4) 
		| ((buf[12] & 0xf0) >> 4);
            fc->channels = ((buf[12]&0x0e)>>1) + 1;
            fc->bps = (((buf[12]&0x01) << 4) | ((buf[13]&0xf0)>>4) ) + 1;

            /* totalsamples is a 36-bit field, but we assume <= 32 bits are 
               used */
            fc->totalsamples = (buf[14] << 24) | (buf[15] << 16) 
		| (buf[16] << 8) | buf[17];

            /* Calculate track length (in ms) and estimate the bitrate 
               (in kbit/s) */
            fc->length = (fc->totalsamples / fc->samplerate) * 1000;

            found_streaminfo=1;
        }
	else 
	    if ((buf[0] & 0x7f) == 3) 	/* 3 is the SEEKTABLE block */
	    { 
		// printf("Seektable length = %d bytes\n",blocklength);
                while (blocklength >= 18) {
		    f_read(fl,buf,18,&br);
		    //if (br < 18) return false;
		    blocklength-=br;

		    p=(uint32_t*)buf;
		    seekpoint_hi=betoh32(*(p++));
		    seekpoint_lo=betoh32(*(p++));
		    offset_hi=betoh32(*(p++));
		    offset_lo=betoh32(*(p++));
            
		    if ((seekpoint_hi != 0xffffffff) && (seekpoint_lo != 0xffffffff))
		    {
			//printf("Seekpoint: %u, Offset=%u\n",seekpoint_lo,offset_lo);
		    }
		}
		f_lseek(fl, blocklength + f_tell(fl));
	    } else 
	    {
		/* Skip to next metadata block */
		if (f_lseek(fl, blocklength + f_tell(fl)))
		{
//		    return false;
		}
	    }
    }
    if (found_streaminfo)
    {
	fc->bitrate = ((fc->filesize-fc->metadatalength) * 8) / fc->length;
    }
    
    dump_headers(fc);

    Flac_fb = malloc(sizeof(uint8_t)*MAX_FRAMESIZE);

    f_read(fl, Flac_fb, MAX_FRAMESIZE, &FLAC_bytesleft);
    printf("open - ok\n");
    
}

uint16_t * flac_getBuf(void)
{
    return (uint16_t*)PCM_buffer;    
}

void flac_swch(uint8_t half)
{
    PCM_buffer_cur = (half != 0) ? PCM_buffer0 : PCM_buffer1;
}

void flac_decode(void)
{
    printf("d %d\n", FLAC_i);
    Flc->decoded0 = (int32_t *)PCM_buffer_cur;
    Flc->decoded1 = (int32_t *)temp_buffer;

    if(flac_decode_frame(Flc,Flac_fb,FLAC_bytesleft,(int16_t *)PCM_buffer_cur) < 0) 
    {
	printf("DECODE ERROR, ABORTING\n");
	//break;
    }

    FLAC_consumed=Flc->gb.index/8;
    memmove(Flac_fb,Flac_fb + FLAC_consumed,FLAC_bytesleft-FLAC_consumed);
    FLAC_bytesleft-=FLAC_consumed;

    uint32_t n;
    f_read(Flac_fil,Flac_fb + FLAC_bytesleft,MAX_FRAMESIZE-FLAC_bytesleft,&n);
    if (n > 0) 
    {
	printf("r %d\n", n);
        FLAC_bytesleft+=n;
    }

}
