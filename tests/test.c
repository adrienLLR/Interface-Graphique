#include <stdio.h>
#include <stdlib.h>

#include "ei_application.h"
#include "ei_event.h"
#include "hw_interface.h"
#include "ei_widget.h"
#include "ei_geometrymanager.h"
#include "ei_widget.h"
#include "ei_utils.h"
#include "ei_button.h"


const char*	windows_xp_wallpaper	= "misc/windowsxp_wallpaper.png";
ei_size_t	screen_size	= {800, 800};
ei_color_t      root_bgcol      = {0x56, 0x7D, 0xDA, 0xff};
ei_color_t      toplevel_color = {0xFF, 0xFF, 0xFF, 0xff};

ei_color_t	button_color	= {0xD4, 0xDE, 0xFF, 0xff};
ei_color_t	button_color2	= {240, 60, 20, 0xff};;
ei_color_t      text_color      = {0x00, 0x00, 0x00, 0xff};
ei_relief_t     relief          = ei_relief_raised;
int             button_border_width    = 4;
int             button_border_width2    = 15;
char*           button_title    = "New window";
char*           button_title2    = "Quit";

ei_color_t	toolbar_color	= {0x2b, 0x4f, 0x9d, 0xff};
ei_size_t       toolbar_size    = {50, 800};
int             toolbar_border_width = 2;
int             toolbar_x = -3;
int             toolbar_y = 0;

float           close_rel_x = 0.0;
float           close_rel_y = 0.0;
float           close_rel_width = 1.0;
float           close_rel_height = 0.1;

ei_anchor_t	button_anchor	= ei_anc_center;
float           button_rel_x    = 0.77;
float           button_rel_y    = 0.10;
int		button_x	= -16;
int		button_y	= -16;
float           button_rel_size_x = 0.5;

ei_size_t       window_size     = {400,400};
char*           window_title    = "Windows generator";
char*           window_title2    = "Window";
ei_color_t      window_color    = {0xA0,0xA0,0xA0, 0xff};
int             window_border_width    = 2;
ei_bool_t       closable        = EI_TRUE;
ei_bool_t       not_closable        = EI_FALSE;
ei_axis_set_t   window_resizable = ei_axis_both;
ei_point_t	window_position	 = {30, 10};
ei_point_t	window_position2 = {200, 50};
float           toplevel_rel_height = 0.75;
float           toplevel_rel_width = 0.75;
float           toplevel_rel_x = 0.25;
float           toplevel_rel_y = 0.50;

int           toplevel_x = 15;
int           toplevel_y = 50;

ei_anchor_t	frame_anchor	= ei_anc_southeast;
float           frame_rel_x    = 1.0;
float           frame_rel_y    = 1.0;

void put_basic_widget(void);

ei_bool_t button_quitt(ei_widget_t* widget, ei_event_t* event, void* user_param)
{
    ei_app_quit_request();
    return EI_TRUE;
}

ei_bool_t button_press(ei_widget_t* widget, ei_event_t* event, void* user_param)
{
    char* window_title = "Window";
    ei_widget_t *parent = widget -> parent;
    ei_point_t	window_position2 = {parent -> screen_location.top_left.x + 80, parent -> screen_location.top_left.y + 80};

    ei_widget_t *toplevel = ei_widget_create("toplevel", ei_app_root_widget(), NULL, NULL);
    ei_toplevel_configure(toplevel, &window_size, &toplevel_color, &window_border_width, &window_title, &closable, &window_resizable, NULL);
    ei_place(toplevel, NULL, &(window_position2.x), &(window_position2.y), NULL, NULL, NULL, NULL, NULL, NULL);

    return EI_TRUE;
}

