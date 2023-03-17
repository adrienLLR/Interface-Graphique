#include "hw_interface.h"
#include "ei_widget.h"
#include "ei_draw.h"
#include "ei_frame.h"
#include "ei_button.h"
#include "ei_types.h"
#include "ei_toplevel.h"
#include "ei_utils.h"
#include "ei_geometrymanager.h"
#include "math.h"
#include "ei_utilsfct.h"

ei_widgetclass_t *ei_class = NULL;

int color_offset = 40;



void ei_widgetclass_register(ei_widgetclass_t *wclass)
{
    if (ei_class == NULL) {
        ei_class = wclass;
    } else {
        wclass -> next = ei_class;
        ei_class = wclass;
    }
}

ei_widgetclass_t *ei_widgetclass_from_name(ei_widgetclass_name_t name)
{
    ei_widgetclass_t *wclass = ei_class;
    while (wclass != NULL) {
        if (strcmp(wclass -> name, name) == 0) {
            return wclass;
        }
        wclass = wclass -> next;
    }
    return NULL;
}

/*
 * Common function
 * */

void draw_text(ei_widget_t *widget, ei_surface_t surface, ei_rect_t *clipper) {
    struct ei_frame *frame = (struct ei_frame *) widget;
    ei_rect_t loc = widget -> screen_location;
    if (frame -> text != NULL) {
        int width, height;
        hw_text_compute_size(frame->text, frame->text_font, &width, &height);
        ei_surface_t surf_txt = hw_text_create_surface(frame->text, frame->text_font, frame->text_color);
        hw_surface_lock(surf_txt);
        ei_point_t pos = loc.top_left;
        switch (frame->text_anchor) {
            case ei_anc_none:
                break;
            case ei_anc_center:
                pos.x = loc.top_left.x + (loc.size.width - width) / 2;
                pos.y = loc.top_left.y + (loc.size.height - height) / 2;
                break;
            case ei_anc_north:
                pos.x = loc.top_left.x + (loc.size.width - width) / 2;
                break;
            case ei_anc_northeast:
                pos.x = loc.size.width - width;
                break;
            case ei_anc_east:
                pos.x = loc.size.width - width;
                pos.y = loc.top_left.y + (loc.size.height - height) / 2;
                break;
            case ei_anc_southeast:
                pos.x = loc.size.width - width;
                pos.y = loc.size.height - height;
                break;
            case ei_anc_south:
                pos.x = loc.top_left.x + (loc.size.width - width) / 2;
                pos.y = loc.size.height - height;
                break;
            case ei_anc_southwest:
                pos.y = loc.size.height - height;
                break;
            case ei_anc_west:
                pos.y = loc.top_left.y + (loc.size.height - height) / 2;
                break;
            case ei_anc_northwest:
            default:
                break;
        }
        ei_rect_t rect_dest = ei_rect(pos, ei_size(width, height));
        rect_dest = intersec_rect(rect_dest, *clipper);
        ei_rect_t rect_src = ei_rect(ei_point_zero(), ei_size(rect_dest.size.width, rect_dest.size.height));
        rect_src.top_left = ei_point_add(rect_src.top_left, ei_point_sub(rect_dest.top_left, pos));
        ei_copy_surface(surface, &rect_dest, surf_txt, &rect_src, EI_TRUE);
        hw_surface_unlock(surf_txt);
        hw_surface_free(surf_txt);
    }
}

