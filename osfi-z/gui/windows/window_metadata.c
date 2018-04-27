#include "system_menu.h"

static MCanvas container; //main container
static MContainer * win_host;
/* static MButton b_fm, b_play, b_close_menu; */
static MLable lable;


static MSList list;

static char        text_buf[1000];
static MSList_Item items[10] = {
    {
	.text = "null",
	.next = 0,
	.prev = 0,
    }
};
static uint32_t    items_i;

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

static void add_item(char* text) {
    items[items_i].text = text;
    m_slist_add(&list, &items[items_i]);
    items_i ++;
    //printf("add item %s\n", text);
}

void window_metadata_update(WTrack *track)
{
    const struct mp3entry *id3 = &track->id3;

    m_slist_clear(&list);
    items_i = 0;

    char *s = text_buf;
    uint32_t len = 1000;

    snprintf(s, len, "Path: %s", id3->path);
    add_item(s);
    len -= strlen(s) + 1;
    s = text_buf + (1000 - len);
    if (id3->title)
    {
	snprintf(s, len, "Title: %s", id3->title);
	add_item(s);
	len -= strlen(s) + 1;
	s = text_buf + (1000 - len);
    }
    if (id3->artist)
    {
	snprintf(s, len, "Artist: %s", id3->artist);
	add_item(s);
	len -= strlen(s) + 1;
	s = text_buf + (1000 - len);
    }
    if (id3->album)
    {
	snprintf(s, len, "Album: %s", id3->album);
	add_item(s);
	len -= strlen(s) + 1;
	s = text_buf + (1000 - len);
    }
    if (id3->genre_string)
    {
	snprintf(s, len, "Genre: %s", id3->genre_string);
	add_item(s);
	len -= strlen(s) + 1;
	s = text_buf + (1000 - len);	
    }
    snprintf(s, len, "Freq: %ld", id3->frequency);
    add_item(s);
    len -= strlen(s) + 1;
    s = text_buf + (1000 - len);
	
    M_E_MUTEX_RELEASE(&lable);
//    m_lable_set_text(&lable, text);
}

MElement * window_metadata_init()
{    
    m_create_canvas(&container, 0,
		    mp_sall(0,0,0,0),
		    &ts_container_black);
    m_canvas_set_isolated(&container, MContainer_Isolated);
    
    win_host = &container.cont;

    
    m_create_slist(&list, win_host,
		   mp_sall(0, 0, 0, 0),
		   0,
		   0, 0,
		   MSList_List,
		   &ts_slist, &ts_slist_item_big);
    m_slist_set_list(&list, items);

    mi_focus(&list.el, M_G_FOCUS_GET);
    
    /* m_create_button(&b_close_menu, 0, */
    /* 		    mp_rel(0, 46, 50, 20), */
    /* 		    &ts_button); */
    /* m_button_set_text(&b_close_menu, "Close menu"); */
    /* m_button_set_click(&b_close_menu, &b_close_click); */
    
    return &container.el;
}
