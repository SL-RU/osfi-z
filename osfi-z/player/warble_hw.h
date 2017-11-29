#ifndef WARBLE_HW_H
#define WARBLE_HW_H
#include "i2s.h"
#include "FreeRTOS.h"
#include "semphr.h"
//#include "tasks.h"
#include "dsp_core.h"
#include "task.h"
#include "timers.h"
#include "cmsis_os.h"

#define PLAYBACK_BUFFER_SIZE 4096
/* MUTEX */
#define W_MUTEX_t       xSemaphoreHandle
#define W_MUTEX_TIMEOUT 100

#include "warble.h"
//create mutex object
uint8_t warble_mutex_create (W_MUTEX_t *sobj);
//delete mutex
uint8_t warble_mutex_delete (W_MUTEX_t *sobj);
//Request Grant to Access some object
uint8_t warble_mutex_request_grant (W_MUTEX_t *sobj);
//Release Grant to Access the Volume
uint8_t warble_mutex_release_grant (W_MUTEX_t *sobj);
/* MUTEX */

uint8_t warble_hw_init();

uint8_t warble_hw_start();
uint8_t warble_hw_stop();

uint8_t warble_hw_start_thread();


uint8_t warble_hw_insert(const void *ch1, const void *ch2,
			 int count, uint8_t stereo_format);


#endif
