#include "warble_playlist.h"



static uint32_t buffer[WP_size];
static uint32_t buffer_i;
uint8_t * wp_buffer_request(uint32_t size)
{
    uint8_t * pos = ((uint8_t*)buffer) + buffer_i;
    buffer_i += size;
    return pos;
}


WResult wp_create(WPlaylist *playlist, WPlaylistType type)
{
    
    return W_OK;
}
