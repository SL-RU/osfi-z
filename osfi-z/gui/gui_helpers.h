#ifndef GUI_HELP_HELPERS_H
#define GUI_HELP_HELPERS_H
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
