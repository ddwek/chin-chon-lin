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

#include <math.h>
#include <set>
#include <list>
#include <gtk/gtk.h>
#include <cairo.h>
#include "Cmdline.h"
#include "UserInterface.h"
#include "Logic.h"
#include "Board.h"
#include "Deck.h"
#include "Stack.h"
#include "Card.h"

typedef enum { TYPE_EMPTY = 0, TYPE_STAIR, TYPE_GROUP } game_type_t;

struct card_st {
	bool operator< (struct card_st& c);
	bool operator< (const card_st& c2) const;
	int idx;
	int suit;
	int number;
	int cnt;
};

typedef struct {
	game_type_t type;
	int length;
	struct card_st cards[8];
} game_combo_t;

class Player {
public:
	Player ();
	Player (Player&) = delete;
	Player (Player&&) = delete;
	Player& operator= (Player&) = delete;
	~Player ();

	int get_id () const;
	std::string get_name () const;
	double get_sep () const;
	double get_xsrc () const;
	double get_ysrc () const;
	Card& get_card (int n);
	std::list<Card>& get_cards (); 
	int get_idx (int suit, int number) const;
	double get_xframe () const;
	double get_yframe () const;
	double get_xoffset (int n) const;
	double get_yoffset (int n) const;
	int get_selected () const;
	int get_ncard_to_play ();
	bool has_extra_card () const;
	bool is_locked () const;
	int get_current_group ();
	game_type_t get_combo_type (int n) const;
	int get_combo_length (int n) const;
	struct card_st *get_combo_card (int ncombo, int ncard);
	game_combo_t *get_game_combo (int ngroup);
	std::set<Card>& get_cards_set ();
	int get_round_pts () const;
	int get_total_pts () const;
	bool points_set () const;

	void set_id (int id);
	void set_name (std::string name);
	void set_sep (double sep);
	void set_xsrc (double xsrc);
	void set_ysrc (double ysrc);
	void set_xframe (double xframe);
	void set_yframe (double yframe);
	void set_xoffset (int idx, double value);
	void set_yoffset (int idx, double value);
	void set_selected (int selected);
	void set_extra_card (bool extra_card);
	void set_locked (bool locked);
	void set_current_group (struct card_st *c, game_type_t type, int length);
	void set_combo_type (int n, game_type_t type);
	void set_combo_length (int n, int length);
	void set_combo_card (int n, struct card_st *card, bool replace);
	void set_round_pts (int round_pts);
	void set_total_pts (int total_pts);
	void set_points (bool are_points_set);

	void init (int nplayer, std::string name);
	void acquire (std::string origin, Card& card, bool reset);
	void conf ();
	void clear (int ncombo);
	void lock ();
	void unlock ();
	void relocate ();
	void draw (cairo_t *cr, int ncard);
	void draw_all (cairo_t *cr);
	void draw_selector (cairo_t *cr, int ncard);
	void render_name (cairo_t *cr);
	void animate (cairo_t *cr);
	void finish (cairo_t *cr);
	void play (cairo_t *cr, int ncard);
	void erase_cards (cairo_t *cr);
	void show_cards (cairo_t *cr);
private:
	int id;
	std::string name;
	std::list<Card> cards;
	std::set<Card> cards_set;
	game_combo_t game_combo[2];
	int current_group;
	double sep;
	double xsrc;
	double ysrc;
	double xframe;
	double yframe;
	double xoffset[8];
	double yoffset[8];
	int selected;
	bool extra_card;
	bool locked;
	int round_pts;
	int total_pts;
	bool are_points_set;
};

static double angle = 0.0;
extern class Cmdline cmdline;
extern class Player player[4];
extern class StackPlayed stack_played;
extern GtkWidget *drawing_area;
extern unsigned deck_dist_tid, play_card_tid, player_ending_round_tid;

bool card_st::operator< (struct card_st& c)
{
//	if (this->cnt < c.cnt)
	if (this->number < c.number)
		return true;
	return false;
}

