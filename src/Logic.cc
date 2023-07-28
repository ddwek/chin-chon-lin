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
#include <string>
#include <set>
#include "Board.h"
#include "Player.h"
#include "Deck.h"
#include "Stack.h"

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

Logic::Logic ()
{
	ordered = false;
}

Logic::~Logic ()
{
}

std::string Logic::get_comp_criteria ()
{
	return comp;
}

int Logic::get_max_total_points () const
{
	return max_total_points;
}

int Logic::get_flexible_ending () const
{
	return flexible_ending;
}

void Logic::set_comp_criteria (std::string comp)
{
	this->comp = comp;
}

void Logic::set_max_total_points (int max_total_points)
{
	this->max_total_points = max_total_points;
}

void Logic::set_flexible_ending (int flexible_ending)
{
	this->flexible_ending = flexible_ending;
}

std::set<struct card_st> Logic::determine_missing_cards (int nplayer)
{
	int i, j, number, suit, prev, next;
	group_t *group = nullptr;
	stair_t *stair = nullptr;
	struct card_st crd = { 0 };
	std::list<Card>::iterator lst_iter;
	std::set<int> group_cards_set;
	std::set<int>::iterator iter;
	std::set<struct card_st> ret_missing;
	std::set<struct card_st>::iterator set_iter;
	Player& p = player[nplayer];

	for (i = 0; i < 2; i++) {
		switch (p.get_combo_type (i)) {
		case TYPE_EMPTY:
			break;
		case TYPE_GROUP:
			for (number = 0; number < 13; number++) {
				group = get_existing_cards_for_groups (nplayer, number);
				if (group && group->length > 0)
					for (j = 0; j < 4; j++)
						if (group->suit[j] == -1)
							group_cards_set.insert (j * 13 + group->number);
			}
			for (iter = group_cards_set.cbegin (); iter != group_cards_set.cend (); iter++) {
				crd.suit = *iter / 13;
				crd.number = *iter % 13;
				ret_missing.insert (crd);
			}
			break;
		case TYPE_STAIR:
			for (suit = 0; suit < 4; suit++) {
				stair = get_existing_cards_for_stair (nplayer, suit);
				if (stair && stair->length > 0)
					for (j = 0; j < 12; j++)
						if (stair->number[j] != -1)
							group_cards_set.insert (stair->suit * 12 + j);
			}
			for (iter = group_cards_set.begin (); iter != group_cards_set.end (); iter++) {
				if (*iter % 12 > 0) {
					prev = *iter % 12 - 1;
					crd.suit = *iter / 12;
					crd.number = prev + 1;
					ret_missing.insert (crd);
				}
				if (*iter % 12 < 11) {
					next = *iter % 12 + 1;
					crd.suit = *iter / 12;
					crd.number = next + 1;
					ret_missing.insert (crd);
				}
			}
			break;
		};
	}

	for (lst_iter = p.get_cards().begin (); lst_iter != p.get_cards().end (); lst_iter++)
		for (set_iter = ret_missing.begin (); set_iter != ret_missing.end (); set_iter++)
			if (set_iter->suit == lst_iter->get_suit () &&
			    set_iter->number == lst_iter->get_number ())
				set_iter = ret_missing.erase (set_iter);
	return ret_missing;
}

int Logic::choose_source ()
{
	int source = 1;
	std::set<struct card_st> cards;
	std::set<struct card_st>::const_iterator iter;
	StackPlayed& s = stack_played;

	cards = determine_missing_cards (board.get_turn ());
	for (iter = cards.cbegin (); iter != cards.cend (); iter++)
		if (iter->suit == s.get_cards().front().get_suit () &&
		    iter->number == s.get_cards().front().get_number ())
			source = 0;

	if (!deck.get_cards().size ())
		source = 0;
	if (!stack_played.get_cards().size ())
		source = 1;

	return source;
}

void Logic::get_sub_stair (stair_t *stair, int *io_start, int *ret_length)
{
	int i, start = 0, length = 0;
	bool set = false;

	if (!stair || !io_start || !ret_length)
		return;

	for (i = *io_start; i < 12; i++) {
		if (i + 1 < 12 && stair->number[i + 1] - stair->number[i] == 1) {
			if (!set) {
				start = i;
				length++;
			}
			length++;
			set = true;
		} else {
			if (!set) {
				*io_start = -1;
				*ret_length = -1;
			} else {
				*io_start = start;
				*ret_length = length;
				return;
			}
		}
	}
}

