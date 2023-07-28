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
#include <string>
#include <gtk/gtk.h>
#include <cairo.h>
#include "Cmdline.h"
#include "Logic.h"
#include "UserInterface.h"
#include "Player.h"
#include "Deck.h"
#include "Stack.h"
#include "Card.h"

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

extern GtkWidget *window;
extern GtkWidget *drawing_area;
extern unsigned deck_dist_tid, play_card_tid;
extern unsigned stack_to_player_tid, deck_to_player_tid;
extern unsigned player_ending_round_tid;
extern class Cmdline cmdline;
extern class Board board;
extern class Player player[4];
extern class Deck deck;
extern class StackPlayed stack_played;
extern gboolean on_deck_distribute_cb (GtkWidget *widget, GdkFrameClock *frame_clock, gpointer data);

Board::Board (int w, int h, double xs, double ys, double frames)
{
	turn = 0;
	width = w;
	height = h;
	x_scale = xs;
	y_scale = ys;
	framerate = frames;
	surface = nullptr;
	cr = nullptr;
}

Board::~Board ()
{
}

int Board::get_turn () const
{
	return turn;
}

status_t Board::get_status () const
{
	return status;
}

int Board::get_width () const
{
	return width;
}

int Board::get_height () const
{
	return height;
}

double Board::get_x_scale () const
{
	return x_scale;
}

double Board::get_y_scale () const
{
	return y_scale;
}

double Board::get_framerate () const
{
	return framerate;
}

cairo_surface_t *Board::get_surface () const
{
	return surface;
}

cairo_t *Board::get_cr () const
{
	return cr;
}

bool Board::get_display_scores () const
{
	return display_scores;
}

bool Board::get_reset () const
{
	return reset;
}

void Board::set_status (status_t status)
{
	this->status = status;
}

void Board::set_turn (int turn)
{
	this->turn = turn;
}

void Board::set_width (int width)
{
	this->width = width;
}

void Board::set_height (int height)
{
	this->height = height;
}

void Board::set_x_scale (double xs)
{
	x_scale = xs;
}

void Board::set_y_scale (double ys)
{
	y_scale = ys;
}

void Board::set_framerate (double f)
{
	framerate = f;
}

void Board::set_surface (cairo_surface_t *surface)
{
	this->surface = surface;
}

void Board::set_cr (cairo_t *cr)
{
	this->cr = cr;
}

void Board::set_display_scores (bool display_scores)
{
	this->display_scores = display_scores;
}

void Board::set_reset (bool reset)
{
	this->reset = reset;
}

void Board::paint (cairo_t *cr)
{
	struct _GdkRGBA bgcolor = { 0.2, 0.3, 0.2, 1.0 };

	cairo_save (cr);
	gdk_cairo_set_source_rgba (cr, &bgcolor);
	cairo_paint (cr);
	cairo_restore (cr);
}

void Board::new_game ()
{
	std::string name[4] = { _("Human"), "Bot_1", "Bot_2", "Bot_3" };
	GSettings *settings;
	GVariant *v;

	settings = g_settings_new ("org.gtk.chin-chon-lin");

	v = g_settings_get_value (settings, "your-name");
	player[0].set_name (*(char **) g_variant_get_data_as_bytes (v));
	v = g_settings_get_value (settings, "total-points");
	logic.set_max_total_points (*(int *) g_variant_get_data (v));
	v = g_settings_get_value (settings, "flex-end");
	logic.set_flexible_ending (*(int *) g_variant_get_data (v));
	v = g_settings_get_value (settings, "deck-pixbuf");
	deck.set_cc (*(char **) g_variant_get_data_as_bytes (v));

	for (int i = 0; i < 4; i++) {
		if (i)
			player[i].set_name (name[i]);
		player[i].set_round_pts (0);
		player[i].set_total_pts (0);
	}
}

void Board::new_round (cairo_t *cr)
{
	int i;
	std::string name[4] = { _("Human"), "Bot_1", "Bot_2", "Bot_3" };
	std::list<Card>::iterator iter1, iter2;
	std::list<Card> swapped_list;
	static bool done = false;

	if (reset)
		done = false;

	if (done)
		return;

	turn = 0;
	ui.clear_all ();
	deck.get_cards().clear ();
	stack_played.get_cards().clear ();
	for (i = 0; i < 48; i++) {
		card[i].init (i / 12, i % 12 + 1);
		deck.acquire (card[i]);
	}
	swapped_list = deck.get_cards ();
	for (iter1 = swapped_list.begin (); iter1 != swapped_list.end (); iter1++) {
		int n = rand () % 48;
		for (i = 0, iter2 = iter1; iter2 != swapped_list.end (); iter2++, i++)
			if (i == n)
				std::iter_swap (iter1, iter2);
	}
	deck.set_cards (swapped_list);
	deck.unlock ("deck", -1);

	for (i = 0; i < 4; i++) {
		player[i].clear (0);
		player[i].clear (1);
		player[i].render_name (cr);
		player[i].get_cards().clear ();
		player[i].set_xframe (0.0);
		player[i].set_yframe (0.0);
		if (i)
			player[i].init (i, name[i]);
		else
			player[i].init (i, player[i].get_name ());
		player[i].conf ();
	}
	player[0].set_locked (true);
	player[0].lock ();

	for (i = 0; i < 7; i++) {
		player[0].acquire ("deck", deck.get_cards().front (), false);
		player[1].acquire ("deck", deck.get_cards().front (), false);
		player[2].acquire ("deck", deck.get_cards().front (), false);
		player[3].acquire ("deck", deck.get_cards().front (), false);
	}
	stack_played.set_only_once_value (false);
	stack_played.acquire ("deck");

	board.paint (cr);
	for (i = 0; i < 7; i++) {
		player[0].get_card(i).set_displayed (true);
		player[0].draw (cr, i);
		player[1].get_card(i).set_displayed (true);
		player[1].draw (cr, i);
		player[2].get_card(i).set_displayed (true);
		player[2].draw (cr, i);
		player[3].get_card(i).set_displayed (true);
		player[3].draw (cr, i);
	}

	deck.draw (cr);
	stack_played.draw (cr);
	stack_played.unlock ("stack", -1);
	board.set_status (IDLE);
	done = true;
}
