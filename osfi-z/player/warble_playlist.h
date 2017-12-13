#ifndef WARBLE_PLAYLIST_H
#define WARBLE_PLAYLIST_H
#include "warble.h"

#define WP_size 1000

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
	WResult (*oncomplete      )(WTrack *track);
	WResult (*state_update    )(WTrack *track);
	WResult (*metadata_update )(WTrack *track);
	WResult (*time_elapsed)   (WTrack *track, uint32_t time);
    } handlers; //handlers of specific events

    enum {
	WP_play,
	WP_end,
    } state;

    WResult (*getnext)   (WTrack *track);
    WResult (*getprev)   (WTrack *track);
    WResult (*getcurrent)(WTrack *track);
    WResult (*getcount)  (uint32_t *count);
    WResult (*getbyindex)(WTrack *track, uint32_t index);
    WResult (*load) ();
    WResult (*save) ();
    WResult (*close)();
} WPlaylist;

WResult wp_create(WPlaylist *playlist, WPlaylistType type);
WResult wp_load  (WPlaylist *playlist, TCHAR *path);
WResult wp_close (WPlaylist *playlist);

uint8_t * wp_buffer_request(uint32_t size);

#endif
