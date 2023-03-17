#include "ei_types.h"
#include "ei_widget.h"
#include "ei_widgetclass.h"
#include "hw_interface.h"
#include "ei_event.h"
#include "ei_geometrymanager.h"
#include "ei_struct_for_bind.h"
#include "ei_button.h"
#include "ei_utilsfct.h"
#include "ei_toplevel.h"

ei_widget_t *root_widget;
ei_surface_t *root_surface;
ei_bool_t running = EI_TRUE;
ei_point_t *last_position = NULL;

ei_linked_rect_t *update_rect = NULL;

void ei_app_invalidate_rect(ei_rect_t *rect1);

/* -------------------------------------------------------------------------- */
void put_children_at_last_place(ei_widget_t *widget, ei_widget_t *parent)
{
    if (widget == parent -> children_tail) return;
    ei_widget_t *cellule_courante = NULL;
    ei_widget_t *cellule_suivante = parent -> children_head;

    while (cellule_suivante != widget)
    {
        cellule_courante = cellule_suivante;
        cellule_suivante = cellule_suivante -> next_sibling;
    }

    if (cellule_courante == NULL)
    {
        parent -> children_head = cellule_suivante -> next_sibling;
        parent -> children_tail -> next_sibling = cellule_suivante;
        parent -> children_tail = cellule_suivante;
        cellule_suivante -> next_sibling = NULL;
    }
    else
    {
        cellule_courante -> next_sibling = cellule_suivante -> next_sibling;
        parent -> children_tail -> next_sibling = cellule_suivante;
        parent -> children_tail = cellule_suivante;
        cellule_suivante -> next_sibling = NULL;
    }
}

/* -------------------------------------------------------------------------- */
static
void draw_rec(ei_widget_t *widget, ei_surface_t *surface, ei_surface_t *pick_surface, ei_rect_t *clipper) {
    if (widget == NULL) return;
    if (widget -> geom_params != NULL) {
        widget->wclass->drawfunc(widget, surface, pick_surface, clipper);
    }
    ei_rect_t child_clipper = *widget->content_rect;
    if (clipper != NULL) child_clipper = intersec_rect(child_clipper, *clipper);
    if (widget -> children_head != NULL) draw_rec(widget -> children_head, surface, pick_surface, &child_clipper);
    if (widget -> next_sibling != NULL) draw_rec(widget -> next_sibling, surface, pick_surface, clipper);
}

/* -------------------------------------------------------------------------- */
static
void draw(ei_widget_t *widget, ei_surface_t *surface, ei_surface_t *pick_surface, ei_rect_t *clipper) {
    hw_surface_lock(surface);
    hw_surface_lock(pick_surface);
    draw_rec(widget, surface, pick_surface, clipper);
    hw_surface_unlock(surface);
    hw_surface_unlock(pick_surface);
    hw_surface_update_rects(surface, update_rect);
    ei_linked_rect_t *cur = update_rect;
    while (cur != NULL) {
        ei_linked_rect_t *act = cur;
        cur = cur -> next;
        free(act);
    }
    update_rect = malloc(sizeof(ei_linked_rect_t));
    update_rect -> rect = ei_rect_zero();
    update_rect -> next = NULL;
}

/* -------------------------------------------------------------------------- */

void assign_last_pos_to(ei_point_t pos) {
    if (last_position == NULL) {
        last_position = malloc(sizeof(ei_point_t));
    }
    last_position -> x = pos.x;
    last_position -> y = pos.y;
}

/* -------------------------------------------------------------------------- */
ei_widget_t* widget_pick(ei_surface_t pick_surface, ei_point_t where)
{
    ei_color_t pointed_color = ei_get_pointed_pick_surface_color(pick_surface, where);
    int pick_id = int_from_pick_color(pointed_color);
    ei_widget_t *widget_pointed_at = recursif_seek_widget_with_pickid(root_widget, pick_id);
    return widget_pointed_at;
}

