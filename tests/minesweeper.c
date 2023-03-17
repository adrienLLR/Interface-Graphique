/*
 *	minesweeper.c
 *	ig
 *
 *	Created by Denis Becker on 12.02.16.
 *	Copyright 2016 Ensimag. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h> // for random seed
#include <string.h> // memset function

#include "ei_application.h"
#include "ei_widget.h"
#include "ei_utils.h"
#include "ei_event.h"
#include "ei_geometrymanager.h"

/* constants */

static const int BUTTON_SIZE = 28;

/* global resources */

static ei_surface_t		glob_flag_img;
static ei_surface_t		glob_bomb_img;
static ei_surface_t		glob_reset_img;

/* structs & typedefs */

struct map_t;

typedef struct map_pos_t {

	int			index;
	ei_bool_t		has_mine;
	ei_bool_t		has_flag;
	ei_bool_t		is_revealed;
	int			nb_mines_around;
	struct map_t*		map_ptr;
	ei_widget_t*		frame_w;
	
} map_pos_t;

typedef struct map_t {

	int			width;
	int			height;
	int			nb_mines;
	map_pos_t*		map_pos;
	ei_bool_t		game_over;
	int			nb_revealed;
	int			flag_count;
	ei_widget_t*		flag_count_widget;
	ei_widget_t*		victory_text_widget;

} map_t;

/* utility functions */

static inline map_pos_t* get_pos_at(map_t* map, int x, int y)
{
	return &map->map_pos[y * map->width + x];
}

static inline int pos_get_x(map_pos_t* pos)
{
	return pos->index % pos->map_ptr->width;
}

static inline int pos_get_y(map_pos_t* pos)
{
	return pos->index / pos->map_ptr->width;
}

static inline ei_bool_t check_bounds(map_t* map, int x, int y)
{
	return x >= 0 && y >= 0 && x < map->width && y < map->height;
}

static inline ei_bool_t can_play(map_pos_t* map_pos)
{
	return ((!map_pos->has_flag) && (!map_pos->is_revealed));
}

void int_to_str(int val, char *txt, int txtlen)
{
	ei_bool_t	positive	= (val >= 0);
	int		idx		= txtlen - 1;

	val = (positive ? val : -val);

	do {
		txt[idx] = '0' + (char) (val % 10);
		val /= 10;
		if (val == 0)
			break;
	} while (idx-- > 0);

	if (!positive && idx > 0)
		txt[idx - 1] = '-';
}

/* core functions */

/*
 * check_victory: configures the victory message if victory.
 */
static ei_bool_t check_victory(map_t* map)
{
	ei_bool_t	victory = (map->nb_revealed + map->nb_mines == map->width * map->height);
	char*		victory_text = "You won!";
	ei_color_t	victory_back = {0x33, 0x33, 0x33, 0xff};
	ei_color_t	victory_color = {0x55, 0xbb, 0x55, 0xff};

	if (victory) {
		map->game_over = EI_TRUE;
		ei_frame_configure(map->victory_text_widget, NULL, &victory_back, NULL, NULL,
					&victory_text, NULL, &victory_color, NULL, NULL, NULL, NULL);
	}
	return victory;
}

void update_flag_count(map_t* map)
{
	char	txt[5];
	memset(txt, ' ', 4);
    txt[4] = '\0';
	char*	txtptr = (char*) &txt;

	int_to_str(map->flag_count, txt, 4);
	ei_frame_configure(map->flag_count_widget, NULL, NULL, NULL, NULL, &txtptr, NULL, NULL,
			NULL, NULL, NULL, NULL);
}

void switch_flag(map_pos_t* map_pos)
{
	map_pos->has_flag = !map_pos->has_flag;

	if (map_pos->has_flag) {
		ei_frame_configure(map_pos->frame_w, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
								NULL, &glob_flag_img, NULL, NULL);
		map_pos->map_ptr->flag_count--;
	} else {
		ei_frame_configure(map_pos->frame_w, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
								NULL, &glob_reset_img, NULL, NULL);
		map_pos->map_ptr->flag_count++;
	}
	update_flag_count(map_pos->map_ptr);
}

/*
 * Reveal one position on the map, don't explore neighbors, don't check
 * if already revealed (must be done by the caller), don't end game if it's a mine.
 */
