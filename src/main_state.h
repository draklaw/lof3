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


#ifndef _LOF3_MAIN_STATE_H
#define _LOF3_MAIN_STATE_H


#include <lair/core/lair.h>
#include <lair/core/log.h>

#include <lair/utils/interp_loop.h>

#include <lair/render_gl2/orthographic_camera.h>

#include <lair/ec/entity.h>
#include <lair/ec/entity_manager.h>
#include <lair/ec/sprite_component.h>

#include "game_state.h"


using namespace lair;


class Game;


class MainState : public GameState {
public:
	MainState(Game* game);
	~MainState();

	virtual void initialize();
	virtual void shutdown();

	virtual void run();
	virtual void quit();

	void layoutScreen();

	void init();

	void updateTick();
	void updateFrame();

	Logger& log();

protected:
	Game* _game;

	EntityManager          _entities;
	SpriteComponentManager _sprites;

	OrthographicCamera _camera;

	bool       _initialized;
	bool       _running;
	InterpLoop _loop;

	Sprite     _bgSprite;

	EntityRef  _bg;
};


#endif
