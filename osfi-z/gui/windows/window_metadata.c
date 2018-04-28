#include "system_menu.h"

static MCanvas container; //main container
static MContainer * win_host;
/* static MButton b_fm, b_play, b_close_menu; */
static MLable lable;


static MSList list;

#define METADATA_GUI_BUFFER 5000
#define METADATA_GUI_ITEMS  20

static char        text_buf[METADATA_GUI_BUFFER];
static MSList_Item items[METADATA_GUI_ITEMS] = {
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



static int32_t buf_len = 0;
static void add_meta(const char *fmt, ...)
{
    if(buf_len <= 0 || items_i >= METADATA_GUI_ITEMS) {
	printf("metadata gui overflow\n");
	return;
    }
    va_list ap;
    va_start(ap, fmt);

    char *s = text_buf + (METADATA_GUI_BUFFER - buf_len);
    vsnprintf(s, buf_len, fmt, ap);
    buf_len -= strlen(s) + 1;

//    vprintf(fmt, ap);
//    printf(":::  len %d \n", buf_len);
    
    va_end(ap);

    items[items_i].text = s;
    m_slist_add(&list, &items[items_i]);
    items_i ++;
}

void window_metadata_update(struct mp3entry *id3)
{
    m_slist_clear(&list);
    items_i = 0;
    buf_len = METADATA_GUI_BUFFER;
    
    
    add_meta("Path: %s", id3->path);

    if (id3->title)
	add_meta("Title: %s", id3->title);

    if (id3->artist)
	add_meta("Artist: %s", id3->artist);

    if (id3->album)
	add_meta("Album: %s", id3->album);

    if (id3->genre_string)
	add_meta("Genre: %s", id3->genre_string);

    if (id3->disc_string || id3->discnum)
	add_meta("Disc: %s (%d)", id3->disc_string, id3->discnum);

    if (id3->track_string || id3->tracknum)
	add_meta("Track: %s (%d)", id3->track_string, id3->tracknum);

    if (id3->year_string || id3->year)
	add_meta("Year: %s (%d)", id3->year_string, id3->year);

    if (id3->composer)
	add_meta("Composer: %s", id3->composer);

    if (id3->comment)
	add_meta("Comment: %s", id3->comment);

    if (id3->albumartist)
	add_meta("Album artist: %s", id3->albumartist);

    if (id3->grouping)
	add_meta("Grouping: %s", id3->grouping);

    if (id3->layer)
	add_meta("Layer: %d", id3->layer);

    if (id3->id3version)
	add_meta("ID3 version: %u", (int)id3->id3version);

    add_meta("Codec: %s", audio_formats[id3->codectype].label);
    add_meta("Bitrate: %d kb/s", id3->bitrate);
    add_meta("Freq: %ld", id3->frequency);
    add_meta("Channels: %d", id3->channels);

    
    //M_E_MUTEX_RELEASE(&lable);
//    m_lable_set_text(&lable, text);
}

void window_metadata_load  (TCHAR *path)
{
    char trackname[13];
    for (int i = 0; i < 13; i++) {
	trackname[i] = (char)
	    (((TCHAR*)path)[i]);
    }
    
    static struct mp3entry id3;
    int descriptor;
    descriptor = open((char*)path, O_RDONLY);
    if (descriptor == -1) {
	printf("error: open %s\n", trackname);
    }
    fseek_init(descriptor);
    if (!get_metadata(&id3, descriptor, trackname))
    {
        printf("error: metadata parsing failed\n");
	close(descriptor);
        return;
    }
    window_metadata_update(&id3);
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
