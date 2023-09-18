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
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <librsvg/rsvg.h>
#include "Cmdline.h"
#include "Menu.h"
#include "UserInterface.h"
#include "Logic.h"
#include "Board.h"
#include "Player.h"
#include "Deck.h"
#include "Stack.h"
#include "Card.h"

GtkWidget *window;
GtkWidget *drawing_area;
unsigned deck_dist_tid, play_card_tid;
unsigned stack_to_player_tid, deck_to_player_tid;
unsigned player_ending_round_tid;
unsigned menubar_height;
class Cmdline cmdline;
class UserInterface ui;
class Logic logic;
class Board board (800, 500, 1.0, 1.0, 5.0);
class Player player[4];
class Deck deck;
class StackPlayed stack_played;
class Card card[48];

extern gboolean on_play_card_cb (GtkWidget *widget, GdkFrameClock *frame_clock, gpointer data);
extern gboolean on_deck_to_player_cb (GtkWidget *widget, GdkFrameClock *frame_clock, gpointer data);
extern gboolean on_stack_to_player_cb (GtkWidget *widget, GdkFrameClock *frame_clock, gpointer data);
extern gboolean on_player_ending_round_cb (GtkWidget *widget, GdkFrameClock *frame_clock, gpointer data);

bool motion_notify_event (GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	int x, y;
	region_t rgn = { 0 };

	gdk_window_get_device_position (event->window, event->device, &x, &y, nullptr);
	ui.query (x, y, &rgn);

	if (board.get_status () == DECK_DISTRIBUTE) {
		return false;
	} else if (board.get_status () == FINISHING_ROUND_START) {
		return false;
	} else {
		if (rgn.cb == "human::hover" && !player[0].is_locked ()) {
			board.set_status (HUMAN_HOVER);
			player[0].set_selected (rgn.ncard);
		} else if (rgn.cb == "deck::hover" && !deck.is_locked ()) {
			board.set_status (DECK_HOVER);
		} else if (rgn.cb == "stack::hover" && !stack_played.is_locked ()) {
			board.set_status (STACK_HOVER);
		}
		gtk_widget_queue_draw (widget);
	}
	return true;
}

