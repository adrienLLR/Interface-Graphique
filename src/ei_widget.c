#include "ei_widget.h"
#include "ei_draw.h"
#include "ei_widgetclass.h"
#include "ei_frame.h"
#include "ei_button.h"
#include "ei_toplevel.h"
#include "ei_utils.h"
#include "ei_utilsfct.h"
#include "ei_application.h"
#include "ei_geometrymanager.h"

int widget_index = 0;

struct ei_widget_t * ei_widget_create (ei_widgetclass_name_t    class_name,
                                       struct ei_widget_t           *parent,
                                       void                  *user_data,
                                       ei_widget_destructor_t destructor)
{
    ei_widgetclass_t *class_t = ei_widgetclass_from_name(class_name);
    if (class_t == NULL) {
        return NULL;
    }
    struct ei_widget_t *new_wid = class_t -> allocfunc();
    if (new_wid == NULL) {
        return NULL;
    }
    new_wid -> wclass = class_t;
    new_wid -> pick_id = widget_index;
    ei_color_t *col = malloc(sizeof(ei_color_t));
    int ir, ig, ib, ia;
    hw_surface_get_channel_indices(pick_surface, &ir, &ig, &ib, &ia);
    ia = -1;
    col -> red = (widget_index >> (8 * ir)) % 256;
    col -> green = (widget_index >> (8 * ig)) % 256;
    col -> blue = (widget_index >> (8 * ib)) % 256;
    col -> alpha = 255;
    new_wid -> pick_color = col;
    widget_index++;
    new_wid -> user_data = user_data;
    new_wid -> destructor = destructor;
    if (parent != NULL) {
        struct ei_widget_t *child = parent->children_tail;
        if (child == NULL) {
            parent -> children_head = new_wid;
            parent -> children_tail = new_wid;
        } else {
            child -> next_sibling = new_wid;
            parent -> children_tail = new_wid;
        }
    }
    new_wid -> parent = parent;
    new_wid -> children_head = NULL;
    new_wid -> children_tail = NULL;
    new_wid -> next_sibling = NULL;
    new_wid -> geom_params = NULL;
    new_wid -> requested_size = ei_size_zero();
    new_wid -> screen_location = ei_rect_zero();
    new_wid -> content_rect = &(new_wid -> screen_location);
    class_t -> setdefaultsfunc(new_wid);
    return new_wid;
}



void ei_widget_destroy_rec (struct ei_widget_t *widget)
{
    if (widget -> children_head != NULL) ei_widget_destroy(widget -> children_head);
    if (widget -> next_sibling != NULL) ei_widget_destroy(widget -> next_sibling);
    if (widget -> parent != NULL) {
        remove_widget_from_children(widget -> parent, widget);
    }
    if (widget -> destructor != NULL) {
        widget -> destructor(widget);
    } else {
        widget -> wclass -> releasefunc(widget);
    }
}

void ei_widget_destroy (struct ei_widget_t *widget)
{
    if (widget -> children_head != NULL) ei_widget_destroy_rec(widget -> children_head);
    if (widget -> parent != NULL) {
        remove_widget_from_children(widget -> parent, widget);
    }
    ei_geometrymanager_unmap(widget);
    if (widget -> destructor != NULL) {
        widget -> destructor(widget);
    } else {
        widget -> wclass -> releasefunc(widget);
    }
}



struct ei_widget_t * ei_widget_pick (ei_point_t *where)
{
    ei_widget_t *root_widget = ei_app_root_widget();
    ei_color_t pointed_color = ei_get_pointed_pick_surface_color(pick_surface, *where);
    int pick_id = int_from_pick_color(pointed_color);
    ei_widget_t *widget_pointed_at = recursif_seek_widget_with_pickid(root_widget, pick_id);
    return widget_pointed_at;
}

