#include "system_menu.h"

static MCanvas container; //main container
static MContainer * win_host;


static MElement *window_play,
    *window_fm,
    *windiw_metadata;

void smenu_open(SMENU_TYPE type)
{
    
}
static void onstart(WTrack *track)
{
}
static void onend(WTrack *track)
{
}

MElement * window_play_init()
{
    warble_init();
    warble_set_onend(&onend);
    warble_set_onstart(&onstart);

    
    m_create_canvas(&container, 0,
		    mp_sall(0,0,0,0),
		    &ts_container_clear);

    win_host = &container.cont;

    window_play = window_play_init();
    window_fm = fm_init();
    
    return &container.el;
}
