#include "makise_e.h"
#include "gui.h"
#include "ffconf.h"
#include "ff.h"
#include <string.h>

#define FM_BUFFERED 16 //how many lines are buffered

//Simple list element.
//Can display items. Supports scrolling.
//Modes: list, radio buttons, checkboxes.
typedef enum _Filelist_Type
{
    Filelist_Viewer,
    Filelist_SingleSelect,
//    Filelist_MultiSelect
} Filelist_Type;
typedef struct _Filelist_Item Filelist_Item;
typedef struct _Filelist Filelist;
typedef struct _Filelist_Item
{
    char name[FF_MAX_LFN + 2];        //text of item
    char fname[13];                 //short file name
    DWORD sclust;                 //start of clust
    uint8_t am_dir;                 //is dir 
    uint32_t size;                  //file's size
    //uint8_t selected;               //is selected
    
    Filelist_Item *prev;
    Filelist_Item *next;

    uint32_t id;       //custom id, if NOT is_array, else - position in the array(will be computed automatically by MFilelist).
} Filelist_Item;

typedef struct _Filelist {
    MakiseGUI *gui;
    MElement el;

    char *header; //header. if ==0, then it won't show
    char *path;   //current folder's path

    Filelist_Item buffer[FM_BUFFERED]; //buffer for the linked list


    //for displaying great amount of files in folder. Caching
    uint32_t displayed_count; //how many lines are on the screen
    uint32_t files_count; //all files in directory
    uint32_t current_chunk_position; //current chunk. Chunk contains FM_BUFFERED lines

    //current position
    uint32_t current_position; //current position in folder
    DWORD current_folder; //current folder's sclust

    //selected file
    uint8_t was_selected; //Flag
    char selected_file[14]; //selected file's name
    uint32_t selected_file_id; //selected file's name
    DWORD selected_folder; //folder which contains selected file. It's sclust

    uint8_t (*onselection)(Filelist *l, Filelist_Item *selected); //when selected item is changing. return 1 for accepting and 0 to not
    void (*click)(Filelist *l, Filelist_Item *selected);      //when OK button clicked

    void (*done)(Filelist *l, Filelist_Item *list);      //when all required elements are selected

    Filelist_Type type;
    MakiseStyle* style;
    MakiseStyle *item_style;

    uint32_t state; //focus state
} Filelist;

void m_create_filelist(Filelist* b, MContainer *c,
		       MPosition pos,
		       char* header,
		       uint8_t (*onselection)(Filelist *l, Filelist_Item *selected),
		       void (*click)(Filelist *l, Filelist_Item *selected),
		       Filelist_Type type,
		       MakiseStyle *style,
		       MakiseStyle *item_style);
void m_filelist_deselect(Filelist *l); //deselect all
void m_filelist_refresh(Filelist *l); //set linked list as new data source.
void m_filelist_loadchunk(Filelist *l, uint32_t required_id); //load chunk with required position

void filelist_open(Filelist *l, char *path);