bool Logic::check_extra_cards_for_stair (stair_t *stair, int *last, int n_extra, int start)
{
	int i, j, length = 0;
	std::list<int> card_number;
	std::list<int>::const_iterator iter, next;

	if (!stair || !last)
		return false;

	if (n_extra > 5)
		start = 0;

	for (i = start; i < 12; i++)
		if (stair->number[i] != -1)
			card_number.push_back (stair->number[i]);
		else
			break;

	for (i = 0, iter = card_number.cbegin (); iter != card_number.cend (); iter++, i++) {
		get_sub_stair (stair, &start, &length);
		if (length > 2) {
			for (j = start; j < start + length + 1; j++) {
				if (n_extra > 5 && card_number.size () == (long unsigned) n_extra && stair->number[j] != -1)
					last[j] = j + 1;
				if (stair->number[j] == -1)
					length--;
			}
			if (length > n_extra - 2)
				return true;
		}
	}

	return false;
}

void Logic::get_next_end_for_stair (stair_t *stair, int suit, int *end1, int *end2)
{
	int i, j;

	if (!stair || !end1 || !end2)
		return;

	for (i = (*end1 == -1) ? 0 : *end1; i < 12; i++) {
		if (stair->suit == suit && stair->number[i] != -1) {
			if (*end1 == -1) {
				*end1 = stair->number[i];
				*end2 = -1;
			} else {
				for (j = *end1; j < 12; j++) {
					if (stair->suit == suit && stair->number[j] != -1) {
						*end2 = stair->number[j];
						return;
					}
				}
			}
		}
	}

	*end2 = -1;
}

int Logic::get_distance_for_stair (int suit, int end1, int end2, int *last, bool reset)
{
	int i;
	static int start = 0;

	if (!ordered || !last)
		return -1;

	if (reset) {
		start = 0;
		for (i = 0; i < 8; i++)
			last[i] = -1;
	}

	if (abs (end2 - end1) > 1) {
		return -1;
	} else {
		for (i = end1; i < end2; i++)
			last[i - end1 + start] = i;
		last[end2 - end1 + start] = i;

		for (i = 0; i < 8; i++) {
			if (last[i] == -1) {
				if (i)
					start = i - 1;
				else
					start = i;
				break;
			}
		}
		return 0;
	}
}

group_t *Logic::get_existing_cards_for_groups (int nplayer, int number)
{
	int i;
	group_t *group = nullptr;
	std::list<Card>::const_iterator iter, next;
	Player& p = player[nplayer];

	group = new group_t;
	group->length = 0;
	for (i = 0; i < 4; i++)
		group->suit[i] = -1;

	comp = "number";
	p.get_cards().sort ();
	ordered = true;
	for (iter = p.get_cards().cbegin (); iter != p.get_cards().cend (); iter++) {
		if (number != iter->get_number ())
			continue;

		group->number = iter->get_number ();
		for (i = 0; i < 4; i++) {
			if (iter->get_suit () == i) {
				group->suit[i] = i;
				group->length++;
				break;
			}
		}
	}

	return group;
}

stair_t *Logic::get_existing_cards_for_stair (int nplayer, int suit)
{
	int i;
	stair_t *stair = nullptr;
	std::list<Card>::const_iterator iter, next;
	Player& p = player[nplayer];

	stair = new stair_t;
	stair->length = 0;
	for (i = 0; i < 12; i++)
		stair->number[i] = -1;

	comp = "number";
	p.get_cards().sort ();
	comp = "suit";
	p.get_cards().sort ();
	ordered = true;
	for (iter = p.get_cards().cbegin (); iter != p.get_cards().cend (); iter++) {
		if (suit != iter->get_suit ())
			continue;
		stair->suit = iter->get_suit ();
		for (i = 0; i < 12; i++) {
			if (iter->get_number () == i + 1) {
				stair->number[i] = i + 1;
				stair->length++;
				break;
			}
		}
		next = iter;
		next++;
		if (next->get_suit () != iter->get_suit ())
			return stair;
	}

	if (!stair)
		return nullptr;
	return stair;
}

