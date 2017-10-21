#include "window_play.h"


static MCanvas container; //main container
static MButton b_play, //main container
    b_next,
    b_prev,
    b_repeat;
static MContainer * win_host;

static MakiseStyle_Canvas container_style =
{
    //bg       border   double_border
    {MC_Transparent, MC_Transparent, 0},  //normal
    {MC_Transparent, MC_Transparent, 0},  //focused
};

static void click_thr(void const * argument)
{   
    vTaskDelete( NULL );
}
osThreadDef(ClickThread, click_thr, osPriorityIdle, 0, 256);
static void click(MButton* b)
{
    osThreadCreate(osThread(ClickThread), NULL);
    b->text = 0;
    
}

MElement * window_play_init(MContainer * host)
{
    
    m_create_canvas(&container, host,
		    mp_sall(0,0,0,0),
		    &container_style);

    win_host = &container.cont;
    
    m_create_button(&b_prev, win_host,
		    mp_rel(10, 10, 20, 20),
		    &ts_button);
    m_button_set_text(&b_prev, "Prev");
    m_button_set_click(&b_prev, &click);
    m_button_set_bitmap(&b_prev, &B_folder);
    
    m_create_button(&b_play, win_host,
		    mp_rel(35, 10, 20, 20),
		    &ts_button);
    m_button_set_text(&b_play, "Play");
    
    m_create_button(&b_repeat, win_host,
		    mp_rel(60, 10, 20, 20),
		    &ts_button);
    m_button_set_text(&b_repeat, "Repeat");
    
    m_create_button(&b_next, win_host,
		    mp_rel(10, 32, 70, 20),
		    &ts_button);
    m_button_set_text(&b_next, "Next");
    m_button_set_bitmap(&b_next, &B_folder);    

    makise_g_focus(&b_prev.el, M_G_FOCUS_GET);
    
    return &container.el;
}
