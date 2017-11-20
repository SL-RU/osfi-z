#include "gui_helpers.h"

void gh_sprint_time(char *s, uint32_t len,
		    uint32_t ms, uint8_t show_ms) {
    if(s == 0 || len == 0)
	return;
    uint32_t h, m, sec, p;

    p = ms % 1000;
    ms /= 1000;
    sec = ms % 60;
    ms /= 60;
    m = ms % 60;
    ms /= 60;
    h = ms;
    
    if(show_ms) {
	if(h != 0)
	    snprintf(s, len, "%02d:%02d:%02d.%03d", h, m, sec, p);
	else
	    snprintf(s, len, "%02d:%02d.%03d", m, sec, ms);
    } else {
	if(h != 0)
	    snprintf(s, len, "%02d:%02d:%02d", h, m, sec);
	else
	    snprintf(s, len, "%02d:%02d", m, sec);
    }
}
