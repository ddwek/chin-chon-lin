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
#ifndef _STACK_H_
#define _STACK_H_
#include <cairo.h>
#include "Tile.h"

class StackPlayed : public Tile {
public:
	StackPlayed ();
	~StackPlayed ();

	std::list<Card>& get_cards ();
	bool get_only_once_value () const;

	void set_only_once_value (bool only_once);

	void acquire (std::string src);
	void relocate ();
	void draw (cairo_t *cr);
	void animate (cairo_t *cr);
	void clear (cairo_t *cr);
private:
	std::list<Card> cards;
        bool only_once;
};

extern class StackPlayed stack_played;

#endif
