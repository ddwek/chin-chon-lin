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
#ifndef _LOGIC_H_
#define _LOGIC_H_
#include <string>
#include <set>

typedef struct {
	int number;
	int length;
	int suit[4];
} group_t;

typedef struct {
	int suit;
	int length;
	int number[12];
} stair_t;

class Logic {
public:
	Logic ();
	Logic (Logic&) = delete;
	Logic (Logic&&) = delete;
	Logic& operator= (Logic&) = delete;
	~Logic ();

	std::string get_comp_criteria ();
	int get_max_total_points () const;
	int get_flexible_ending () const;

	void set_comp_criteria (std::string comp);
	void set_max_total_points (int max_total_points);
	void set_flexible_ending (int flexible_ending);

	std::set<struct card_st> determine_missing_cards (int nplayer);
	int choose_source ();
	void get_sub_stair (stair_t *stair, int *io_start, int *ret_length);
	bool check_extra_cards_for_stair (stair_t *stair, int *last, int n_extra, int start);
	void get_next_end_for_stair (stair_t *stair, int suit, int *end1, int *end2);
	int get_distance_for_stair (int suit, int end1, int end2, int *last, bool reset);
	group_t *get_existing_cards_for_groups (int nplayer, int number);
	stair_t *get_existing_cards_for_stair (int nplayer, int suit);
	void get_game_combos ();
	void rearrange_common_cards ();
	int advise_to_finish ();
	void calc_scores (int nplayer);
private:
	std::string comp;
	bool ordered;
	int max_total_points;
	int flexible_ending;
};

extern class Logic logic;
#endif
