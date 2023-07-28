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
#include <sstream>
#include <math.h>
#include <list>
#include <gtk/gtk.h>
#include <cairo.h>
#include <librsvg/rsvg.h>
#include "UserInterface.h"
#include "Board.h"
#include "Player.h"
#include "Stack.h"
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
extern GtkWidget *drawing_area;
extern unsigned deck_to_player_tid;

Deck::Deck ()
{
	gfile = g_file_new_for_path (CHIN_CHON_LIN_DATADIR "data/tiles/deck-ar.svg");
	handler = rsvg_handle_new_from_gfile_sync (gfile, RSVG_HANDLE_FLAGS_NONE, NULL, NULL);
	rsvg_handle_set_dpi (handler, 300.0);
	rsvg_handle_get_geometry_for_layer (handler, "#deck", &viewport, NULL, &logical, NULL);
	relocate ();
}

Deck::~Deck ()
{
	this->cards.clear ();
}

std::list<Card>& Deck::get_cards ()
{
	return cards;
}

std::string Deck::get_cc () const
{
	return cc;
}

void Deck::set_cards (std::list<Card>& cards)
{
	this->cards = cards;
}

void Deck::set_cc (std::string cc)
{
	std::ostringstream filename;

	filename << CHIN_CHON_LIN_DATADIR << "data/tiles/";
	if (cc == "es")
		filename << "deck-es.svg";
	else if (cc == "gb")
		filename << "deck-gb.svg";
	else if (cc == "us")
		filename << "deck-us.svg";
	else
		filename << "deck-ar.svg";

	this->cc = cc;

	gfile = g_file_new_for_path (filename.str().c_str ());
	handler = rsvg_handle_new_from_gfile_sync (gfile, RSVG_HANDLE_FLAGS_NONE, NULL, NULL);
	rsvg_handle_set_dpi (handler, 300.0);
	rsvg_handle_get_geometry_for_layer (handler, "#deck", &viewport, NULL, &logical, NULL);
	relocate ();
}

/*
 * Provide cards randomly sorted to the deck
 */
void Deck::acquire (Card& card)
{
	cards.push_back (card);
}

/*
 * Update deck coords when the window is resized
 */
void Deck::relocate ()
{
	x = (board.get_width () - logical.width * 10.0 * board.get_x_scale ()) / 2.0 - 10.0;
	y = (board.get_height () - logical.height * 5.0 * board.get_y_scale ()) / 2.0;
	w = logical.width * 5.0 * board.get_x_scale ();
	h = logical.height * 5.0 * board.get_y_scale ();
}

void Deck::draw (cairo_t *cr)
{
	if (cards.size () == 0) {
		deck.draw_empty (cr);
		return;
	}

	// Update depth of the deck as players acquire cards from the deck. It can
	// be seen as a size-decreasing deck in the middle of the window
	for (unsigned long i = 0; i < cards.size (); i += 4) {
		cairo_save (cr);
		cairo_translate (cr, x + i / 4, y - i / 4);
		cairo_scale (cr, 5.0 * board.get_x_scale (), 5.0 * board.get_y_scale ());
		rsvg_handle_render_cairo (handler, cr);
		cairo_stroke (cr);
		cairo_restore (cr);
	}
}

/*
 * Only for a moment, if you don't move your mouse when the deck
 * has no more cards to be acquiring for the players, you will see
 * a big white "X" in its coordinates
 */
void Deck::draw_empty (cairo_t *cr)
{
	int i, n;

	std::list<Card> swapped_list;
	std::list<Card>::iterator iter1, iter2;
	StackPlayed& s = stack_played;

	cairo_save (cr);
	cairo_set_source_rgb (cr, 0.2, 0.3, 0.2);
	cairo_rectangle (cr, x, y, logical.width * 5.0 * board.get_x_scale (),
					logical.height * 5.0 * board.get_y_scale ());
	cairo_scale (cr, 5.0 * board.get_x_scale (), 5.0 * board.get_y_scale ());
	cairo_fill (cr);
	cairo_restore (cr);

	cairo_save (cr);
	cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
	cairo_rectangle (cr, x, y, logical.width * 5.0 * board.get_x_scale (),
					logical.height * 5.0 * board.get_y_scale ());
	cairo_scale (cr, 5.0 * board.get_x_scale (), 5.0 * board.get_y_scale ());
	cairo_set_line_width (cr, 0.5);
	cairo_stroke (cr);
	cairo_restore (cr);

	cairo_save (cr);
	cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
	cairo_set_line_width (cr, 3.0);
	cairo_move_to (cr, x, y);
	cairo_line_to (cr, x + logical.width * 5.0 * board.get_x_scale (),
				y + logical.height * 5.0 * board.get_y_scale ());
	cairo_stroke (cr);
	cairo_restore (cr);

	cairo_save (cr);
	cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
	cairo_set_line_width (cr, 3.0);
	cairo_move_to (cr, x + logical.width * 5.0 * board.get_x_scale (), y);
	cairo_line_to (cr, x, y + logical.height * 5.0 * board.get_y_scale ());
	cairo_stroke (cr);
	cairo_restore (cr);

	swapped_list = s.get_cards ();
	for (iter1 = swapped_list.begin (); iter1 != swapped_list.end (); iter1++) {
		n = rand () % s.get_cards().size ();
		for (i = 0, iter2 = iter1; iter2 != swapped_list.end (); iter2++, i++) {
			if (i == n)
				std::iter_swap (iter1, iter2);
		}
	}
	deck.set_cards (swapped_list);
	s.clear (cr);
}

/*
 * Update X and Y frames asynchronously every time the @frame_clock changes.
 * This is the callback function activated when a player gets one card from the deck
 */
gboolean on_deck_to_player_cb (GtkWidget *widget, GdkFrameClock *frame_clock, gpointer data)
{
	double xframe, yframe;
	Player& p = player[board.get_turn ()];
	Tile& d = deck;

	xframe = p.get_xframe () + (p.get_xsrc () + p.get_xoffset (7) - d.get_x ()) / board.get_framerate ();
	yframe = p.get_yframe () + (p.get_ysrc () + p.get_yoffset (7) - d.get_y ()) / board.get_framerate ();
	p.set_xframe (xframe);
	p.set_yframe (yframe);

	gtk_widget_queue_draw (drawing_area);

	return G_SOURCE_CONTINUE;
}

void Deck::animate (cairo_t *cr)
{
	Player& p = player[board.get_turn ()];
	Tile& d = deck;

	if (fabs (d.get_x () + p.get_xframe () - (p.get_xsrc () + p.get_xoffset (7))) < 1.0 &&
	    fabs (d.get_y () + p.get_yframe () - (p.get_ysrc () + p.get_yoffset (7)) < 1.0)) {
		p.set_xframe (0.0);
		p.set_yframe (0.0);
		p.acquire ("deck", deck.get_cards().front (), false);
		p.set_extra_card (true);
		p.get_card(7).set_displayed (true);
		board.set_status (DECK_TO_PLAYER_STOP);
		gtk_widget_remove_tick_callback (drawing_area, deck_to_player_tid);
	}

	if (!cards.size ())
		return;
	cairo_save (cr);
	cairo_translate (cr, d.get_x () + p.get_xframe (), d.get_y () + p.get_yframe ());
	cairo_scale (cr, 5.0 * board.get_x_scale (), 5.0 * board.get_y_scale ());
	rsvg_handle_render_cairo (cards.front().get_handler (), cr);
	cairo_stroke (cr);
	cairo_restore (cr);
}
