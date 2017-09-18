#ifndef controls_gpio_H
#define controls_gpio_H 1

#include "controls.h"
#include "gpio.h"

//buttons are connected one pin to gpio register and other to the input pin of MCU
typedef struct 
{
    uint8_t enabled;
    uint32_t id; //global id;
    
    GPIO_TypeDef* gpio; //input MCU port
    uint16_t pin;       //input MCU pin

    uint8_t polarity_pressed; //if 1 - pressed when readpin = 1, else when 0
    uint8_t pressed;
    uint32_t press_start_time;
} controls_gpio_ButtonState;

typedef struct
{
    uint8_t enabled;

    GPIO_TypeDef* gpio; //input MCU port
    uint16_t pin;       //input MCU pin

    
    uint8_t val; //output value
} controls_gpio_OutputState;

void controls_gpio_init(controls_gpio_ButtonState *buttons,
		       uint8_t buttons_len,
		       controls_gpio_OutputState *outs,
		       uint8_t outs_len,
		       void (*buttons_handler)(
			   uint32_t id,
			   uint8_t event,
			   uint32_t time));

void controls_gpio_update();
#endif
