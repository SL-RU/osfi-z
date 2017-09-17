#include "gui_filelist.h"

static uint8_t draw   (MElement* b);
static MFocusEnum focus(MElement* b,  MFocusEnum act);
static MInputResultEnum input  (MElement* b, MInputData data);

static char *name = "Filelist";
void m_create_filelist(Filelist* b, MContainer *c,
		       MPosition pos,
		       char* header,
		       uint8_t (*onselection)(Filelist *l, Filelist_Item *selected),
		       void (*click)(Filelist *l, Filelist_Item *selected),
		       Filelist_Type type,
		       MakiseStyle *style,
		       MakiseStyle *item_style)
{
    MElement *e = &b->el;
    m_element_create(e, (c == 0) ? 0 : c->gui, name, b,
		     1, 1, pos,
		     &draw,
		     0,
		     0,
		     &input,
		     &focus,
		     0, 0);
    
    b->header = header;
    
    b->onselection = onselection;
    b->click = click;

    b->path = 0;
    
    b->selected_file[0] = 0;
    b->current_position = 0;
    b->files_count = 0;

    b->type = type;

    b->was_selected = 0;
    b->current_folder = 0;
    
    b->style = style;
    b->item_style = item_style;
    
    makise_g_cont_add(c, e);

    printf("Filelist %d created\n", e->id);
}

//draw single item of the list
static void draw_item   (Filelist_Item *ci, Filelist *l, MakiseStyleTheme *c_th, uint32_t x, uint32_t y, uint32_t w, uint32_t eh)
{
    //background
    makise_d_rect_filled(l->el.gui->buffer,
			 x, y,  w, eh,
			 c_th->bg_color, c_th->bg_color);
    //selection
    if(l->type == Filelist_SingleSelect)
    {
	if(l->was_selected && ci->id == l->selected_file_id
	   && l->selected_folder == l->current_folder)
	{
	    makise_d_circle_filled(l->el.gui->buffer,
				   x + eh / 2, y + eh / 2, eh / 2 - 3,
				   c_th->font_col, l->item_style->active.font_col);
	    x += eh + 1;
	    w -= eh + 1;
	}       
    }
    //directory
    if(ci->am_dir)
    {
	makise_d_rect_filled(l->el.gui->buffer,
			     x + 2, y + 2, eh - 4, eh - 4,
			     c_th->font_col, l->item_style->active.font_col);
	x += eh + 1;
	w -= eh + 1;
    }

    //text
    makise_d_string_frame(l->el.gui->buffer, ci->name, MDTextAll,
			  x + 1,
			  y,
			  w - 2, eh,
			  l->item_style->font,
			  l->item_style->font_line_spacing,
			  c_th->font_col);
	    
    
}
static uint8_t draw   (MElement* b)
{
    Filelist *l = (Filelist*)b->data;
    MakiseStyleTheme *th = l->state ? &l->style->focused : &l->style->normal;
    MakiseStyleTheme *i_foc =l->state ? &l->item_style->focused : &l->item_style->active;
    MakiseStyleTheme *i_nom = &l->item_style->normal,
	*c_th = 0;
    

    //printf("%d %d %d %d\n", b->position.real_x, b->position.real_y, b->position.width, b->position.height);
    _m_e_helper_draw_box(b->gui->buffer, &b->position, th);

    uint32_t i = 0, start = 0, end = 0;
    int16_t y = b->position.real_y + 1,
	x = b->position.real_x + 2;
    uint32_t
	w = b->position.width - 5,
	h = b->position.height - 4,
	eh = l->item_style->font->height + l->item_style->font_line_spacing + 1,
	ec = h / (eh + 1), //count of elements on the screen
	sh = 0,   //scroll line height
	cuid = 0, //current id
	len = 0;  //count of items


    //header
    if(l->header != 0)
    {
	makise_d_string(b->gui->buffer,
			l->header, MDTextAll,
			x, y, MDTextPlacement_LeftUp,
			l->style->font, th->font_col);
	y += l->style->font->height;
	h -= l->style->font->height;
	makise_d_line(b->gui->buffer, b->position.real_x, y,
		      b->position.real_x + b->position.width, y,
		      th->border_c);
	ec = h / (eh + 1);
    }
    y += 1;
    
    Filelist_Item *ci = 0;

    if(ec == 0)
	return M_ERROR;
    
    len = l->files_count;

    l->displayed_count = ec;

    cuid = i = l->current_position;
    
    //compute start index of element to display & the last
    if(ec >= len)
    {
	start = 0;
	end = len;
    }
    else if ((i >= (ec / 2)) && ((len - i) > (ec - 1) / 2))
    {
	start = i - (ec / 2);
	end = start + ec;
    }
    else if ((i > (ec / 2) && (len - i) <= (ec - 1) / 2))
    {
	end = len;
	start = len - ec;
    }
    else if (i < (ec / 2) && (len - i) > (ec - 1) / 2)
    {
	start = 0;
	end = ec;
    }
    
    //check if required chunk is loaded
    if(start < l->current_chunk_position ||
       end >= (l->current_chunk_position + FM_BUFFERED))
    {
	//load chunk
	m_filelist_loadchunk(l, i);
    }

    //printf("start %d end %d\n", start, end);
    //array
    for (i = start; i < end; i++)
    {
	ci = &l->buffer[i - l->current_chunk_position];
	//printf("draw %d %s\n", i, ci->name);
	ci->id = i;
	c_th = (i == l->current_position) ? i_foc : i_nom;
	
	draw_item(ci, l, c_th, x, y, w, eh);
	y += eh + 1;
    }
    

    //scroll bar calculations
    h = b->position.height - 2;
    sh = h / len;
    if(sh < 5)
    {
	y = cuid * (h + sh - 5) / len;
	sh = 5;
    }
    else
	y = cuid * (h) / len;
    y += b->position.real_y + 1;

    //scroll bar
    makise_d_line(b->gui->buffer,
    		  x + w - 1, b->position.real_y,
    		  x + w - 1, b->position.real_y + b->position.height - 1,
    		  th->border_c);
    makise_d_line(b->gui->buffer,
    		  x + w    , y,
    		  x + w    , y + sh,
    		  th->font_col);
    makise_d_line(b->gui->buffer,
    		  x + w + 1, y,
    		  x + w + 1, y + sh,
    		  th->font_col);
    return M_OK;
}

