#include "gui.h"


MakiseGUI    *mGui;
MHost        *host;
MakiseGUI    Gu;
MakiseBuffer Bu;
MakiseDriver Dr;
MHost        hs;
MContainer   co;
uint32_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8 + 1];
uint32_t Makise_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8 + 1];



void gui_predraw(MakiseGUI * g)
{
    makise_gui_input_perform(host);
    makise_g_host_call(host, M_G_CALL_PREDRAW);
}
void gui_draw(MakiseGUI* gui)
{
    makise_g_host_call(host, M_G_CALL_DRAW);
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

MPosition ma_g_hpo;
MakiseGUI* gui_init()
{
    //malloc structures
    MakiseGUI    * gu = &Gu;
    MakiseBuffer * bu = &Bu;
    MakiseDriver * dr = &Dr;
    host = &hs;
    host->host = &co;
    host->host->gui = gu;
    makise_gui_init(host); //init gui host
    //if input event wasn't handled by gui. We need to handle it
    host->input.result_handler = &inp_handler;
    
    //init sizes
    ma_g_hpo = mp_rel(0,0,128,64);
    ma_g_hpo.real_x = 0;
    ma_g_hpo.real_y = 0;
    host->host->position = &ma_g_hpo;

	
    //init driver structure
    ssd1306_driver(dr);
    //ili9340_driver(dr);
    //alloc small buffer
    dr->buffer = SSD1306_Buffer;
    printf("%d\n", (uint32_t)(dr->size));
    //init gui struct

    uint32_t sz = makise_init(gu, dr, bu);
    //alloc big buffer
    bu->buffer = Makise_Buffer;//malloc(sz);
    memset(bu->buffer, 0, sz);
    printf("%d\n", (uint32_t)(sz));    
    
    mGui = gu;
    //ili9340_init(gu);
    ssd1306_init(gu);
    makise_start(gu);

    mGui->predraw = &gui_predraw;
    mGui->draw = &gui_draw;
    return mGui;
}
