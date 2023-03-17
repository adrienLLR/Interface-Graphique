#ifndef PROJETC_IG_EI_FRAME_H
#define PROJETC_IG_EI_FRAME_H

typedef struct ei_frame {
    ei_widget_t widget;
    ei_color_t color;
    ei_relief_t relief;
    int border_width;
    char *text;
    int text_size;
    ei_font_t text_font;
    ei_color_t text_color;
    ei_anchor_t text_anchor;
    ei_surface_t img;
    ei_rect_t *img_rect;
    ei_anchor_t img_anchor;
} ei_frame;

#endif //PROJETC_IG_EI_FRAME_H
