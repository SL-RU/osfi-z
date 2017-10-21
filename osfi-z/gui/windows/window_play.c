#include "window_play.h"


static MCanvas container; //main container
static MButton b_play, //main container
    b_next,
    b_prev,
    b_bat;
static MLable l_title,
    l_artist,
    l_status;
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
		    mp_rel(0, 41, 23, 23),
		    &ts_button);
    m_button_set_click(&b_prev, &click);
    m_button_set_bitmap(&b_prev, &B_backButton);
    
    m_create_button(&b_play, win_host,
		    mp_rel(52, 41, 23, 23),
		    &ts_button);
    m_button_set_bitmap(&b_play, &B_playButton);
    m_button_set_click(&b_play, &click);
    
    /* m_create_button(&b_repeat, win_host, */
    /* 		    mp_rel(52, 11, 23, 23), */
    /* 		    &ts_button); */
    /* m_button_set_bitmap(&b_repeat, &B_repeatButton); */
    
    m_create_button(&b_next, win_host,
		    mp_rel(105, 41, 23, 23),
		    &ts_button);
    m_button_set_bitmap(&b_next, &B_nextButton);
    m_button_set_click(&b_next, &click);

    m_create_button(&b_bat, win_host,
		    mp_rel(112, 0, 15, 10),
		    &ts_button);
    m_button_set_bitmap(&b_bat, &B_battery_full);



    m_create_lable(&l_artist, win_host,
		    mp_rel(0, 10, 128, 15),
		    &ts_lable);
    m_lable_set_text(&l_artist, "Jean Jarre Michele");

    m_create_lable(&l_title, win_host,
		    mp_rel(10, 25, 128, 15),
		    &ts_lable);
    m_lable_set_text(&l_title, "Oxygen 7");

    m_create_lable(&l_status, host,
		    mp_rel(1, 1, 70, 10),
		    &ts_lable_small);
    m_lable_set_text(&l_status, "Status");

    
    makise_g_focus(&b_play.el, M_G_FOCUS_GET);
    
    return &container.el;
}
