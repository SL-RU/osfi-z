#ifndef W_METADATA_H
#define W_METADATA_H
#include "gui.h"
#include "gui_styles.h"
#include "makise_e.h"
#include "warble.h"
#include "gui_helpers.h"
#include "window_play.h"
#include "system_windows.h"
#include "fm.h"


MElement * window_metadata_init();

void window_metadata_update(WTrack *track);

#endif
