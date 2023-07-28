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
#include <iostream>
#include <iomanip>
#include <sstream>
#include "gettext.h"
#define _(String) gettext (String)
#include <math.h>
#include <list>
#include <gtk/gtk.h>
#include <cairo.h>
#include "Logic.h"
#include "Board.h"
#include "Player.h"
#include "Tile.h"
#include "Stack.h"
#include "Deck.h"
#include "Card.h"

typedef struct region_st {
	double x0;
	double y0;
	double x1;
	double y1;
	std::string cb;
	int ncard;
} region_t;

class UserInterface {
public:
	UserInterface ();
	UserInterface (UserInterface&) = delete;
	UserInterface (UserInterface&&) = delete;
	UserInterface& operator= (UserInterface&) = delete;
	~UserInterface ();

	std::string get_language () const;
	void set_language (std::string lang);

	void display_table_of_scores ();
	void query (double x, double y, region_t *ret_region);
	void register_region (double x0, double y0, double x1, double y1, std::string cb, int ncard);
	void unregister_region (double x0, double y0, double x1, double y1, std::string cb, int ncard);
	void clear_all ();
	void foreach (cairo_t *cr, double x, double y);
private:
	std::string lang;
	std::list<region_t> region;
};

extern GtkWidget *window, *drawing_area;
extern unsigned play_card_tid, stack_to_player_tid, deck_to_player_tid;
extern gboolean on_play_card_cb (GtkWidget *widget, GdkFrameClock *frame_clock, gpointer data);
extern gboolean on_deck_to_player_cb (GtkWidget *widget, GdkFrameClock *frame_clock, gpointer data);
extern gboolean on_stack_to_player_cb (GtkWidget *widget, GdkFrameClock *frame_clock, gpointer data);

UserInterface::UserInterface ()
{
}

UserInterface::~UserInterface ()
{
}

std::string UserInterface::get_language () const
{
	GVariant *v;
	std::string lang;

	GSettings *settings = g_settings_new ("org.gtk.chin-chon-lin");

	v = g_settings_get_value (settings, "language");
	lang = std::string (*(char **) g_variant_get_data_as_bytes (v));

        if (lang.find ("(US)") != std::string::npos || lang.find ("Estados Unidos") != std::string::npos)
                return "en_US";
        else if (lang.find ("Argentina") != std::string::npos)
                return "es_AR";
	else
		return "en_US";
}

void UserInterface::set_language (std::string lang)
{
	this->lang = lang;
	setlocale (LC_ALL, lang.c_str ());
	bindtextdomain (PACKAGE, LOCALEDIR);
	textdomain (PACKAGE);
}

void close_scores (GtkWidget *widget)
{
	if (board.get_display_scores ()) {
		gtk_window_close (GTK_WINDOW (widget));
		board.set_display_scores (false);
		for (int i = 0; i < 4; i++)
			player[i].set_points (false);
	}
}

typedef struct total_st {
	bool operator< (const total_st& t2) const;
	int nplayer;
	int points;
} total_t;

bool total_st::operator< (const total_st& t2) const
{
	if (this->points < t2.points)
		return true;
	return false;
}

void UserInterface::display_table_of_scores ()
{
	GObject *dialog, *button, *label;
	GtkBuilder *builder;
	GError *error = nullptr;
	GResource *res;
	static bool unique_instance = false;
	total_t my_struct;
	std::list<total_t> game_total;
	std::list<total_t>::const_iterator iter;
	std::ostringstream id, name, round, total;

	if (board.get_reset ())
		unique_instance = false;

	if (unique_instance)
		return;

	res = g_resource_load (CHIN_CHON_LIN_DATADIR "data/ui/ui.gresource", &error);
	g_resources_register (res);
	builder = gtk_builder_new_from_resource ("/org/gtk/chin-chon-lin/scores.ui");
	g_resources_unregister (res);
	dialog = gtk_builder_get_object (builder, "scores-dialog");

	for (int i = 0; i < 4; i++) {
		name << player[i].get_name ();
		id << "player-" << i << "-name";
		label = gtk_builder_get_object (builder, id.str().c_str ());
		gtk_label_set_text (GTK_LABEL (label), name.str().c_str ());
		name.str ("");
		name.clear ();
		id.str ("");
		id.clear ();

		round << player[i].get_round_pts ();
		id << "player-" << i << "-round";
		label = gtk_builder_get_object (builder, id.str().c_str ());
		gtk_label_set_text (GTK_LABEL (label), round.str().c_str ());
		round.str ("");
		round.clear ();
		id.str ("");
		id.clear ();

		total << player[i].get_total_pts ();
		id << "player-" << i << "-total";
		label = gtk_builder_get_object (builder, id.str().c_str ());
		gtk_label_set_text (GTK_LABEL (label), total.str().c_str ());
		total.str ("");
		total.clear ();
		id.str ("");
		id.clear ();

		my_struct.nplayer = i;
		my_struct.points = player[i].get_total_pts ();
		game_total.push_back (my_struct);
	}

	game_total.sort ();
	for (iter = game_total.cbegin (); iter != game_total.cend (); iter++) {
		if (iter->points > logic.get_max_total_points ()) {
			label = gtk_builder_get_object (builder, "winner-label");
			gtk_label_set_text (GTK_LABEL (label), player[game_total.cbegin()->nplayer].get_name().c_str ());
			label = gtk_builder_get_object (builder, "winner-msg");
			gtk_label_set_text (GTK_LABEL (label), _("won !!!"));
			board.new_game ();
			break;
		} else {
			label = gtk_builder_get_object (builder, "winner-label");
			gtk_label_set_text (GTK_LABEL (label), "");
			label = gtk_builder_get_object (builder, "winner-msg");
			gtk_label_set_text (GTK_LABEL (label), "");
		}
	}

	button = gtk_builder_get_object (builder, "close-button");
	g_signal_connect_swapped (button, "clicked", G_CALLBACK (close_scores), dialog);
	gtk_widget_show_all (GTK_WIDGET (dialog));
	unique_instance = true;
	board.set_reset (false);
	game_total.clear ();
}