void draw_img(ei_widget_t *widget, ei_surface_t surface, ei_rect_t *clipper)
{
    struct ei_frame *frame = (struct ei_frame *) widget;
    ei_rect_t loc = widget -> screen_location;
    if (frame -> img != NULL) {
        ei_surface_t surf_img = frame->img;
        hw_surface_lock(surf_img);
        ei_size_t size = frame -> img_rect != NULL ? frame -> img_rect -> size : hw_surface_get_size(surf_img);
        if (frame -> img_rect != NULL) size = frame->img_rect->size;
        ei_point_t pos = loc.top_left;
        switch (frame->img_anchor) {
            case ei_anc_none:
                break;
            case ei_anc_center:
                pos.x = loc.top_left.x + (loc.size.width - size.width) / 2;
                pos.y = loc.top_left.y + (loc.size.height - size.height) / 2;
                break;
            case ei_anc_north:
                pos.x = loc.top_left.x + (loc.size.width - size.width) / 2;
                break;
            case ei_anc_northeast:
                pos.x = loc.size.width - size.width;
                break;
            case ei_anc_east:
                pos.x = loc.size.width - size.width;
                pos.y = loc.top_left.y + (loc.size.height - size.height) / 2;
                break;
            case ei_anc_southeast:
                pos.x = loc.size.width - size.width;
                pos.y = loc.size.height - size.height;
                break;
            case ei_anc_south:
                pos.x = loc.top_left.x + (loc.size.width - size.width) / 2;
                pos.y = loc.size.height - size.height;
                break;
            case ei_anc_southwest:
                pos.y = loc.size.height - size.height;
                break;
            case ei_anc_west:
                pos.y = loc.top_left.y + (loc.size.height - size.height) / 2;
                break;
            case ei_anc_northwest:
            default:
                break;
        }
        ei_rect_t rect_dest = ei_rect(pos, ei_size(size.width, size.height));
        rect_dest = intersec_rect(rect_dest, *clipper);
        ei_rect_t rect_src = ei_rect(ei_point_zero(), ei_size(rect_dest.size.width, rect_dest.size.height));
        if (frame->img_rect != NULL) rect_src.top_left = frame->img_rect -> top_left;
        rect_src.top_left = ei_point_add(rect_src.top_left, ei_point_sub(rect_dest.top_left, pos));
        ei_copy_surface(surface, &rect_dest, surf_img, &rect_src, EI_FALSE);
        hw_surface_unlock(surf_img);
    }
}

ei_linked_point_t *ei_linked_point(ei_point_t pt, ei_linked_point_t *next) {
    ei_linked_point_t *ret = malloc(sizeof(ei_linked_point_t));
    ret -> point = pt;
    ret -> next = next;
    return ret;
}

/*
 * Frame Class
 * */

ei_widget_t* frame_allocfunc()
{
    struct ei_frame* frame = malloc(sizeof(struct ei_frame));
    return (ei_widget_t *) frame;
}
void frame_releasefunc(struct ei_widget_t*	widget)
{
    ei_frame *frame = (ei_frame *) widget;
    free(widget -> pick_color);

    if (widget -> geom_params != NULL) widget -> geom_params -> manager -> releasefunc(widget);
    if (widget -> content_rect != &(widget -> screen_location)) free(widget -> content_rect);
    if (frame -> text != NULL) free(frame -> text);
    if (frame -> img != NULL) hw_surface_free(frame -> img);
    if (frame -> img_rect != NULL) free(frame -> img_rect);
    free(frame);
}

