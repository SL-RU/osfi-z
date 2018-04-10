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

static uint8_t inited = 0;

static char s_name[200] = "";
static char s_time[200] = "";
static uint32_t ldden;

static void gotmetadata(WTrack *track)
{
    snprintf(s_name, 200, "%s", track->id3.title);
    m_lable_set_text(&l_artist, s_name);
    ldden = track->id3.length;
    m_slider_set_range(&slider, 0, ldden);
}
static void ontimeelapsed(WTrack *track, uint32_t time)
{
}

void window_play_update()
{
    MAKISE_MUTEX_REQUEST(&warble_get_player()->mutex);
    if(!inited)
    {
	MAKISE_MUTEX_RELEASE(&warble_get_player()->mutex);
	return;
    }

    s_time[0] = 0;
    gh_sprint_time(s_time, 30, 
		   warble_get_player()->time_elapsed, 0);
    snprintf(s_time + strlen(s_time), 30 - strlen(s_time), "/");
    gh_sprint_time(s_time + strlen(s_time),
		   30 - strlen(s_time), ldden, 0);
    
    m_slider_set_value(&slider, warble_get_player()->time_elapsed);		 

    MAKISE_MUTEX_RELEASE(&warble_get_player()->mutex);
}


static void bseek_click(MButton* b)
{
    warble_seek(-10000);
}
static void fseek_click(MButton* b)
{
    warble_seek(10000);
}
static void pause_click(MButton* b)
{
    warble_pause();
}
static void stop_click(MButton* b)
{
    warble_stop();
}

MElement * window_play_init()
{
    m_create_canvas(&container, 0,
		    mp_sall(0,0,0,0),
		    &ts_container_clear);

    win_host = &container.cont;
    
    m_create_button(&b_prev, win_host,
		    mp_rel(0, 41, 23, 23),
		    &ts_button);
    m_button_set_click(&b_prev, &bseek_click);
    m_button_set_bitmap(&b_prev, &B_backButton);
    
    m_create_button(&b_play, win_host,
		    mp_rel(52, 41, 23, 23),
		    &ts_button);
    m_button_set_bitmap(&b_play, &B_playButton);
    m_button_set_click(&b_play, &pause_click);
    
    /* m_create_button(&b_repeat, win_host, */
    /* 		    mp_rel(52, 11, 23, 23), */
    /* 		    &ts_button); */
    /* m_button_set_bitmap(&b_repeat, &B_repeatButton); */
    
    m_create_button(&b_next, win_host,
		    mp_rel(105, 41, 23, 23),
		    &ts_button);
    m_button_set_bitmap(&b_next, &B_nextButton);
    m_button_set_click(&b_next, &fseek_click);

    m_create_button(&b_bat, win_host,
		    mp_rel(112, 0, 15, 10),
		    &ts_button);
    m_button_set_bitmap(&b_bat, &B_battery_full);
    m_button_set_click(&b_bat, &stop_click);


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

    inited = 1;
    
    makise_g_focus(&b_play.el, M_G_FOCUS_GET);
    
    return &container.el;
}
