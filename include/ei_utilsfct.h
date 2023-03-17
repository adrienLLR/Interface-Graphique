

#ifndef PROJETC_IG_EI_UTILSFCT_H
#define PROJETC_IG_EI_UTILSFCT_H

#include "ei_widget.h"

ei_surface_t *pick_surface;

ei_color_t color_from_uint32(uint32_t val, int ir, int ig, int ib, int ia);

int int_from_pick_color(ei_color_t color);

ei_rect_t intersec_rect(ei_rect_t rect1, ei_rect_t rect2);

void remove_widget_from_children(ei_widget_t *widget, ei_widget_t *children);

int dist_sq(ei_point_t pt1, ei_point_t pt2);

int max(int i1, int i2);

int min(int i1, int i2);

ei_rect_t union_rect(ei_rect_t rect1, ei_rect_t rect2);

ei_widget_t* recursif_seek_widget_with_pickid(ei_widget_t *widget_courant, int pickid);

ei_color_t ei_get_pointed_pick_surface_color(ei_surface_t pick_surface, ei_point_t where);

#endif //PROJETC_IG_EI_UTILSFCT_H