/*
 * Operator "less-than" overloaded to be used for "insert" method
 * of "set" containers. Particularly needed for Logic::determine_missing_cards ()
 */
bool card_st::operator< (const card_st& c2) const
{
	if (this->suit * 12 + this->number < c2.suit * 12 + c2.number)
		return true;
	return false;
}

Player::Player ()
{
	xframe = 0.0;
	yframe = 0.0;
	selected = -1;
	current_group = 0;
}

Player::~Player ()
{
}

int Player::get_id () const
{
	return id;
}

std::string Player::get_name () const
{
	return name;
}

double Player::get_sep () const
{
	return sep;
}

double Player::get_xsrc () const
{
	return xsrc;
}

double Player::get_ysrc () const
{
	return ysrc;
}

Card& Player::get_card (int n)
{
	int i;
	std::list<Card>::iterator iter;

	for (i = 0, iter = cards.begin (); iter != cards.end (); iter++, i++) {
		if (i == n)
			return *iter;
	}

	return card[0];
}

std::list<Card>& Player::get_cards ()
{
	return cards;
}

int Player::get_idx (int suit, int number) const
{
	int i;
	std::list<Card>::const_iterator iter;

	for (i = 0, iter = cards.cbegin (); iter != cards.cend (); iter++, i++)
		if (iter->get_suit () == suit && iter->get_number () == number)
			return i;
	return -1;
}

double Player::get_xframe () const
{
	return xframe;
}

double Player::get_yframe () const
{
	return yframe;
}

double Player::get_xoffset (int n) const
{
	return xoffset[n];
}

double Player::get_yoffset (int n) const
{
	return yoffset[n];
}

int Player::get_selected () const
{
	return selected;
}

int Player::get_ncard_to_play ()
{
	int i;
	card_st c = { 0 };
	std::list<struct card_st> cnt;
	std::list<struct card_st>::iterator cnt_iter, cnt_next;
	std::list<Card>::iterator iter;
	std::set<Card>::iterator pos;
	bool gc0 = false, gc1 = false;

	cards_set.clear ();
	for (iter = cards.begin (); iter != cards.end (); iter++) {
		for (i = 0; i < game_combo[0].length; i++) {
			if (iter->get_suit () != game_combo[0].cards[i].suit &&
			    iter->get_number () != game_combo[0].cards[i].number) {
				gc0 = true;
				break;
			} else {
				gc0 = false;
			}
		}

		for (i = 0; i < game_combo[1].length; i++) {
			if (iter->get_suit () != game_combo[1].cards[i].suit &&
			    iter->get_number () != game_combo[1].cards[i].number) {
				gc1 = true;
				break;
			} else {
				gc1 = false;
			}
		}

		if (gc0 && gc1) {
			cards_set.insert (*iter);
			gc0 = false;
			gc1 = false;
		}
	}

	for (pos = cards_set.begin (); pos != cards_set.end (); pos++) {
		c.suit = pos->get_suit ();
		c.number = pos->get_number ();
		std::cout << __FUNCTION__ << " (not making combos): suit = " << c.suit << ", number = " << c.number << std::endl;
		c.idx = get_idx (c.suit, c.number);
		c.cnt = cards_set.count (*pos);
		cnt.push_back (c);
	}

	cnt.sort ();
	for (cnt_iter = cnt.begin (); cnt_iter != cnt.end (); cnt_iter++) {
		cnt_next = cnt_iter;
		cnt_next++;
		if (cnt_next == cnt.end ())
			return cnt_iter->idx;
	}

	return 7;
}

bool Player::has_extra_card () const
{
	return extra_card;
}

bool Player::is_locked () const
{
	return locked;
}

int Player::get_current_group ()
{
	return current_group;
}

game_type_t Player::get_combo_type (int n) const
{
	return game_combo[n].type;
}

int Player::get_combo_length (int n) const
{
	return game_combo[n].length;
}

struct card_st *Player::get_combo_card (int ncombo, int ncard)
{
	return &game_combo[ncombo].cards[ncard];
}