/* -------------------------------------------------------------------------- */
ei_bool_t is_in_widget(ei_widget_t *widget, ei_point_t where) {
    if (widget == widget_pick(pick_surface, where)) return EI_TRUE;
    return EI_FALSE;
}

/* -------------------------------------------------------------------------- */

ei_bool_t button_drag(ei_widget_t *widget, ei_event_t *event, void *user_param) {
    ei_frame *frame = (ei_frame *) widget;
    if (is_in_widget(widget, event -> param.mouse.where) == EI_TRUE) {
        frame -> relief = ei_relief_sunken;
    } else {
        frame -> relief = ei_relief_raised;
    }
    return EI_FALSE;
}

ei_bool_t button_release(ei_widget_t *widget, ei_event_t *event, void *user_param) {
    if (event -> param.mouse.button == ei_mouse_button_left) {
        ei_frame *frame = (ei_frame *) widget;
        frame->relief = ei_relief_raised;
        if (is_in_widget(widget, event->param.mouse.where) == EI_TRUE) {
            ei_button *button = (ei_button *) widget;
            button -> callback(widget, event, button -> user_param);
        }
        ei_unbind(ei_ev_mouse_move, widget, NULL, button_drag, NULL);
        ei_unbind(ei_ev_mouse_buttonup, widget, NULL, button_release, NULL);
    }
    return EI_FALSE;
}

ei_bool_t ei_button_press(ei_widget_t *widget, ei_event_t *event, void *user_param) {
    if (event -> param.mouse.button == ei_mouse_button_left) {
        ei_frame *frame = (ei_frame *) widget;
        frame->relief = ei_relief_sunken;
        assign_last_pos_to(event -> param.mouse.where);
        ei_bind(ei_ev_mouse_move, widget, NULL, button_drag, NULL);
        ei_bind(ei_ev_mouse_buttonup, widget, NULL, button_release, NULL);
    }
    return EI_FALSE;
}

ei_bool_t top_drag(ei_widget_t *widget, ei_event_t *event, void *user_param) {
    int x = widget -> screen_location.top_left.x + (event -> param.mouse.where.x - last_position -> x);
    int y = widget -> screen_location.top_left.y + (event -> param.mouse.where.y - last_position -> y);
    if (widget -> parent != NULL) {
        x -= widget -> parent -> content_rect -> top_left.x;
        y -= widget -> parent -> content_rect -> top_left.y;
    }
    ei_rect_t rect1 = widget -> screen_location;
    ei_rect_t rect2 = widget -> screen_location;
    rect2.top_left.x = x;
    rect2.top_left.y = y;
    ei_rect_t rect_unified = union_rect(rect1, rect2);
    ei_anchor_t new_anchor = ei_anc_northwest;
    ei_place(widget, &new_anchor, &x, &y, NULL, NULL, NULL, NULL, NULL, NULL);
    assign_last_pos_to(event -> param.mouse.where);
    return EI_FALSE;
}

