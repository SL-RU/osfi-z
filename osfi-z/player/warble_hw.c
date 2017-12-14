#include "warble_hw.h"
#include "fixedpoint.h"

xSemaphoreHandle xI2S_semaphore;
xSemaphoreHandle xI2S_semaphore_h;

static uint8_t playback_running = 0;
static uint16_t playback_buffer[2][PLAYBACK_BUFFER_SIZE];
static int playback_decode_ind;
static int playback_decode_pos;
static int playback_decode_first;

static void dmain(void const * argument);
osThreadDef(WPlayerTask, dmain, osPriorityHigh, 0, 2048);
static osThreadId WPlayerThread;


static int32_t  dsp_history[2][3];
static uint32_t dsp_frequency;         /* input  samplerate */
static uint32_t dsp_frequency_out = 48000;     /* output samplerate */
#define TMP_BUF_COUNT 16
static int32_t tmp_buf[2][TMP_BUF_COUNT];
static int32_t tmp_buf_pos;
static uint32_t dsp_phase[2];
static uint32_t dsp_delta;

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    static BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xI2S_semaphore_h, &xHigherPriorityTaskWoken);
    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}
void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    static BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xI2S_semaphore, &xHigherPriorityTaskWoken);
    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}


uint8_t warble_hw_init()
{
    return 1;
}

uint8_t warble_hw_start()
{
#ifdef I2S_Z1
    HAL_GPIO_WritePin(D_MUTE_GPIO_Port, D_MUTE_Pin, GPIO_PIN_SET);
#endif
    xI2S_semaphore = xSemaphoreCreateCounting(10, 0);
    xI2S_semaphore_h = xSemaphoreCreateCounting(10, 0);
    
    HAL_I2S_Transmit_DMA(&hi2s3,
			 (uint16_t*)playback_buffer,
			 PLAYBACK_BUFFER_SIZE * 2);

    playback_running = 1;
    playback_decode_ind = 0;
    playback_decode_pos = 0;
    playback_decode_first = 1;
    return 1;
}
static inline int32_t FRACMUL(int32_t x, int32_t y)
{
    return (int32_t) (((int64_t)x * y) >> 31);
}
int resample_hermite(uint8_t ch,
		     int32_t *out)
{
    uint32_t count = MIN(4, 0x8000);
    uint32_t delta = dsp_delta;
    uint32_t phase, pos;
    
    const int32_t *s = tmp_buf[ch];

    /* Restore state */
    phase = dsp_phase[ch];
    pos = phase >> 16;

    int x0, x1, x2, x3;
    if (pos < 3) {
	x3 = dsp_history[ch][pos+0];
	x2 = pos < 2 ? dsp_history[ch][pos+1] : s[pos-2];
	x1 = pos < 1 ? dsp_history[ch][pos+2] : s[pos-1];
    }
    else {
	x3 = s[pos-3];
	x2 = s[pos-2];
	x1 = s[pos-1];
    }

    x0 = s[pos];

    int32_t frac = (phase & 0xffff) << 15;

    /* 4-point, 3rd-order Hermite/Catmull-Rom spline (x-form):
     * c1 = -0.5*x3 + 0.5*x1
     *    = 0.5*(x1 - x3)                <--
     *
     * v = x1 - x2, -v = x2 - x1
     * c2 = x3 - 2.5*x2 + 2*x1 - 0.5*x0
     *    = x3 + 2*(x1 - x2) - 0.5*(x0 + x2)
     *    = x3 + 2*v - 0.5*(x0 + x2)     <--
     *
     * c3 = -0.5*x3 + 1.5*x2 - 1.5*x1 + 0.5*x0
     *    = 0.5*x0 - 0.5*x3 + 0.5*(x2 - x1) + (x2 - x1)
     *    = 0.5*(x0 - x3 - v) - v        <--
     *
     * polynomial coefficients */
    int32_t c1 = (x1 - x3) >> 1;
    int32_t v = x1 - x2;
    int32_t c2 = x3 + 2*v - ((x0 + x2) >> 1);
    int32_t c3 = ((x0 - x3 - v) >> 1) - v;

    /* Evaluate polynomial at time 'frac'; Horner's rule. */
    int32_t acc;
    acc = FRACMUL(c3, frac) + c2;
    acc = FRACMUL(acc, frac) + c1;
    acc = FRACMUL(acc, frac) + x2;

    *out = acc;

    phase += delta;
    dsp_phase[ch] = phase;
    pos = phase >> 16;


    /* Save delay samples for next time. Must do this even if pos was
     * clamped before loop in order to keep record up to date. */
    dsp_history[ch][0] = pos < 3 ?
	dsp_history[ch][pos+0] : s[pos-3];
    dsp_history[ch][1] = pos < 2 ?
	dsp_history[ch][pos+1] : s[pos-2];
    dsp_history[ch][2] = pos < 1 ?
	dsp_history[ch][pos+2] : s[pos-1];

    if(phase > (count >> 16))
	return 1;
    else
	return 0;
}


