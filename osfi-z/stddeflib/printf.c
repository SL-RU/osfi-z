//#include <stdio.h>
#include "usart.h"
#include  <errno.h>
#include  <sys/unistd.h> // STDOUT_FILENO, STDERR_FILENO
/***************************************************************************/

/* For GCC compiler revise _write() function for printf functionality */
int _write(int file, char *ptr, int len)
{
    file = file;
    HAL_UART_Transmit(&huart4, (uint8_t*)ptr, len, 10);
    return len;
}
