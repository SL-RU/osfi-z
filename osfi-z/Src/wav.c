#include "wav.h"

uint16_t WAV_Buffer[WAV_len*2] = {0};
double M_PI = 0;

FIL * WAV_fil = 0;
WAV_HEADER WAV_Head;
unsigned long WAV_pos = 0;


void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

    wav_swch(1);
    wav_decode();
}
void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
    wav_swch(0);
    wav_decode();
}

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

	f_read(WAV_fil, WAV_Buffer, WAV_len*4, &br);
    }
    else
	printf("Wav header error\n");
}

void wav_swch(uint8_t half)
{
    if(half)
	WAV_pos = 0;
    else
	WAV_pos = WAV_len;
}

void wav_decode(void)
{
    if(WAV_fil != 0)
    {
	//printf("d");
	uint32_t br;
	f_read(WAV_fil, WAV_Buffer + WAV_pos, WAV_len*2, &br);
	if(br < WAV_len*2)
	{
	    WAV_fil = 0;
	    //printf("done");
	}
    }
    else
    {
	for(int i = 0; i<24000; i++)
	    WAV_Buffer[i] = 0;
    }
	
    /* return; */
	
    /* M_PI = acos(0) * 2.0; */
    /* double d = 0, v = 3; */
    /* for(int i = 0; i < 480; i++) */
    /* { */
    /* 	d = 65530.0 / 2 - sin(M_PI * ((double)(i) / 24.0)) * 65530.0 / v; */
    /* 	WAV_Buffer[i*2] =  */
    /* 	    WAV_Buffer[i*2 + 1] = (uint16_t)round(d); */
    /* 	//printf("%d\n", (uint16_t)round(d)); */
    /* } */
}
