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
#include <iostream>
#include <iomanip>
#include <math.h>
#include <list>
#include <gtk/gtk.h>
#include <cairo.h>
#include <librsvg/rsvg.h>
#include "UserInterface.h"
#include "Board.h"
#include "Player.h"
#include "Tile.h"
#include "Deck.h"
#include "Card.h"

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
extern GtkWidget *drawing_area;
extern unsigned stack_to_player_tid;

/*
 * Default constructor for the stack of played cards. We load "deck-ar.svg"
 * as a workaround to supply it a generic @handler pointer and @logical struct,
 * but as you can see when playing, only regular cards are rendered at its
 * location on screen
 */
StackPlayed::StackPlayed ()
{
	gfile = g_file_new_for_path (CHIN_CHON_LIN_DATADIR "data/tiles/deck-ar.svg");
	handler = rsvg_handle_new_from_gfile_sync (gfile, RSVG_HANDLE_FLAGS_NONE, NULL, NULL);
	rsvg_handle_set_dpi (handler, 300.0);
	rsvg_handle_get_geometry_for_layer (handler, "#deck", &viewport, NULL, &logical, NULL);
	relocate ();
}

StackPlayed::~StackPlayed ()
{
}

std::list<Card>& StackPlayed::get_cards ()
{
	return cards;
}

bool StackPlayed::get_only_once_value () const
{
	return only_once;
}

void StackPlayed::set_only_once_value (bool only_once)
{
	this->only_once = only_once;
}

void StackPlayed::acquire (std::string src)
{
	int i;
	std::list<Card>::iterator iter;
	Player& p = player[board.get_turn ()];

	if (src == "deck") {
		if (deck.get_cards().size ()) {
			cards.push_front (*deck.get_cards().begin ());
			deck.get_cards().erase (deck.get_cards().begin ());
		}
	} else if (src == "player") {
		std::list<Card>& c = p.get_cards ();
		for (i = 0, iter = c.begin (); iter != c.end (); iter++, i++) {
			if (i == p.get_selected ()) {
				if (only_once) {
					cards.push_front (*iter);
					std::cout << p.get_id () << _(" played suit = ") <<
						cards.front().get_suit () << _(", number = ") <<
						cards.front().get_number () <<
						std::endl;
					c.erase (iter);
					p.set_selected (7);
					only_once = false;
				}
				break;
			}
		}
	}
}

void StackPlayed::relocate ()
{
	x = (board.get_width () - logical.width * 10.0 * board.get_x_scale ()) / 2.0;
	x += logical.width * 5.0 * board.get_x_scale ();
	y = (board.get_height () - logical.height * 5.0 * board.get_y_scale ()) / 2.0;
	w = logical.width * 5.0 * board.get_x_scale ();
	h = logical.height * 5.0 * board.get_y_scale ();
}

void StackPlayed::draw (cairo_t *cr)
{
	x = (board.get_width () - logical.width * 10.0 * board.get_x_scale ()) / 2.0;
	x += logical.width * 5.0 * board.get_x_scale ();
	y = (board.get_height () - logical.height * 5.0 * board.get_y_scale ()) / 2.0;
	for (unsigned long i = 0; i < cards.size (); i += 4) {
		cairo_save (cr);
		cairo_translate (cr, x + i / 4.0, y - i / 4.0);
		cairo_scale (cr, 5.0 * board.get_x_scale (), 5.0 * board.get_y_scale ());
		cairo_set_source_rgb (cr, 0.4, 0.5, 0.8);
		cairo_rectangle (cr, 0, 0, logical.width, logical.height);
		cairo_fill (cr);
		cairo_restore (cr);
	}

	if (!cards.size ())
		return;
	x = (board.get_width () - logical.width * 10.0 * board.get_x_scale ()) / 2.0;
	x += logical.width * 5.0 * board.get_x_scale ();
	x += cards.size () / 4.0;
	y = (board.get_height () - logical.height * 5.0 * board.get_y_scale ()) / 2.0;
	y -= cards.size () / 4.0;

	cairo_save (cr);
	cairo_translate (cr, x, y);
	cairo_scale (cr, 5.0 * board.get_x_scale (), 5.0 * board.get_y_scale ());
	rsvg_handle_render_cairo (cards.front().get_handler (), cr);
	cairo_stroke (cr);
	cairo_restore (cr);
}

gboolean on_stack_to_player_cb (GtkWidget *widget, GdkFrameClock *frame_clock, gpointer data)
{
	double xframe, yframe;
	Player& p = player[board.get_turn ()];
	Tile& s = stack_played;

	xframe = p.get_xframe () + (p.get_xsrc () + p.get_xoffset (7) - s.get_x ()) / board.get_framerate ();
	yframe = p.get_yframe () + (p.get_ysrc () + p.get_yoffset (7) - s.get_y ()) / board.get_framerate ();
	p.set_xframe (xframe);
	p.set_yframe (yframe);
	gtk_widget_queue_draw (drawing_area);

	return G_SOURCE_CONTINUE;
}

void StackPlayed::animate (cairo_t *cr)
{
	Player& p = player[board.get_turn ()];
	Tile& s = stack_played;

	if (fabs (s.get_x () + p.get_xframe () - (p.get_xsrc () + p.get_xoffset (7))) < 1.0 &&
	    fabs (s.get_y () + p.get_yframe () - (p.get_ysrc () + p.get_yoffset (7)) < 1.0)) {
		p.set_xframe (0.0);
		p.set_yframe (0.0);
		p.acquire ("stack", stack_played.get_cards().front (), false);
		p.set_extra_card (true);
		p.get_card(7).set_displayed (true);
		board.set_status (STACK_TO_PLAYER_STOP);
		gtk_widget_remove_tick_callback (drawing_area, stack_to_player_tid);
	}

	if (!cards.size ())
		return;
	cairo_save (cr);
	cairo_translate (cr, s.get_x () + p.get_xframe (), s.get_y () + p.get_yframe ());
	cairo_scale (cr, 5.0 * board.get_x_scale (), 5.0 * board.get_y_scale ());
	rsvg_handle_render_cairo (cards.front().get_handler (), cr);
	cairo_stroke (cr);
	cairo_restore (cr);
}

void StackPlayed::clear (cairo_t *cr)
{
	cards.clear ();
}
