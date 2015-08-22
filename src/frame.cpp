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

#include "frame.h"


Frame::Frame(Sprite* bg, const Vector2& size)
    : position(Vector3::Zero()),
      size(size),
      background(bg) {
	lairAssert(bg);
}


void Frame::render(Renderer* renderer) {
	Batch& batch = renderer->mainBatch();
	VertexBuffer& buff = batch.getBuffer(
				renderer->spriteShader()->program(),
				background->texture(), renderer->spriteFormat());
	GLuint index = buff.vertexCount();

	unsigned tw = background->width();
	unsigned th = background->height();

	unsigned nHTiles = size.x() / tw;
	unsigned nVTiles = size.y() / th;

	Vector4 offset;
	offset << position, 0;

	for(unsigned y = 0; y < nVTiles ; ++y) {
		for(unsigned x = 0; x < nHTiles; ++x) {
			unsigned ti = 0;
			if(x == nHTiles-1) { ti += 2; }
			else if(x > 0)     { ti += 1; }
			if(y == nVTiles-1) { ti += 6; }
			else if(y > 0)     { ti += 3; }

			Box2 region = background->tileBox(ti);
			Vector4 v0(float(x + 0) * tw, -float(y + 0) * th, 0, 1);
			Vector4 v1(float(x + 0) * tw, -float(y + 1) * th, 0, 1);
			Vector4 v2(float(x + 1) * tw, -float(y + 0) * th, 0, 1);
			Vector4 v3(float(x + 1) * tw, -float(y + 1) * th, 0, 1);
			Vector4 color(1, 1, 1, 1);
			buff.addVertex(SpriteVertex{
					v0 + offset, color, region.corner(Box2::BottomLeft) });
			buff.addVertex(SpriteVertex{
					v1 + offset, color, region.corner(Box2::TopLeft) });
			buff.addVertex(SpriteVertex{
					v2 + offset, color, region.corner(Box2::BottomRight) });
			buff.addVertex(SpriteVertex{
					v3 + offset, color, region.corner(Box2::TopRight) });
			buff.addIndex(index + 0);
			buff.addIndex(index + 1);
			buff.addIndex(index + 2);
			buff.addIndex(index + 2);
			buff.addIndex(index + 1);
			buff.addIndex(index + 3);
			index += 4;
		}
	}
}