gint draw_cb (GtkWidget *widget, cairo_t *cr, gpointer data)
{
	int i, ncard = 0;
	static bool is_selected = false, set_player = false;
	std::string suit[] = { "clubs", "cups", "golds", "swords" };
	Player& p = player[board.get_turn ()];
	std::set<struct card_st>::const_iterator iter;

	board.paint (cr);
	if (board.get_status () == DECK_DISTRIBUTE)
		for (i = 0; i < 4; i++)
			player[i].animate (cr);

	for (i = 0; i < 7; i++) {
		player[0].draw (cr, i);
		player[1].draw (cr, i);
		player[2].draw (cr, i);
		player[3].draw (cr, i);
		if (i == 6) {
			if (player[0].has_extra_card ())
				player[0].draw (cr, 7);
			else if (player[1].has_extra_card ())
				player[1].draw (cr, 7);
			else if (player[2].has_extra_card ())
				player[2].draw (cr, 7);
			else if (player[3].has_extra_card ())
				player[3].draw (cr, 7);
		}
	}

	if (board.get_status () == PLAY_CARD_STOP)
		return -1;
	deck.draw (cr);
	for (i = 0; i < 4; i++)
		player[i].render_name (cr);
	stack_played.draw (cr);

	if (board.get_status () == PLAY_CARD_START) {
		board.set_framerate (10.0);
		if (board.get_turn () != 0) {
			if (is_selected == false) {
				logic.get_game_combos ();
				logic.rearrange_common_cards ();
				ncard = p.get_ncard_to_play ();
				p.set_selected (ncard & 7);
				is_selected = true;
				p.set_xframe (0.0);
				p.set_yframe (0.0);
			}
		}
		p.play (cr, p.get_selected ());
	} else if (board.get_status () == STACK_TO_PLAYER_START) {
		stack_played.animate (cr);
	} else if (board.get_status () == DECK_TO_PLAYER_START) {
		deck.animate (cr);
	} else if (board.get_status () == FINISHING_ROUND_START) {
		if (!set_player) {
			player_ending_round_tid = gtk_widget_add_tick_callback (GTK_WIDGET (drawing_area), on_player_ending_round_cb, NULL, NULL);
			if (board.get_turn () != 0) {
				ncard = p.get_ncard_to_play ();
				p.set_selected (ncard);
			}
			set_player = true;
		}
		p.finish (cr);
	}

	if (board.get_status () == PLAY_CARD_STOP) {
		if (board.get_turn () == 0) {
			board.set_status (IDLE);
		} else {
			logic.get_game_combos ();
			if (logic.choose_source ()) {
				board.set_status (DECK_TO_PLAYER_START);
				deck_to_player_tid = gtk_widget_add_tick_callback (GTK_WIDGET (drawing_area), on_deck_to_player_cb, NULL, NULL);
			} else {
				board.set_status (STACK_TO_PLAYER_START);
				stack_to_player_tid = gtk_widget_add_tick_callback (GTK_WIDGET (drawing_area), on_stack_to_player_cb, NULL, NULL);
			}
		}
		stack_played.draw (cr);
		p.draw_all (cr);
		is_selected = false;
	} else if (board.get_status () == STACK_TO_PLAYER_STOP) {
		logic.choose_source ();
		logic.get_game_combos ();
		logic.rearrange_common_cards ();

		if (board.get_turn () == 0) {
			board.set_status (IDLE);
		} else {
			if (logic.advise_to_finish ()) {
				board.set_status (FINISHING_ROUND_START);
				gtk_widget_queue_draw (widget);
				return 0;
			} else {
				board.set_status (PLAY_CARD_START);
				play_card_tid = gtk_widget_add_tick_callback (GTK_WIDGET (drawing_area), on_play_card_cb, NULL, NULL);
			}
		}

		stack_played.draw (cr);
		p.draw_all (cr);
	} else if (board.get_status () == DECK_TO_PLAYER_STOP) {
		logic.choose_source ();
		logic.get_game_combos ();
		logic.rearrange_common_cards ();

		if (board.get_turn () == 0) {
			board.set_status (IDLE);
		} else {
			if (logic.advise_to_finish ()) {
				board.set_status (FINISHING_ROUND_START);
				gtk_widget_queue_draw (widget);
				return 0;
			} else {
				board.set_status (PLAY_CARD_START);
				play_card_tid = gtk_widget_add_tick_callback (GTK_WIDGET (drawing_area), on_play_card_cb, NULL, NULL);
			}
		}

		deck.draw (cr);
		p.draw_all (cr);
	} else if (board.get_status () == FINISHING_ROUND_STOP) {
		if (board.get_display_scores ()) {
			p.set_extra_card (false);
			stack_played.set_only_once_value (true);
			stack_played.acquire ("player");
			for (i = 0; i < 4; i++) {
				player[i].show_cards (cr);
				player[i].lock ();
				logic.calc_scores (i);
			}
			ui.display_table_of_scores ();
		} else {
			board.set_display_scores (true);
			is_selected = false;
			set_player = false;
			board.set_reset (true);
			board.new_round (cr);
		}
	} else if (board.get_status () == DECK_HOVER) {
		deck.draw_selector (cr);
	} else if (board.get_status () == STACK_HOVER) {
		stack_played.draw_selector (cr);
	} else if (board.get_status () == HUMAN_HOVER) {
		player[0].draw_selector (cr, player[0].get_selected ());
	}

	return 0;
}

gboolean on_deck_distribute_cb (GtkWidget *widget, GdkFrameClock *frame_clock, gpointer data)
{
	double xframe, yframe;
	Player& p = player[board.get_turn ()];
	Deck& d = deck;

	xframe = p.get_xframe () + (p.get_xsrc () + p.get_xoffset (0) - (d.get_x () + d.get_cards().size () / 4.0)) / board.get_framerate ();
	yframe = p.get_yframe () + (p.get_ysrc () + p.get_yoffset (0) - (d.get_y () - d.get_cards().size () / 4.0)) / board.get_framerate ();
	p.set_xframe (xframe);
	p.set_yframe (yframe);

	gtk_widget_queue_draw (GTK_WIDGET (drawing_area));

	return G_SOURCE_CONTINUE;
}