void frame_drawfunc(struct ei_widget_t* widget,
                    ei_surface_t surface,
                    ei_surface_t pick_surface,
                    ei_rect_t* clipper)
{
    widget->geom_params->manager->runfunc(widget);
    ei_rect_t *rect_to_update = malloc(sizeof(ei_rect_t));
    *rect_to_update = widget -> screen_location;
//    ei_app_invalidate_rect(rect_to_update);
    struct ei_frame *frame = (struct ei_frame *) widget;
    ei_rect_t loc = widget -> screen_location;
    ei_point_t top_left_p = loc.top_left;

    ei_color_t light_color = frame -> color;
    ei_color_t dark_color = frame -> color;

    int h = min(loc.size.height, loc.size.width);
    int b = frame->border_width;

    switch (frame -> relief) {
        case ei_relief_none:
            break;
        case ei_relief_raised:
            light_color.red = min(255, light_color.red + color_offset);
            light_color.green = min(255, light_color.green + color_offset);
            light_color.blue = min(255, light_color.blue + color_offset);

            dark_color.red = max(0, dark_color.red - color_offset);
            dark_color.green = max(0, dark_color.green - color_offset);
            dark_color.blue = max(0, dark_color.blue - color_offset);
            break;
        case ei_relief_sunken:
            dark_color.red = min(255, dark_color.red + color_offset);
            dark_color.green = min(255, dark_color.green + color_offset);
            dark_color.blue = min(255, dark_color.blue + color_offset);

            light_color.red = max(0, light_color.red - color_offset);
            light_color.green = max(0, light_color.green - color_offset);
            light_color.blue = max(0, light_color.blue - color_offset);
            break;
    }

    ei_linked_point_t bottom_left = {ei_point_add(top_left_p, ei_point(0, loc.size.height)), NULL};
    ei_linked_point_t top_left = {top_left_p, &bottom_left};
    ei_linked_point_t top_right = {ei_point_add(top_left_p, ei_point(loc.size.width, 0)), &top_left};
    ei_linked_point_t bottom_right = {ei_point_add(top_left_p, ei_size_as_point(loc.size)), &bottom_left};

    ei_linked_point_t middle_first = {ei_point_add(bottom_left.point, ei_point(h / 2, -h / 2)), NULL};
    ei_linked_point_t middle_second = {ei_point_add(top_right.point, ei_point(-h / 2, h / 2)), &top_right};
    bottom_left.next = &middle_first;

    ei_draw_polygon(surface, &middle_second, light_color, clipper);
    if (pick_surface != NULL) ei_draw_polygon(pick_surface, &middle_second, *widget->pick_color, clipper);


    top_right.next = &bottom_right;

    ei_draw_polygon(surface, &middle_second, dark_color, clipper);
    if (pick_surface != NULL) ei_draw_polygon(pick_surface, &middle_second, *widget->pick_color, clipper);

    top_right.next = &bottom_right;

    ei_linked_point_t bottom_left_shift = {ei_point_add(bottom_left.point, ei_point(frame -> border_width, - frame -> border_width)), NULL};
    ei_linked_point_t bottom_right_shift = {ei_point_add(bottom_right.point, ei_point(- frame -> border_width, - frame -> border_width)), &bottom_left_shift};
    ei_linked_point_t top_right_shift = {ei_point_add(top_right.point, ei_point(- frame -> border_width, frame -> border_width)), &bottom_right_shift};
    ei_linked_point_t top_left_shift = {ei_point_add(top_left.point, ei_point(frame -> border_width, frame -> border_width)), &top_right_shift};

    ei_draw_polygon(surface, &top_left_shift, frame->color, clipper);
    if (pick_surface != NULL) ei_draw_polygon(pick_surface, &top_left_shift, *widget->pick_color, clipper);

    ei_rect_t intern_clipper;
    if (clipper != NULL) intern_clipper = intersec_rect(*widget -> content_rect, *clipper);

    draw_text(widget, surface, &intern_clipper);

    draw_img(widget, surface, &intern_clipper);

}
void frame_setdefaultsfunc(struct ei_widget_t*	widget)
{
    struct ei_frame *frame = (struct ei_frame *) widget;
    widget -> requested_size = ei_size_zero();
    frame -> color = ei_default_background_color;
    frame -> relief = ei_relief_none;
    frame -> border_width = 0;
    frame -> text = NULL;
    frame -> text_size = ei_font_default_size;
    frame -> text_font = ei_default_font;
    frame -> text_color = ei_font_default_color;
    frame -> text_anchor = ei_anc_center;
    frame -> img = NULL;
    frame -> img_rect = NULL;
    frame -> img_anchor = ei_anc_center;
}
void frame_geomnotifyfunc(struct ei_widget_t*	widget)
{
    widget -> content_rect = &widget -> screen_location;
}

ei_widgetclass_t frame_class = {
        "frame",
        frame_allocfunc,
        frame_releasefunc,
        frame_drawfunc,
        frame_setdefaultsfunc,
        frame_geomnotifyfunc,
        NULL
};

