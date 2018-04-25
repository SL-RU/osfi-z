#include "system_menu.h"

static MCanvas container; //main container
static MContainer * win_host;
static MButton b_fm, b_play, b_meta, b_close_menu;
static MLable lable;


static uint8_t opened = 0;

void smenu_open()
{
    if(opened)
	_sw_menu_hide();
    else
	_sw_menu_show();
    
    opened = !opened;
    makise_g_print_tree(host);
}

static void b_fm_click(MButton* b)
{
    smenu_open();
    sw_open(SW_FM);
    makise_g_print_tree(host);
}
static void b_play_click(MButton* b)
{
    smenu_open();
    sw_open(SW_PLAY);
    makise_g_print_tree(host);
}
static void b_meta_click(MButton* b)
{
    smenu_open();
    sw_open(SW_METADATA);
    makise_g_print_tree(host);
}
static void b_close_click(MButton* b)
{
    smenu_open();
    makise_g_print_tree(host);
}

MElement * system_menu_init()
{    
    m_create_canvas(&container, 0,
		    mp_sall(0,0,0,0),
		    &ts_container_black);
    m_canvas_set_isolated(&container, MContainer_Isolated);
    
    win_host = &container.cont;
    opened = 0;

    m_create_button(&b_fm, win_host,
    		    mp_rel(0, 0, 60, 20),
    		    &ts_button);
    m_button_set_text(&b_fm, "Files");
    m_button_set_click(&b_fm, &b_fm_click);

    m_create_button(&b_play, win_host,
    		    mp_rel(0, 23, 50, 20),
    		    &ts_button);
    m_button_set_text(&b_play, "Play");
    m_button_set_click(&b_play, &b_play_click);
    
    m_create_button(&b_meta, win_host,
    		    mp_rel(0, 50, 50, 20),
    		    &ts_button);
    m_button_set_text(&b_meta, "Meta");
    m_button_set_click(&b_meta, &b_meta_click);

    /* m_create_lable(&lable, win_host, */
    /* 		   mp_rel(0, 10, 60, 15), */
    /* 		   &ts_lable); */
    /* m_lable_set_text(&lable, "LOL"); */


    /* m_create_button(&b_close_menu, 0, */
    /* 		    mp_rel(0, 46, 50, 20), */
    /* 		    &ts_button); */
    /* m_button_set_text(&b_close_menu, "Close menu"); */
    /* m_button_set_click(&b_close_menu, &b_close_click); */
    
    return &container.el;
}
