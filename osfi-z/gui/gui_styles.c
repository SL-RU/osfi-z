#include "gui_styles.h"

MakiseStyle_Button ts_button =
{
    .font = &F_Default10x20,
    .bitmap_gap = 10,
    //bg       font     border   double_border
    .normal =  {MC_Black, MC_White, MC_White, 0}, //normal
    .focused = {MC_White, MC_Black, MC_White, 0}, //focused
    .active =  {MC_Black, MC_White, MC_White, 0}, //active
};

MakiseStyle_Button ts_button_small =
{
    &F_Default8x13,
    3,
     //bg       font     border   double_border
    {MC_Black, MC_White, MC_White, 0},//normal
    {MC_White, MC_Black, MC_White, 0}, //focused
    {MC_Black, MC_White, MC_White, 0}, //active
};

MakiseStyle ts_slider =
{
    MC_White,
    &F_Default10x20,
    0,
    //bg       font     border   double_border
    {0, 0,  0, 0},  //unactive
    {MC_Black, MC_White, MC_Black, 0},//normal
    {MC_Black, MC_White, MC_White, 0}, //focused
    {MC_Black, MC_White, MC_White, 0}, //active
};
MakiseStyle_Lable ts_lable =
{
    &F_Default10x20,
    //font       bg     border   double_border
    MC_White, MC_Black, MC_Black, 0
};
MakiseStyle_Lable ts_lable_small =
{
    &F_Default6x10,
    //font       bg     border   double_border
    MC_White, MC_Black, MC_Black, 0
};


MakiseStyle ts_textfield =
{
    MC_White,
    &F_Default6x10,
    3,
    //bg       font     border   double_border
    {MC_Black, MC_White, MC_Black, 0},  //unactive
    {MC_Black, MC_White, MC_Black, 0},  //unactive
    {0, 0, 0, 0}, //focused
    {0, 0, 0, 0}, //active
};

MakiseStyle_Canvas ts_canvas =
{
    //bg       border   double_border
    {MC_Black, MC_White, 0},  //normal
    {MC_Black, MC_White, 0},  //focused
};

MakiseStyle ts_tabs =
{
    MC_White,
    &F_Default6x10,
    0,
    //bg       font     border   double_border
    {MC_Black, MC_Black, MC_Black,    0},  //unactive
    {MC_Black, MC_White, MC_White, 0},  //normal
    {MC_Black, MC_Black, MC_White,   0},  //focused
    {MC_Black, MC_Black, MC_White, 0},  //active
};

MakiseStyle_SListItem ts_slist_item =
{
    &F_Default6x10,
    0,
    //bg       font     border   double_border
    {MC_Black, MC_White, MC_White, 0},  //normal
    {MC_White, MC_Black, MC_White,   0},  //focused
    {MC_Black, MC_White, MC_White, 0},  //active
};
MakiseStyle_SList ts_slist =
{
    &F_Default6x10,
    0,
    0, //left margin
    0, //item margin
    //scroll
    //width  bg color        scroll color
    5,     MC_Transparent, MC_White,
    //bg       font     border   double_border
    {MC_Black, MC_White, MC_Black, 0},  //normal
    {MC_Black, MC_White, MC_White,   0},  //focused
    {MC_Black, MC_White, MC_White, 0},  //active
};
MakiseStyle_SList ts_slist_small =
{
    &F_Default8x13,
    0,
    0, //left margin
    0, //item margin
    //scroll
    //width  bg color        scroll color
    5,     MC_White, MC_Black,
    //bg       font     border   double_border
    {MC_Black, MC_Black, MC_Black, 0},  //normal
    {MC_Black, MC_White, MC_White,   0},  //focused
    {MC_Black, MC_White, MC_White, 0},  //active
};
MakiseStyle_SListItem ts_slist_item_big =
{
    &F_Default5x7,
    0,
    //bg       font     border   double_border
    {MC_Transparent, MC_White, MC_White, 0},  //normal
    {MC_Black, MC_White, MC_White,   0},      //focused
    {MC_Black, MC_Black, MC_Transparent, 0},   //active
};

static const uint8_t B_folder_data[] = {
    0x1e, 0xc2, 0x05, 0xcc, 0x5f, 0xb0, 0x60, 0x41, 0x7f, 
    0x00,  };
const MakiseBitmap B_folder = { 
    .width = 9,
    .height = 8,
    .data = B_folder_data
};

MakiseStyle_FSViewer ts_fsviewer = {
    .font               = &F_Default6x10,
    .font_line_spacing  = 0,
    .bitmap_folder      = &B_folder,
    .left_margin        = 0,
    .item_margin        = 0,
    .scroll_width       = 3,
    .scroll_bg_color    = MC_Black,
    .scroll_color       = MC_White,
    
    .normal  = {MC_Black, MC_Black,  MC_Black, 0},
    .focused = {MC_Black, MC_White, MC_Black, 0},
    .active  = {MC_Black, MC_White, MC_Black, 0},
};
MakiseStyle_FSViewer_Item ts_fsviewer_item = {
    .font               = &F_Default6x10,
    .font_line_spacing  = 0,

    .normal  = {MC_Black, MC_White,  MC_Black,  MC_White, 0},
    .focused = {MC_White, MC_Black, MC_White, MC_Black, 0},
    .active  = {MC_Black,  MC_White, MC_White, MC_Black, 0},
};