void ei_frame_register_class() {
    ei_widgetclass_register(&frame_class);
}

/*
 * Button Class
 * */

ei_widget_t* button_allocfunc()
{
    struct ei_button* button = malloc(sizeof(struct ei_button));
    return (ei_widget_t *) button;
}
void button_releasefunc(struct ei_widget_t*	widget)
{
    struct ei_button *button = (struct ei_button *) widget;
    free(widget -> pick_color);
    if (widget -> geom_params != NULL) widget -> geom_params -> manager -> releasefunc(widget);
    if (button -> frame.text != NULL) free(button -> frame.text);
    if (button -> frame.img != NULL) hw_surface_free(button  -> frame.img);
    if (button -> frame.img_rect != NULL) free(button  -> frame.img_rect);
    if (widget -> content_rect != &(widget -> screen_location)) free(widget -> content_rect);
    free(button);
}

ei_linked_point_t *rounded_corner(ei_point_t center, int radius, float angle_start, float angle_end, ei_linked_point_t *last) {
    float conv = (float) 3.14159265358979323846264 / 180;
    ei_linked_point_t *start = malloc(sizeof(ei_linked_point_t));
    ei_linked_point_t *current = last;
    float pas = (angle_end - angle_start) / (float) radius;
    for (float i = angle_start; angle_start > angle_end ? i > angle_end : i < angle_end; i += pas) {
        ei_point_t shift = ei_point((int) ((float) radius * cosf(i * conv)), (int) ((float) radius * sinf(i * conv)));
        ei_linked_point_t *prev = malloc(sizeof(ei_linked_point_t));
        prev -> point = ei_point_add(center, shift);
        prev -> next = current;
        current = prev;
    }
    ei_point_t shift = ei_point((int) ((float) radius * cosf(angle_end * conv)), (int) ((float) radius * sinf(angle_end * conv)));
    start -> point = ei_point_add(center, shift);
    start -> next = current;
    return start;
}

