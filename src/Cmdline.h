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
#ifndef _CMDLINE_H_
#define _CMDLINE_H_

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
#endif
