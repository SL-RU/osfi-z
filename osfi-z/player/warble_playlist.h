#ifndef WARBLE_PLAYLIST_H
#define WARBLE_PLAYLIST_H
#include "warble.h"

typedef enum {
    WPlaylist_Normal,
    WPlaylist_Audiobook,
    WPlaylist_CUE,
} WPlaylistType;

typedef struct {
    char *name;
    
    WPlaylistType type;
    void *data;
    struct {
	void (*oncomplete)(WTrack *track);
	void (*state_update)(WTrack *track);
	void (*metadata_update)(WTrack *track);
	void (*time_elapsed)(WTrack *track, uint32_t time);
    } handlers; //handlers of specific events

    void (*getnext)();
    void (*getprev)();
    void (*getcount)();
    void (*getbyindex)();
} WPlaylist;



#endif