game_combo_t *Player::get_game_combo (int ngroup)
{
	return &game_combo[ngroup];
}

std::set<Card>& Player::get_cards_set ()
{
	return cards_set;
}

int Player::get_round_pts () const
{
	return round_pts;
}

int Player::get_total_pts () const
{
	return total_pts;
}

bool Player::points_set () const
{
	return are_points_set;
}

void Player::set_id (int id)
{
	this->id = id;
}

void Player::set_name (std::string name)
{
	this->name = name;
}

void Player::set_sep (double sep)
{
	this->sep = sep;
}

void Player::set_xsrc (double xsrc)
{
	this->xsrc = xsrc;
}

void Player::set_ysrc (double ysrc)
{
	this->ysrc = ysrc;
}

void Player::set_xframe (double xframe)
{
	this->xframe = xframe;
}

void Player::set_yframe (double yframe)
{
	this->yframe = yframe;
}

void Player::set_xoffset (int idx, double value)
{
	this->xoffset[idx] = value;
}

void Player::set_yoffset (int idx, double value)
{
	this->yoffset[idx] = value;
}

void Player::set_selected (int selected)
{
	this->selected = selected;
}

void Player::set_extra_card (bool extra_card)
{
	this->extra_card = extra_card;
}

void Player::set_locked (bool locked)
{
	this->locked = locked;
}

void Player::set_current_group (struct card_st *c, game_type_t type, int length)
{
	bool has_sub_stair = false;

	if (type == TYPE_EMPTY) {
		if (game_combo[0].length == 0)
			current_group = 0;
		else
			current_group = 1;
	} else if (type == TYPE_STAIR) {
		for (int i = 0; i < game_combo[0].length; i++) {
			if ((game_combo[0].cards[i].suit == c->suit &&
			     game_combo[0].cards[i].number < c->number) ||
			    game_combo[0].cards[i].number != -1) {
				has_sub_stair = true;
			} else {
				has_sub_stair = false;
				break;
			}
		}

		if ((game_combo[0].cards[0].suit == c->suit || game_combo[0].cards[0].suit == -1) && !has_sub_stair)
			current_group = 0;
		else
			current_group = 1;
	} else {
		if (game_combo[0].cards[0].number == c->number || game_combo[0].cards[0].number == -1)
			current_group = 0;
		else
			current_group = 1;
	}
}

void Player::set_combo_type (int n, game_type_t type)
{
	this->game_combo[n].type = type;
}

void Player::set_combo_length (int n, int length)
{
	this->game_combo[n].length = length;
}

void Player::set_combo_card (int n, struct card_st *card, bool replace)
{
	if (game_combo[n].type == TYPE_GROUP) {
		if (replace) {
			game_combo[n].cards[card->idx].idx = card->idx;
			game_combo[n].cards[card->idx].suit = card->suit;
			game_combo[n].cards[card->idx].number = card->number;
			game_combo[n].cards[card->idx].cnt = card->cnt;
			return;
		}

		for (int i = 0; i < 4; i++) {
			if (game_combo[n].cards[i].idx == -1) {
				game_combo[n].cards[i].idx = card->idx;
				game_combo[n].cards[i].suit = card->suit;
				game_combo[n].cards[i].number = card->number;
				game_combo[n].cards[i].cnt = card->cnt;
				break;
			}
		}
	} else {
		game_combo[n].cards[card->idx].idx = card->idx;
		game_combo[n].cards[card->idx].suit = card->suit;
		game_combo[n].cards[card->idx].number = card->number;
		game_combo[n].cards[card->idx].cnt = card->cnt;
	}
}

void Player::set_round_pts (int round_pts)
{
	this->round_pts = round_pts;
}

void Player::set_total_pts (int total_pts)
{
	this->total_pts = total_pts;
}

void Player::set_points (bool are_points_set)
{
	this->are_points_set = are_points_set;
}

void Player::init (int nplayer, std::string name)
{
	id = nplayer;
	this->name = name;
}