static MFocusEnum focus   (MElement* b,  MFocusEnum act)
{
    //Filelist *e = ((Filelist*)b->data);
    if(act & M_G_FOCUS_GET)
    {
	((Filelist*)b->data)->state = 1;
    }
    if(act == M_G_FOCUS_LEAVE)
    {
	((Filelist*)b->data)->state = 0;
    }

    return (act == M_G_FOCUS_PREV || act == M_G_FOCUS_NEXT)
	? M_G_FOCUS_NOT_NEEDED
	: M_G_FOCUS_OK;

}

//after selection or dirrectory movement
static void update_selected_state()
{
}

static MInputResultEnum input  (MElement* b, MInputData data)
{
    Filelist *e = ((Filelist*)b->data);

    if(e->files_count == 0)
	return M_INPUT_NOT_HANDLED;

    uint8_t handled = 0; //was event handled
    
    if(e->current_position >= e->files_count)
    {
	e->current_position = 0;
    }

    
    if(data.event == M_INPUT_CLICK)
    {
	if(data.key == M_KEY_DOWN)
	{
	    if(e->current_position < e->files_count - 1)
	    {
		e->current_position ++;
	    }
	    else
		e->current_position = 0;
	    handled = 1;
	} else if(data.key == M_KEY_UP)
	{
	    if(e->current_position > 0)
	    {
		e->current_position --;
	    }
	    else
		e->current_position = e->files_count - 1;
	    handled = 1;
	}
	//send selected event before 
	/* if(e->onselection != 0 && last_item != e->selected) */
	/*     e->onselection(e, e->selected); */

	if(data.key == M_KEY_OK)
	{
	    Filelist_Item *it = &e->buffer[e->current_position -
					  e->current_chunk_position];
	    if(it->am_dir)
		//if directory
		filelist_open(e, it->name);
	    else
	    {
		e->was_selected = 1;
		e->selected_folder = e->current_folder;
		printf("fileviewer ok  onselection\n");
		if(strncmp(e->selected_file, it->fname, 13) != 0
		   && e->onselection != 0 &&
		   //call onselection
		   e->onselection(e, it))
		{
		    //strncpy(e->selected_file, it->fname, 13);
		    e->selected_file_id = it->id;
		    if(e->click != 0)
		    {
			e->click(e, it);
		    }
		}
	    }
	    handled = 1;
	}
	
    }
    
    return handled ? M_INPUT_HANDLED : M_INPUT_NOT_HANDLED;

}


