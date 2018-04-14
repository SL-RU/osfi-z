#include "gui.h"

MakiseGUI    *mGui;
MHost        *host;
static MakiseGUI    Gu;
static MakiseBuffer Bu;
static MakiseDriver Dr;
static MHost        hs;
static MContainer   co;
static uint32_t Makise_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 32 + 1];


static uint8_t ldf;
void gui_predraw(MakiseGUI * gui)
{
    if(ldf)
    	window_play_update();
    ldf = !ldf;
    makise_gui_input_perform(host);
    makise_g_host_call(host, gui, M_G_CALL_PREDRAW);
}
void gui_draw(MakiseGUI* gui)
{
    makise_g_host_call(host, gui, M_G_CALL_DRAW);
}


MInputData inp_handler(MInputData d, MInputResultEnum res)
{
    if(d.event == M_INPUT_CLICK && res == M_INPUT_NOT_HANDLED)
    {
	//when click wasn't handled in the GUI
	
	//printf("not h %d\n", d.key);
	//if(d.key == M_KEY_LEFT) //if left wasn't handled - we'll switch focus
	//    fm_switch();
	//Cmakise_g_host_focus_prev(host);
	if(d.key == M_KEY_UP) //the same
	    makise_g_host_focus_prev(host);
	if(d.key == M_KEY_DOWN)
	    makise_g_host_focus_next(host);
	if(d.key == M_KEY_TAB_NEXT)
	    makise_g_host_focus_next(host);
	if(d.key == M_KEY_TAB_BACK)
	    makise_g_host_focus_prev(host);
	if(d.key == M_KEY_USER_FOCUS_NEXT)
	    makise_g_host_focus_next(host);
	if(d.key == M_KEY_USER_FOCUS_PREV)
	    makise_g_host_focus_prev(host);



    }
    return (MInputData){0};
}

#if MAKISE_MUTEX
uint8_t m_mutex_create (MAKISE_MUTEX_t *sobj)
{
    *sobj = xSemaphoreCreateMutex();
    xSemaphoreGive(*sobj);
    return (int)(*sobj != NULL);
}
//delete mutex
uint8_t m_mutex_delete (MAKISE_MUTEX_t *sobj)
{
    if(sobj == 0 || *sobj == 0) {
	printf("Mutex null\n");
	return 0;
    }
    vSemaphoreDelete(*sobj);
    return 1;
}
//Request Grant to Access some object
uint8_t m_mutex_request_grant (MAKISE_MUTEX_t *sobj)
{
    if(sobj == 0 || *sobj == 0) {
	printf("Mutex null\n");
	return 0;
    }
    int res = (int)(xSemaphoreTake(*sobj, MAKISE_MUTEX_TIMEOUT) == pdTRUE);
    if(res == 0) {
	printf("Mutex err mak\n");
//	while(1);
    }
    return res;
}
//Release Grant to Access the Volume
uint8_t m_mutex_release_grant (MAKISE_MUTEX_t *sobj)
{
    if(sobj == 0 || *sobj == 0) {
	printf("Mutex null\n");
	return 0;
    }
    xSemaphoreGive(*sobj);
    return 1;
}
#endif

static uint32_t* _get_gui_buffer(uint32_t size)
{
    return Makise_Buffer;
}

MPosition ma_g_hpo;
MakiseGUI* gui_init()
{
    //malloc structures
    MakiseGUI    * gu = &Gu;
    MakiseDriver * dr = &Dr;
    host = &hs;

    ssd1306_driver(dr);

    makise_gui_autoinit(host,
			gu, dr,
			&_get_gui_buffer,
			&inp_handler,
			&gui_draw, &gui_predraw, 0);
    ssd1306_init(gu);
    
    makise_start(gu);
    mGui = gu;
    return gu;
}