void Logic::get_game_combos ()
{
	int i, j, n, last[8], length = 0, end1 = -1, end2 = -1, start = 0;
	struct card_st my_cards = { 0 }, *my_cards_ptr = nullptr;
	bool reset = true;
	stair_t *stair = nullptr;
	group_t *group = nullptr;
	Player& p = player[board.get_turn ()];

	std::cout << _("Player #") << board.get_turn () << ":" << std::endl;
	p.clear (0);
	p.clear (1);
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 8; j++)
			last[j] = -1;

		stair = get_existing_cards_for_stair (p.get_id (), i);
		if (stair) {
			do {
				get_next_end_for_stair (stair, i, &end1, &end2);
				if (end2 == -1)
					break;
				if (get_distance_for_stair (i, end1, end2, last, reset) != -1) {
					for (j = (start == -1) ? 0 : start; j < 8; j++) {
						if (last[j] != -1) {
							length++;
							start++;
						}
					}

					if (length > 2) {
						if (check_extra_cards_for_stair (stair, last, 7, start))
							length = 7;
						else if (check_extra_cards_for_stair (stair, last, 6, start))
							length = 6;
						else if (check_extra_cards_for_stair (stair, last, 4, start))
							length = 4;
						else if (check_extra_cards_for_stair (stair, last, 3, start))
							length = 3;

						for (j = 0; j < length; j++) {
							if (last[j] != -1) {
								my_cards.idx = j;
								my_cards.suit = i;
								my_cards.number = last[j];
								my_cards.cnt = 0;
								p.set_current_group (&my_cards, TYPE_STAIR, length);
								n = p.get_current_group ();
								p.set_combo_card (n, &my_cards);
								p.set_combo_type (n, TYPE_STAIR);
								p.set_combo_length (n, length);
							} else {
								length--;
								n = p.get_current_group ();
								p.set_combo_length (n, length);
							}
						}

						std::cout << "\033[01;34m" << _("Group #") << n << _(" has ") << length <<
								_(" cards") << "\033[00m" << std::endl;
						for (j = 0; j < length; j++) {
							my_cards_ptr = p.get_combo_card (n, j);
							std::cout << "\033[01;33m" << _("Suit = ") << my_cards_ptr->suit <<
								_(", Number = ") << my_cards_ptr->number << "\033[00m" << std::endl;
						}
						if (length > 3) {
							p.set_current_group (&my_cards, TYPE_STAIR, length);
							goto next_stair;
						}
					}

					if (start == 0)
						while (stair->number[start] == -1)
							start++;

					reset = false;
				} else {
					start = 0;
					length = 0;
					reset = true;
				}
				end1 = end2;
				end2++;
			} while (end2 < 13);
next_stair:
			end1 = -1;
			end2 = -1;
			length = 0;
			start = 0;
			reset = true;
		}
	}

	for (i = 1; i < 13; i++) {
		group = get_existing_cards_for_groups (p.get_id (), i);
		if (group && group->length > 2) {
			for (j = 0; j < 4; j++) {
				if (group->suit[j] != -1) {
					my_cards.suit = j;
					my_cards.number = i;
					my_cards.cnt = 0;
					p.set_current_group (&my_cards, TYPE_GROUP, group->length);
					n = p.get_current_group ();
					if (p.get_combo_type (n) != TYPE_GROUP)
						p.clear (n);
					p.set_combo_type (n, TYPE_GROUP);
					p.set_combo_length (n, group->length);
					p.set_combo_card (n, &my_cards);
				}
			}

			if (group->length > 2) {
				n = p.get_current_group ();
				std::cout << "\033[01;34m" << _("Group #") << n << _(" has ") <<
						group->length << _(" cards") << "\033[00m" << std::endl;
				for (j = 0; j < 4; j++) {
					if (group->suit[j] == -1)
						continue;
					std::cout << "\033[01;33m" << _("Suit = ") << group->suit[j] <<
						_(", Number = ") << group->number << "\033[00m" << std::endl;
				}
			}
		}
	}
}

