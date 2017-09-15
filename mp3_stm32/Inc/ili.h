#include "stm32f4xx_hal.h"
#include "spi.h"
#include "gpio.h"

#define TFT_PORT	GPIOA
#define TFT_SPI		&hspi1

#define PIN_LED		GPIO_PIN_1
#define PIN_RST		GPIO_PIN_3
#define PIN_DC		GPIO_PIN_4
#define PIN_CS		GPIO_PIN_2
#define PIN_SCK		GPIO_PIN_5
#define PIN_MISO	GPIO_PIN_6
#define PIN_MOSI	GPIO_PIN_7

// Colors
#define RED		0xf800
#define GREEN		0x07e0
#define BLUE		0x001f
#define BLACK		0x0000
#define YELLOW		0xffe0
#define WHITE		0xffff
#define CYAN		0x07ff
#define BRIGHT_RED	0xf810
#define GRAY1		0x8410
#define GRAY2		0x4208

#define TFT_CS_LOW		HAL_GPIO_WritePin(TFT_PORT, PIN_CS, GPIO_PIN_RESET);
#define TFT_CS_HIGH		HAL_GPIO_WritePin(TFT_PORT, PIN_CS, GPIO_PIN_SET);
#define TFT_DC_LOW		HAL_GPIO_WritePin(TFT_PORT, PIN_DC, GPIO_PIN_RESET);
#define TFT_DC_HIGH		HAL_GPIO_WritePin(TFT_PORT, PIN_DC, GPIO_PIN_SET);
#define TFT_RST_LOW		HAL_GPIO_WritePin(TFT_PORT, PIN_RST, GPIO_PIN_RESET);
#define TFT_RST_HIGH	        HAL_GPIO_WritePin(TFT_PORT, PIN_RST, GPIO_PIN_SET);

void TFT_setPixel(int poX, int poY, int color);
void TFT_led(int state);
void TFT_init();
