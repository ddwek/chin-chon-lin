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
#ifndef _DECK_H_
#define _DECK_H_
#include <list>
#include <string>
#include <cairo.h>
#include <librsvg/rsvg.h>
#include "Card.h"

class Deck : public Tile {
public:
	Deck ();
	Deck (Deck&) = delete;
	Deck (Deck&&) = delete;
	Deck& operator= (Deck&) = delete;
	~Deck ();

	std::list<Card>& get_cards ();
	std::string get_cc () const;

	void set_cards (std::list<Card>& cards);
	void set_cc (std::string cc);

	void acquire (Card& card);
	void relocate ();
	void draw (cairo_t *cr);
	void draw_empty (cairo_t *cr);
	void animate (cairo_t *cr);
private:
	std::list<Card> cards;
	std::string cc;
};

extern class Deck deck;
#endif
