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
#include <gtk/gtk.h>
#include "UserInterface.h"
#include "Logic.h"
#include "Board.h"
#include "Player.h"
#include "Deck.h"

static GtkWidget *entry;
static GtkWidget *spin_button_1, *spin_button_2;
static GtkWidget *button, *combobox;
static GtkWidget *rb_group, *rb2, *rb3, *rb4;
static GtkBuilder *builder;
static GSettings *settings;
static std::string cc[4] = { "ar", "es", "gb", "us" };
static int n_deck[4];

extern GtkWidget *window;

static void button_clicked_cb (GtkButton *button, gpointer user_data)
{
	g_settings_set_string (settings, "deck-pixbuf", cc[*(int *) user_data].c_str ());
}

void show_table_of_scores_cb ()
{
	board.set_reset (true);
	ui.display_table_of_scores ();
	board.set_reset (false);
}

static void prefs_reload_default_settings (gpointer dialog)
{
	GVariant *v;

	settings = g_settings_new ("org.gtk.chin-chon-lin");

	v = g_settings_get_default_value (settings, "your-name");
	g_settings_set_string (settings, "your-name", *(char **) g_variant_get_data_as_bytes (v));
	v = g_settings_get_default_value (settings, "total-points");
	g_settings_set_int (settings, "total-points", *(int *) g_variant_get_data (v));
	v = g_settings_get_default_value (settings, "flex-end");
	g_settings_set_int (settings, "flex-end", *(int *) g_variant_get_data (v));
	v = g_settings_get_default_value (settings, "language");
	g_settings_set_string (settings, "language", *(char **) g_variant_get_data_as_bytes (v));
	v = g_settings_get_default_value (settings, "deck-pixbuf");
	g_settings_set_string (settings, "deck-pixbuf", *(char **) g_variant_get_data_as_bytes (v));
}