void Player::acquire (std::string origin, Card& card, bool reset)
{
	static int i = 0;
	int j;
	std::list<Card>::iterator iter;
	std::list<Card>& src = origin == "deck" ? deck.get_cards () : stack_played.get_cards ();

	if (reset)
		i = 0;

	for (j = 0, iter = src.begin(); iter != src.end(); iter++, j++) {
		if (iter->get_suit () == card.get_suit () && iter->get_number () == card.get_number ()) {
			cards.push_back (card);
			src.erase (iter);
			Card& c = get_card (i);

			if (id != 0)
				break;

			c.set_x (xsrc + xoffset[i]);
			c.set_y (ysrc);
			c.set_w (c.get_logical().width * 5.0 * board.get_x_scale ());
			c.set_h (c.get_logical().height * 5.0 * board.get_x_scale ());
			ui.register_region (c.get_x (), c.get_y (),
					c.get_x () + c.get_w (),
					c.get_y () + c.get_h (),
					"human::hover", i);
			i++;
			i &= 7;
			break;
		}
	}
}

void Player::conf ()
{
	double sep = 0.0, xsrc = 0.0, ysrc = 0.0;

	switch (id) {
	case 0:
	case 2:
		sep = (deck.get_logical().width + 1) * 5.0;
		xsrc = ((800.0 - sep * 7.0) / 2) * board.get_x_scale ();
		for (int i = 0; i < 8; i++) {
			set_xoffset (i, i * sep * board.get_x_scale ());
			set_yoffset (i, 0.0);
		}
		if (id == 0)
			ysrc = (500.0 - (deck.get_logical().height + 2) * 5.0) * board.get_y_scale ();
		else
			ysrc = 10.0;
		break;
	case 1:
	case 3:
		sep = (500.0 - deck.get_logical().height * 5.0) / 7.0;
		ysrc = (500.0 * board.get_y_scale () - deck.get_logical().height * 5.0) / 7.0;
		for (int i = 0; i < 8; i++) {
			set_xoffset (i, 0.0);
			set_yoffset (i, i * deck.get_logical().height * 2.5 * board.get_y_scale ());
		}
		if (id == 1)
			xsrc = 10.0 * board.get_x_scale ();
		else
			xsrc = 800.0 * board.get_x_scale () - (deck.get_logical().width + 2) * 5.0 * board.get_x_scale ();
		break;
	};

	set_sep (sep);
	set_xsrc (xsrc);
	set_ysrc (ysrc);
}

void Player::clear (int ncombo)
{
	game_combo[ncombo].type = TYPE_EMPTY;
	game_combo[ncombo].length = 0;
	for (int j = 0; j < 8; j++) {
		game_combo[ncombo].cards[j].idx = -1;
		game_combo[ncombo].cards[j].suit = -1;
		game_combo[ncombo].cards[j].number = -1;
		game_combo[ncombo].cards[j].cnt = -1;
	}
}

void Player::lock ()
{
	unsigned long i;
	std::string str;
	double x, y, w, h;

	for (i = 0; i < cards.size (); i++) {
		x = xsrc + xoffset[i];
		y = ysrc + yoffset[i];
		w = get_card(i).get_logical().width * 5.0 * board.get_x_scale ();
		h = get_card(i).get_logical().height * 5.0 * board.get_y_scale ();
		str = "human::hover";
		ui.unregister_region (x, y, x + w, y + h, str, i);
	}
}

void Player::unlock ()
{
	unsigned long i;
	std::string str;
	double x, y, w, h;

	for (i = 0; i < cards.size (); i++) {
		x = xsrc + xoffset[i];
		y = ysrc + yoffset[i];
		w = get_card(i).get_logical().width * 5.0 * board.get_x_scale ();
		h = get_card(i).get_logical().height * 5.0 * board.get_y_scale ();
		str = "human::hover";
		ui.register_region (x, y, x + w, y + h, str, i);
	}
}

