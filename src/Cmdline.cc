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
#include <fstream>
#include <gtk/gtk.h>
#include "Deck.h"
#include "Card.h"

class Cmdline {
public:
	Cmdline ();
	Cmdline (Cmdline& c) = delete;
	Cmdline (Cmdline&& c) = delete;
	Cmdline& operator= (Cmdline& c) = delete;
	~Cmdline ();

	bool is_testing_file () const;
	bool is_debug_mode () const;
	void set_testing_file (bool testing_file);
	void set_debug_mode (bool debug);
	void parse_cmdline_options (int *argc, char ***argv);
private:
	bool testing_file;
	bool debug;
};

extern class Cmdline cmdline;

Cmdline::Cmdline ()
{
}

Cmdline::~Cmdline ()
{
}

bool Cmdline::is_testing_file () const
{
	return testing_file;
}

bool Cmdline::is_debug_mode () const
{
	return debug;
}

void Cmdline::set_testing_file (bool testing_file)
{
	this->testing_file = testing_file;
}

/*
 * When testing chin-chon-lin, you might want to see your adversaries' cards.
 * By setting 'debug' to 'true' you enable this feature
 */
void Cmdline::set_debug_mode (bool debug)
{
	this->debug = debug;
}

/*
 * Parse special text files used as input files for manual testing.
 * Such files live under "test/" subdirectory and you can use them
 * to test different scenarios running chin-chon-lin with the
 * command line option '--test-deck test/input-...'
 */
static bool parse_test_file (const char *filename)
{
	int i = 0, j, pos, suit, number;
	char buf[32] = { '\0' };
	std::fstream ifile;
	std::string str, suit_str, number_str;

	if (!filename)
		return false;

	ifile.open (filename, std::ios::in);
	while (!ifile.eof ()) {
		ifile.getline (buf, 32, '\n');
		str = buf;
		if (str.empty ())
			break;

		pos = str.find ("suit =");
		for (j = pos; str[j] != ','; j++)
			if (str[j] >= '0' && str[j] <= '3')
				suit_str += str[j];
		suit = atoi (suit_str.c_str ());
		suit_str.clear ();

		pos = str.find ("number =");
		for (j = pos; str[j] != ','; j++)
			if (str[j] >= '0' && str[j] <= '9')
				number_str += str[j];
		number = atoi (number_str.c_str ());
		number_str.clear ();

		card[i].init (suit, number);
		deck.acquire (card[i]);
		i++;
	}

	ifile.close ();
	return true;
}

static bool test_deck_cb (const char *option_name, const char *filename, void *data, GError **error)
{
	cmdline.set_testing_file (true);
	return parse_test_file (filename);
}

static bool debug_mode_cb ()
{
	cmdline.set_debug_mode (true);
	return true;
}

void Cmdline::parse_cmdline_options (int *argc, char ***argv)
{
	GOptionContext *context = nullptr;
	const GOptionEntry entries[] = {
		{ "test-deck", 0, G_OPTION_FLAG_FILENAME, G_OPTION_ARG_CALLBACK,
			(void *) test_deck_cb, _("Feed deck with this input file"), "I" },
		{ "debug", 0, G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
			(void *) debug_mode_cb, _("Enable debug mode"), "D" },
		{ NULL },
	};

	context = g_option_context_new (_("- test different decks"));
	g_option_context_add_main_entries (context, entries, nullptr);
	g_option_context_add_group (context, gtk_get_option_group (TRUE));

	context = g_option_context_new (_("- enable debug mode"));
	g_option_context_add_main_entries (context, entries, nullptr);
	g_option_context_add_group (context, gtk_get_option_group (TRUE));

	gtk_init_with_args (argc, argv, nullptr, entries, nullptr, nullptr);
}
