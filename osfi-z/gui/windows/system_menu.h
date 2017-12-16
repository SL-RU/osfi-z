#ifndef W_SMENU_H
#define W_SMENU_H
#include "gui.h"
#include "gui_styles.h"
#include "makise_e.h"
#include "warble.h"
#include "gui_helpers.h"
#include "window_play.h"
#include "fm.h"


MElement * system_menu_init();

typedef enum {
    SMENU_PLAY,
    SMENU_FM,
    SMENU_METADATA
} SMENU_TYPE;

void smenu_open(SMENU_TYPE type);
void smenu_hide();

#endif