static void prefs_discard_changes (gpointer dialog)
{
	gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void prefs_apply_changes (gpointer dialog)
{
	GVariant *v;
	std::string lang;

	settings = g_settings_new ("org.gtk.chin-chon-lin");

	v = g_settings_get_value (settings, "your-name");
	player[0].set_name (*(char **) g_variant_get_data_as_bytes (v));
	v = g_settings_get_value (settings, "total-points");
	logic.set_max_total_points (*(int *) g_variant_get_data (v));
	v = g_settings_get_value (settings, "flex-end");
	logic.set_flexible_ending (*(int *) g_variant_get_data (v));
	v = g_settings_get_value (settings, "language");
	lang = *(char **) g_variant_get_data_as_bytes (v);

	/*
	 * This stuff is a little bit tricky... I think we should add an attribute to the XML
	 * UI file which describes universally each country having translations in a common
	 * UTF-8 charset. It could be something like "lang_cc", getting values such as "en_US"
	 * for English spoken in United States of America or "es_AR" for the Castilian Spanish
	 * spoken in Argentina
	 *
	 * "<item id="English (US)" lang_cc="en_US" translatable="yes">English (US)</item>"
	 */
	if (lang.find ("(US)") != std::string::npos || lang.find ("(Estados Unidos)") != std::string::npos)
		ui.set_language ("en_US");
	else if (lang.find ("(Argentina)") != std::string::npos)
		ui.set_language ("es_AR");

	v = g_settings_get_value (settings, "deck-pixbuf");
	deck.set_cc (*(char **) g_variant_get_data_as_bytes (v));

	gtk_widget_destroy (GTK_WIDGET (dialog));
}

void show_preferences_cb ()
{
	int i;
	GObject *dialog;
	GError *error = nullptr;
	GResource *res;
	std::ostringstream filename;

	res = g_resource_load (CHIN_CHON_LIN_DATADIR "data/ui/ui.gresource", &error);
	g_resources_register (res);
	builder = gtk_builder_new_from_resource ("/org/gtk/chin-chon-lin/preferences.ui");
	g_resources_unregister (res);

	dialog = gtk_builder_get_object (builder, "preferences-dialog");
	entry = GTK_WIDGET (gtk_builder_get_object (builder, "name-entry"));
	spin_button_1 = GTK_WIDGET (gtk_builder_get_object (builder, "total-points"));
	spin_button_2 = GTK_WIDGET (gtk_builder_get_object (builder, "flex-end"));
	combobox = GTK_WIDGET (gtk_builder_get_object (builder, "language"));

	for (i = 0; i < 4; i++)
		n_deck[i] = i;

	rb_group = GTK_WIDGET (gtk_builder_get_object (builder, "deck-ar-rb"));
	g_signal_connect (rb_group, "clicked", G_CALLBACK (button_clicked_cb), &n_deck[0]);
	rb2 = GTK_WIDGET (gtk_builder_get_object (builder, "deck-es-rb"));
	gtk_radio_button_join_group (GTK_RADIO_BUTTON (rb2), GTK_RADIO_BUTTON (rb_group));
	g_signal_connect (rb2, "clicked", G_CALLBACK (button_clicked_cb), &n_deck[1]);
	rb3 = GTK_WIDGET (gtk_builder_get_object (builder, "deck-gb-rb"));
	gtk_radio_button_join_group (GTK_RADIO_BUTTON (rb3), GTK_RADIO_BUTTON (rb_group));
	g_signal_connect (rb3, "clicked", G_CALLBACK (button_clicked_cb), &n_deck[2]);
	rb4 = GTK_WIDGET (gtk_builder_get_object (builder, "deck-us-rb"));
	gtk_radio_button_join_group (GTK_RADIO_BUTTON (rb4), GTK_RADIO_BUTTON (rb_group));
	g_signal_connect (rb4, "clicked", G_CALLBACK (button_clicked_cb), &n_deck[3]);

	button = GTK_WIDGET (gtk_builder_get_object (builder, "clear-button"));
	g_signal_connect_swapped (button, "clicked", G_CALLBACK (prefs_reload_default_settings), dialog);
	button = GTK_WIDGET (gtk_builder_get_object (builder, "cancel-button"));
	g_signal_connect_swapped (button, "clicked", G_CALLBACK (prefs_discard_changes), dialog);
	button = GTK_WIDGET (gtk_builder_get_object (builder, "ok-button"));
	g_signal_connect_swapped (button, "clicked", G_CALLBACK (prefs_apply_changes), dialog);

	settings = g_settings_new ("org.gtk.chin-chon-lin");
	g_settings_bind (settings, "your-name", entry, "text", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind (settings, "total-points", spin_button_1, "value", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind (settings, "flex-end", spin_button_2, "value", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind (settings, "language", combobox, "active-id", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind (settings, "deck-pixbuf", rb_group, "active-id", G_SETTINGS_BIND_DEFAULT);

	gtk_widget_show_all (GTK_WIDGET (dialog));
}

void quit_cb ()
{
	gtk_widget_destroy (window);
}

void show_rules_cb ()
{
	GtkWidget *rules_window, *stack, *sw, *view, *image;
	GtkTextBuffer *buffer;
	GtkTextIter iter, match_start, match_end;
	GResource *res;
	GError *error = nullptr;
	GBytes *bytes;
	std::ostringstream res_path;
	std::string text_before_image;
	const gchar *markup;
	GdkPixbuf *pixbuf;

	rules_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (rules_window), _("Rules and how to play"));
	gtk_window_set_default_size (GTK_WINDOW (rules_window), 640, 480);
	g_signal_connect (rules_window, "destroy", G_CALLBACK (gtk_widget_destroyed), &rules_window);

	stack = gtk_stack_new ();
	gtk_container_add (GTK_CONTAINER (rules_window), stack);

	view = gtk_text_view_new ();
	gtk_text_view_set_editable (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), GTK_WRAP_WORD);
	gtk_text_view_set_left_margin (GTK_TEXT_VIEW (view), 10);
	gtk_text_view_set_right_margin (GTK_TEXT_VIEW (view), 10);

	sw = gtk_scrolled_window_new (nullptr, nullptr);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (sw), view);
	gtk_container_add (GTK_CONTAINER (stack), sw);

	if (ui.get_language () == "en_US") {
		res_path << CHIN_CHON_LIN_DATADIR << "data/help/C/rules.gresource";
		text_before_image = "Note that the suit of each card is meaningless";
	} else if (ui.get_language () == "es_AR") {
		res_path << CHIN_CHON_LIN_DATADIR << "data/help/es_AR/rules.gresource";
		text_before_image = "Fijate que el palo de cada carta es insignificante";
	}
	res = g_resource_load (res_path.str().c_str (), &error);
	g_resources_register (res);
	bytes = g_resources_lookup_data ("/org/gtk/chin-chon-lin/rules.txt", (GResourceLookupFlags) 0, NULL);
	markup = (const gchar *) g_bytes_get_data (bytes, NULL);

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
	gtk_text_buffer_get_start_iter (buffer, &iter);
	gtk_text_buffer_insert_markup (buffer, &iter, markup, -1);
	gtk_text_iter_backward_search (&iter,
					text_before_image.c_str (),
					GTK_TEXT_SEARCH_TEXT_ONLY,
					&match_start,
					&match_end,
					nullptr);
	gtk_text_iter_forward_char (&match_end);
	gtk_text_iter_backward_cursor_position (&match_end);
	image = gtk_image_new_from_resource ("/org/gtk/chin-chon-lin/chin-chon-lin_screenshot.png");
	pixbuf = gtk_image_get_pixbuf (GTK_IMAGE (image));
	gtk_text_buffer_insert_pixbuf (buffer, &match_end, pixbuf);

	g_bytes_unref (bytes);
	g_resources_unregister (res);
	gtk_widget_show_all (rules_window);
}

void about_cb ()
{
	GtkWidget *dialog, *image;
	GdkPixbuf *logo;
	GResource *res;
	GError *error = nullptr;
	const gchar *authors[] = { "Daniel Dwek", NULL };
	const gchar *documenters[] = { "Daniel Dwek", NULL };
	const gchar *translators = "Daniel Dwek";

	dialog = gtk_about_dialog_new ();
	gtk_about_dialog_set_program_name (GTK_ABOUT_DIALOG (dialog), "chin-chon-lin");
	gtk_about_dialog_set_version (GTK_ABOUT_DIALOG (dialog), "0.2.0");
	gtk_about_dialog_set_copyright (GTK_ABOUT_DIALOG (dialog), "Copyright 2023 Daniel Dwek");
	gtk_about_dialog_set_comments (GTK_ABOUT_DIALOG (dialog), _("Make combos with your cards and defeat the bots"));
	gtk_about_dialog_set_license_type (GTK_ABOUT_DIALOG (dialog), GTK_LICENSE_GPL_3_0);
	gtk_about_dialog_set_website (GTK_ABOUT_DIALOG (dialog), "https://github.com/ddwek/chin-chon-lin.git");
	gtk_about_dialog_set_website_label (GTK_ABOUT_DIALOG (dialog), _("Get the repo from GitHub"));
	gtk_about_dialog_set_authors (GTK_ABOUT_DIALOG (dialog), authors);
	gtk_about_dialog_set_documenters (GTK_ABOUT_DIALOG (dialog), documenters);
	gtk_about_dialog_set_translator_credits (GTK_ABOUT_DIALOG (dialog), translators);

	res = g_resource_load (CHIN_CHON_LIN_DATADIR "data/help/rules.gresource", &error);
	g_resources_register (res);
	image = gtk_image_new_from_resource ("/org/gtk/chin-chon-lin/chin-chon-lin_logo.png");
	logo = gtk_image_get_pixbuf (GTK_IMAGE (image));
	gtk_about_dialog_set_logo (GTK_ABOUT_DIALOG (dialog), logo);
	g_resources_unregister (res);

	gtk_dialog_run (GTK_DIALOG (dialog));
}
