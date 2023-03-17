
#ifndef PROJETC_IG_EI_TOPLEVEL_H
#define PROJETC_IG_EI_TOPLEVEL_H

#include "ei_utils.h"

static inline int ei_top_level_height()
{
    return 27;
}

static inline int ei_top_level_res_button()
{
    return 13;
}

typedef struct ei_toplevel {
    ei_widget_t widget;
    ei_color_t color;
    int border_width;
    char *title;
    ei_font_t title_font;
    ei_bool_t closable;
    ei_relief_t close_relief;
    ei_axis_set_t resizable;
    ei_size_t min_size;
} ei_toplevel;

#endif //PROJETC_IG_EI_TOPLEVEL_H
