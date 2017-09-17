#ifndef _MAKISE_H_G_CONTAINER
#define _MAKISE_H_G_CONTAINER 1

typedef struct _MContainer MContainer;


#include "makise_gui.h"
#include "makise_gui_elements.h"

typedef struct _MContainer
{
    MakiseGUI *gui;
    MElement* el; //element which is host for the container. Can be zero if it is MHost
    MPosition *position; //container position
    MElement* first; //pointer to the first element or 0 if container is empty
    MElement* last; //pointer to the last element
    MElement* focused; //pointer to the focused element. Element must be in the container
    
//    void (*on_add)(MElement* el);
} MContainer;

void makise_g_cont_init(MContainer *c);
/**
 * add element to new container
 *
 * @param cont container
 * @param el element
 * @return 
 */
void makise_g_cont_add(MContainer * cont, MElement *el);
/**
 * remove element from it's parent container
 *
 * @param el element
 * @return 
 */
void makise_g_cont_rem(MElement *el);
void makise_g_cont_clear(MContainer *cont);
/**
 * insert element in exact place of container children's list
 *
 * @param cont container
 * @param el element
 * @param index required index in the list
 * @return new index or -1 if error
 */
int32_t makise_g_cont_insert(MContainer * cont, MElement *el, uint32_t index);
/**
 * replace two elements in container's lists even they have different parents
 *
 * @param e1 
 * @param e2 
 * @return 
 */
void makise_g_cont_replace(MElement *e1, MElement *e2);
/**
 * determines if container contains element
 *
 * @param container
 * @param element
 * @return -1 if element wasn't found and index of element in list if was
 */
int32_t makise_g_cont_contains(MContainer * cont, MElement *el);
/**
 * return element's index in the parent container's children list
 *
 * @param container
 * @param element
 * @return -1 if element wasn't found and index of element if was
 */
int32_t makise_g_cont_index(MElement *el);


uint8_t makise_g_cont_call   (MContainer *cont, uint8_t type);
MInputResultEnum makise_g_cont_input  (MContainer *cont, MInputData data);
MFocusEnum makise_g_cont_focus_next(MContainer *cont);
MFocusEnum makise_g_cont_focus_prev(MContainer *cont);
/**
 * Send leave event to all children elements recursevely
 *
 * @param cont 
 * @return 
 */
void makise_g_cont_focus_leave(MContainer *cont);
MElement* makise_g_cont_element_on_point(MContainer *cont, int32_t  x, int32_t y);
#endif