void button_drawfunc(struct ei_widget_t* widget,
                    ei_surface_t surface,
                    ei_surface_t pick_surface,
                    ei_rect_t* clipper)
{
    widget->geom_params->manager->runfunc(widget);
    ei_rect_t *rect_to_update = malloc(sizeof(ei_rect_t));
    *rect_to_update = widget -> screen_location;
//    ei_app_invalidate_rect(rect_to_update);
    struct ei_button *button = (struct ei_button *) widget;
    struct ei_frame *frame = (struct ei_frame *) widget;
    ei_rect_t loc = widget -> screen_location;
    ei_point_t top_left_p = loc.top_left;

    ei_color_t light_color = frame -> color;
    ei_color_t dark_color = frame -> color;

    int h = min(loc.size.height, loc.size.width);
    int r = button -> corner_radius;
    r = min(r, h / 2);
    int b = frame -> border_width;

    switch (frame -> relief) {
        case ei_relief_none:
            break;
        case ei_relief_raised:

            light_color.red = min(255, light_color.red + color_offset);
            light_color.green = min(255, light_color.green + color_offset);
            light_color.blue = min(255, light_color.blue + color_offset);

            dark_color.red = max(0, dark_color.red - color_offset);
            dark_color.green = max(0, dark_color.green - color_offset);
            dark_color.blue = max(0, dark_color.blue - color_offset);
            break;
        case ei_relief_sunken:
            dark_color.red = min(255, dark_color.red + color_offset);
            dark_color.green = min(255, dark_color.green + color_offset);
            dark_color.blue = min(255, dark_color.blue + color_offset);

            light_color.red = max(0, light_color.red - color_offset);
            light_color.green = max(0, light_color.green - color_offset);
            light_color.blue = max(0, light_color.blue - color_offset);
            break;
    }

    ei_linked_point_t bottom_left = {ei_point_add(top_left_p, ei_point(0, loc.size.height)), NULL};
    ei_linked_point_t top_left = {top_left_p, &bottom_left};
    ei_linked_point_t top_right = {ei_point_add(top_left_p, ei_point(loc.size.width, 0)), &top_left};
    ei_linked_point_t bottom_right = {ei_point_add(top_left_p, ei_size_as_point(loc.size)), &bottom_left};

    ei_linked_point_t *middle_first = ei_linked_point(ei_point_add(bottom_left.point, ei_point(h / 2,  - h / 2)), NULL);

    ei_linked_point_t *bottom_left_rounded = rounded_corner(ei_point_add(bottom_left.point, ei_point(r, -r)), r, 135, 180, middle_first);
    ei_linked_point_t *top_left_rounded = rounded_corner(ei_point_add(top_left.point, ei_point(r, r)), r, 180, 270, bottom_left_rounded);
    ei_linked_point_t *top_right_rounded = rounded_corner(ei_point_add(top_right.point, ei_point(-r, r)), r, 270, 315, top_left_rounded);


    ei_linked_point_t *middle_second = ei_linked_point(ei_point_add(top_right.point, ei_point(- h / 2, h / 2)), top_right_rounded);

    ei_draw_polygon(surface, middle_second, light_color, clipper);
    if (pick_surface != NULL) ei_draw_polygon(pick_surface, middle_second, *widget -> pick_color, clipper);

    ei_linked_point_t *cur = middle_second;
    while (cur != NULL) {
        ei_linked_point_t *act = cur;
        cur = cur -> next;
        free(act);
    }

    ei_linked_point_t *middle_first_2 = ei_linked_point(ei_point_add(bottom_left.point, ei_point(h / 2,  - h / 2)), NULL);

    ei_linked_point_t *bottom_left_rounded_2 = rounded_corner(ei_point_add(bottom_left.point, ei_point(r, -r)), r, 135, 90, middle_first_2);
    ei_linked_point_t *bottom_right_rounded = rounded_corner(ei_point_add(bottom_right.point, ei_point(-r, -r)), r, 90, 0, bottom_left_rounded_2);
    ei_linked_point_t *top_right_rounded_2 = rounded_corner(ei_point_add(top_right.point, ei_point(-r, r)), r, 360, 315, bottom_right_rounded);

    ei_linked_point_t *middle_second_2 = ei_linked_point(ei_point_add(top_right.point, ei_point(- h / 2, h / 2)), top_right_rounded_2);

    ei_draw_polygon(surface, middle_second_2, dark_color, clipper);
    if (pick_surface != NULL) ei_draw_polygon(pick_surface, middle_second_2, *widget -> pick_color, clipper);

    cur = middle_second_2;
    while (cur != NULL) {
        ei_linked_point_t *act = cur;
        cur = cur -> next;
        free(act);
    }

    ei_linked_point_t *bottom_left_start = ei_linked_point(ei_point_add(bottom_left.point, ei_point(b, -r - b)), NULL);

    ei_linked_point_t *bottom_left_shift = rounded_corner(ei_point_add(bottom_left.point, ei_point(r + b, -r - b)), r, 180, 90, bottom_left_start);
    ei_linked_point_t *bottom_right_shift = rounded_corner(ei_point_add(bottom_right.point, ei_point(-r - b, -r - b)), r, 90, 0, bottom_left_shift);
    ei_linked_point_t *top_right_shift = rounded_corner(ei_point_add(top_right.point, ei_point(-r - b, r + b)), r, 360, 270, bottom_right_shift);
    ei_linked_point_t *top_left_shift = rounded_corner(ei_point_add(top_left.point, ei_point(r + b, r + b)), r, 270, 180, top_right_shift);

    ei_draw_polygon(surface, top_left_shift, frame -> color, clipper);
    if (pick_surface != NULL) ei_draw_polygon(pick_surface, top_left_shift, *widget -> pick_color, clipper);

    cur = top_left_shift;
    while (cur != NULL) {
        ei_linked_point_t *act = cur;
        cur = cur -> next;
        free(act);
    }

    ei_rect_t intern_clipper;
    if (clipper != NULL) intern_clipper = intersec_rect(*widget -> content_rect, *clipper);

    draw_text(widget, surface, &intern_clipper);

    draw_img(widget, surface, &intern_clipper);

}

