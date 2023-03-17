#include "ei_geometrymanager.h"
#include "ei_placer.h"
#include "ei_types.h"
#include "ei_widget.h"
#include "ei_application.h"
#include "ei_utils.h"

ei_geometrymanager_t *ei_geom = NULL;

void ei_geometrymanager_register(ei_geometrymanager_t* geometrymanager) {
    if (geometrymanager == NULL) {
        ei_geom = geometrymanager;
    } else {
        geometrymanager -> next = ei_geom;
        ei_geom = geometrymanager;
    }
}

ei_geometrymanager_t*	ei_geometrymanager_from_name	(ei_geometrymanager_name_t name) {
    ei_geometrymanager_t *geometrymanager = ei_geom;
    while (geometrymanager != NULL) {
        if (strcmp(geometrymanager -> name, name) == 0) {
            return geometrymanager;
        }
        geometrymanager = geometrymanager -> next;
    }
    return NULL;
}

void ei_geometrymanager_unmap 	( 	ei_widget_t *  	widget	)
{
    widget -> geom_params -> manager -> releasefunc(widget);
    ei_app_invalidate_rect(&widget -> screen_location);
    free(widget -> geom_params);
    widget -> geom_params = NULL;
    widget -> screen_location = ei_rect_zero();
}

void placer_runfunc(struct ei_widget_t*	widget) {
    ei_app_invalidate_rect(&widget -> screen_location);
    ei_point_t pos;
    if (widget -> parent != NULL) {
        pos = widget -> parent -> content_rect -> top_left;
    } else {
        pos = ei_point_zero();
    }
    ei_size_t size = widget -> requested_size;
    ei_placer *geom = (ei_placer *) widget -> geom_params;
    pos.x += geom -> x;
    pos.y += geom -> y;
    size.width = geom -> width;
    size.height = geom -> height;
    if (widget -> parent != NULL) {
        pos.x += (int) (geom->rel_x * (float) (widget->parent -> content_rect -> size.width));
        pos.y += (int) (geom->rel_y * (float) (widget->parent -> content_rect -> size.height));
        size.width += (int) (geom -> rel_width * (float) (widget -> parent -> content_rect -> size.width));
        size.height += (int) (geom -> rel_height * (float) (widget -> parent -> content_rect -> size.height));
    }
    switch (geom -> anchor) {
        case ei_anc_none:
            break;
        case ei_anc_center:
            pos.x -= size.width / 2;
            pos.y -= size.height / 2;
            break;
        case ei_anc_north:
            pos.x -= size.width / 2;
            break;
        case ei_anc_northeast:
            pos.x -= size.width;
            break;
        case ei_anc_east:
            pos.x -= size.width;
            pos.y -= size.height / 2;
            break;
        case ei_anc_southeast:
            pos.x -= size.width;
            pos.y -= size.height;;
            break;
        case ei_anc_south:
            pos.x -= size.width / 2;
            pos.y -= size.height;
            break;
        case ei_anc_southwest:
            pos.y -= size.height;
            break;
        case ei_anc_west:
            pos.y -= size.height / 2;
            break;
        case ei_anc_northwest:
        default:
            break;
    }
    widget -> screen_location = ei_rect(pos, size);
    widget -> wclass -> geomnotifyfunc(widget);
    ei_app_invalidate_rect(&widget -> screen_location);
}

void placer_releasefunc(struct ei_widget_t*	widget)
{
    ei_geometry_param_t *geoparam = widget -> geom_params;
    widget -> geom_params = NULL;
    free(geoparam);
}

ei_geometrymanager_t placer_class = {
        "placer",
        placer_runfunc,
        placer_releasefunc,
        NULL
};

void ei_register_placer_manager() {
    ei_geometrymanager_register(&placer_class);
}

void ei_place (ei_widget_t *widget,
               ei_anchor_t *anchor,
               int         *x,         int         *y,
               int         *width,     int         *height,
               float       *rel_x,     float       *rel_y,
               float       *rel_width, float       *rel_height)
{
    ei_geometry_param_t *geom = widget -> geom_params;
    if ((geom != NULL) && (strcmp(geom -> manager -> name, "placer") == 0)) {
        ei_placer *placer = (ei_placer *) geom;
        if (anchor != NULL) placer -> anchor = *anchor;
        if (x != NULL) placer -> x = *x;
        if (y != NULL) placer -> y = *y;
        if (width != NULL) placer -> width = *width;
        if (height != NULL) placer -> height = *height;
        if (rel_x != NULL) placer -> rel_x = *rel_x;
        if (rel_y != NULL) placer -> rel_y = *rel_y;
        if (rel_width != NULL) placer -> rel_width = *rel_width;
        if (rel_height != NULL) placer -> rel_height = *rel_height;
    } else {
        ei_placer *placer = malloc(sizeof(ei_placer));
        widget->geom_params = (ei_geometry_param_t *) placer;
        placer->manager = ei_geometrymanager_from_name("placer");
        placer->anchor = anchor != NULL ? *anchor : ei_anc_northwest;
        placer->x = x != NULL ? *x : 0;
        placer->y = y != NULL ? *y : 0;
        placer->rel_x = rel_x != NULL ? *rel_x : 0;
        placer->rel_y = rel_y != NULL ? *rel_y : 0;
        placer->width = width != NULL ? *width : (rel_width != NULL ? 0 : widget->requested_size.width);
        placer->height = height != NULL ? *height : (rel_height != NULL ? 0 : widget->requested_size.height);
        placer->rel_width = rel_width != NULL ? *rel_width : 0;
        placer->rel_height = rel_height != NULL ? *rel_height : 0;
    }
//    widget -> geom_params -> manager -> runfunc(widget);
}
