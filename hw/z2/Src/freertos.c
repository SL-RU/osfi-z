/**
******************************************************************************
* File Name          : freertos.c
* Description        : Code for freertos applications
******************************************************************************
*
* Copyright (c) 2017 STMicroelectronics International N.V. 
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted, provided that the following conditions are met:
*
* 1. Redistribution of source code must retain the above copyright notice, 
*    this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
* 3. Neither the name of STMicroelectronics nor the names of other 
*    contributors to this software may be used to endorse or promote products 
*    derived from this software without specific written permission.
* 4. This software, including modifications and/or derivative works of this 
*    software, must execute solely and exclusively on microcontroller or
*    microprocessor devices manufactured by or for STMicroelectronics.
* 5. Redistribution and use of this software other than as permitted under 
*    this license is void and will automatically terminate your rights under 
*    this license. 
*
* THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
* AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
* PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
* RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
* SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */     
#include "i2s.h"
#include "sdio.h"
#include "usart.h"
#include "gpio.h"
#include "fatfs.h"
#include "string.h"
#include "gui.h"
#include "gui_controls.h"
#include "fm.h"
#include "warble.h"
/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId defaultTaskHandle;
osThreadId guiThreadHandle;

/* USER CODE BEGIN Variables */
FATFS fileSystem;
static char* err_msg = 0;
static MLable err_labl;
/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const * argument);
void guiStart(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */
static uint8_t addr = 0b0010000 << 1;

static void setr(uint8_t reg, uint8_t val)
{
    uint8_t tr[2] = {reg, val};
    HAL_I2C_Master_Transmit(&hi2c1, addr, tr, 2, 100);
}

static void init_error() //show error message with required err_msg
{
    gui_init();
    m_create_lable(&err_labl,
    		   host->host,
    		   mp_sall(0, 0, 20, 0), err_msg,
    		   &ts_lable);

    SSD1306_UpdateScreen(mGui);
}
osThreadDef(defaultTask, StartDefaultTask, osPriorityHigh, 0, 4048);

void start_warble()
{
    
    defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);
}
/* USER CODE END FunctionPrototypes */

/* Hook prototypes */

/* Init FreeRTOS */

void MX_FREERTOS_Init(void) {
    /* USER CODE BEGIN Init */
       
    /* USER CODE END Init */

    /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
    /* USER CODE END RTOS_MUTEX */

    /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
    /* USER CODE END RTOS_SEMAPHORES */

    /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
    /* USER CODE END RTOS_TIMERS */

    /* Create the thread(s) */
    /* definition and creation of defaultTask */
    osThreadDef(guiThread, guiStart, osPriorityNormal, 0, 512);
    guiThreadHandle = osThreadCreate(osThread(guiThread), NULL);

    /* definition and creation of guiThread */
    /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
    /* USER CODE END RTOS_THREADS */

    /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
    /* USER CODE END RTOS_QUEUES */
}

/* StartDefaultTask function */
void StartDefaultTask(void const * argument)
{

    /* USER CODE BEGIN StartDefaultTask */
    dmain();
    /* USER CODE END StartDefaultTask */
}

/* guiStart function */
void guiStart(void const * argument)
{
    /* USER CODE BEGIN guiStart */
    /* Infinite loop */
    FRESULT res;
    if((res = f_mount(&fileSystem, SD_Path, 1)) == FR_OK)
    {
    	printf("SD CARD OK\n");
    }
    else
    {
    	printf("no sd %d\n", res);
	err_msg = "NO SD!!!";
	init_error();
	return;
    }
    
    HAL_GPIO_WritePin(DAC_PDN_GPIO_Port, DAC_PDN_Pin, GPIO_PIN_RESET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(DAC_PDN_GPIO_Port, DAC_PDN_Pin, GPIO_PIN_SET);
    setr(0x00, 0b10000000);
    setr(0x01, 0b00100010);
    setr(0x02, 0b00010000);
    setr(0x03, 0b11111111);
    setr(0x04, 0b11111111);
    setr(0x05, 0b00000000);
    setr(0x06, 0b10000000);
    setr(0x00, 0b10000001);

    gui_init();
    gui_controls_init();
    fm_init();
    SSD1306_UpdateScreen(mGui);
    /* Infinite loop */
    for(;;)
    {
	gui_controls_update();
	xSemaphoreTake(SSD1306_semaphore, portMAX_DELAY);
	ssd1306_send();
	makise_gui_input_perform(host);
	ssd1306_render();
    }
    /* USER CODE END guiStart */
}
/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
