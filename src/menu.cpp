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


#include <lair/render_gl2/renderer.h>

#include "menu.h"


Menu::Menu(Sprite* bg, const Vector2& size)
    : _selected(0),
      _entries(),
      _visible(false),
      _anchor(0, 0),
      _frame(bg, size) {
	lairAssert(bg);
}


unsigned Menu::selected() const {
	return _selected;
}


void Menu::setSelected(unsigned index) {
	lairAssert(index < _entries.size());
	_selected = index;
}


unsigned Menu::addEntry(const std::string& label) {
	_entries.emplace_back(label);
	return _entries.size() - 1;
}


void Menu::show(const Vector3& position) {
	_visible  = true;
	_frame.position = position;
}


void Menu::hide() {
	_visible = false;
}


void Menu::render(Renderer* renderer) {
	if(!_visible) {
		return;
	}

	_frame.render(renderer);
}