void Player::relocate ()
{
	int i;
	std::list<Card>::iterator iter;

	if (id != 0)
		return;

	for (i = 0, iter = cards.begin (); iter != cards.end (); iter++, i++) {
		ui.unregister_region (iter->get_x (), iter->get_y (),
					iter->get_x () + iter->get_w (),
					iter->get_y () + iter->get_h (),
					"human::hover", i);
		conf ();
		iter->set_x (xsrc + xoffset[i]);
		iter->set_y (ysrc);
		iter->set_w (iter->get_logical().width * 5.0 * board.get_x_scale ());
		iter->set_h (iter->get_logical().height * 5.0 * board.get_y_scale ());
		ui.register_region (iter->get_x (), iter->get_y (),
					iter->get_x () + iter->get_w (),
					iter->get_y () + iter->get_h (),
					"human::hover", i);
	}
}

void Player::draw (cairo_t *cr, int ncard)
{
	RsvgHandle *handler;

	if (!get_card(ncard).is_displayed ())
		return;

	if (cmdline.is_debug_mode ())
		handler = get_card(ncard).get_handler ();
	else
		handler = deck.get_handler ();

	if (!cmdline.is_debug_mode () && id == 0)
		handler = get_card(ncard).get_handler ();

	cairo_save (cr);
	cairo_translate (cr, xsrc + xoffset[ncard], ysrc + yoffset[ncard]);
	cairo_scale (cr, 5.0 * board.get_x_scale (), 5.0 * board.get_y_scale ());
	rsvg_handle_render_cairo (handler, cr);
	cairo_stroke (cr);
	cairo_restore (cr);
}

void Player::draw_all (cairo_t *cr)
{
	int end = extra_card ? 8 : 7;

	for (int i = 0; i < end; i++) {
		cairo_save (cr);
		cairo_translate (cr, xsrc + xoffset[i], ysrc + yoffset[i]);
		cairo_scale (cr, 5.0 * board.get_x_scale (), 5.0 * board.get_y_scale ());
		rsvg_handle_render_cairo (get_card(i).get_handler (), cr);
		cairo_stroke (cr);
		cairo_restore (cr);
	}
}

void Player::draw_selector (cairo_t *cr, int ncard)
{
	if (id != 0 || ncard == -1)
		return;

	Card& c = get_card (ncard);
	relocate ();
	cairo_save (cr);
	cairo_translate (cr, c.get_x () - 5.0, c.get_y () - 5.0);
	cairo_set_source_rgb (cr, 1.0, 1.0, 0.0);
	cairo_rectangle (cr, 0, 0, c.get_w () + 10.0, c.get_h () + 10.0);
	cairo_stroke (cr);
	cairo_restore (cr);
}

