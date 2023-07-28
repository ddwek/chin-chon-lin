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
#include "gettext.h"
#define _(String) gettext (String)
#include <sstream>
#include <iomanip>
#include <math.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include "Logic.h"
#include "Board.h"
#include "Tile.h"

static std::string suitname[4] = { _("clubs"), _("cups"), _("golds"), _("swords") };

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

Card::Card ()
{
}

bool Card::operator< (Card& c)
{
	bool ret = false;

	if (logic.get_comp_criteria () == "number") {
		if (this->number < c.number)
			ret = true;
		else
			ret = false;
	} else if (logic.get_comp_criteria () == "suit") {
		if (this->suit < c.suit)
			ret = true;
		else
			ret = false;
	}

	return ret;
}

bool Card::operator< (const Card& c2) const
{
	bool ret;

	if (this->number < c2.number)
		ret = true;
	else
		ret = false;

	return ret;
}

int Card::get_suit () const
{
	return suit;
}

int Card::get_number () const
{
	return number;
}

void Card::set_suit (int suit)
{
	this->suit = suit;
}

void Card::set_number (int number)
{
	this->number = number;
}

void Card::init (int suit, int number)
{
	std::ostringstream filename, svg_id;

	filename << CHIN_CHON_LIN_DATADIR << "data/tiles/";
	filename << suitname[suit] << std::setw (2) << std::setfill ('0') << number << ".svg";
	svg_id << "#" << suitname[suit] << std::setw (2) << std::setfill ('0') << number;

	x = .0;
	y = .0;
	w = .0;
	h = .0;
	this->suit = suit;
	this->number = number;

	gfile = g_file_new_for_path (filename.str().c_str ());
	handler = rsvg_handle_new_from_gfile_sync (gfile, RSVG_HANDLE_FLAGS_NONE, NULL, NULL);
	rsvg_handle_set_dpi (handler, 300);
	rsvg_handle_get_geometry_for_layer (handler, svg_id.str().c_str (), &viewport, NULL, &logical, NULL);
	displayed = false;
}