void reveal_position_no_check(map_pos_t* map_pos)
{
	char		nb_txt[2] = " ";
	char*		nb_txt_ptr	= (char*) nb_txt;
	ei_relief_t	relief		= ei_relief_none;
	
	map_pos->is_revealed = EI_TRUE;
	map_pos->map_ptr->nb_revealed++;
	ei_frame_configure(map_pos->frame_w, NULL, NULL, NULL, &relief, NULL, NULL, NULL, NULL,
										NULL, NULL, NULL);
	if (map_pos->has_mine) {
		ei_frame_configure(map_pos->frame_w, NULL, NULL, NULL, &relief, NULL, NULL,
							NULL, NULL, &glob_bomb_img, NULL, NULL);
		return;
	}

	if (map_pos->nb_mines_around > 0) {
		int_to_str(map_pos->nb_mines_around, nb_txt, 1);
		ei_frame_configure(map_pos->frame_w, NULL, NULL, NULL, NULL, &nb_txt_ptr,
							NULL, NULL, NULL, NULL, NULL, NULL);
	}
}

void defeat(map_t* map)
{
	char*		defeat_text = "You lost!";
	ei_color_t	defeat_back = {0x33, 0x33, 0x33, 0xff};
	ei_color_t	defeat_color = {0xbb, 0x55, 0x55, 0xff};

	map->game_over = EI_TRUE;

	for (int i = 0; i < map->width * map->height; i++)
		if (map->map_pos[i].has_mine)
			reveal_position_no_check(&map->map_pos[i]);
	
	ei_frame_configure(map->victory_text_widget, NULL, &defeat_back, NULL, NULL, &defeat_text, NULL, &defeat_color,
			NULL, NULL, NULL, NULL);
}

/*
 * Caller must ensure that map_pos isn't already revealed.
 */
void reveal_and_explore(map_pos_t* map_pos)
{
	int			x;
	int			y;
	int			cx	= pos_get_x(map_pos);
	int			cy	= pos_get_y(map_pos);
	map_t*			map	= map_pos->map_ptr;
	map_pos_t*		next;

	reveal_position_no_check(map_pos);
	if (map_pos->has_mine) {
		defeat(map);
		return;

	} else if (map_pos->nb_mines_around > 0)
		// explosion risk around, stop exploration
		return;

	for (y = cy - 1; y <= cy + 1; y++)
		for (x = cx - 1; x <= cx + 1; x++) {
			next = get_pos_at(map, x, y);
			if ((x != cx || y != cy) && check_bounds(map, x, y)
					&& (!next->is_revealed) && (!next->has_flag))
				reveal_and_explore(next);
		}
}

/*
 * set_mine_at_pos: places a mine at the given position.
 */
void set_mine_at_pos(map_pos_t *map_pos)
{
	int	x;
	int	y;
	int	cx	= pos_get_x(map_pos);
	int	cy	= pos_get_y(map_pos);
	map_t*	map	= map_pos->map_ptr;

	map_pos->has_mine = EI_TRUE;

	// update neighbors mine count
	for (y = cy - 1; y <= cy + 1; y++)
		for (x = cx - 1; x <= cx + 1; x++)
			if ((x != cx || y != cy) && check_bounds(map, x, y))
				get_pos_at(map, x, y)->nb_mines_around++;
}

/*
 * place_mines: places the mines on an empty map.
 */
void place_mines(map_t* map)
{
	int i, n;

	for (n = 0; n < map->nb_mines; n++) {
		do {
			i = rand() % (map->width * map->height);
		} while (map->map_pos[i].has_mine);

		set_mine_at_pos(&map->map_pos[i]);
	}
}


/*
 * reset_game: creates a new grid.
 */
void reset_game(map_t* map)
{
	int			i;
	char*		nulltext = NULL;
	ei_relief_t relief = ei_relief_raised;
	ei_color_t	color = {0xbb, 0xbb, 0xbb, 0xff};

	map->flag_count = map->nb_mines;
	map->game_over = EI_FALSE;
	map->nb_revealed = 0;
	for (i = 0; i < map->width * map->height; i++) {
		map->map_pos[i].has_mine = EI_FALSE;
		map->map_pos[i].has_flag = EI_FALSE;
		map->map_pos[i].is_revealed = EI_FALSE;
		map->map_pos[i].nb_mines_around = 0;
		// reset text & reset image separately
	ei_frame_configure(map->map_pos[i].frame_w, NULL, NULL, NULL, &relief, &nulltext, NULL, NULL, NULL, NULL, NULL, NULL);
	ei_frame_configure(map->map_pos[i].frame_w, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &glob_reset_img, NULL, NULL);
	}
	place_mines(map);
	update_flag_count(map);
	ei_frame_configure(map->victory_text_widget, NULL, &color, NULL, NULL, &nulltext, NULL, NULL,
			NULL, NULL, NULL, NULL);
}

