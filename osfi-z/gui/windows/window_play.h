#ifndef W_PLAY_H
#define W_PLAY_H 1
#include "gui.h"
#include "makise_e.h"
#include "warble.h"

MElement * window_play_init (MContainer * host);

void window_play_update();
void gotmetadata(WTrack *track);

#endif