void size_allocate_cb (GtkWidget *widget, GdkRectangle *r, gpointer data)
{
	gtk_widget_set_size_request (drawing_area, r->width, r->height - menubar_height);
}

int configure_event_cb (GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
	if (!board.get_surface ()) {
		board.set_surface (gdk_window_create_similar_surface (gtk_widget_get_window (widget),
				CAIRO_CONTENT_COLOR,
				gtk_widget_get_allocated_width (widget),
				gtk_widget_get_allocated_height (widget)));
		board.set_cr (cairo_create (board.get_surface ()));
	}

	menubar_height = event->y;
	board.set_width (gtk_widget_get_allocated_width (widget));
	board.set_height (gtk_widget_get_allocated_height (widget));
	board.set_x_scale (board.get_width () / 800.0);
	board.set_y_scale (board.get_height () / 500.0);

	deck.relocate ();
	for (int i = 0; i < 4; i++)
		player[i].conf ();
	player[0].relocate ();

	stack_played.relocate ();
	if (!player[0].has_extra_card ())
		stack_played.unlock ("stack", -1);

	return 0;
}

bool button_press_event_cb (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	Player& p = player[0];

	if (event->type == GDK_BUTTON_PRESS) {
		if (event->button == GDK_BUTTON_PRIMARY) {
			ui.foreach (board.get_cr (), event->x, event->y);
			gtk_widget_queue_draw (widget);
			if (event->type == GDK_BUTTON_RELEASE)
				if (event->type == GDK_BUTTON_PRESS)
					if (event->type == GDK_2BUTTON_PRESS)
						if (event->type == GDK_BUTTON_RELEASE)
							return false;
		} else if (event->button == GDK_BUTTON_SECONDARY) {
			if (board.get_status () == FINISHING_ROUND_START) {
				return false;
			} else if (p.get_cards().size() == 7) {
				return false;
			} else if (logic.get_flexible_ending () == true) {
				board.set_status (FINISHING_ROUND_START);
				gtk_widget_queue_draw (GTK_WIDGET (drawing_area));
			} else {
				if ((p.get_combo_length (0) == 3 && p.get_combo_length (1) == 4) ||
				    (p.get_combo_length (0) == 4 && p.get_combo_length (1) == 3) ||
				    (p.get_combo_length (0) == 7)) {
					board.set_status (FINISHING_ROUND_START);
					gtk_widget_queue_draw (GTK_WIDGET (drawing_area));
				}
			}
		}
	}

	return true;
}

