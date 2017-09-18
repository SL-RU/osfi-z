#include "controls_gpio.h"

controls_gpio_ButtonState *_controls_gpio_buttons;
controls_gpio_OutputState *_controls_gpio_outs;
uint8_t                   _controls_gpio_buttons_len;
uint8_t                   _controls_gpio_outs_len;
void                    (*_controls_gpio_buttons_handler)(
    uint32_t id,
    uint8_t event,
    uint32_t time);

void controls_gpio_init(controls_gpio_ButtonState *buttons,
		       uint8_t buttons_len,
		       controls_gpio_OutputState *outs,
		       uint8_t outs_len,
		       void (*buttons_handler)(
			   uint32_t id,
			   uint8_t event,
			   uint32_t time))
{
    _controls_gpio_buttons       = buttons;
    _controls_gpio_outs          = outs;
    _controls_gpio_buttons_len   = buttons_len;
    _controls_gpio_outs_len      = outs_len;
    _controls_gpio_buttons_handler = buttons_handler;

    controls_gpio_ButtonState b;
}

void _controls_gpio_handle_button(controls_gpio_ButtonState *b)
{
    uint8_t s = HAL_GPIO_ReadPin(b->gpio, b->pin) ==
	(b->polarity_pressed ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    //printf("bb %d\n", s);//HAL_GPIO_ReadPin(b->gpio, b->pin) == GPIO_PIN_RESET);
    uint32_t d;
    if(s) //if pressed
    {
	if(!b->pressed) //if pressing started only now
	{
	    b->pressed = 1;
	    b->press_start_time = HAL_GetTick();
	}
	else //if it's pressing already
	{
	    d = HAL_GetTick() - b->press_start_time;
	    if(d > CONTROLS_LongClickTime)
	    {
		if(_controls_gpio_buttons_handler != 0)
		    _controls_gpio_buttons_handler(b->id, CONTROLS_LONG_PRESSING, d);
	    }
	    else if(d > CONTROLS_ClickTime)
		if(_controls_gpio_buttons_handler != 0)
		    _controls_gpio_buttons_handler(b->id, CONTROLS_PRESSING, d);
	}
    }
    else if(b->pressed) //if released
    {
	//printf("but %d \n", b->id);
	b->pressed = 0;
	d = HAL_GetTick() - b->press_start_time;
	if(d > CONTROLS_LongClickTime)
	{
	    if(_controls_gpio_buttons_handler != 0)
		_controls_gpio_buttons_handler(b->id, CONTROLS_LONG_CLICK, d);
	}
	else if(d > CONTROLS_ClickTime)
	{
	    if(_controls_gpio_buttons_handler != 0)
		_controls_gpio_buttons_handler(b->id, CONTROLS_CLICK, d);	    
	}
    }
}

void controls_gpio_update()
{
    //uint32_t v = 0;

    //write outs values
    /* for(uint8_t i = 0; i < _controls_gpio_outs_len; i++) */
    /* 	if(_controls_gpio_outs[i].enabled)	     */
    /* 	    HAL_GPIO_WritePin(_controls_gpio_outs[i].gpio, */
    /* 			      _controls_gpio_outs[i].pin, */
    /* 			      _controls_gpio_outs[i].val ? */
    /* 			      GPIO_PIN_SET : */
    /* 			      GPIO_PIN_RESET); */
    //printf("----------\n");

    //uint8_t l = 0, j = 0;
    for(uint8_t i = 0; i < _controls_gpio_buttons_len; i++){
	_controls_gpio_handle_button(&_controls_gpio_buttons[i]);
    }
}
