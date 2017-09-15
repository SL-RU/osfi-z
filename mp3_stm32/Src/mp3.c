
#include "mp3.h"


HMP3Decoder hMP3Decoder;
MP3FrameInfo mp3FrameInfo;


FIL* mp3_file;							// указатель на играемый файл
uint32_t fileAddr=0;						// адрес первого непрочитанного из файла байта

uint8_t   inputBuf[MP3_FILEBUFF_SIZE];	// буфер для входного потока
//int16_t   outBuff[DAC_BUFFER_SIZE];	// буфер выходного потока
//uint32_t  outBuffPtr = 0;					// указатель на свободный буфер

uint8_t   * inputDataPtr = 0;	// указатель начала данных в файловом буфере
int32_t   bytes_left = 0;		// указатель количества данных в файловом буфере
uint32_t  samprate = 0;
uint32_t  frameCNT = 0;			// счётчик декодированных фреймов


uint16_t mp3_buf[DAC_BUFFER_SIZE * 2];
uint16_t * mp3_buf_p = mp3_buf;

void mp3_init(void)
{
    hMP3Decoder = MP3InitDecoder();
}

void mp3_fopen(FIL* f)
{
    mp3_file = f;
    f_lseek(mp3_file, fileAddr);
    
}

/*************************************************************************************************
 * @brief	Вспомогательная функция чтения потока MP3 из файла
 *
 * @param	mp3DecoderState - указатель на переменную состояния проигрывателя
 * @return	результат выполнения команды
 ************************************************************************************************/
int ReadMP3buff()
{
    // если буфер полный, ничего не делать
    if (bytes_left >= MP3_FILEBUFF_SIZE)
	return 1;

    // если в файловом буфере остались данные, переместить их в начало буфера
    if (bytes_left > 0)
    {
	memcpy(inputBuf, inputDataPtr, bytes_left);
    }
    assert_param(bytes_left >= 0 && bytes_left <= MP3_FILEBUFF_SIZE);

    // прочитать очередную порцию данных
    uint32_t bytes_to_read = MP3_FILEBUFF_SIZE - bytes_left;
    uint32_t bytes_read;
	
    FRESULT result = f_read(mp3_file,
			    (BYTE *)inputBuf + bytes_left,
			    bytes_to_read,
			    (UINT*)&bytes_read);
    printf("fr %ld\n", bytes_read);
    fileAddr += bytes_to_read;

    if (result != FR_OK)
    {
	return 0;
    }

    inputDataPtr = inputBuf;
    bytes_left += bytes_read;

    if (bytes_read == bytes_to_read)
	return 1;
    else
	return 0;
}

uint16_t * mp3_getBuf(void)
{
    return mp3_buf;
}

void mp3_swch(uint8_t half)
{
    mp3_buf_p = (half == 0) ? mp3_buf + DAC_BUFFER_SIZE : mp3_buf;
}

void mp3_decode(void)
{
    ReadMP3buff();
    //Поиск синхрослова
    int offset = MP3FindSyncWord(inputDataPtr, bytes_left);
    if (offset < 0)
    {
	// синхро не найдено, очистить буфер и прочитать из файла следующую порцию
	bytes_left = 0;
	mp3_decode();
	return;
    }
    inputDataPtr += offset;
    bytes_left -= offset;

    if(0)
    {
	int err = MP3GetNextFrameInfo(&hMP3Decoder,
				      &mp3FrameInfo,
				      inputDataPtr);
	if(err < 0 || mp3FrameInfo.layer != 3)
	{
	    bytes_left -= 2;
	    inputDataPtr += 2;
	    if (offset < 0)
	    {
		// следующее синхро не найдено, очистить буфер
		bytes_left = 0;
	    }
	    else
	    {
		// найден следующий фрейм, повторить цикл
		inputDataPtr += offset;
		bytes_left -= offset;
	    }
	}
    }
    printf("s %d b %d n %d\n", mp3FrameInfo.samprate, mp3FrameInfo.bitrate, mp3FrameInfo.nChans);

    uint16_t * outbuf = mp3_buf_p;

    MP3Decode(hMP3Decoder, &inputDataPtr, (int *)&bytes_left, (short int *)outbuf, 0);
    MP3GetLastFrameInfo(hMP3Decoder, &mp3FrameInfo);
    
    
}
