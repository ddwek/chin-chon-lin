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
#include <string>
#include <gdk/gdk.h>
#include <librsvg/rsvg.h>
#include "UserInterface.h"
#include "Board.h"

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

Tile::Tile ()
{
}

Tile::~Tile ()
{
}

double Tile::get_x () const
{
	return x;
}

double Tile::get_y () const
{
	return y;
}

double Tile::get_w () const
{
	return w;
}

double Tile::get_h () const
{
	return h;
}

GFile *Tile::get_gfile () const
{
	return gfile;
}

RsvgHandle *Tile::get_handler () const
{
	return handler;
}

RsvgRectangle Tile::get_viewport () const
{
	return viewport;
}

RsvgRectangle Tile::get_logical () const
{
	return logical;
}

bool Tile::is_displayed () const
{
	return displayed;
}

bool Tile::is_locked () const
{
	return locked;
}

void Tile::set_x (double x)
{
	this->x = x;
}

void Tile::set_y (double y)
{
	this->y = y;
}

void Tile::set_w (double w)
{
	this->w = w;
}

void Tile::set_h (double h)
{
	this->h = h;
}

void Tile::set_gfile (GFile *gfile)
{
	this->gfile = gfile;
}

void Tile::set_handler (RsvgHandle *handler)
{
	this->handler = handler;
}

void Tile::set_viewport (RsvgRectangle viewport)
{
	this->viewport = viewport;
}

void Tile::set_logical (RsvgRectangle logical)
{
	this->logical = logical;
}

void Tile::set_displayed (bool displayed)
{
	this->displayed = displayed;
}

void Tile::set_locked (bool locked)
{
	this->locked = locked;
}

void Tile::lock (std::string obj, int ncard)
{
	std::string str;
	int number;

	if (obj == "deck") {
		str = "deck::hover";
		number = -1;
	} else if (obj == "stack") {
		str = "stack::hover";
		number = -2;
	} else {
		str = "human::hover";
		number = ncard;
	}

	ui.unregister_region (x, y, x + w, y + h, str, number);
}

void Tile::unlock (std::string obj, int ncard)
{
	std::string str;
	int number;

	if (obj == "deck") {
		str = "deck::hover";
		number = -1;
	} else if (obj == "stack") {
		str = "stack::hover";
		number = -2;
	} else {
		str = "human::hover";
		number = ncard;
	}

	ui.register_region (x, y, x + w, y + h, str, number);
}

void Tile::draw_selector (cairo_t *cr)
{
	cairo_save (cr);
	cairo_set_source_rgb (cr, 1.0, 1.0, 0.0);
	cairo_rectangle (cr, x - 5.0, y - 5.0, logical.width * 5.0 * board.get_x_scale () + 10.0,
					logical.height * 5.0 * board.get_y_scale () + 10.0);
	cairo_scale (cr, 5.0 * board.get_x_scale (), 5.0 * board.get_y_scale ());
	cairo_set_line_width (cr, 0.2);
	cairo_stroke (cr);
	cairo_restore (cr);
}
