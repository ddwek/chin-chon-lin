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
#ifndef _CARD_H_
#define _CARD_H_
#include <string>
#include <cairo.h>
#include "Tile.h"

class Card : public Tile {
public:
	Card ();
	bool operator< (Card&);
	bool operator< (const Card& c2) const;

	int get_suit () const;
	int get_number () const;
	void set_suit (int suit);
	void set_number (int number);

	void init (int suit, int number);
private:
	int suit;
	int number;
};

extern class Card card[48];
#endif
