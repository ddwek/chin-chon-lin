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
#ifndef _PLAYER_H_
#define _PLAYER_H_
#include <set>
#include <list>
#include <gtk/gtk.h>
#include <cairo.h>
#include "Tile.h"
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
	std::multiset<Card>& get_cards_multiset ();
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
	void set_combo_card (int n, struct card_st *card);
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
	std::multiset<Card> cards_multiset;
	game_combo_t game_combos[2];
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

extern class Player player[4];
#endif