ei_bool_t process_key(ei_widget_t* widget, ei_event_t* event, void* user_param)
{
    if (event->param.key.key_code == SDLK_ESCAPE) {
        ei_app_quit_request();
        return EI_TRUE;
    }
    if (ei_has_modifier(event->param.key.modifier_mask, ei_mod_ctrl_left)) {
        if (event->param.key.key_code == SDLK_n)
        {
            put_basic_widget();
            return EI_TRUE;
        }
    }

    return EI_FALSE;
}

void put_basic_widget()
{
    ei_widget_t*    button;
    ei_widget_t*    toplevel;
    ei_widget_t*    toplevel2;

    ei_callback_t button_callback = button_press;

    toplevel = ei_widget_create("toplevel", ei_app_root_widget(), NULL, NULL);
    toplevel2 = ei_widget_create("toplevel", toplevel, NULL, NULL);
    button = ei_widget_create("button", toplevel, NULL, NULL);

    ei_toplevel_configure(toplevel, &window_size, &toplevel_color, &window_border_width, &window_title, &closable, &window_resizable, NULL);
    ei_toplevel_configure(toplevel2, &window_size, &toplevel_color, &window_border_width, &window_title2, &not_closable, &window_resizable, NULL);

    ei_button_configure(button, NULL, &button_color,
                        &button_border_width, NULL, &relief, &button_title, NULL, &text_color, NULL,
                        NULL, NULL, NULL, &button_callback, NULL);

    ei_place(toplevel, NULL, &(window_position2.x), &(window_position2.y), NULL, NULL, NULL, NULL, NULL, NULL);
    ei_place(toplevel2, NULL, &toplevel_x, &toplevel_y, NULL, NULL, NULL, NULL, &toplevel_rel_width, &toplevel_rel_height);
    ei_place(button, &button_anchor, &button_x, &button_y, NULL,NULL, &button_rel_x, &button_rel_y, &button_rel_size_x, NULL);
}

// main --

int main(int argc, char** argv)
{
    ei_widget_t*    frame;
    ei_widget_t*    toolbar;
    ei_widget_t*    button_toolbar;
    ei_callback_t   button_callback2 = button_quitt;
    //create the root widget
    ei_app_create(screen_size, EI_FALSE);
    ei_frame_configure(ei_app_root_widget(), NULL, &root_bgcol, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

    //create the logo windows xp
    ei_surface_t image = hw_image_load(windows_xp_wallpaper, ei_app_root_surface());
    ei_size_t image_size = hw_surface_get_size(image);
    frame = ei_widget_create("frame", ei_app_root_widget(), NULL, NULL);
    ei_frame_configure(frame, &image_size, NULL, 0, NULL, NULL, NULL, NULL, NULL, &image, NULL, NULL);
    ei_place(frame, &frame_anchor, NULL, NULL, NULL, NULL, &frame_rel_x, &frame_rel_y, NULL, NULL);

    //create the toolbar
    toolbar = ei_widget_create("frame", ei_app_root_widget(), NULL, NULL);
    button_toolbar = ei_widget_create("button", toolbar, NULL, NULL);
    ei_frame_configure(toolbar, &toolbar_size, &toolbar_color,
                       &toolbar_border_width, &relief, NULL, NULL, NULL, NULL,
                       NULL, NULL, NULL);

    ei_button_configure(button_toolbar, NULL, &button_color2,
                        &button_border_width2, NULL, &relief, &button_title2, NULL, &text_color, NULL,
                        NULL, NULL, NULL, &button_callback2, NULL);

    ei_place(toolbar, NULL, NULL, NULL, NULL,NULL, NULL, NULL, NULL, NULL);
    ei_place(button_toolbar, NULL, NULL, NULL, NULL,NULL, &close_rel_x, &close_rel_y, &close_rel_width, &close_rel_height);

    //create the basic widgets
    put_basic_widget();


    ei_bind(ei_ev_keydown, NULL, "all", process_key, NULL);
    ei_app_run();

    ei_unbind(ei_ev_keydown, NULL, "all", process_key, NULL);
    ei_app_free();

    return (EXIT_SUCCESS);
}