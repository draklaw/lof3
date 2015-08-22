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


#ifndef _LOF3_GAME_H
#define _LOF3_GAME_H


#include <lair/core/lair.h>
#include <lair/core/log.h>

#include <lair/utils/path.h>

#include <lair/sys_sdl2/sys_module.h>
#include <lair/sys_sdl2/window.h>

#include <lair/render_gl2/render_module.h>
#include <lair/render_gl2/renderer.h>

#include "main_state.h"

#include "game_state.h"


using namespace lair;


class Game {
public:
	Game();
	~Game();

	Path dataPath() const;

	SysModule*    sys();
	Window*       window();

	RenderModule* renderModule();
	Renderer*     renderer();

	void initialize();
	void shutdown();

	void setNextState(GameState* state);
	void run();
	void quit();

	MainState* mainState();

	Logger& log() { return _logger; }


protected:
	MasterLogger  _mlogger;
	OStreamLogger _logBackend;
	Logger        _logger;

	Path          _dataPath;

	SysModule*    _sys;
	Window*       _window;

	RenderModule* _renderModule;
	Renderer*     _renderer;

	GameState*    _nextState;
	GameState*    _currentState;

	std::unique_ptr<MainState> _mainState;
};


#endif