void UserInterface::query (double x, double y, region_t *ret_region)
{
	std::list<region_t>::const_iterator iter;

	if (!ret_region)
		return;

	for (iter = region.cbegin (); iter != region.cend (); iter++) {
		if (x > iter->x0 && x < iter->x1 && y > iter->y0 && y < iter->y1) {
			ret_region->x0 = iter->x0;
			ret_region->y0 = iter->y0;
			ret_region->x1 = iter->x1;
			ret_region->y1 = iter->y1;
			ret_region->cb = iter->cb;
			ret_region->ncard = iter->ncard;
			return;
		}
	}
}

void UserInterface::register_region (double x0, double y0, double x1, double y1, std::string cb, int ncard)
{
	std::list<region_t>::const_iterator iter;
	region_t rectangle = { x0, y0, x1, y1, cb, ncard };

	for (iter = region.cbegin (); iter != region.cend (); iter++) {
		if (fabs (iter->x0 - x0) < 2.0 && fabs (iter->y0 - y0) < 2.0 &&
		    fabs (iter->x1 - x1) < 2.0 && fabs (iter->y1 - y1) < 2.0 &&
		    iter->cb == cb && iter->ncard == ncard)
			return;
	}

	region.push_front (rectangle);
}

void UserInterface::unregister_region (double x0, double y0, double x1, double y1, std::string cb, int ncard)
{
	std::list<region_t>::iterator iter;

	for (iter = region.begin (); iter != region.end (); iter++) {
		if (fabs (iter->x0 - x0) < 2.0 && fabs (iter->y0 - y0) < 2.0 &&
		    fabs (iter->x1 - x1) < 2.0 && fabs (iter->y1 - y1) < 2.0 &&
		    iter->cb == cb && iter->ncard == ncard) {
			region.erase (iter);
			break;
		}
	}
}

void UserInterface::clear_all ()
{
	region.clear ();
}

void UserInterface::foreach (cairo_t *cr, double x, double y)
{
	unsigned long i;
	std::list<region_t>::iterator iter;

	for (i = 0, iter = region.begin (); i < region.size (); iter++, i++) {
		if (x > iter->x0 && x < iter->x1 && y > iter->y0 && y < iter->y1) {
			if (iter->cb == "deck::hover") {
				if (deck.get_cards().size () == 0)
					return;
				player[0].set_xframe (0.0);
				player[0].set_yframe (0.0);
				deck.draw_selector (cr);
				deck.set_locked (true);
				deck.lock ("deck", -1);
				stack_played.set_locked (true);
				stack_played.lock ("stack", -1);
				player[0].set_locked (false);
				player[0].unlock ();
				player[0].set_selected (iter->ncard);
				board.set_status (DECK_TO_PLAYER_START);
				deck_to_player_tid = gtk_widget_add_tick_callback (drawing_area, on_deck_to_player_cb, NULL, NULL);
				break;
			} else if (iter->cb == "stack::hover") {
				if (stack_played.get_cards().size () == 0)
					return;
				player[0].set_xframe (0.0);
				player[0].set_yframe (0.0);
				deck.set_locked (true);
				deck.lock ("deck", -1);
				stack_played.set_locked (true);
				stack_played.lock ("stack", -1);
				player[0].set_locked (false);
				player[0].unlock ();
				player[0].set_selected (iter->ncard);
				board.set_status (STACK_TO_PLAYER_START);
				stack_to_player_tid = gtk_widget_add_tick_callback (drawing_area, on_stack_to_player_cb, NULL, NULL);
				break;
			} else if (iter->cb == "human::hover") {
				if (player[0].get_cards().size () < 7)
					return;
				player[0].set_extra_card (false);
				player[0].set_selected (iter->ncard);
				board.set_status (PLAY_CARD_START);
				play_card_tid = gtk_widget_add_tick_callback (drawing_area, on_play_card_cb, NULL, NULL);
				player[0].play (cr, iter->ncard);
				deck.set_locked (false);
				deck.unlock ("deck", -1);
				stack_played.set_locked (false);
				stack_played.unlock ("stack", -1);
				player[0].set_locked (true);
				player[0].lock ();
				break;
			}
		}
	}
}