void button_setdefaultsfunc(struct ei_widget_t*	widget)
{
    frame_setdefaultsfunc(widget);
    struct ei_button *button = (struct ei_button *) widget;
    button -> frame.border_width = k_default_button_border_width;
    button -> frame.relief = ei_relief_raised;
    button -> corner_radius = k_default_button_corner_radius;
    button -> callback = NULL;
    button -> user_param = NULL;
}
void button_geomnotifyfunc(struct ei_widget_t*	widget)
{
    widget -> content_rect = &widget -> screen_location;
}

ei_widgetclass_t button_class = {
        "button",
        button_allocfunc,
        button_releasefunc,
        button_drawfunc,
        button_setdefaultsfunc,
        button_geomnotifyfunc,
        NULL
};

void ei_button_register_class() {
    ei_widgetclass_register(&button_class);
}

/*
 * Top_level Class
 * */

ei_color_t grey = {0x60, 0x60, 0x60, 0xff};
ei_color_t white = {255, 255, 255, 0xff};
ei_color_t red = {240, 60, 20, 0xff};
ei_color_t lightred = {255, 100, 60, 0xff};
ei_color_t darkred = {240, 20, 0, 0xff};

ei_widget_t* toplevel_allocfunc()
{
    struct ei_toplevel* toplevel = malloc(sizeof(struct ei_toplevel));
    return (ei_widget_t *) toplevel;
}
void toplevel_releasefunc(struct ei_widget_t*	widget)
{
    free(widget -> pick_color);
    if (widget -> geom_params != NULL) widget -> geom_params -> manager -> releasefunc(widget);
    if (widget -> content_rect != &(widget -> screen_location)) free(widget -> content_rect);
    free((ei_toplevel *) widget);
}

