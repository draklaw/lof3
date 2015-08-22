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


#ifndef _LOF3_FONT_H
#define _LOF3_FONT_H


#include <unordered_map>

#include <lair/core/lair.h>
#include <lair/core/log.h>


using namespace lair;


namespace lair {
class Texture;
class Renderer;
}


class Font {
public:
	Font(const Json::Value font, Texture* tex);

	void render(Renderer* renderer, const Vector3& position,
	            const std::string& msg, unsigned maxWidth = 0) const;

	Texture*    texture;

protected:
	struct Glyph {
		Box2     region;
		Vector2  size;
		Vector2  offset;
		unsigned advance;
	};

	typedef std::unordered_map<unsigned, Glyph,std::hash<unsigned>, std::equal_to<unsigned>,
	                           Eigen::aligned_allocator<std::pair<unsigned, Glyph>>> GlyphMap;

protected:
	unsigned _height;
	GlyphMap _glyphMap;
};


#endif
