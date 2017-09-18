#ifndef CONTROLS_H 
#define CONTROLS_H 
#include "controls_gpio.h"

#define CONTROLS_ClickTime 30
#define CONTROLS_LongClickTime 500
#define CONTROLS_CLICK 0b0001            /*after release & > ClickTime*/
#define CONTROLS_PRESSING 0b0010        /*>ClickTime*/
#define CONTROLS_LONG_CLICK 0b0100       /*after release & > LongClickTime*/
#define CONTROLS_LONG_PRESSING 0b1000    /*>LongClickTime*/
#define CONTROLS_ALL_PRESSING (CONTROLS_PRESSING | CONTROLS_LONG_PRESSING)
#define CONTROLS_ALL_CLICK (CONTROLS_CLICK | CONTROLS_LONG_CLICK)
#endif