void toplevel_drawfunc(struct ei_widget_t *widget,
                              ei_surface_t surface,
                              ei_surface_t pick_surface,
                              ei_rect_t   *clipper)
{
    widget->geom_params->manager->runfunc(widget);
    ei_toplevel *toplevel = (struct ei_toplevel *) widget;
    ei_rect_t loc = widget -> screen_location;
    ei_point_t top_left_p = loc.top_left;

    int r = 2;
    int b = toplevel -> border_width;

    int width  = 0, height = ei_top_level_height();
    if (toplevel -> title != NULL) {
        hw_text_compute_size(toplevel->title, toplevel->title_font, &width, &height);
    }
    height = ei_top_level_height();

    ei_linked_point_t bottom_left = {ei_point_add(top_left_p, ei_point(0, loc.size.height)), NULL};
    ei_linked_point_t top_left = {top_left_p, &bottom_left};
    ei_linked_point_t top_right = {ei_point_add(top_left_p, ei_point(loc.size.width, 0)), &top_left};
    ei_linked_point_t bottom_right = {ei_point_add(top_left_p, ei_point(loc.size.width, loc.size.height)), &bottom_left};

    ei_linked_point_t *bottom_left_linked = ei_linked_point(bottom_left.point, NULL);

    ei_linked_point_t *top_left_rounded = rounded_corner(ei_point_add(top_left.point, ei_point(r, r)), r, 180, 270, bottom_left_linked);
    ei_linked_point_t *top_right_rounded = rounded_corner(ei_point_add(top_right.point, ei_point(-r, r)), r, 270, 360, top_left_rounded);
    ei_linked_point_t *bottom_right_linked = ei_linked_point(bottom_right.point, top_right_rounded);

    ei_linked_point_t *bottom_left_linked_2 = ei_linked_point(bottom_left.point, bottom_right_linked);
    ei_linked_point_t *bottom_left_shift = ei_linked_point(ei_point_add(bottom_left.point, ei_point(b, -b)), bottom_left_linked_2);
    ei_linked_point_t *bottom_right_shift;
    if (toplevel -> resizable != ei_axis_none) {
        ei_linked_point_t *bottom_right_res = ei_linked_point(ei_point_add(bottom_right.point, ei_point(min(-b,-ei_top_level_res_button()), -b)), bottom_left_shift);
        ei_linked_point_t *bottom_right_res_2 = ei_linked_point(ei_point_add(bottom_right.point, ei_point(min(-b,-ei_top_level_res_button()), min(-b,-ei_top_level_res_button()))), bottom_right_res);
        bottom_right_shift = ei_linked_point(ei_point_add(bottom_right.point, ei_point(-b, min(-b,-ei_top_level_res_button()))), bottom_right_res_2);
    } else {
        bottom_right_shift = ei_linked_point(ei_point_add(bottom_right.point, ei_point(-b, -b)), bottom_left_shift);
    }
    ei_linked_point_t *top_right_shift = ei_linked_point(ei_point_add(top_right.point, ei_point(-b, height + b)), bottom_right_shift);
    ei_linked_point_t *top_left_shift = ei_linked_point(ei_point_add(top_left.point, ei_point(b, height + b)), top_right_shift);
    ei_linked_point_t *bottom_left_shift_2 = ei_linked_point(ei_point_add(bottom_left.point, ei_point(b, -b)), top_left_shift);

    ei_color_t grey_alpha = grey;
    grey_alpha.alpha = toplevel -> color.alpha;
    ei_draw_polygon(surface, bottom_left_shift_2, grey_alpha, clipper);
    if (pick_surface != NULL) ei_draw_polygon(pick_surface, bottom_left_shift_2, *widget -> pick_color, clipper);

    ei_linked_point_t *cur = bottom_left_shift_2;
    while (cur != NULL) {
        ei_linked_point_t *act = cur;
        cur = cur -> next;
        free(act);
    }

    if (toplevel -> title != NULL) {
        ei_surface_t surf_txt = hw_text_create_surface(toplevel->title, toplevel->title_font, white);
        hw_surface_lock(surf_txt);
        ei_point_t dest_top_left = ei_point_add(loc.top_left, ei_point(r, b));
        ei_rect_t rect_dest = ei_rect(dest_top_left, ei_size(width, height));
        rect_dest = intersec_rect(rect_dest, intersec_rect(widget -> screen_location, *clipper));
        ei_rect_t rect_src = ei_rect(ei_point_sub(rect_dest.top_left, dest_top_left), ei_size(rect_dest.size.width, rect_dest.size.height));
        ei_copy_surface(surface, &rect_dest, surf_txt, &rect_src, EI_TRUE);
        hw_surface_unlock(surf_txt);
        hw_surface_free(surf_txt);
    }

    ei_linked_point_t top_left_core = {ei_point_add(top_left.point, ei_point(b, height + b)), NULL};
    ei_linked_point_t top_right_core = {ei_point_add(top_right.point, ei_point(-b, height + b)), &top_left_core};
    ei_linked_point_t bottom_right_core;
    ei_linked_point_t bottom_right_core_res = {ei_point_add(bottom_right.point, ei_point(-b, min(-b, -ei_top_level_res_button()))), &top_right_core};
    ei_linked_point_t bottom_right_core_res_2 = {ei_point_add(bottom_right.point, ei_point(min(-b, -ei_top_level_res_button()), min(-b, -ei_top_level_res_button()))), &bottom_right_core_res};
    if (toplevel -> resizable != ei_axis_none) {
        bottom_right_core.point = ei_point_add(bottom_right.point, ei_point(min(-b, -ei_top_level_res_button()), -b));
        bottom_right_core.next = &bottom_right_core_res_2;
    } else {
        bottom_right_core.point = ei_point_add(bottom_right.point, ei_point(-b, -b));
        bottom_right_core.next = &top_right_core;
    }
    ei_linked_point_t bottom_left_core = {ei_point_add(bottom_left.point, ei_point(b, -b)), &bottom_right_core};

    ei_draw_polygon(surface, &bottom_left_core, toplevel -> color, clipper);
    if (pick_surface != NULL) ei_draw_polygon(pick_surface, &bottom_left_core, *widget -> pick_color, clipper);

    if (toplevel -> closable == EI_TRUE) {
        int rad = ei_top_level_height() / 2;
        ei_point_t center = ei_point_add(top_right.point, ei_point(-b - rad - 1, b + ei_top_level_height() / 2 + 1));
        ei_color_t li_red = lightred;
        ei_color_t dk_red = darkred;
        if (toplevel -> close_relief == ei_relief_sunken) {
            ei_color_t tmp = li_red;
            li_red = dk_red;
            dk_red = tmp;
        }
        ei_linked_point_t *up_close = rounded_corner(center, (ei_top_level_height() / 2) * 2/3 + 1, 135, 315, NULL);
        ei_draw_polygon(surface, up_close, li_red, clipper);
        cur = up_close;
        while (cur != NULL) {
            ei_linked_point_t *act = cur;
            cur = cur -> next;
            free(act);
        }
        ei_linked_point_t *dwn_close = rounded_corner(center, (ei_top_level_height() / 2) * 2/3 + 1, 135, -45, NULL);
        ei_draw_polygon(surface, dwn_close, dk_red, clipper);
        cur = dwn_close;
        while (cur != NULL) {
            ei_linked_point_t *act = cur;
            cur = cur -> next;
            free(act);
        }
        ei_linked_point_t *close = rounded_corner(center, (ei_top_level_height() / 2) * 2/3 - 1, 0, 360, NULL);
        ei_draw_polygon(surface, close, red, clipper);
        cur = close;
        while (cur != NULL) {
            ei_linked_point_t *act = cur;
            cur = cur -> next;
            free(act);
        }

        ei_linked_point_t top_left_cross = {ei_point_add(center, ei_point(-3, -3)), NULL};
        ei_linked_point_t bottom_right_cross = {ei_point_add(center, ei_point(3, 3)), &top_left_cross};
        ei_draw_polyline(surface, &bottom_right_cross, white, clipper);
        ei_linked_point_t top_right_cross = {ei_point_add(center, ei_point(3, -3)), NULL};
        ei_linked_point_t bottom_left_cross = {ei_point_add(center, ei_point(-3, 3)), &top_right_cross};
        ei_draw_polyline(surface, &bottom_left_cross, white, clipper);
    }

}

