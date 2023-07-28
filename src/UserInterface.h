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
#ifndef _USER_INTERFACE_H_
#define _USER_INTERFACE_H_
#include <string>
#include <list>
#include <cairo.h>

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

extern class UserInterface ui;
#endif
