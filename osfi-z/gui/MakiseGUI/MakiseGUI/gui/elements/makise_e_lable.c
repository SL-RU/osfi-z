#include "makise_e.h"

static uint8_t draw   (MElement* b);

static char name[] = "Lable";
void m_create_lable(MLable* b, MContainer *c,
		    MPosition pos,
		    char* text,
		    MDTextPlacement place,
		    MakiseStyle *style)
{
    MElement *e = &b->el;

    m_element_create(e, (c == 0) ? 0 : c->gui, name, b,
		     1, 1, pos,
		     &draw,
		     0,
		     0,
		     0,
		     0,
		     0, 0);
    
    b->text = text;
    b->style = style;
    b->place = place;

    makise_g_cont_add(c, e);
    
    printf("Lable %d created\n", e->id);
}

static uint8_t draw   (MElement* b)
{
    MakiseStyleTheme *th = &((MLable*)b->data)->style->normal;
    
    _m_e_helper_draw_box(b->gui->buffer, &b->position, th);
    if(((MLable*)b->data)->text != 0)
	makise_d_string(b->gui->buffer,
			((MLable*)b->data)->text, MDTextAll,
			b->position.real_x +
			((((MLable*)b->data)->place == MDTextPlacement_RightUp) ?
			 (b->position.width - 2) :
			 2),// + b->position.width / 2,
			b->position.real_y,// + b->position.height / 2,
			((MLable*)b->data)->place,
			((MLable*)b->data)->style->font,
			th->font_col);
    b->position.left = (b->position.left + 1) % 100;
    
    return M_OK;
}
