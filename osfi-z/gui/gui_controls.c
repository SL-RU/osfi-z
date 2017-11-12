#include "gui_controls.h"

static controls_gpio_ButtonState buttons[] = {
    {1, GUI_KEY_PREV, B_PREV_GPIO_Port, B_PREV_Pin, 0, 0, 0}, //Play/Pause
    {1, GUI_KEY_NEXT, B_NEXT_GPIO_Port, B_NEXT_Pin , 0, 0, 0}, //main encoder
    {1, GUI_KEY_STOP, B_STOP_GPIO_Port, B_STOP_Pin, 0, 0, 0}, //Left encoder
};
static uint32_t cont_buttons = 4;

static controls_gpio_OutputState leds[] = {
    {1, GPIOA, GPIO_PIN_4 , 0}, //Play state
    {1, GPIOC, GPIO_PIN_9 , 0}, //Right mute state
    {1, GPIOC, GPIO_PIN_14 , 0}, //Left mute state
};
static uint32_t cont_leds = 0;

void button_handler(
    uint32_t id,
    uint8_t event,
    uint32_t time)
{
    if(event & CONTROLS_ALL_CLICK)
    {
	switch (id) {
	case GUI_KEY_NEXT:
	    if(event & CONTROLS_LONG_CLICK)
	    {
		// TODO: Delete!!!
		tgl();
	    }
	    else
		makise_gui_input_send_button(host,
					     M_KEY_DOWN,
					     M_INPUT_CLICK, time);
	    break;
	case GUI_KEY_PREV:
	    makise_gui_input_send_button(host,
					 M_KEY_UP,
					 M_INPUT_CLICK, time);
  	    break;
	case GUI_KEY_STOP:
	    makise_gui_input_send_button(host,
					 M_KEY_OK,
					 M_INPUT_CLICK, time);
  	    break;
	default:
	    break;
	}
    }
}


void gui_controls_init()
{
    controls_gpio_init(buttons, cont_buttons,
		       leds, cont_leds, &button_handler);
}
void gui_controls_update()
{
    controls_gpio_update();
}

void gui_controls_setled(uint16_t led, uint8_t state)
{
    if(led < cont_leds)
	leds[led].val = state;
}
