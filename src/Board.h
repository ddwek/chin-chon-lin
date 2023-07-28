/*
 *
 * Copyright 2023 Daniel Dwek
 *
 * This file is part of chin-chon-lin.
 *
 *  chin-chon-lin is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  chin-chon-lin is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with chin-chon-lin.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef _BOARD_H_
#define _BOARD_H_
#include <string>
#include <gtk/gtk.h>
#include <cairo.h>

typedef enum { IDLE = 0, DECK_DISTRIBUTE,
		DECK_TO_PLAYER_START, DECK_TO_PLAYER_STOP,
		STACK_TO_PLAYER_START, STACK_TO_PLAYER_STOP,
		PLAY_CARD_START, PLAY_CARD_STOP,
		HUMAN_HOVER, DECK_HOVER, STACK_HOVER,
		FINISHING_ROUND_START, FINISHING_ROUND_STOP,
} status_t;

class Board {
public:
	Board (int w, int h, double xs, double ys, double frames);
	~Board ();

	int get_turn () const;
	status_t get_status () const;
	int get_width () const;
	int get_height () const;
	double get_x_scale () const;
	double get_y_scale () const;
	double get_framerate () const;
	cairo_surface_t *get_surface () const;
	cairo_t *get_cr () const;
	bool get_display_scores () const;
	bool get_reset () const;

	void set_turn (int turn);
	void set_status (status_t status);
	void set_width (int w);
	void set_height (int h);
	void set_x_scale (double xs);
	void set_y_scale (double ys);
	void set_framerate (double f);
	void set_surface (cairo_surface_t *surface);
	void set_cr (cairo_t *cr);
	void set_display_scores (bool display_scores);
	void set_reset (bool reset);

	void paint (cairo_t *cr);
	void new_game ();
	void new_round (cairo_t *cr);
private:
	int turn;
	status_t status;
	int width;
	int height;
	double x_scale;
	double y_scale;
	double framerate;
	cairo_surface_t *surface;
	cairo_t *cr;
	bool display_scores;
	bool reset;
};

extern class Board board;
#endif