/*
 * Create and fill the mine map.
 * 'map' must point to an allocated 'map_t' struct.
 */
void create_mine_map(map_t* map, int width, int height, int nb_mines)
{
	int i;
	
	map->width = width;
	map->height = height;
	map->nb_mines = nb_mines;
	map->flag_count = nb_mines;
	map->game_over = EI_FALSE;
	map->nb_revealed = 0;
	map->map_pos = (map_pos_t*) calloc(width * height, sizeof(map_pos_t));
	for (i = 0; i < map->width * map->height; i++) {
		map->map_pos[i].index = i;
		map->map_pos[i].map_ptr = map;
	}
	place_mines(map);
}

void destroy_mine_map(map_t* map)
{
	free(map->map_pos);
}

/* event handlers */

ei_bool_t handle_keydown(ei_widget_t* widget, ei_event_t* event, void* user_param)
{
	if (event->param.key.key_code == SDLK_ESCAPE) {
		ei_app_quit_request();
		return EI_TRUE;
	}

	return EI_FALSE;
}

ei_bool_t handle_restart(ei_widget_t* widget, ei_event_t* event, void* user_param)
{
	map_t* map = (map_t*) user_param;
	reset_game(map);
	return EI_TRUE;
}

ei_bool_t handle_pos_event(ei_widget_t* widget, ei_event_t* event, void* user_param)
{
	map_pos_t* map_pos = (map_pos_t*) user_param;

	if (map_pos->map_ptr->game_over) {
		// may be better...
		return EI_FALSE;
	}

	if (event->param.mouse.button == ei_mouse_button_left) {
		if (can_play(map_pos)) {
			reveal_and_explore(map_pos);
			check_victory(map_pos->map_ptr);
			return EI_TRUE;
		}
	} else if (event->param.mouse.button == ei_mouse_button_right) {
		if (!map_pos->is_revealed) {
			switch_flag(map_pos);
			return EI_TRUE;
		}
	}

	return EI_FALSE;
}

/*
 * Create the game window, 'map' must have been created before (with 'create_mine_map').
 */
