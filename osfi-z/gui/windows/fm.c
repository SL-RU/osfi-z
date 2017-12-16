#include "fm.h"
#include "ff.h"
#include "task.h"
#include "system_menu.h"


static MCanvas container; //main container
static MContainer * win_host;

static MLable    lable;
static MFSViewer flist;
static MSList    slist;


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


uint8_t onselection(MFSViewer *l, MFSViewer_Item *selected)
{
    if(selected != 0)
    {
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

//handle vs1053 SD error
void vs1053_sd_error()
{
    makise_gui_input_send_button(host,
				 M_KEY_USER_SD_ERROR,
				 M_INPUT_CLICK, 100);
}

void vTaskCode()
{
    uint32_t i = 0;
    for( ;; )
    {
	i++;
        osDelay(20);
	MAKISE_MUTEX_REQUEST(&lable.el.mutex);
	sprintf(str, "lol %d", i);
	MAKISE_MUTEX_RELEASE(&lable.el.mutex);
    }
}

static MSList_Item items[10];
void fm_cre(char *art, char *tit, char *alb)
{
    printf("FM cre %ld %s %s %s\n", xPortGetFreeHeapSize(), art, tit, alb);
    items[0].text = art;
    items[1].text = tit;
    items[2].text = alb;
    /* m_create_slist(&slist, host->host, */
    /* 		   mp_sall(0,0,0,0), */
    /* 		   "sdf", */
    /* 		   0, 0, */
    /* 		   MSList_List, */
    /* 		   &ts_slist, */
    /* 		   &ts_slist_item); */
    /* m_create_lable(&lable, host->host, */
    /* 	   mp_rel(20, 20, 80, 30), */
    /* 	   str, */
    /* 	   &ts_lable); */

    /* m_slist_set_array(&slist, items, 3); */
    m_slist_set_array(&slist, items, 3);
    makise_g_cont_rem(&flist.el);
    makise_g_cont_add(host->host, &slist.el);
    makise_g_focus(&slist.el, M_G_FOCUS_GET);
}

MElement * fm_init()
{
    printf("FM initing\n");

    m_create_canvas(&container, 0,
		    mp_sall(0,0,0,0),
		    &ts_container_clear);
    win_host = &container.cont;
    
    //initialize gui elements
    m_create_fsviewer(&flist, host->host,
    		      mp_sall(0,0,0,0), //position
    		      MFSViewer_SingleSelect,
    		      &ts_fsviewer, &ts_fsviewer_item);
    m_fsviewer_set_onselection(&flist, &onselection);
	
    fsviewer_open(&flist, (TCHAR*)"/");
    
    /* m_create_lable(&lable, host->host, */
    /* 		   mp_rel(20, 20, 80, 30), */
    /* 		   str, */
    /* 		   &ts_lable); */

    /* osThreadDef(FM_Task, vTaskCode, osPriorityNormal, 0, 512); */
    /* osThreadCreate(osThread(FM_Task), NULL); */

    items[0].text = "lol";
    items[1].text = "kek";
    items[2].text = "Привет";
    m_create_slist(&slist, host->host,
    		   mp_sall(0,0,0,0),
    		   "sdf",
    		   0, 0,
    		   MSList_List,
    		   &ts_slist,
    		   &ts_slist_item);

    makise_g_cont_rem(&slist.el);

    makise_g_focus(&flist.el, M_G_FOCUS_GET);
    
    printf("FM inited\n");

    return &container.el;
}
