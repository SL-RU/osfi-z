#include "window_play.h"


static MCanvas container; //main container
static MButton b_play, //main container
    b_next,
    b_prev,
    b_repeat;

static MakiseStyle_Canvas container_style =
{
    //bg       border   double_border
    {MC_Transparent, MC_Transparent, 0},  //normal
    {MC_Transparent, MC_Transparent, 0},  //focused
};

MElement * window_play_init(MContainer * host)
{
    m_create_canvas(&container, host,
		    mp_sall(0,0,0,0),
		    &container_style);

    m_create_button(&b_prev, host,
		    mp_rel(10, 10, 20, 20),
		    "PREv",
		    0, 0, 0,
		    &ts_button);
    m_create_button(&b_play, host,
		    mp_rel(35, 10, 20, 20),
		    "Play",
		    0, 0, 0,
		    &ts_button);
    m_create_button(&b_repeat, host,
		    mp_rel(60, 10, 20, 20),
		    "Repeat",
		    0, 0, 0,
		    &ts_button);
    m_create_button(&b_next, host,
		    mp_rel(85, 10, 70, 20),
		    "Next",
		    0, 0, 0,
		    &ts_button);
    return &container.el;
}
