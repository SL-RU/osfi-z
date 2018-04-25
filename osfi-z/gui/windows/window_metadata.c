#include "system_menu.h"

static MCanvas container; //main container
static MContainer * win_host;
/* static MButton b_fm, b_play, b_close_menu; */
static MLable lable;

static char text[100];


/* void smenu_open() */
/* { */
/*     if(opened) */
/* 	_sw_menu_hide(); */
/*     else */
/* 	_sw_menu_show(); */
    
/*     opened = !opened; */
/*     makise_g_print_tree(host); */
/* } */

/* static void b_fm_click(MButton* b) */
/* { */
/*     smenu_open(); */
/*     sw_open(SW_FM); */
/*     makise_g_print_tree(host); */
/* } */
/* static void b_play_click(MButton* b) */
/* { */
/*     smenu_open(); */
/*     sw_open(SW_PLAY); */
/*     makise_g_print_tree(host); */
/* } */
/* static void b_close_click(MButton* b) */
/* { */
/*     smenu_open(); */
/*     makise_g_print_tree(host); */
/* } */

void window_metadata_update(WTrack *track)
{
    M_E_MUTEX_REQUEST(&lable);
    uint32_t len = 100;
    snprintf(text, len, "Path: %s Title: %s Artist: %s Genre: %s Album: %s Year: %s (%d)",
	     track->id3.path,
	     track->id3.title,
	     track->id3.artist,
	     track->id3.genre_string,
	     track->id3.album,
	     track->id3.year_string, track->id3.year
	     
	);
    if (track->id3.title)
    {
	printf("Title: %s\n", track->id3.title);
    }
    if (track->id3.artist)
    {
	printf("Artist: %s\n", track->id3.artist);
    }
    if (track->id3.album)
    {
	
	printf("Album: %ld\n", track->id3.frequency);
    }
    if (track->id3.genre_string)
    {
	printf("Genre: %s\n", track->id3.genre_string);
    }
    if (track->id3.disc_string || track->id3.discnum) printf("Disc: %s (%d)\n", track->id3.disc_string, track->id3.discnum);
    if (track->id3.track_string || track->id3.tracknum) printf("Track: %s (%d)\n", track->id3.track_string, track->id3.tracknum);
    if (track->id3.year_string || track->id3.year) printf("Year: %s (%d)\n", track->id3.year_string, track->id3.year);

    M_E_MUTEX_RELEASE(&lable);
    m_lable_set_text(&lable, text);
}

MElement * window_metadata_init()
{    
    m_create_canvas(&container, 0,
		    mp_sall(0,0,0,0),
		    &ts_container_black);
    m_canvas_set_isolated(&container, MContainer_Isolated);
    
    win_host = &container.cont;

    /* m_create_button(&b_fm, win_host, */
    /* 		    mp_rel(0, 0, 60, 20), */
    /* 		    &ts_button); */
    /* m_button_set_text(&b_fm, "File viewer"); */
    /* m_button_set_click(&b_fm, &b_fm_click); */

    /* m_create_button(&b_play, win_host, */
    /* 		    mp_rel(0, 23, 50, 20), */
    /* 		    &ts_button); */
    /* m_button_set_text(&b_play, "Play"); */
    /* m_button_set_click(&b_play, &b_play_click); */

    m_create_lable(&lable, win_host,
    		   mp_rel(0, 10, 128, 15),
    		   &ts_lable);
    m_lable_set_text(&lable, text);


    /* m_create_button(&b_close_menu, 0, */
    /* 		    mp_rel(0, 46, 50, 20), */
    /* 		    &ts_button); */
    /* m_button_set_text(&b_close_menu, "Close menu"); */
    /* m_button_set_click(&b_close_menu, &b_close_click); */
    
    return &container.el;
}
