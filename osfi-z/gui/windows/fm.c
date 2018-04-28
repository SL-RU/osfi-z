#include "fm.h"
#include "ff.h"
#include "task.h"
#include "system_menu.h"


static MCanvas container; //main container
static MContainer * win_host;

static MLable    lable;
static MFSViewer flist;
//static MSList    slist;

static MCanvas action_container;
static MSList  action_list;

static MSList_Item action_items[3] = {
    {
	.text = "Play",
	.value = 1
    },
    {
	.text = "View metadata",
	.value = 2
    },
    {
	.text = "Add playlist",
	.value = 3
    },
};

char str[100] = "Hello!";
void start_warble();
void w_set_file(char *file);

static char* determine_file_extention(char* path)
{
    if(path == 0)
	return 0;
    uint32_t i = strlen(path) - 1;
    while(path[i] != '.' && i > 0)
	i --;
    if(i == 0) return 0;
    return path + i + 1;
}
static char* to_uppercase(char *s)
{
    int i = 0;
    char    *str = s;
    while (str[i]){
        if (str[i] >= 97 && str[i] <= 122)
            str[i] -= 32;
        i++; }
    return (str);
}

static MFSViewer_Item *selected_file;
uint8_t onselection(MFSViewer *l, MFSViewer_Item *selected)
{
    if(selected != 0)
    {
	selected_file = selected;
	mi_cont_add(win_host, &action_container.el);
	mi_focus(&action_list.el, M_G_FOCUS_GET);
	return 1;
	/* char b[100] = {0}; */
	/* f_read(&selected_fil, b, 20, 0); */
	/* printf("readed: %s\n", b); */
	char ext[10] = {0};
	strncpy(ext, determine_file_extention(selected->name), 4);
	to_uppercase(ext);
	printf("EXT: %s\n", ext);
	if(strcmp(ext, "MP3") == 0  ||
	   strcmp(ext, "WAV") == 0  ||
	   strcmp(ext, "FLAC") == 0 ||
	   strcmp(ext, "AIFF") == 0 )
	{
	    warble_stop();
	    warble_play_file(selected->fname);
	    return 1;
	}
	else
	{
	    return 0;
	}
    }
    return 0;
}


static void onstart(WTrack *track)
{
    sw_open(SW_PLAY);
}
static void onend(WTrack *track)
{
    sw_open(SW_FM);
}

static void list_click (MSList *l, MSList_Item *selected ) {
    M_E_MUTEX_RELEASE(l);
    mi_cont_rem(&action_container.el);
    mi_focus(&flist.el, M_G_FOCUS_GET);
    M_E_MUTEX_REQUEST(l);
    if(selected->value == 1) {
	char ext[10] = {0};
	strncpy(ext, determine_file_extention(selected_file->name), 4);
	to_uppercase(ext);
	printf("EXT: %s\n", ext);
	if(strcmp(ext, "MP3") == 0  ||
	   strcmp(ext, "WAV") == 0  ||
	   strcmp(ext, "FLAC") == 0 ||
	   strcmp(ext, "AIFF") == 0 )
	{
	    warble_stop();
	    warble_play_file(selected_file->fname);
	}	
    } else if(selected->value == 2) {
	M_E_MUTEX_RELEASE(l);
	window_metadata_load(selected_file->fname);
	sw_open(SW_METADATA);
	M_E_MUTEX_REQUEST(l);
    }
}

MElement * fm_init()
{

    warble_set_onend(&onend);
    warble_set_onstart(&onstart);
    printf("FM initing\n");

    m_create_canvas(&container, 0,
		    mp_sall(0,0,0,0),
		    &ts_container_clear);
    win_host = &container.cont;
    
    //initialize gui elements
    m_create_fsviewer(&flist, win_host,
    		      mp_sall(0,0,0,0), //position
    		      MFSViewer_SingleSelect,
    		      &ts_fsviewer, &ts_fsviewer_item);
    m_fsviewer_set_onselection(&flist, &onselection);
	
    fsviewer_open(&flist, (TCHAR*)"/");


    
    m_create_canvas(&action_container, 0,
		    mp_sall(20,20,5,5),
		    &ts_container_clear);
    m_canvas_set_isolated(&action_container, MContainer_Isolated);
    m_create_slist(&action_list, &action_container.cont,
		   mp_sall(0, 0, 0, 0),
		   0,
		   0, &list_click,
		   MSList_List,
		   &ts_slist, &ts_slist_item_big);
    m_slist_set_array(&action_list, action_items, 3);
    mi_focus(&action_list.el, M_G_FOCUS_GET);

    mi_cont_rem(&action_container.el);
    
    /* m_create_lable(&lable, host->host, */
    /* 		   mp_rel(20, 20, 80, 30), */
    /* 		   str, */
    /* 		   &ts_lable); */

    /* items[0].text = "lol"; */
    /* items[1].text = "kek"; */
    /* items[2].text = "Привет"; */
    /* m_create_slist(&slist, host->host, */
    /* 		   mp_sall(0,0,0,0), */
    /* 		   "sdf", */
    /* 		   0, 0, */
    /* 		   MSList_List, */
    /* 		   &ts_slist, */
    /* 		   &ts_slist_item); */

    /* makise_g_cont_rem(&slist.el); */

    mi_focus(&flist.el, M_G_FOCUS_GET);
    
    printf("FM inited\n");

    return &container.el;
}