ei_bool_t top_resize(ei_widget_t *widget, ei_event_t *event, void *user_param)
{
    ei_toplevel *toplevel = (struct ei_toplevel *) widget;
    ei_anchor_t anchor = (ei_anchor_t) user_param;

    int delta_x, delta_y;
    ei_rect_t loc = widget -> screen_location;
    loc.size.height -= ei_top_level_height();
    int new_x, new_y, new_width, new_height;

    new_x = loc.top_left.x;
    new_y = loc.top_left.y;
    new_width = loc.size.width;
    new_height = loc.size.height;
    float zero = 0;

    ei_anchor_t new_anchor = ei_anc_northwest;
    ei_place(widget, &new_anchor, &new_x, &new_y, &new_width, &new_height, &zero, &zero, &zero, &zero);

    switch (anchor) {
        case ei_anc_northwest:
            delta_x = min(event -> param.mouse.where.x, loc.top_left.x + loc.size.width - toplevel -> min_size.width)
                    - min(last_position -> x, loc.top_left.x + loc.size.width - toplevel -> min_size.width);
            delta_y = min(event -> param.mouse.where.y, loc.top_left.y + loc.size.height - toplevel -> min_size.height)
                    - min(last_position -> y, loc.top_left.y + loc.size.height - toplevel -> min_size.height);
            new_x = min(loc.top_left.x + delta_x, loc.top_left.x + loc.size.width - toplevel -> min_size.width);
            new_y = min(loc.top_left.y + delta_y, loc.top_left.y + loc.size.height - toplevel -> min_size.height);
            if (widget -> parent != NULL) {
                new_x -= widget -> parent -> content_rect -> top_left.x;
                new_y -= widget -> parent -> content_rect -> top_left.y;
            }
            new_width = max(loc.size.width - delta_x, toplevel -> min_size.width);
            new_height = max(loc.size.height - delta_y, toplevel -> min_size.height);
            break;
        case ei_anc_northeast:
            delta_x = max(event -> param.mouse.where.x, loc.top_left.x + toplevel -> min_size.width)
                      - max(last_position -> x, loc.top_left.x + toplevel -> min_size.width);
            delta_y = min(event -> param.mouse.where.y, loc.top_left.y + loc.size.height - toplevel -> min_size.height)
                      - min(last_position -> y, loc.top_left.y + loc.size.height - toplevel -> min_size.height);
            new_x = loc.top_left.x;
            new_y = min(loc.top_left.y + delta_y, loc.top_left.y + loc.size.height - toplevel -> min_size.height);
            if (widget -> parent != NULL) {
                new_x -= widget -> parent -> content_rect -> top_left.x;
                new_y -= widget -> parent -> content_rect -> top_left.y;
            }
            new_width = max(loc.size.width + delta_x, toplevel -> min_size.width);
            new_height = max(loc.size.height - delta_y, toplevel -> min_size.height);
            break;
        case ei_anc_southwest:
            delta_x = min(event -> param.mouse.where.x, loc.top_left.x + loc.size.width - toplevel -> min_size.width)
                      - min(last_position -> x, loc.top_left.x + loc.size.width - toplevel -> min_size.width);
            delta_y = max(event -> param.mouse.where.y, loc.top_left.y + toplevel -> min_size.height)
                      - max(last_position -> y, loc.top_left.y + toplevel -> min_size.height);
            new_x = min(loc.top_left.x + delta_x, loc.top_left.x + loc.size.width - toplevel -> min_size.width);
            new_y = loc.top_left.y;
            if (widget -> parent != NULL) {
                new_x -= widget -> parent -> content_rect -> top_left.x;
                new_y -= widget -> parent -> content_rect -> top_left.y;
            }
            new_width = max(loc.size.width - delta_x, toplevel -> min_size.width);
            new_height = max(loc.size.height + delta_y, toplevel -> min_size.height);
            break;
        case ei_anc_southeast:
            delta_x = max(event -> param.mouse.where.x, loc.top_left.x + toplevel -> min_size.width)
                      - max(last_position -> x, loc.top_left.x + toplevel -> min_size.width);
            delta_y = max(event -> param.mouse.where.y, loc.top_left.y + toplevel -> min_size.height)
                      - max(last_position -> y, loc.top_left.y + toplevel -> min_size.height);
            new_x = loc.top_left.x;
            new_y = loc.top_left.y;
            if (widget -> parent != NULL) {
                new_x -= widget -> parent -> content_rect -> top_left.x;
                new_y -= widget -> parent -> content_rect -> top_left.y;
            }
            new_width = max(loc.size.width + delta_x, toplevel -> min_size.width);
            new_height = max(loc.size.height + delta_y, toplevel -> min_size.height);
            break;
        case ei_anc_north:
            delta_x = 0;
            delta_y = min(event -> param.mouse.where.y, loc.top_left.y + loc.size.height - toplevel -> min_size.height)
                      - min(last_position -> y, loc.top_left.y + loc.size.height - toplevel -> min_size.height);
            new_x = loc.top_left.x;
            new_y = min(loc.top_left.y + delta_y, loc.top_left.y + loc.size.height - toplevel -> min_size.height);
            if (widget -> parent != NULL) {
                new_x -= widget -> parent -> content_rect -> top_left.x;
                new_y -= widget -> parent -> content_rect -> top_left.y;
            }
            new_width = loc.size.width;
            new_height = max(loc.size.height - delta_y, toplevel -> min_size.height);
            break;
        case ei_anc_south:
            delta_x = 0;
            delta_y = max(event -> param.mouse.where.y, loc.top_left.y + toplevel -> min_size.height)
                      - max(last_position -> y, loc.top_left.y + toplevel -> min_size.height);
            new_x = loc.top_left.x;
            new_y = loc.top_left.y;
            if (widget -> parent != NULL) {
                new_x -= widget -> parent -> content_rect -> top_left.x;
                new_y -= widget -> parent -> content_rect -> top_left.y;
            }
            new_width = loc.size.width;
            new_height = max(loc.size.height + delta_y, toplevel -> min_size.height);
            break;
        case ei_anc_east:
            delta_x = max(event -> param.mouse.where.x, loc.top_left.x + toplevel -> min_size.width)
                      - max(last_position -> x, loc.top_left.x + toplevel -> min_size.width);
            delta_y = 0;
            new_x = loc.top_left.x;
            new_y = loc.top_left.y;
            if (widget -> parent != NULL) {
                new_x -= widget -> parent -> content_rect -> top_left.x;
                new_y -= widget -> parent -> content_rect -> top_left.y;
            }
            new_width = max(loc.size.width + delta_x, toplevel -> min_size.width);
            new_height = loc.size.height;;
            break;
        case ei_anc_west:
            delta_x = min(event -> param.mouse.where.x, loc.top_left.x + loc.size.width - toplevel -> min_size.width)
                      - min(last_position -> x, loc.top_left.x + loc.size.width - toplevel -> min_size.width);
            delta_y = 0;
            new_x = min(loc.top_left.x + delta_x, loc.top_left.x + loc.size.width - toplevel -> min_size.width);
            new_y = loc.top_left.y;
            if (widget -> parent != NULL) {
                new_x -= widget -> parent -> content_rect -> top_left.x;
                new_y -= widget -> parent -> content_rect -> top_left.y;
            }
            new_width = max(loc.size.width - delta_x, toplevel -> min_size.width);
            new_height = loc.size.height;
            break;
        default:
            break;
    }

    ei_place(widget, NULL, &new_x, &new_y, &new_width, &new_height, NULL, NULL, NULL, NULL);
    assign_last_pos_to(event -> param.mouse.where);
    return EI_FALSE;
}

