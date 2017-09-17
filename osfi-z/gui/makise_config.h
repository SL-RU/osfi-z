#ifndef MAKISE_CONFIG_H
#define  MAKISE_CONFIG_H 1

//comment unused displays
//#define MAKISE_SDL2_USE 1
//#define MAKISE_ILI9340_USE 1


#define MAKISEGUI_DRIVER_DEPTH 1
#define MAKISEGUI_BUFFER_DEPTH 1
#define MAKISEGUI_BUFFER_DEPTHMASK 0b1


#define MAKISE_BUF_H 64
#define MAKISE_BUF_W 128

//Control section
//len of buffer with input events
#define MAKISE_GUI_INPUT_BUFFER_LEN 10
//use x-y pointing control devices. like mouse or touchscreen
#define MAKISE_GUI_INPUT_POINTER_ENABLE 0



#ifdef MAKISE_ILI9340_USE
#include "ili9340.h"
#endif

#endif
