#include "makise_ssd1306.h"



#define SSD1306_COMMAND                                 0x00
#define SSD1306_DATA                                    0xC0
#define SSD1306_DATA_CONTINUE 0x40

static MakiseGUI *mgui;

void ssd1306_render();
void ssd1306_send();

void ssd1306_driver(MakiseDriver * d)
{
    d->lcd_width     = 128;
    d->lcd_height    = 64;
    d->buffer_height = MAKISE_BUF_H;
    d->buffer_width  = MAKISE_BUF_W;
    d->pixeldepth    = 1;
    d->buffer        = 0;
    d->size          = MAKISE_BUF_H * MAKISE_BUF_W / 8;
    d->posx          = 0;
    d->posy          = 0;
    d->init          = &ssd1306_init;
    d->start         = 0;
    d->sleep         = 0;
    d->awake         = 0;
    d->set_backlight = 0;
}

void SSD1306_sendCmd(uint8_t cmd)
{
    uint8_t data[1];
    data[0] = cmd;
    HAL_I2C_Mem_Write(&SSD1306_I2C, SSD1306_I2C_ADDR, SSD1306_COMMAND, I2C_MEMADD_SIZE_8BIT,
                      data, 1, 100);
}

uint8_t ssd1306_init (MakiseGUI* gui)
{
    mgui = gui;
    /* Init I2C */
    /* Check if LCD connected to I2C */
    if (HAL_I2C_IsDeviceReady(&SSD1306_I2C, SSD1306_I2C_ADDR, 1, 20000) != HAL_OK) {
	/* Return false */
	return M_ERROR;
    }
	
    /* A little delay */
    uint32_t p = 2500;
    while(p>0)
	p--;
    
    /* Init LCD */
    SSD1306_sendCmd(SSD1306_DISPLAY_OFF);
    SSD1306_sendCmd(SSD1306_SET_DISPLAY_CLOCK_DIV_RATIO);
    SSD1306_sendCmd(0x80);
    SSD1306_sendCmd(SSD1306_SET_MULTIPLEX_RATIO);
    SSD1306_sendCmd(0x3F);
    SSD1306_sendCmd(SSD1306_SET_DISPLAY_OFFSET);
    SSD1306_sendCmd(0x0);
    SSD1306_sendCmd(SSD1306_SET_START_LINE | 0x0);
    SSD1306_sendCmd(SSD1306_CHARGE_PUMP);
    SSD1306_sendCmd(0x14);
    SSD1306_sendCmd(SSD1306_MEMORY_ADDR_MODE);
    SSD1306_sendCmd(0b00);
    SSD1306_sendCmd(SSD1306_SET_SEGMENT_REMAP | 0x1);
    SSD1306_sendCmd(SSD1306_COM_SCAN_DIR_DEC);
    SSD1306_sendCmd(SSD1306_SET_COM_PINS);
    SSD1306_sendCmd(0x12);
    SSD1306_sendCmd(SSD1306_SET_CONTRAST_CONTROL);
    SSD1306_sendCmd(0xCF);
    SSD1306_sendCmd(SSD1306_SET_PRECHARGE_PERIOD);
    SSD1306_sendCmd(0xF1);
    SSD1306_sendCmd(SSD1306_SET_VCOM_DESELECT);
    SSD1306_sendCmd(0x40);
    SSD1306_sendCmd(SSD1306_DISPLAY_ALL_ON_RESUME);
    SSD1306_sendCmd(SSD1306_NORMAL_DISPLAY);
    SSD1306_sendCmd(SSD1306_DISPLAY_ON);


    memset(gui->driver->buffer, 0, SSD1306_WIDTH * SSD1306_HEIGHT / 8 + 1);
    
    /* ssd1306_render(); */
    /* ssd1306_send(); */
    /* ssd1306_render(); */

    
    /* Return OK */
    return M_OK;
}

void ssd1306_render()
{
    memset(mgui->buffer->buffer, 0, SSD1306_WIDTH * SSD1306_HEIGHT / 8);
    if(mgui->predraw != 0)
    {
	mgui->predraw(mgui);
    }
    //printf("r");
    if(mgui->draw != 0)
    {
	mgui->draw(mgui);
    }
//    printf("d");
    if(mgui->update != 0)
    {
	mgui->update(mgui);
    }
//    printf("u");


    /* for (uint8_t x = 0; x < 128; x++) { */
    /* 	for (uint8_t y = 0; y < 64; y++) { */
    /* 	    if(makise_pget_fast(gui->buffer, x, y)) */
    /* 		((uint8_t*)gui->driver->buffer)[x+ (y/8)*SSD1306_WIDTH] |= (1 << y%8); */
    /* 	    else */
    /* 		((uint8_t*)gui->driver->buffer)[x+ (y/8)*SSD1306_WIDTH] &= ~(1 << y%8);  */
	   
    /* 	}	 */
    /* } */
    //memcpy((uint8_t*)mgui->driver->buffer, (uint8_t*)mgui->buffer->buffer, SSD1306_WIDTH * SSD1306_HEIGHT / 8);
    

}

void ssd1306_send()
{
    memcpy((uint8_t*)mgui->driver->buffer, (uint8_t*)mgui->buffer->buffer, SSD1306_WIDTH * SSD1306_HEIGHT / 8);

    //printf("send\n");
    SSD1306_sendCmd(SSD1306_SET_COLUMN_ADDR);
    SSD1306_sendCmd(0);
    SSD1306_sendCmd(127);
    SSD1306_sendCmd(SSD1306_SET_PAGE_ADDR);
    SSD1306_sendCmd(0);
    SSD1306_sendCmd(7);

//    printf("o%d",
    HAL_I2C_Mem_Write_DMA(&SSD1306_I2C,
			  SSD1306_I2C_ADDR,
			  SSD1306_DATA_CONTINUE,
			  I2C_MEMADD_SIZE_8BIT,
			  (uint8_t*)mgui->driver->buffer,
			  SSD1306_WIDTH * SSD1306_HEIGHT / 8)
//	)
	;
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    ssd1306_send();
//    printf("s");
    ssd1306_render();
}
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    printf("OLED I2C ERROR!!!\n\r");
}
static uint8_t rendered = 0;
static uint8_t sent = 0;
void SSD1306_UpdateScreen(MakiseGUI* gui) {
    if(!rendered)
    {
	rendered = 1;
	ssd1306_render();
	ssd1306_send();
	ssd1306_render();
    }
    /* if(HAL_DMA_GetState(SSD1306_I2C.hdmatx) == HAL_DMA_STATE_READY && rendered) */
    /* { */
    /* 	printf("send oled\n"); */

    /* } */
    //HAL_Delay(100);
    //memset(gui->driver->buffer, 1, SSD1306_WIDTH * SSD1306_HEIGHT / 8);

    /* while(HAL_DMA_GetState(SSD1306_I2C.hdmatx) != HAL_DMA_STATE_READY) */
    /* { */
    /* 	if(sys_isInited() == SYS_OK) */
    /* 		osDelay(5); */
    /* 	else */
    /* 		HAL_Delay(1); */
    /* } */

	
}