void create_game_window(map_t* map)
{
	ei_size_t		size;
	int const		info_height = 40;
	char*			restart_text = "Restart";
	ei_anchor_t		anchor = ei_anc_north;

	ei_widget_t*	toplevel = ei_widget_create("toplevel", ei_app_root_widget(), NULL, NULL);
	ei_size_t		toplevel_size = ei_size(BUTTON_SIZE * map->width, info_height + BUTTON_SIZE * map->height);
	ei_color_t		toplevel_bg = {0xbb, 0xbb, 0xbb, 0xff};
	ei_point_t		coord;
	ei_axis_set_t	resizable = ei_axis_none;
	char*			title = "Minesweeper";
	int				border_width;
	ei_relief_t		relief = ei_relief_raised;
	int				corner_radius = 0;

	ei_widget_t*	widget;
	ei_color_t		grey = {0x88, 0x88, 0x88, 0xff};
	ei_color_t		light = {0xcc, 0xcc, 0xcc, 0xcc};
	ei_callback_t	callback = handle_restart;

	map_pos_t*		current_pos;
	int				x, y;
	char			text[5];
	memset(text, ' ', 4);
    text[4] = '\0';
	char*			textptr = (char*) &text;

	// toplevel
	coord.x = 40;
	coord.y = 40;
	border_width = 0;
	ei_toplevel_configure(toplevel, &toplevel_size, &toplevel_bg, &border_width, &title, NULL, &resizable, NULL);
	ei_place(toplevel, NULL, &(coord.x), &(coord.y), NULL, NULL, NULL, NULL, NULL, NULL);

	// restart button
	coord.x = toplevel_size.width / 2;
	coord.y = 4;
	size = ei_size(100, info_height - 2 * coord.y);
	border_width = 2;
	widget = ei_widget_create("button", toplevel, NULL, NULL);
	ei_button_configure(widget, &size, &grey, &border_width, &corner_radius, NULL, &restart_text, NULL, NULL,
			NULL, NULL, NULL, NULL, &callback, (void*) &map);
	ei_place(widget, &anchor, &(coord.x), &(coord.y), NULL, NULL, NULL, NULL, NULL, NULL);

	// flag count label
	coord.x = 4;
	size.width = 70;
	anchor = ei_anc_northwest;
	int_to_str(map->nb_mines, text, 4);
	widget = ei_widget_create("frame", toplevel, NULL, NULL);
	ei_frame_configure(widget, &size, &light, &border_width, NULL, &textptr, NULL, NULL,
			NULL, NULL, NULL, NULL);
	ei_place(widget, &anchor, &(coord.x), &(coord.y), NULL, NULL, NULL, NULL, NULL, NULL);
	map->flag_count_widget = widget;

	// game result text
	coord.x = toplevel_size.width - 4;
	size.width = 90;
	anchor = ei_anc_northeast;
	widget = ei_widget_create("frame", toplevel, NULL, NULL);
	ei_frame_configure(widget, &size, &toplevel_bg, &border_width, NULL, NULL, NULL, NULL,
			NULL, NULL, NULL, NULL);
	ei_place(widget, &anchor, &(coord.x), &(coord.y), NULL, NULL, NULL, NULL, NULL, NULL);
	map->victory_text_widget = widget;

	// minefield (= "array" of game buttons)
	size = ei_size(BUTTON_SIZE, BUTTON_SIZE);
	border_width = 2;
	callback = handle_pos_event;
	for (y = 0; y < map->height; y++) {
		for (x = 0; x < map->width; x++) {
			coord.x = x * size.width;
			coord.y = info_height + y * size.height;

			widget = ei_widget_create("frame", toplevel, NULL, NULL);
			current_pos = get_pos_at(map, x, y);
			current_pos->frame_w = widget;
			ei_frame_configure(widget, &size, &grey, &border_width, &relief, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
			ei_bind(ei_ev_mouse_buttondown, widget, NULL, callback, (void*)current_pos);
			ei_place(widget, NULL, &(coord.x), &(coord.y), NULL, NULL, NULL, NULL, NULL, NULL);
		}
	}
}

int get_arg_value(char *arg, int min, int max, int def)
{
	int try = atoi(arg);
	if (try < min || try > max) {
		return def;
	}
	return try;
}



// main -

int main(int argc, char* argv[])
{
	srand((unsigned)time(NULL)); // random seed

	const int	w_min = 12, w_max = 32;
	const int	h_min = 4, h_max = 22;
	int		size_w = 20;
	int		size_h = 16;
	int		nb_mines = 40;

	if (argc == 4) {
		size_w = get_arg_value(argv[1], w_min, w_max, size_w);
		size_h = get_arg_value(argv[2], h_min, h_max, size_h);
		nb_mines = get_arg_value(argv[3], 1, size_w * size_h, nb_mines);
	} else if (argc > 1) {
		printf("Using default values\n");
		printf("Usage: minesweeper WIDTH HEIGHT NB_MINES\n	WIDTH range = (%d, %d) def. %d\n", w_min, w_max, size_w);
		printf("  HEIGHT range = (%d, %d) def. %d\n	 NB_MINES > 0 and < WIDTH * HEIGHT def. %d\n", h_min, h_max, size_h, nb_mines);
	}

	map_t		map;
	ei_size_t	root_window_size		= { 1024, 768 };
	ei_color_t	root_bgcol			= { 0x52, 0x7f, 0xb4, 0xff };
	ei_bool_t	fullscreen			= EI_FALSE;

	ei_app_create(root_window_size, fullscreen);
	ei_frame_configure(ei_app_root_widget(), NULL, &root_bgcol, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

	if ((glob_flag_img = hw_image_load("misc/flag.png", ei_app_root_surface())) == NULL) {
		printf("ERROR: could not load image \"misc/flag.png\"");
	return 1;
	}
	if ((glob_bomb_img = hw_image_load("misc/bomb.png", ei_app_root_surface())) == NULL) {
		printf("ERROR: could not load image \"misc/bomb.png\"");
	return 1;
	}
	glob_reset_img = NULL;

	create_mine_map(&map, size_w, size_h, nb_mines);
	create_game_window(&map);
	
	ei_bind(ei_ev_keydown, NULL, "all", handle_keydown, NULL);

	ei_app_run();

	destroy_mine_map(&map);

	hw_surface_free(glob_flag_img);
	hw_surface_free(glob_bomb_img);

	ei_app_free();

	return (EXIT_SUCCESS);
}
