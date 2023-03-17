#include "ei_utils.h"
#include "ei_types.h"
#include "ei_application.h"
#include "ei_widget.h"

ei_color_t color_from_uint32(uint32_t val, int ir, int ig, int ib, int ia)
{
    ei_color_t col = {0, 0, 0, 0};
    col.red = ( val >> ( 8 * ir ) )%256;
    col.green = ( val >> ( 8 * ig ) )%256;
    col.blue = ( val >> ( 8 * ib ) )%256;
    if (ia != -1) col.alpha = ( val >> ( 8 * ia ) )%256;
    return col;
}

void remove_widget_from_children(ei_widget_t *widget, ei_widget_t *children) {
    ei_widget_t *current = widget -> children_head;
    if (current == children) {
        widget -> children_head = children -> next_sibling;
        if (widget -> children_head == NULL) widget -> children_tail = NULL;
        return;
    }
    while (current -> next_sibling != NULL && current -> next_sibling != children) {
        current = current -> next_sibling;
    }
    if (current -> next_sibling == children) {
        if (widget -> children_tail == children) widget -> children_tail = current;
        current -> next_sibling = children -> next_sibling;
    }
}

int int_from_pick_color(ei_color_t color) {
    int ret = 0;
    ret += color.red << (8 * 2);
    ret += color.green << (8 * 1);
    ret += color.blue << (8 * 0);
    return ret;
}

int dist_sq(ei_point_t pt1, ei_point_t pt2) {
    return (pt1.x - pt2.x) * (pt1.x - pt2.x) + (pt1.y - pt2.y) * (pt1.y - pt2.y);
}

int max(int i1, int i2) {
    if (i1 < i2) return i2;
    return i1;
}

int min(int i1, int i2) {
    return - max(-i1, -i2);
}

ei_rect_t intersec_rect(ei_rect_t rect1, ei_rect_t rect2) {
    ei_rect_t ret_rect = ei_rect_zero();
    ret_rect.top_left.x = max(rect1.top_left.x, rect2.top_left.x);
    ret_rect.top_left.y = max(rect1.top_left.y, rect2.top_left.y);
    ret_rect.size.width = max(0, min(rect1.top_left.x + rect1.size.width, rect2.top_left.x + rect2.size.width) - ret_rect.top_left.x);
    ret_rect.size.height = max(0, min(rect1.top_left.y + rect1.size.height, rect2.top_left.y + rect2.size.height) - ret_rect.top_left.y);
    return ret_rect;
}

ei_rect_t union_rect(ei_rect_t rect1, ei_rect_t rect2)
{
    if (rect1.size.width == 0 && rect1.size.height == 0) return rect2;
    if (rect2.size.width == 0 && rect2.size.height == 0) return rect1;
    ei_rect_t ret_rect = ei_rect_zero();
    ret_rect.top_left.x = max(0, min(rect1.top_left.x, rect2.top_left.x));
    ret_rect.top_left.y = max(0, min(rect1.top_left.y, rect2.top_left.y));
    ret_rect.size.width = max(rect1.top_left.x + rect1.size.width, rect2.top_left.x + rect2.size.width) - ret_rect.top_left.x;
    ret_rect.size.height = max(rect1.top_left.y + rect1.size.height, rect2.top_left.y + rect2.size.height) - ret_rect.top_left.y;
    return ret_rect;
}

ei_widget_t* recursif_seek_widget_with_pickid(ei_widget_t *widget_courant, int pickid)
{
    if ( widget_courant == NULL )
    {
        return NULL;
    }
    ei_widget_t* widget_enfant = widget_courant -> children_head;
    ei_widget_t* widget_potentiel1 = recursif_seek_widget_with_pickid(widget_enfant, pickid);
    ei_widget_t* widget_frere = widget_courant -> next_sibling;
    ei_widget_t* widget_potentiel2 = recursif_seek_widget_with_pickid(widget_frere, pickid);

    if ( widget_potentiel2 != NULL && widget_potentiel2 -> pick_id == pickid ) return widget_potentiel2;
    if ( widget_potentiel1 != NULL && widget_potentiel1 -> pick_id == pickid ) return widget_potentiel1;
    if ( widget_courant -> pick_id == pickid ) return widget_courant;
    return NULL;
}

/* -------------------------------------------------------------------------- */
ei_color_t ei_get_pointed_pick_surface_color(ei_surface_t pick_surface, ei_point_t where)
{
    ei_widget_t *root_widget = ei_app_root_widget();
    uint32_t *address = (uint32_t*) hw_surface_get_buffer(pick_surface);
    uint32_t *address_temp = address + where.y * (root_widget -> requested_size.width) + where.x;
    int ir, ig, ib, ia;
    hw_surface_get_channel_indices(pick_surface, &ir, &ig, &ib, &ia);
    ia = -1;
    return color_from_uint32(*address_temp, ir, ig, ib, -1);
}