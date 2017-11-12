//#include <stdio.h>
#include "usart.h"
#include  <errno.h>
#include  <sys/unistd.h> // STDOUT_FILENO, STDERR_FILENO
#include "FreeRTOS.h"
#include "semphr.h"
/***************************************************************************/


static uint8_t mut_inited = 0;
static xSemaphoreHandle pr_mutex;

void output_init(void)
{
    printf("output init\n");
    pr_mutex = xSemaphoreCreateMutex();
    xSemaphoreGive(pr_mutex);
    mut_inited = 1;
    //return (int)(pr_mutex != NULL);
}

/* For GCC compiler revise _write() function for printf functionality */
int _write(int file, char *ptr, int len)
{
    uint8_t res = 0;
    if(mut_inited)
	res = (int)(xSemaphoreTake(pr_mutex, 1000) == pdTRUE);

    file = file;
    HAL_UART_Transmit(&huart4, (uint8_t*)ptr, len, 10);

    if(mut_inited)
	xSemaphoreGive(pr_mutex);
    
    return len;
}
