#include "system_menu.h"

static MCanvas container; //main container
static MContainer * win_host;
static MButton b_fm, b_play, b_meta, b_close_menu;
static MLable lable;

static MSList list;

static MSList_Item items[3] = {
    {
	.text = "Files",
	.value = SW_FM
    },
    {
	.text = "Play",
	.value = SW_PLAY
    },
    {
	.text = "Metadata",
	.value = SW_METADATA
    },
};

static uint8_t opened = 0;

void smenu_open()
{
    if(opened)
	_sw_menu_hide();
    else
	_sw_menu_show();
    
    opened = !opened;
    makise_g_print_tree(host);

    if(opened) {
	for (int i = 0; i < 3; i++) {
	    if(sw_get_current_window() == items[i].value)
		list.selected = &items[i];
	}
    }
}

void list_click (MSList *l, MSList_Item *selected ) {
    smenu_open();
    sw_open(selected->value);
}

MElement * system_menu_init()
{    
    m_create_canvas(&container, 0,
		    mp_sall(0,0,0,0),
		    &ts_container_black);
    
    win_host = &container.cont;
    opened = 0;

    m_create_slist(&list, win_host,
		   mp_sall(0, 0, 0, 0),
		   0,
		   0, &list_click,
		   MSList_List,
		   &ts_slist, &ts_slist_item_big);
    m_slist_set_array(&list, items, 3);

    mi_focus(&list.el, M_G_FOCUS_GET);

    
    return &container.el;
}
