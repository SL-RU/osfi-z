#include <stdio.h>
#include "usart.h"
/***************************************************************************/

/* For GCC compiler revise _write() function for printf functionality */
int _write(int file, char *ptr, int len)
{
    file = file;
    HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, 10);
    return len;
}
