#include "system_windows.h"

static MCanvas container, menu_container; //main container
static MContainer *win_host,
    *menu_host;


static MElement *window_play,
    *window_fm,
    *window_metadata,
    *system_menu,
    *current_window = 0;

static SW_TYPE current_type = 0;

void sw_open(SW_TYPE type)
{
    if(type == current_type)
	return;

    mi_cont_rem(current_window);
    current_type = type;
    switch (type) {
    case SW_FM: {
	current_window = window_fm;
	break;
    }
    case SW_PLAY: {
	current_window =  window_play;
	break;
    }
    case SW_METADATA: {
	current_window =  window_play;
	break;
    }
    default:
	current_window = window_fm;
	break;
    }

    mi_cont_add(win_host, current_window);
    makise_g_print_tree(host);
}

void _sw_menu_show()
{
    mi_cont_add(menu_host, system_menu);
    mi_focus(system_menu, M_G_FOCUS_GET);
}
void _sw_menu_hide()
{
    mi_cont_rem(system_menu);
    mi_focus(current_window, M_G_FOCUS_GET);
}

void system_windows_init()
{
    warble_init();

    m_create_canvas(&container, &host->host,
		    mp_sall(0,0,0,0),
		    &ts_container_clear);
    m_create_canvas(&menu_container, &host->host,
		    mp_rel(0,0,64,64),
		    &ts_container_clear);

    win_host = &container.cont;
    menu_host = &menu_container.cont;

    window_play = window_play_init();
    window_fm = fm_init();
    system_menu = system_menu_init();

    sw_open(SW_FM);
}

