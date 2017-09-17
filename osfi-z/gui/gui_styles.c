#include "gui_styles.h"

MakiseStyle ts_button =
{
    MC_White,
    &F_Arial12,
    0,
    //bg       font     border   double_border
    {MC_Black, MC_White, MC_White, 0},  //unactive
    {MC_Black, MC_White, MC_White, 0},//normal
    {MC_White, MC_White, MC_White, 0}, //focused
    {MC_White, MC_White, MC_White, 0}, //active
};
MakiseStyle ts_button_small =
{
    MC_White,
    &F_Arial12,
    0,
    //bg       font     border   double_border
    {MC_Black, MC_White, MC_White, 0},  //unactive
    {MC_Black, MC_White, MC_White, 0},//normal
    {MC_White, MC_White, MC_White, 0}, //focused
    {MC_White, MC_White, MC_White, 0}, //active
};


MakiseStyle ts_slider =
{
    MC_White,
    &F_Arial12,
    0,
    //bg       font     border   double_border
    {0, 0,  0, 0},  //unactive
    {MC_Black, MC_White, MC_White, 0},//normal
    {MC_Black, MC_White, MC_White, 0}, //focused
    {MC_Black, MC_White, MC_White, 0}, //activec
};
MakiseStyle ts_slider_mute =
{
    MC_White,
    &F_Arial12,
    0,
    //bg       font     border   double_border
    {0, 0,  0, 0},  //unactive
    {MC_Black, MC_Black, MC_White, 0},//normal
    {MC_Black, MC_Black, MC_White, 0}, //focused
    {MC_White, MC_Black, MC_White, 0}, //active
};

MakiseStyle ts_lable =
{
    MC_White,
    &F_Arial12,
    0,
    //bg       font     border   double_border
    {0, 0, 0, 0},  //unactive
    {MC_Black, MC_White, MC_Black, 0},//normal
    {0, 0, 0, 0}, //focused
    {0, 0, 0, 0}, //active
};
MakiseStyle ts_lable_small =
{
    MC_White,
    &F_Arial16,
    0,
    //bg       font     border   double_border
    {0, 0, 0, 0},  //unactive
    {MC_Black, MC_White, MC_Black, 0},//normal
    {0, 0, 0, 0}, //focused
    {0, 0, 0, 0}, //active
};


MakiseStyle ts_textfield =
{
    MC_White,
    &F_Arial24,
    3,
    //bg       font     border   double_border
    {MC_Transparent, MC_White, MC_Transparent, 0},  //unactive
    {MC_Transparent, MC_White, MC_Transparent, 0},  //unactive
    {0, 0, 0, 0}, //focused
    {0, 0, 0, 0}, //active
};

MakiseStyle ts_canvas =
{
    MC_White,
    &F_Arial12,
    0,
    //bg       font     border   double_border
    {MC_Transparent, MC_Transparent, MC_Transparent, 0},  //unactive
    {MC_Transparent, MC_Transparent, MC_Transparent, 0},  //normal
    {MC_Transparent, MC_Transparent, MC_Transparent, 0},  //focused
    {MC_Transparent, MC_Transparent, MC_Transparent, 0},  //active
};

MakiseStyle ts_tabs =
{
    MC_White,
    &F_Arial12,
    0,
    //bg       font     border   double_border
    {MC_Black, MC_White, MC_White,    0},  //unactive
    {MC_White, MC_White, MC_White, 0},  //normal
    {MC_Black, MC_White, MC_White,   0},  //focused
    {MC_Black, MC_White, MC_White, 0},  //active
};

MakiseStyle ts_slist_item =
{
    MC_White,
    &F_Arial12,
    0,
    //bg       font     border   double_border
    {MC_Black, MC_White, MC_White,    0},  //unactive
    {MC_Black, MC_White, MC_White, 0},  //normal
    {MC_White, MC_Black, MC_White,   0},  //focused
    {MC_White, MC_Black, MC_Black, 0},  //active
};
MakiseStyle ts_slist =
{
    MC_White,
    &F_Arial12,
    0,
    //bg       font     border   double_border
    {MC_Black, MC_White, MC_Black,    0},  //unactive
    {MC_Black, MC_White, MC_Black, 0},  //normal
    {MC_Black, MC_White, MC_Black,   0},  //focused
    {MC_Black, MC_White, MC_Black, 0},  //active
};
MakiseStyle ts_slist_small =
{
    MC_White,
    &F_Arial12,
    0,
    //bg       font     border   double_border
    {MC_Black, MC_White, MC_White,    0},  //unactive
    {MC_Black, MC_White, MC_White, 0},  //normal
    {MC_Black, MC_White, MC_White,   0},  //focused
    {MC_Black, MC_White, MC_White, 0},  //active
};
MakiseStyle ts_slist_item_big =
{
    MC_White,
    &F_Arial12,
    0,
    //bg       font     border   double_border
    {MC_Transparent, MC_White, MC_White,    0},  //unactive
    {MC_Transparent, MC_White, MC_White, 0},  //normal
    {MC_White, MC_White, MC_White,   0},  //focused
    {MC_Transparent, MC_White, MC_White, 0},  //active
};
