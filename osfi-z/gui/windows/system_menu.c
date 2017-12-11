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
static MSlider slider;

static MakiseStyle_Canvas container_style =
{
    //bg       border   double_border
    {MC_Transparent, MC_Transparent, 0},  //normal
    {MC_Transparent, MC_Transparent, 0},  //focused
};
static uint8_t inited = 0;

static char s_name[200] = "";
static char s_time[200] = "";
static uint32_t ldden;


MElement * window_play_init()
{
    m_create_canvas(&container, 0,
		    mp_sall(0,0,0,0),
		    &container_style);

    win_host = &container.cont;
    
    inited = 1;   
    
    return &container.el;
}