void activate (GtkApplication *app, gpointer user_data)
{
	int i, now;
	std::string name[4] = { _("Human"), "Bot_1", "Bot_2", "Bot_3" };
	std::list<Card>::iterator iter1, iter2;
	std::list<Card> swapped_list;
	GtkBuilder *builder;
	GtkIconTheme *icon_theme;
	GdkPixbuf *pixbuf;
	GError *error = nullptr;
	GResource *res;
	GObject *win, *menuitem;

	icon_theme = gtk_icon_theme_get_default ();
	pixbuf = gtk_icon_theme_load_icon (icon_theme, "chin-chon-lin", 256, (GtkIconLookupFlags) 0, &error);
	if (!pixbuf)
		g_error_free (error);
	else
		g_object_unref (pixbuf);

	res = g_resource_load (CHIN_CHON_LIN_DATADIR "data/ui/ui.gresource", &error);
	g_resources_register (res);
	builder = gtk_builder_new_from_resource ("/org/gtk/chin-chon-lin/main-window.ui");
	g_resources_unregister (res);
	/*
	 * FIXME: We use @win for getting the whole building info for "main-window",
	 * but this variable cannot be used to create the application window (that is
	 * achieved using @window variable). However, and after have tried every single
	 * permutation between @win and @window a few lines below it, you will not
	 * be able to close the window by the traditional way by clicking the WM's
	 * close button. Doing so, the program will remain running on background and
	 * future invocations from your graphical desktop manager will yield program
	 * misbehaviors and impossible user interaction.
	 *
	 * You can still either make a clean closing of the window by choosing
	 * File->Quit from the menu bar or executing the game from the command line
	 * and finishing its runtime execution by pressing ^C. In the unusual case
	 * that the game is even alive, you can run "killall chin-chon-lin" and it
	 * should guarantee to you that there's no more instances of the game on memory.
	 */
	win = gtk_builder_get_object (builder, "main-window");
	window = gtk_application_window_new (app);

	menuitem = gtk_builder_get_object (builder, "table-of-scores");
	g_signal_connect (GTK_WIDGET (menuitem), "activate", G_CALLBACK (show_table_of_scores_cb), NULL);
	menuitem = gtk_builder_get_object (builder, "preferences");
	g_signal_connect (GTK_WIDGET (menuitem), "activate", G_CALLBACK (show_preferences_cb), NULL);
	menuitem = gtk_builder_get_object (builder, "quit");
	g_signal_connect (GTK_WIDGET (menuitem), "activate", G_CALLBACK (quit_cb), NULL);
	menuitem = gtk_builder_get_object (builder, "rules-and-how-to-play");
	g_signal_connect (GTK_WIDGET (menuitem), "activate", G_CALLBACK (show_rules_cb), NULL);
	menuitem = gtk_builder_get_object (builder, "about");
	g_signal_connect (GTK_WIDGET (menuitem), "activate", G_CALLBACK (about_cb), NULL);

	/*
	 * See the fixme explained above
	 */
	g_signal_connect (window, "destroy", G_CALLBACK (gtk_widget_destroy), &window);
	g_signal_connect (GTK_WIDGET (win), "size-allocate", G_CALLBACK (size_allocate_cb), NULL);
	drawing_area = GTK_WIDGET (gtk_builder_get_object (builder, "drawing-area-id"));
	g_signal_connect (drawing_area, "configure-event", G_CALLBACK (configure_event_cb), NULL);
	g_signal_connect (drawing_area, "button-press-event", G_CALLBACK (button_press_event_cb), NULL);
	g_signal_connect (drawing_area, "motion-notify-event", G_CALLBACK (motion_notify_event), NULL);
	g_signal_connect (drawing_area, "draw", G_CALLBACK (draw_cb), NULL);
	gtk_widget_set_events (drawing_area, (GdkEventMask) gtk_widget_get_events (drawing_area)
					| GDK_BUTTON_PRESS_MASK
					| GDK_BUTTON_RELEASE_MASK
					| GDK_POINTER_MOTION_MASK);
	now = time (NULL);
	srand (now);
	std::cout << "now = " << now << std::endl;
	if (!cmdline.is_testing_file ()) {
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
	} else {
		for (i = 0, iter1 = swapped_list.begin (); iter1 != swapped_list.end (); iter1++, i++)
			std::cout << std::setw (2) << std::setfill (' ') << i <<
				_(": suit = ") << iter1->get_suit () <<
				_(", number = ") << iter1->get_number () <<
				std::endl;
	}
	deck.unlock ("deck", -1);

	for (i = 0; i < 4; i++) {
		player[i].init (i, name[i]);
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

	board.set_display_scores (true);
	board.set_status (DECK_DISTRIBUTE);
	board.new_game ();
	deck_dist_tid = gtk_widget_add_tick_callback (drawing_area, on_deck_distribute_cb, NULL, NULL);
	gtk_widget_show_all (GTK_WIDGET (win));
}

int main (int argc, char **argv)
{
	GtkApplication *app;
	int status;
	std::string lang;

	lang = ui.get_language ();
	setenv ("LANGUAGE", lang.c_str (), true);
	setlocale (LC_ALL, lang.c_str ());
	bindtextdomain (PACKAGE, LOCALEDIR);
	textdomain (PACKAGE);

	cmdline.parse_cmdline_options (&argc, &argv);
	app = gtk_application_new ("org.gtk.chin-chon-lin", G_APPLICATION_FLAGS_NONE);
	g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);

	return status;
}