uint8_t warble_hw_stop()
{
    HAL_I2S_DMAStop(&hi2s3);
#ifdef I2S_Z1
    HAL_GPIO_WritePin(D_MUTE_GPIO_Port, D_MUTE_Pin, GPIO_PIN_RESET);
#endif

    playback_running = 0;
    playback_decode_ind = 0;
    playback_decode_pos = 0;
    return 1;
}

uint8_t warble_hw_insert(const void *ch1, const void *ch2,
			 int count,
			 uint8_t stereo_mode)
{
    int i;
    
    if(stereo_mode == STEREO_INTERLEAVED) {
	count *= 2;
    }
    for (i = 0; i < count; i ++) {	
	tmp_buf[0][tmp_buf_pos] = ((int32_t*)ch1)[i];
	
	if(stereo_mode == STEREO_NONINTERLEAVED) {
	    tmp_buf[1][tmp_buf_pos] = ((int32_t*)ch2)[i];
	} else {
	    tmp_buf[1][tmp_buf_pos] = ((int32_t*)ch1)[i];
	}
	
	tmp_buf_pos ++;

	
	playback_decode_pos ++;
	playback_decode_pos ++;
	playback_buffer[playback_decode_ind][playback_decode_pos] =
		(uint16_t)(((uint32_t*)ch2)[i] >> 13);
	playback_buffer[playback_decode_ind][playback_decode_pos] =
	    (uint16_t)(((uint32_t*)ch1)[i] >> 13);
	if(playback_decode_pos >= PLAYBACK_BUFFER_SIZE)
	{
	    if (!playback_running && playback_decode_ind)
		warble_hw_start();
	    
	    if (playback_running && playback_decode_ind && !playback_decode_first)
		xSemaphoreTake(xI2S_semaphore_h, portMAX_DELAY);
	    if (playback_running && !playback_decode_ind && !playback_decode_first)
		xSemaphoreTake(xI2S_semaphore, portMAX_DELAY);

	    playback_decode_first = 0;
	    playback_decode_pos = 0;
	    playback_decode_ind = !playback_decode_ind;
	}
	    
    }

    return 1;
}

uint8_t warble_hw_set_input_freq(uint32_t f)
{
    dsp_frequency = f;
    dsp_delta = fp_div(dsp_frequency, dsp_frequency_out, 16);
    return 0;
}

uint8_t warble_mutex_create (W_MUTEX_t *sobj)
{
    *sobj = xSemaphoreCreateMutex();
    xSemaphoreGive(*sobj);
    return (int)(*sobj != NULL);
}
//delete mutex
uint8_t warble_mutex_delete (W_MUTEX_t *sobj)
{
    vSemaphoreDelete(*sobj);
    return 1;
}
//Request Grant to Access some object
uint8_t warble_mutex_request_grant (W_MUTEX_t *sobj)
{
    int res = (int)(xSemaphoreTake(*sobj, W_MUTEX_TIMEOUT)
		    == pdTRUE);
    if(res == 0)
	printf("Mutex err\n");
    return res;
}
//Release Grant to Access the Volume
uint8_t warble_mutex_release_grant (W_MUTEX_t *sobj)
{
    xSemaphoreGive(*sobj);
    return 1;
}

static void dmain(void const * argument)
{
    warble_decode_file();
    
    vTaskDelete( NULL );
}


uint8_t warble_hw_start_thread()
{
    WPlayerThread = osThreadCreate(osThread(WPlayerTask), NULL);
    return 0;
}
