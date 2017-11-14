#ifndef APPS_H
#define APPS_H 1
#include "gui.h"

/**
 * Print time to string
 *
 * @param s string
 * @param len length of string
 * @param ms time in milliseconds
 * @param show_ms do need to show milliseconds
 * @return 
 */
void gh_sprint_time(char *s, uint32_t len,
		    uint32_t ms, uint8_t show_ms);

#endif
