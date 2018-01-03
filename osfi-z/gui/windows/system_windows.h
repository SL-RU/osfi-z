#ifndef SWINDOWS_H
#define SWINDOWS_H

typedef enum {
    SW_PLAY = 1,
    SW_FM = 2,
    SW_METADATA = 3
} SW_TYPE;


#include "gui.h"
#include "gui_styles.h"
#include "makise_e.h"
#include "warble.h"
#include "gui_helpers.h"
#include "window_play.h"
#include "system_menu.h"
#include "fm.h"


void system_windows_init();

void sw_open(SW_TYPE type);

//Use smenu_open() instead!
void _sw_menu_show();
void _sw_menu_hide();


#endif