void ei_frame_configure (struct ei_widget_t  *widget,
                         ei_size_t    *requested_size,
                         const  ei_color_t   *color,
                         int          *border_width,
                         ei_relief_t  *relief,
                         char        **text,
                         ei_font_t    *text_font,
                         ei_color_t   *text_color,
                         ei_anchor_t  *text_anchor,
                         ei_surface_t *img,
                         ei_rect_t   **img_rect,
                         ei_anchor_t  *img_anchor) {
    struct ei_frame *frame = (struct ei_frame *) widget;
    if (requested_size != NULL) widget->requested_size = *requested_size;
    if (color != NULL) frame->color = *color;
    if (relief != NULL) frame->relief = *relief;
    if (border_width != NULL) frame->border_width = *border_width;
    if (text != NULL) {
        if (*text != NULL) {
            frame->text = strdup(*text);
        } else {
            frame->text = NULL;
        }
    }
    if (text_font != NULL) frame->text_font = *text_font;
    if (text_color != NULL) frame->text_color = *text_color;
    if (text_anchor != NULL) frame->text_anchor = *text_anchor;
    if (img != NULL) {
        if (frame->img != NULL) free(frame->img);
        if (*img != NULL) {
            ei_size_t size = hw_surface_get_size(*img);
            ei_surface_t img_cpy = hw_surface_create(*img, size, EI_FALSE);
            ei_rect_t rect = ei_rect(ei_point_zero(), size);
            ei_copy_surface(img_cpy, &rect, *img, &rect, EI_FALSE);
            frame->img = img_cpy;
        } else {
            frame->img = NULL;
        }
    }
    if (img_rect != NULL) {
        if (frame->img_rect != NULL) free(frame->img_rect);
        ei_rect_t *img_rect_cpy = malloc(sizeof(ei_rect_t));
        *img_rect_cpy = **img_rect;
        frame->img_rect = img_rect_cpy;
    }
    if (img_anchor != NULL) frame->img_anchor = *img_anchor;
    if (requested_size == NULL && text != NULL) {
        int width, height;
        hw_text_compute_size(frame->text, frame->text_font, &width, &height);
        widget->requested_size = ei_size(width, height);
    }
    ei_app_invalidate_rect(&widget -> screen_location);
}

void ei_button_configure(       ei_widget_t   *widget,
                                ei_size_t     *requested_size,
                         const  ei_color_t    *color,
                                int           *border_width,
                                int           *corner_radius,
                                ei_relief_t   *relief,
                                char         **text,
                                ei_font_t     *text_font,
                                ei_color_t    *text_color,
                                ei_anchor_t   *text_anchor,
                                ei_surface_t  *img,
                                ei_rect_t    **img_rect,
                                ei_anchor_t   *img_anchor,
                                ei_callback_t *callback,
                                void **       user_param)
{
    struct ei_button *button = (struct ei_button *) widget;
    if (corner_radius != NULL) button -> corner_radius = *corner_radius;
    if (callback != NULL) button -> callback = *callback;
    if (user_param != NULL) button -> user_param = *user_param;
    ei_frame_configure(widget,
                       requested_size,
                       color,
                       border_width,
                       relief,
                       text,
                       text_font,
                       text_color,
                       text_anchor,
                       img,
                       img_rect,
                       img_anchor);
}

void ei_toplevel_configure (ei_widget_t*		widget,
                            ei_size_t*		requested_size,
                            ei_color_t*		color,
                            int*			border_width,
                            char**			title,
                            ei_bool_t*		closable,
                            ei_axis_set_t*		resizable,
                            ei_size_t**		min_size)
{
    struct ei_toplevel *toplevel = (struct ei_toplevel *) widget;
    if (requested_size != NULL) widget -> requested_size = *requested_size;
    if (color != NULL) toplevel -> color = *color;
    if (border_width != NULL) toplevel -> border_width = *border_width;
    if (title != NULL) toplevel -> title = *title;
    if (closable != NULL) toplevel -> closable = *closable;
    if (resizable != NULL) toplevel -> resizable = *resizable;
    if (min_size != NULL) toplevel -> min_size = **min_size;
    ei_app_invalidate_rect(&widget -> screen_location);
}
