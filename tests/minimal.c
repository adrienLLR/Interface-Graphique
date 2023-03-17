#include <stdlib.h>

#include "ei_types.h"
#include "ei_draw.h"
#include "ei_event.h"
#include "hw_interface.h"

int main(int argc, char** argv)
{
	ei_surface_t			main_window		= NULL;
	ei_size_t			main_window_size;
	ei_event_t			event;
	int				i;
	ei_linked_point_t		points[4];
	int				coords[]		= { 20, 20, 620, 20, 20, 460, 620, 460 };
	ei_color_t			transp_blue		= { 0x00, 0x00, 0xff, 0x88 };
	uint32_t*			pixel_ptr;

	// Init access to hardware.
	hw_init();

	// Create the main window.
	main_window_size.width	= 640;
	main_window_size.height	= 480;
	main_window = hw_create_window(main_window_size, EI_FALSE);

	// Fill the main window in white,
	// draw a transparent blue polygon on top of it.

	// Define the polygon vertices.
	for (i = 0; i < 4; i++) {
		points[i].point.x	= coords[i*2];
		points[i].point.y	= coords[i*2 + 1];
		if (i < 3)
			points[i].next	= &(points[i+1]);
		else
			points[i].next	= NULL;
	}

	// Lock the surface for drawing
	hw_surface_lock(main_window);

	// fill in white
	pixel_ptr = (uint32_t *)hw_surface_get_buffer(main_window);
	for (i = 0; i < main_window_size.width * main_window_size.height; i++)
		*pixel_ptr++ = 0xffffffff;

	// draw the polygon
	ei_draw_polygon(main_window, points, transp_blue, NULL);

	// unlock the surface and update the screen.
	hw_surface_unlock(main_window);
	hw_surface_update_rects(main_window, NULL);

	// Wait for a key press.
	event.type = ei_ev_none;
	while (event.type != ei_ev_keydown)
		hw_event_wait_next(&event);

	// Free hardware resources.
	hw_quit();

	// Terminate program with no error code.
	return 0;
}
