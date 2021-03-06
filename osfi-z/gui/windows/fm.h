#ifndef APPS_H
#define APPS_H
#include "gui.h"
#include "makise_e.h"
#include <string.h>
#include "warble.h"


typedef enum
{
    FM_MODE_FM,
    FM_MODE_Volume,
    FM_MODE_SD_Error,
} FM_MODE;


void fm_cre(char *art, char *tit, char *alb);

MElement *fm_init();
void fm_switch();
void fm_switch_to(uint8_t cur_mode);
void fm_playpause();
void fm_encoder(uint8_t left, int32_t value);
void fm_update();
#endif
