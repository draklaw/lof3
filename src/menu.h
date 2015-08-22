//
//  Copyright (C) 2015 the authors (see AUTHORS)
//
//  This file is part of lof3.
//
//  lair is free software: you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  lair is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with lair.  If not, see <http://www.gnu.org/licenses/>.
//


#ifndef _LOF3_MENU_H
#define _LOF3_MENU_H


#include <lair/core/lair.h>
#include <lair/core/log.h>

#include "frame.h"


using namespace lair;


class Menu {
public:
	Menu(Sprite* bg, const Vector2& size);

	unsigned selected() const;
	void setSelected(unsigned index);

	unsigned addEntry(const std::string& label);

	void show(const Vector3& position);
	void hide();

	void render(Renderer* renderer);

protected:
	typedef std::vector<std::string> EntryList;

protected:
	unsigned   _selected;
	EntryList  _entries;

	bool       _visible;
	Vector2    _anchor;
	Frame      _frame;
};


#endif
