#include "gui_styles.h"

MakiseStyle_Button ts_button =
{
    .font = &F_Default10x20,
    .bitmap_gap = 10,
    //bg       font     border   double_border
    .normal =  {MC_Black, MC_White, MC_Black, 0}, //normal
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
    {MC_Black, MC_White, MC_Black, 0}, //focused
    {MC_Black, MC_White, MC_Black, 0}, //active
};
MakiseStyle_Lable ts_lable =
{
    .font = &F_Default8x13,
    //font       bg     border   double_border
    .font_col = MC_White,
    .bg_color = MC_Black,
    .border_c = MC_Black,
    .double_border = 0,
    .scroll_speed = 100
};
MakiseStyle_Lable ts_lable_small =
{
    .font = &F_Default5x7,
    //font       bg     border   double_border
    .font_col = MC_White,
    .bg_color = MC_Black,
    .border_c = MC_Black,
    .double_border = 0,
    .scroll_speed = 200
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
    .font              = &F_Default6x10,
    .font_line_spacing = 0,
    .left_margin       = 0,
    .item_margin       = 0,
    //scroll
    .scroll_width    = 3,
    .scroll_bg_color = MC_Transparent,
    .scroll_color     = MC_White,
    
    //bg       font     border   double_border
    .normal  = {MC_Black, MC_White, MC_Black, 0},
    .focused = {MC_Black, MC_White, MC_Black, 0},
    .active  = {MC_Black, MC_White, MC_White, 0},
};
MakiseStyle_SListItem ts_slist_item_big =
{
    .font              = &F_Default8x13,
    .font_line_spacing = 0,
    .text_scroll_speed = 100,
    //bg       font     border   double_border
    .normal  = {MC_Transparent, MC_White, MC_Black, 0}, 
    .focused = {MC_White,       MC_Black, MC_White, 0}, 
    .active  = {MC_Transparent, MC_White, MC_White, 0}, 
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

    .text_scroll_speed = 100,
    
    .normal  = {MC_Black, MC_White,  MC_Black,  MC_White, 0},
    .focused = {MC_White, MC_Black, MC_White, MC_Black, 0},
    .active  = {MC_Black,  MC_White, MC_White, MC_Black, 0},
};

MakiseStyle_Canvas ts_container_clear =
{
    //bg       border   double_border
    {MC_Transparent, MC_Transparent, 0},  //normal
    {MC_Transparent, MC_Transparent, 0},  //focused
};
MakiseStyle_Canvas ts_container_black =
{
    //bg       border   double_border
    {MC_Black, MC_Black, 0},  //normal
    {MC_Black, MC_Black, 0},  //focused
};