uint32_t filelist_count_files(char* path)
{
    uint32_t count = 0;
    DIR dir;
    static FILINFO fno;
    FRESULT res;
//  static char b[260] = {
    //fno.lname = 0;

    
    res = f_opendir(&dir, path);                       /* Open the directory */
    //printf("opdir %d\n", res);
    if (res == FR_OK) {
	res = f_readdir(&dir, 0   );
	for (;;) {
	    res = f_readdir(&dir, &fno);                   /* Read a directory item */
	    //printf("%d %d %s\n", count, res, fno.fname);
	    if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
	    count ++;
	}
	f_closedir(&dir);
    }
    return count;
}


void m_filelist_loadchunk(Filelist *l, uint32_t required_id)
{
    char bu[5] = "";
    //do we need to show [..](go back)
    uint8_t isroot = f_getcwd(bu, 5);
    isroot = (bu[0] == '/') && (bu[1] == 0);
    //printf("root %s| %d\n", bu, isroot);
    uint32_t count = l->files_count = filelist_count_files("") + !isroot;

    //printf("files coint: %d\n", count);
    
    //calculate chunk's start position
    if(required_id >= FM_BUFFERED/2)
    {
	l->current_chunk_position = required_id - FM_BUFFERED/2;
	if(l->current_chunk_position != 0 &&
	   l->current_chunk_position + FM_BUFFERED/2 - 1 >= l->files_count)
	{
	    uint32_t d = l->current_chunk_position + FM_BUFFERED/2 - 1 - l->files_count;
	    if(d > l->current_chunk_position)
	    {
		l->current_chunk_position = 0;
	    }
	    else
	    {
		l->current_chunk_position -= d;
	    }
	}
    }
    else
    {
	l->current_chunk_position = 0;
    }
    //printf("ch st: %d req %d\n", l->current_chunk_position, required_id);
    
    FRESULT res;
    DIR dir;
    static FILINFO fno;

    uint32_t ci = !isroot, //file's ID
	bi = 0; //id in the buffer
    uint8_t sel_dir = 0; //was current dir selected

    //add go back items
    if(l->current_chunk_position == 0 && !isroot)
    {
	strcpy(l->buffer[0].name, "..");
	strcpy(l->buffer[0].fname, "..");
	l->buffer[0].am_dir = 1;
    }
    
    res = f_opendir(&dir, "");                       /* Open current directory */
    if (res == FR_OK) {
	l->current_folder = dir.obj.sclust; //set current dir
	sel_dir = l->current_folder == l->selected_folder; //is current dir the selected
	
	//printf("sclust %d\n", dir.obj.sclust);
        for (;;) {
	    if(ci >= l->current_chunk_position)
	    {
		bi = ci - l->current_chunk_position;
		//fno.lfname = l->buffer[bi].name;
		//fno.lfsize = _MAX_LFN + 1;
	    }
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
	    //printf("cl %d %s ", ci, fno.fname);
	    if(ci >= l->current_chunk_position)
	    {
		//printf("buf %d\n", bi);
		//if(fno.lfname[0] == 0)
		strcpy(l->buffer[bi].name, fno.fname);
		strcpy(l->buffer[bi].fname, fno.altname);
		//l->buffer[bi].sclust = 

		l->buffer[bi].am_dir = fno.fattrib & AM_DIR;

		l->buffer[bi].id = ci;
		bi ++;
	    }
	    //else
	    //printf("\n");
	    ci ++;
	    if(bi >= FM_BUFFERED)
		break;
        }
        f_closedir(&dir);
    }
    else
    {
	printf("ERROR while oppenning directory! code: %d\n\r", res);
    }    
}

void m_filelist_deselect(Filelist *l)
{
    l->was_selected = 0;
}

void filelist_open(Filelist *l, char *path)
{
    //set path
    l->path = path;
    //go to dir
    f_chdir(path);

    //reset selection and etc
    //l->selected_file[0] = 0;
    l->current_position = 0;
    l->files_count = 0;
    l->current_folder = 0;

    //load first chunk
    m_filelist_loadchunk(l, 0);
}
