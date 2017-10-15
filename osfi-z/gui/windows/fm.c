#include "fm.h"
#include "ff.h"
#include "task.h"

//static MSList list;
static MLable    lable;
static MFSViewer flist;
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
	    w_set_file(selected->name);
	    start_warble();
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

void fm_init()
{
    printf("FM initing\n");
    //initialize gui elements
    m_create_fsviewer(&flist, host->host,
    		      mp_sall(0,0,0,0), //position
    		      0, //header
    		      &onselection, 0, //events
    		      MFSViewer_SingleSelect,
    		      &ts_fsviewer, &ts_fsviewer_item);
	
	
    fsviewer_open(&flist, "/");
    makise_g_focus(&flist.el, M_G_FOCUS_GET); //focus file list

    m_create_lable(&lable, host->host,
		   mp_rel(20, 20, 80, 30),
		   str,
		   &ts_lable);

    osThreadDef(FM_Task, vTaskCode, osPriorityNormal, 0, 512);
    osThreadCreate(osThread(FM_Task), NULL);

		   
    
    printf("FM inited\n");
}
