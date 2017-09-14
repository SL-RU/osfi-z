#include "wav.h"

uint16_t WAV_Buffer[24000] = {0};
//double M_PI = 0;

FIL * WAV_fil = 0;
WAV_HEADER WAV_Head;
unsigned long WAV_pos = 0;


uint16_t * wav_getBuf(void)
{
	return WAV_Buffer;
}

void open_f(FIL *f)
{
	WAV_fil = f;
	uint32_t br;
	f_read(f, &WAV_Head, sizeof(WAV_Head), &br);
	if(br == sizeof(WAV_Head))
	{
		printf("Wav header ok, sample_rate = %ld, chann %d, bits %d\n", WAV_Head.sampleRate, WAV_Head.numChannels, WAV_Head.bitsPerSample);
		
	}
}

void wav_swch(uint8_t half)
{
	if(half)
		WAV_pos = 0;
	else
		WAV_pos = 12000;
}

void wav_decode(void)
{
	if(WAV_fil != 0)
	{
		printf("d");
		uint32_t br;
		f_read(WAV_fil, WAV_Buffer + WAV_pos, 24000, &br);
		if(br != 24000)
		{
			WAV_fil = 0;
			printf("done");
		}
	}
	else
	{
		for(int i = 0; i<960; i++)
			WAV_Buffer[i] = 0;
	}
	
	return;
}