ei_bool_t top_close_move(ei_widget_t *widget, ei_event_t *event, void *user_param) {
    ei_toplevel *toplevel = (ei_toplevel *) widget;
    ei_rect_t loc = widget -> screen_location;
    int b = toplevel -> border_width;
    int rad = 1 + ei_top_level_height() / 2;
    if (dist_sq(ei_point_add(loc.top_left, ei_point(loc.size.width - rad - b, rad + b)), event->param.mouse.where) <= (rad * 2 / 3) * (rad * 2 / 3)) {
        toplevel -> close_relief = ei_relief_sunken;
        ei_toplevel_configure(widget, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    } else {
        toplevel -> close_relief = ei_relief_raised;
        ei_toplevel_configure(widget, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    }
    return EI_FALSE;
}

ei_bool_t top_release(ei_widget_t *widget, ei_event_t *event, void *user_param) {
    ei_rect_t loc = widget -> screen_location;
    ei_toplevel *toplevel = (ei_toplevel *) widget;
    if (event -> param.mouse.button == ei_mouse_button_left) {
        ei_unbind(ei_ev_mouse_move, widget, NULL, top_drag, NULL);
        ei_unbind(ei_ev_mouse_move, widget, NULL, top_resize, user_param);
        ei_unbind(ei_ev_mouse_buttonup, widget, NULL, top_release, user_param);
        ei_unbind(ei_ev_mouse_move, widget, NULL, top_close_move, NULL);
        int rad = 1 + ei_top_level_height() / 2;
        int b = toplevel -> border_width;
        if (dist_sq(ei_point_add(loc.top_left, ei_point(loc.size.width - rad - b, rad + b)), event->param.mouse.where) <= (rad * 2 / 3) * (rad * 2 / 3)) {
            ei_widget_destroy(widget);
        }
    }
    return EI_FALSE;
}

ei_bool_t top_press(ei_widget_t *widget, ei_event_t *event, void *user_param) {
    ei_widget_t *parent = widget -> parent;
    put_children_at_last_place(widget, parent);
    ei_toplevel *toplevel = (struct ei_toplevel *) widget;
    int b = toplevel -> border_width;
    ei_rect_t loc = widget -> screen_location;
    int rad = 1 + ei_top_level_height() / 2;
    if (event -> param.mouse.button == ei_mouse_button_left)
    {
        assign_last_pos_to(event -> param.mouse.where);

        if (toplevel -> closable == EI_TRUE &&
            dist_sq(ei_point_add(loc.top_left, ei_point(loc.size.width - rad - b, rad + b)), event->param.mouse.where) <= (rad * 2 / 3) * (rad * 2 / 3))
        {
            toplevel -> close_relief = ei_relief_sunken;
            ei_toplevel_configure(widget, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
            ei_bind(ei_ev_mouse_move, widget, NULL, top_close_move, NULL);
            ei_bind(ei_ev_mouse_buttonup, widget, NULL, top_release, NULL);
        }
        else if (toplevel -> resizable == ei_axis_both &&
                 event -> param.mouse.where.x >= loc.top_left.x + loc.size.width + min(-b, -ei_top_level_res_button()) &&
                 event -> param.mouse.where.y >= loc.top_left.y + loc.size.height + min(-b, -ei_top_level_res_button()))
        {
            ei_bind(ei_ev_mouse_move, widget, NULL, top_resize, (void *) ei_anc_southeast);
            ei_bind(ei_ev_mouse_buttonup, widget, NULL, top_release, (void *) ei_anc_southeast);
        }
        else if (toplevel -> resizable == ei_axis_both &&
                 event -> param.mouse.where.x <= loc.top_left.x + b &&
                 event -> param.mouse.where.y >= loc.top_left.y + loc.size.height - b)
        {
            ei_bind(ei_ev_mouse_move, widget, NULL, top_resize, (void *) ei_anc_southwest);
            ei_bind(ei_ev_mouse_buttonup, widget, NULL, top_release, (void *) ei_anc_southwest);
        }
        else if (toplevel -> resizable == ei_axis_both &&
                 event -> param.mouse.where.x <= loc.top_left.x + b &&
                 event -> param.mouse.where.y <= loc.top_left.y + b)
        {
            ei_bind(ei_ev_mouse_move, widget, NULL, top_resize, (void *) ei_anc_northwest);
            ei_bind(ei_ev_mouse_buttonup, widget, NULL, top_release, (void *) ei_anc_northwest);
        }
        else if (toplevel -> resizable == ei_axis_both &&
                 event -> param.mouse.where.x >= loc.top_left.x + loc.size.width - b &&
                 event -> param.mouse.where.y <= loc.top_left.y + b)
        {
            ei_bind(ei_ev_mouse_move, widget, NULL, top_resize, (void *) ei_anc_northeast);
            ei_bind(ei_ev_mouse_buttonup, widget, NULL, top_release, (void *) ei_anc_northeast);
        }
        else if ((toplevel -> resizable == ei_axis_both || toplevel -> resizable == ei_axis_x) &&
                 event -> param.mouse.where.x >= loc.top_left.x + loc.size.width - b)
        {
            ei_bind(ei_ev_mouse_move, widget, NULL, top_resize, (void *) ei_anc_east);
            ei_bind(ei_ev_mouse_buttonup, widget, NULL, top_release, (void *) ei_anc_east);
        }
        else if ((toplevel -> resizable == ei_axis_both || toplevel -> resizable == ei_axis_y) &&
                 event -> param.mouse.where.y >= loc.top_left.y + loc.size.height - b)
        {
            ei_bind(ei_ev_mouse_move, widget, NULL, top_resize, (void *) ei_anc_south);
            ei_bind(ei_ev_mouse_buttonup, widget, NULL, top_release, (void *) ei_anc_south);
        }
        else if ((toplevel -> resizable == ei_axis_both || toplevel -> resizable == ei_axis_y) &&
                 event -> param.mouse.where.y <= loc.top_left.y + b)
        {
            ei_bind(ei_ev_mouse_move, widget, NULL, top_resize, (void *) ei_anc_north);
            ei_bind(ei_ev_mouse_buttonup, widget, NULL, top_release, (void *) ei_anc_north);
        }
        else if ((toplevel -> resizable == ei_axis_both || toplevel -> resizable == ei_axis_x) &&
                 event -> param.mouse.where.x <= loc.top_left.x + b)
        {
            ei_bind(ei_ev_mouse_move, widget, NULL, top_resize, (void *) ei_anc_west);
            ei_bind(ei_ev_mouse_buttonup, widget, NULL, top_release, (void *) ei_anc_west);
        }
        else if (event -> param.mouse.where.y < loc.top_left.y + ei_top_level_height() + b)
        {
            ei_bind(ei_ev_mouse_move, widget, NULL, top_drag, NULL);
            ei_bind(ei_ev_mouse_buttonup, widget, NULL, top_release, NULL);
        }
    }
    return EI_FALSE;
}

/* -------------------------------------------------------------------------- */
void ei_app_create(ei_size_t main_window_size,
                   ei_bool_t fullscreen)
{
    // initializes the hardware
    hw_init();

    // registers all classes of widget
    ei_frame_register_class();
    ei_button_register_class();
    ei_toplevel_register_class();

    // registers all geometry managers
    ei_register_placer_manager();

    // creates the root window
    root_surface = hw_create_window(main_window_size, fullscreen);
    ei_size_t size = hw_surface_get_size(root_surface);
    pick_surface = hw_surface_create(root_surface, size, EI_FALSE);

    ei_bind(ei_ev_mouse_buttondown, NULL, "button", ei_button_press, NULL);
    ei_bind(ei_ev_mouse_buttondown, NULL, "toplevel", top_press, NULL);

    // creates the root widget to access the root window
    root_widget = ei_widget_create("frame", NULL, NULL, NULL);
    ei_frame_configure(root_widget, &size, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    ei_place(root_widget, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL );

    ei_rect_t glob = ei_rect(ei_point_zero(), hw_surface_get_size(root_surface));
    ei_app_invalidate_rect(&glob);
}

void ei_app_free()
{
    // Releases all the resources of the application, and releases the hardware
    ei_widget_destroy(root_widget);
    hw_surface_free(pick_surface);
    hw_surface_free(root_surface);
    hw_quit();
}

/* -------------------------------------------------------------------------- */
ei_bool_t recursif_scour_widget_process_callback_on_taggedwidget(ei_widget_t *widget_courant, ei_tag_t tag, ei_callback_t callback, ei_event_t *event, void* user_param)
{
    if ( widget_courant == NULL )
    {
        return EI_FALSE;
    }

    if ( strcmp(tag, "all") == 0 || strcmp(tag, widget_courant->wclass->name) == 0 )
    {
        if (event -> type == ei_ev_mouse_buttondown) {
            if (is_in_widget(widget_courant, event -> param.mouse.where) == EI_TRUE) {
                ei_bool_t arret_ou_continue = callback(widget_courant, event, user_param);
                if (arret_ou_continue == EI_TRUE) return EI_TRUE;
            }
        } else {
            ei_bool_t arret_ou_continue = callback(widget_courant, event, user_param);
            if (arret_ou_continue == EI_TRUE) return EI_TRUE;
        }
    }

    ei_widget_t* widget_enfant = (*widget_courant).children_head;
    ei_bool_t arret_ou_continue_enfant = recursif_scour_widget_process_callback_on_taggedwidget(widget_enfant, tag, callback, event, user_param);
    if ( arret_ou_continue_enfant == EI_TRUE ) return EI_TRUE;

    ei_widget_t* widget_frere = (*widget_courant).next_sibling;
    ei_bool_t arret_ou_continue_frere = recursif_scour_widget_process_callback_on_taggedwidget(widget_frere, tag, callback, event, user_param);
    if ( arret_ou_continue_frere == EI_TRUE ) return EI_TRUE;

    return EI_FALSE;
}

/* -------------------------------------------------------------------------- */
void run_all_callback_associated_with_eventtype(ei_eventtype_t eventtype, ei_event_t *event)
{
    ei_bool_t booleen = EI_FALSE;
    ei_hooked_callback_t **struct_hook = seek_the_struct();
    ei_hooked_callback_t *cellule_courante = *struct_hook;

    while ( cellule_courante != NULL && booleen == EI_FALSE )
    {
        if ( cellule_courante -> eventtype == eventtype )
        {
            ei_widget_t *widget = cellule_courante -> widget;
            ei_tag_t tag = cellule_courante -> tag;
            ei_callback_t callback = cellule_courante -> callback;
            void *user_param = cellule_courante -> user_param;

            if ( tag != NULL )
            {
                booleen = recursif_scour_widget_process_callback_on_taggedwidget(root_widget, tag, callback, event, user_param);
            }
            else
            {
                if (eventtype == ei_ev_mouse_buttondown) 
                {
                    if (is_in_widget(widget, event -> param.mouse.where) == EI_TRUE) {
                        booleen = callback(widget, event, user_param);
                        
                    }
                } 
                else 
                {
                    booleen = callback(widget, event, user_param);
                }
            }
        }
        cellule_courante = cellule_courante -> next;
    }
}

/* -------------------------------------------------------------------------- */
void ei_app_run()
{
    draw(root_widget, root_surface, pick_surface, NULL);
    ei_event_t *event = malloc(sizeof(ei_event_t));
    ei_event_t *next_event = malloc((sizeof(ei_event_t)));

    while ( running == EI_TRUE )
    {
        hw_event_wait_next(event);

//        if (event -> type == ei_ev_mouse_move) {
//
//            hw_event_post_app(NULL);
//            hw_event_wait_next(next_event);
//            while (next_event -> type == ei_ev_mouse_move) {
//                event = next_event;
//                hw_event_wait_next(next_event);
//            }
//
//            run_all_callback_associated_with_eventtype(event -> type, event);
//
//            event = next_event;
//        }

        run_all_callback_associated_with_eventtype(event -> type, event);

        draw(root_widget, root_surface, pick_surface, &update_rect -> rect);

    }

    free(event);
}

/* -------------------------------------------------------------------------- */
void ei_app_invalidate_rect(ei_rect_t *rect)
{
    ei_rect_t *to_add = malloc(sizeof(ei_rect_t));
    to_add -> top_left.x = rect -> top_left.x;
    to_add -> top_left.y = rect -> top_left.y;
    to_add -> size.width = rect -> size.width + 1;
    to_add -> size.height = rect -> size.height + 1;
    if (update_rect == NULL) {
            update_rect = malloc(sizeof(ei_linked_rect_t));
    }
    ei_rect_t glob = ei_rect(ei_point_zero(), hw_surface_get_size(root_surface));

    update_rect -> rect = intersec_rect(glob, union_rect(update_rect -> rect, *to_add));
    update_rect -> next = NULL;

    free(to_add);
}

/* -------------------------------------------------------------------------- */
void ei_app_quit_request()
{
    running = EI_FALSE;
}

/* -------------------------------------------------------------------------- */
struct ei_widget_t *ei_app_root_widget()
{
    return root_widget;
}

/* -------------------------------------------------------------------------- */
ei_surface_t ei_app_root_surface()
{
    return root_surface;
}