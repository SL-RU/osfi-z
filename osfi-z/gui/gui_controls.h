#ifndef GUI_CONTROLS_H
#define GUI_CONTROLS_H
#include "controls_gpio.h"
#include "gui.h"
#include "gpio.h"


#define GUI_KEY_PREV     1
#define GUI_KEY_NEXT     2
#define GUI_KEY_STOP     3

#define GUI_LED_MAIN     1


void gui_controls_init();
void gui_controls_update();

void gui_controls_setled(uint16_t led, uint8_t state);

#endif
