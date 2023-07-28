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
#ifndef _TILE_H_
#define _TILE_H_
#include <gdk/gdk.h>
#include <librsvg/rsvg.h>

class Tile {
public:
	Tile ();
	Tile (int suit, int number);
	~Tile ();

	double get_x () const;
	double get_y () const;
	double get_w () const;
	double get_h () const;
	GFile *get_gfile () const;
	RsvgHandle *get_handler () const;
	RsvgRectangle get_viewport () const;
	RsvgRectangle get_logical () const;
	bool is_displayed () const;
	bool is_locked () const;

	void set_x (double x);
	void set_y (double y);
	void set_w (double w);
	void set_h (double h);
	void set_gfile (GFile *gfile);
	void set_handler (RsvgHandle *handler);
	void set_viewport (RsvgRectangle viewport);
	void set_logical (RsvgRectangle logical);
	void set_displayed (bool displayed);
	void set_locked (bool locked);

	void lock (std::string obj, int ncard);
	void unlock (std::string obj, int ncard);
	void draw_selector (cairo_t *cr);
protected:
	double x;
	double y;
	double w;
	double h;
	GFile *gfile;
	RsvgHandle *handler;
	RsvgRectangle viewport;
	RsvgRectangle logical;
	bool displayed;
	bool locked;
};
#endif