void Player::render_name (cairo_t *cr)
{
	double x = 0.0, y = 0.0, rotation_factor = 0.0;

	switch (id) {
	case 0:
	case 2:
		x = -10.0 * board.get_x_scale ();
		y = deck.get_logical().height * 5.0 * board.get_y_scale ();
		rotation_factor = 3.0 / 2.0;
		break;
	case 1:
	case 3:
		x = 0.0;
		y = -10.0;
		rotation_factor = 0.0;
		break;
	};

	cairo_save (cr);
	cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
	cairo_move_to (cr, xsrc + x, ysrc + y);
	cairo_rotate (cr, M_PI * rotation_factor);
	cairo_scale (cr, board.get_x_scale (), board.get_y_scale ());

	/*
	 * Here is where Pango API could be the natural replacement for printing
	 * graphical messages
	 */
	cairo_select_font_face (cr, "cairo:monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_show_text (cr, name.c_str ());
	cairo_restore (cr);
}

void Player::animate (cairo_t *cr)
{
	static int i = 0;

	switch (board.get_turn ()) {
	case 0:
		if (fabs (deck.get_y () + yframe - ysrc) < board.get_framerate ()) {
			xframe = 0.0;
			yframe = 0.0;
			board.set_turn ((board.get_turn () + 1) & 3);
			get_card(i & 7).set_displayed (true);
		}
		break;
	case 1:
		if (fabs (deck.get_y () + yframe - ysrc) < board.get_framerate ()) {
			xframe = 0.0;
			yframe = 0.0;
			board.set_turn ((board.get_turn () + 1) & 3);
			get_card(i & 7).set_displayed (true);
		}
		break;
	case 2:
		if (fabs (deck.get_y () + yframe - ysrc) < board.get_framerate ()) {
			xframe = 0.0;
			yframe = 0.0;
			board.set_turn ((board.get_turn () + 1) & 3);
			get_card(i & 7).set_displayed (true);
		}
		break;
	case 3:
		if (fabs (deck.get_y () + yframe - ysrc) < board.get_framerate ()) {
			xframe = 0.0;
			yframe = 0.0;
			board.set_turn ((board.get_turn () + 1) & 3);
			get_card(i & 7).set_displayed (true);
			if (i == 6 && !player[0].has_extra_card ()) {
				gtk_widget_remove_tick_callback (drawing_area, deck_dist_tid);
				return;
			} else {
				i++;
			}
		}
		break;
	};

	if (player[0].get_cards().size () != 8) {
		cairo_save (cr);
		cairo_translate (cr, deck.get_x () + xframe, deck.get_y () + yframe);
		cairo_scale (cr, 5.0 * board.get_x_scale (), 5.0 * board.get_y_scale ());
		rsvg_handle_render_cairo (player[board.get_turn ()].get_card(i & 7).get_handler (), cr);
		cairo_stroke (cr);
		cairo_restore (cr);
	}
}

gboolean on_player_ending_round_cb (GtkWidget *widget, GdkFrameClock *frame_clock, gpointer data)
{
	Player& p = player[board.get_turn ()];
	StackPlayed& s = stack_played;
	double xc = s.get_x () + s.get_logical().width / 2.0 * 5.0 * board.get_x_scale ();
	double yc = s.get_y () + s.get_logical().height / 2.0 * 5.0 * board.get_y_scale ();

	p.set_xframe (p.get_xframe () + 10.0 / (xc / (xc - (p.get_xsrc () + p.get_xoffset (p.get_selected ())))));
	p.set_yframe (p.get_yframe () + 10.0 / (xc / (yc - (p.get_ysrc () + p.get_yoffset (p.get_selected ())))));
	angle += M_PI / 18.0;

	if (fabs (p.get_xsrc () + p.get_xoffset (p.get_selected ()) + p.get_xframe () - s.get_x ()) < 5.0 &&
	    fabs (p.get_ysrc () + p.get_yoffset (p.get_selected ()) + p.get_yframe () - s.get_y ()) < 5.0) {
		p.set_xframe (0.0);
		p.set_yframe (0.0);
		board.set_status (FINISHING_ROUND_STOP);
	}
	gtk_widget_queue_draw (drawing_area);

	return G_SOURCE_CONTINUE;
}

static bool fn_less (double v, double bound)
{
	if (v < bound)
		return true;
	return false;
}

static bool fn_greater (double v, double bound)
{
	if (v > bound)
		return true;
	return false;
}

typedef bool (*cond_fn_t) (double v, double bound);

void Player::finish (cairo_t *cr)
{
	StackPlayed& s = stack_played;
	double x = xsrc + xoffset[selected];
	double y = ysrc + yoffset[selected];
	double xc = s.get_x () + s.get_logical().width / 2.0 * 5.0 * board.get_x_scale ();
	double yc = s.get_y () + s.get_logical().height / 2.0 * 5.0 * board.get_y_scale ();
	static bool cond_set = false;
	static cond_fn_t x_op = nullptr, y_op = nullptr;

	if (!cond_set) {
		if (x < xc)
			x_op = fn_less;
		else
			x_op = fn_greater;
		if (y < yc)
			y_op = fn_less;
		else
			y_op = fn_greater;
		cond_set = true;
	}

	if (x_op (x + xframe, xc) && y_op (y + yframe, yc)) {
		cairo_save (cr);
		cairo_translate (cr, xsrc + xoffset[selected] + xframe, ysrc + yoffset[selected] + yframe);
		cairo_rotate (cr, angle);
		cairo_translate (cr, - (s.get_logical().width / 2.0) * 5.0 * board.get_x_scale (),
					- (s.get_logical().height / 2.0) * 5.0 * board.get_y_scale ());
		cairo_scale (cr, 5.0 * board.get_x_scale (), 5.0 * board.get_y_scale ());
		rsvg_handle_render_cairo (deck.get_handler (), cr);
		cairo_stroke (cr);
		cairo_restore (cr);
	} else {
		x_op = nullptr;
		y_op = nullptr;
		cond_set = false;
		gtk_widget_remove_tick_callback (drawing_area, player_ending_round_tid);
		board.set_status (FINISHING_ROUND_STOP);
	}
}

gboolean on_play_card_cb (GtkWidget *widget, GdkFrameClock *frame_clock, gpointer data)
{
	Player& p = player[board.get_turn ()];
	Tile& s = stack_played;
	double xframe = p.get_xframe ();
	double yframe = p.get_yframe ();

	switch (board.get_turn ()) {
	case 0:
		xframe = p.get_xframe () + ((p.get_xsrc () + p.get_xoffset (p.get_selected ()) - s.get_x ()) / (s.get_y () - p.get_ysrc ())) * 20.0 * board.get_framerate ();
		yframe = p.get_yframe () - 1.0 * 20.0 * board.get_framerate ();
		break;
	case 1:
		xframe = p.get_xframe () + 1.0 * 20.0 * board.get_framerate ();
		yframe = p.get_yframe () - ((p.get_ysrc () + p.get_yoffset (p.get_selected ()) - s.get_y ()) / (s.get_x () - p.get_xsrc ())) * 20.0 * board.get_framerate ();
		break;
	case 2:
		xframe = p.get_xframe () - ((p.get_xsrc () + p.get_xoffset (p.get_selected ()) - s.get_x ()) / (s.get_y () - p.get_ysrc ())) * 20.0 * board.get_framerate ();
		yframe = p.get_yframe () + 1.0 * 20.0 * board.get_framerate ();
		break;
	case 3:
		xframe = p.get_xframe () - 1.0 * 20.0 * board.get_framerate ();
		yframe = p.get_yframe () + ((p.get_ysrc () + p.get_yoffset (p.get_selected ()) - s.get_y ()) / (s.get_x () - p.get_xsrc ())) * 20.0 * board.get_framerate ();
		break;
	};

	p.set_xframe (xframe);
	p.set_yframe (yframe);
	gtk_widget_queue_draw (drawing_area);

	return G_SOURCE_CONTINUE;
}

void Player::play (cairo_t *cr, int ncard)
{
	if (fabs (stack_played.get_x () - (xsrc + xframe + xoffset[ncard])) < 20.0 * board.get_framerate () &&
	    fabs (stack_played.get_y () - (ysrc + yframe + yoffset[ncard])) < 20.0 * board.get_framerate ()) {
		xframe = 0.0;
		yframe = 0.0;
		extra_card = false;
		stack_played.set_only_once_value (true);
		stack_played.acquire ("player");
		board.set_status (PLAY_CARD_STOP);
		gtk_widget_remove_tick_callback (drawing_area, play_card_tid);
		board.set_turn ((board.get_turn () + 1) & 3);
	}

	cairo_save (cr);
	cairo_translate (cr, xsrc + xoffset[ncard] + xframe, ysrc + yoffset[ncard] + yframe);
	cairo_scale (cr, 5.0 * board.get_x_scale (), 5.0 * board.get_y_scale ());
	rsvg_handle_render_cairo (get_card(ncard).get_handler (), cr);
	cairo_stroke (cr);
	cairo_restore (cr);
}

void Player::erase_cards (cairo_t *cr)
{
	StackPlayed& s = stack_played;

	for (int i = 0; i < 8; i++) {
		cairo_save (cr);
		cairo_set_source_rgb (cr, .2, .3, .2);
		cairo_translate (cr, xsrc + xoffset[i], ysrc + yoffset[i]);
		cairo_scale (cr, 5.0 * board.get_x_scale (), 5.0 * board.get_y_scale ());
		cairo_rectangle (cr, -1.0, -1.0, s.get_logical().width + 2.0, s.get_logical().height + 2.0);
		cairo_fill (cr);
		cairo_restore (cr);
	}
}

void Player::show_cards (cairo_t *cr)
{
	int i, j = 0;
	double angle = 0.0, incr = 0.0;
	struct {
		double x;
		double y;
	} coords[7] = { .0 };
	std::list<Card>::const_iterator iter;
	StackPlayed& s = stack_played;

	erase_cards (cr);

	switch (id) {
	case 0:
		for (i = 0; i < 7; i++) {
			coords[i].x = 400 - 250.0 * cos (((60.0 * i) / 7.0 + 60.0) * M_PI / 180.0);
			coords[i].x *= board.get_x_scale ();
			coords[i].y = 570 - 180.0 * sin (((60.0 * i) / 7.0 + 60.0) * M_PI / 180.0);
			coords[i].y *= board.get_y_scale ();
		}
		angle = -30.0 * M_PI / 180.0;
		incr = (60.0 / 7.0) * M_PI / 180.0;
		break;
	case 1:
		for (i = 0; i < 7; i++) {
			coords[i].x = -50 + 175.0 * sin (((80.0 * i) / 7.0 + 60.0) * M_PI / 180.0);
			coords[i].x *= board.get_x_scale ();
			coords[i].y = 250 - 125.0 * cos (((80.0 * i) / 7.0 + 60.0) * M_PI / 180.0);
			coords[i].y *= board.get_y_scale ();
		}
		angle = 60.0 * M_PI / 180.0;
		incr = (90.0 / 7.0) * M_PI / 180.0;
		break;
	case 2:
		for (i = 0; i < 7; i++) {
			coords[i].x = 400 - 250.0 * cos (((60.0 * i) / 7.0 + 60.0) * M_PI / 180.0);
			coords[i].x *= board.get_x_scale ();
			coords[i].y = -80 + 180.0 * sin (((60.0 * i) / 7.0 + 60.0) * M_PI / 180.0);
			coords[i].y *= board.get_y_scale ();
		}
		angle = 30.0 * M_PI / 180.0;
		incr = -1.0 * (60.0 / 7.0) * M_PI / 180.0;
		break;
	case 3:
		for (i = 0; i < 7; i++) {
			coords[i].x = 850 - 175.0 * sin (((80.0 * i) / 7.0 + 60.0) * M_PI / 180.0);
			coords[i].x *= board.get_x_scale ();
			coords[i].y = 250 - 125.0 * cos (((80.0 * i) / 7.0 + 60.0) * M_PI / 180.0);
			coords[i].y *= board.get_y_scale ();
		}
		angle = -60.0 * M_PI / 180.0;
		incr = -1.0 * (80.0 / 7.0) * M_PI / 180.0;
		break;
	};

	for (iter = get_cards().cbegin (); iter != get_cards().cend (); iter++) {
		cairo_save (cr);
		cairo_translate (cr, coords[j].x, coords[j].y);
		cairo_rotate (cr, angle);
		angle += incr;
		cairo_translate (cr, - (s.get_logical().width / 2.0) * 5.0 * board.get_x_scale (),
				- (s.get_logical().height / 2.0) * 5.0 * board.get_y_scale ());
		cairo_scale (cr, 5.0 * board.get_x_scale (), 5.0 * board.get_y_scale ());
		rsvg_handle_render_cairo (get_card(j).get_handler (), cr);
		cairo_stroke (cr);
		cairo_restore (cr);
		j++;
	}
}
