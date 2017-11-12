#include "window_play.h"
#include "warble.h"

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

static char s_name[30] = "";
static char s_time[30] = "";
static uint32_t ldden;

static void gotmetadata(WTrack *track)
{
    MAKISE_MUTEX_REQUEST(&l_artist.el.mutex);
    snprintf(s_name, 30, "%s", track->id3.title);
    MAKISE_MUTEX_RELEASE(&l_artist.el.mutex);
    ldden = track->id3.length;
    m_slider_set_range(&slider, 0, ldden);
}
static void ontimeelapsed(WTrack *track, uint32_t time)
{
    MAKISE_MUTEX_REQUEST(&l_title.el.mutex);
    snprintf(s_time, 30, "%d / %d",	     
	     warble_get_player()->time_elapsed / 1000,
	     ldden / 1000);
    MAKISE_MUTEX_RELEASE(&l_title.el.mutex);
    
    m_slider_set_value(&slider, warble_get_player()->time_elapsed);		 
}

void window_play_update()
{
}

MElement * window_play_init()
{
    m_create_canvas(&container, 0,
		    mp_sall(0,0,0,0),
		    &container_style);

    win_host = &container.cont;
    
    m_create_button(&b_prev, win_host,
		    mp_rel(0, 41, 23, 23),
		    &ts_button);
    //m_button_set_click(&b_prev, &click);
    m_button_set_bitmap(&b_prev, &B_backButton);
    
    m_create_button(&b_play, win_host,
		    mp_rel(52, 41, 23, 23),
		    &ts_button);
    m_button_set_bitmap(&b_play, &B_playButton);
    //m_button_set_click(&b_play, &click);
    
    /* m_create_button(&b_repeat, win_host, */
    /* 		    mp_rel(52, 11, 23, 23), */
    /* 		    &ts_button); */
    /* m_button_set_bitmap(&b_repeat, &B_repeatButton); */
    
    m_create_button(&b_next, win_host,
		    mp_rel(105, 41, 23, 23),
		    &ts_button);
    m_button_set_bitmap(&b_next, &B_nextButton);
    //m_button_set_click(&b_next, &click);

    m_create_button(&b_bat, win_host,
		    mp_rel(112, 0, 15, 10),
		    &ts_button);
    m_button_set_bitmap(&b_bat, &B_battery_full);



    m_create_lable(&l_artist, win_host,
		    mp_rel(0, 10, 128, 15),
		    &ts_lable);
    m_lable_set_text(&l_artist, s_name);

    m_create_lable(&l_title, win_host,
		    mp_rel(10, 25, 128, 15),
		    &ts_lable);
    m_lable_set_text(&l_title, s_time);

    m_create_lable(&l_status, win_host,
		    mp_rel(1, 1, 70, 10),
		    &ts_lable_small);
    m_lable_set_text(&l_status, "Status");

    m_create_slider(&slider, win_host,
		    mp_sall(0, 0, 51, 0),
		    MSlider_Type_Read,
		    &ts_slider);
    
    warble_set_ontimeelapsed(&ontimeelapsed);
    warble_set_gotmetadata(&gotmetadata);
    
    makise_g_focus(&b_play.el, M_G_FOCUS_GET);
    
    return &container.el;
}