void toplevel_setdefaultsfunc(struct ei_widget_t*	widget)
{
    struct ei_toplevel *toplevel = (struct ei_toplevel *) widget;
    widget -> requested_size = ei_size(320, 240);
    toplevel -> color = ei_default_background_color;
    toplevel -> border_width = 4;
    toplevel -> title = NULL;
    toplevel -> title_font = ei_default_font;
    toplevel -> closable = EI_TRUE;
    toplevel -> close_relief = ei_relief_raised;
    toplevel -> resizable = ei_axis_both;
    toplevel -> min_size = ei_size(160, 120);
}
void toplevel_geomnotifyfunc(struct ei_widget_t*	widget)
{
    ei_toplevel *top = (ei_toplevel *) widget;
    ei_rect_t *content = malloc(sizeof(ei_rect_t));
    widget -> screen_location.size.height += ei_top_level_height();
    content -> top_left = ei_point_add(widget -> screen_location.top_left, ei_point(top -> border_width, top -> border_width + ei_top_level_height()));
    content -> size = ei_size_add(widget -> screen_location.size, ei_size(-2 * top -> border_width, -2 * top -> border_width - ei_top_level_height()));
    widget -> content_rect = content;
}

ei_widgetclass_t toplevel_class = {
        "toplevel",
        toplevel_allocfunc,
        toplevel_releasefunc,
        toplevel_drawfunc,
        toplevel_setdefaultsfunc,
        toplevel_geomnotifyfunc,
        NULL
};

void ei_toplevel_register_class() {
    ei_widgetclass_register(&toplevel_class);
}