void Logic::rearrange_common_cards ()
{
	int i, j, n;
	Player& p = player[board.get_turn ()];
	struct card_st *group_card = nullptr, *stair_card = nullptr, new_card = { 0 };
	game_combo_t *g_combo = nullptr;

	if (p.get_combo_type (0) == TYPE_STAIR && p.get_combo_type (1) == TYPE_GROUP) {
		for (i = 0; i < p.get_combo_length (1); i++) {
			group_card = p.get_combo_card (1, i);
			for (j = 0; j < p.get_combo_length (0); j++) {
				stair_card = p.get_combo_card (0, j);
				if (group_card->suit == stair_card->suit && group_card->number == stair_card->number) {
					g_combo = p.get_game_combo (0);
					p.set_combo_type (0, TYPE_STAIR);
					p.set_combo_length (0, g_combo->length - 1);
					for (n = 0; n < g_combo->length; n++) {
						new_card.idx = n;
						new_card.suit = g_combo->cards[n].suit;
						new_card.number = g_combo->cards[n].number;
						new_card.cnt = 0;
						p.set_combo_card (0, &new_card);
					}
				}
			}
		}
	} else if (p.get_combo_type (0) == TYPE_GROUP && p.get_combo_type (1) == TYPE_STAIR) {
		for (i = 0; i < p.get_combo_length (0); i++) {
			group_card = p.get_combo_card (0, i);
			for (j = 0; j < p.get_combo_length (1); j++) {
				stair_card = p.get_combo_card (1, j);
				if (group_card->suit == stair_card->suit && group_card->number == stair_card->number) {
					g_combo = p.get_game_combo (1);
					p.set_combo_type (1, TYPE_STAIR);
					p.set_combo_length (1, g_combo->length - 1);
					for (n = 0; n < g_combo->length; n++) {
						new_card.idx = n;
						new_card.suit = g_combo->cards[n].suit;
						new_card.number = g_combo->cards[n].number;
						new_card.cnt = 0;
						p.set_combo_card (1, &new_card);
					}
				}
			}
		}
	}
}

int Logic::advise_to_finish ()
{
	Player& p = player[board.get_turn ()];

	if (p.get_combo_length (0) == 7 || p.get_combo_length (1) == 7) {
		std::cout << "\033[00;35m" << _("Warning: player ") << board.get_turn () <<
			_(" might end this round with a big stairway right now!") << "\033[00m" << std::endl;
		return 3;
	} else if (((p.get_combo_length (0) == 3 && p.get_combo_length (1) == 4) ||
		(p.get_combo_length (0) == 4 && p.get_combo_length (1) == 3))) {
			std::cout << "\033[00;35m" << _("Warning: player ") << board.get_turn () <<
					_(" might end this round right now!") << "\033[00m" << std::endl;
			for (int i = 0; i < p.get_game_combo(0)->length; i++)
				std::cout << _("suit = ") << p.get_combo_card(0, i)->suit <<
					_(", number = ") << p.get_combo_card(0, i)->number << std::endl;
			for (int i = 0; i < p.get_game_combo(1)->length; i++)
				std::cout << _("suit = ") << p.get_combo_card(1, i)->suit <<
					_(", number = ") << p.get_combo_card(1, i)->number << std::endl;

			return 2;
	} else if (p.get_combo_length (0) == 3 && p.get_combo_length (1) == 3 && flexible_ending == 1) {
		std::cout << _("Warning: ") << player[board.get_turn ()].get_name () <<
			_(" has two combos of three cards each") << std::endl;
		return 1;
	}

	return 0;
}

void Logic::calc_scores (int nplayer)
{
	int i, length, points = 0, group_pts = 0, stair_pts = 0;
	struct card_st *card_ptr = nullptr;
	std::list<Card>::const_iterator iter;
	Player& p = player[nplayer];

	for (iter = p.get_cards().cbegin (); iter != p.get_cards().cend (); iter++)
		points += iter->get_number ();

	length = p.get_game_combo(0)->length;
	for (i = 0; i < length; i++) {
		card_ptr = p.get_combo_card (0, i);
		if (p.get_combo_type (0) == TYPE_STAIR)
			stair_pts += card_ptr->number;
		else
			group_pts += card_ptr->number;
	}

	length = p.get_game_combo(1)->length;
	for (i = 0; i < length; i++) {
		card_ptr = p.get_combo_card (1, i);
		if (p.get_combo_type (1) == TYPE_STAIR)
			stair_pts += card_ptr->number;
		else
			group_pts += card_ptr->number;
	}

	if (!p.points_set ()) {
		p.set_round_pts (points - group_pts - stair_pts);
		p.set_total_pts (p.get_total_pts () + p.get_round_pts ());
		p.set_points (true);
	}
}
