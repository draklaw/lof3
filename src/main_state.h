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


#include <vector>
#include <deque>

#include <lair/core/lair.h>
#include <lair/core/log.h>
#include <lair/core/signal.h>

#include <lair/utils/interp_loop.h>
#include <lair/utils/input.h>

#include <lair/render_gl2/orthographic_camera.h>

#include <lair/ec/entity.h>
#include <lair/ec/entity_manager.h>
#include <lair/ec/sprite_component.h>

#include "menu.h"

#include "game_state.h"


using namespace lair;


class Game;

class Font;


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

	void showMessage(const std::string& message);
	void nextMessage();
	void layoutMessage();

	void openMenu(Menu* menu);
	void closeMenu();

	Logger& log();

protected:
	Game* _game;

	EntityManager          _entities;
	SpriteComponentManager _sprites;
	InputManager           _inputs;

	SlotTracker _slotTracker;

	OrthographicCamera _camera;

	bool        _initialized;
	bool        _running;
	InterpLoop  _loop;
	int64       _fpsTime;
	unsigned    _fpsCount;

	MenuInputs  _menuInputs;

	Texture*    _fontTex;
	Json::Value _fontJson;
	std::unique_ptr<Font>
	            _font;

	Sprite      _bgSprite;
	Sprite      _healthEmptySprite;
	Sprite      _healthFullSprite;
	Sprite      _menuBgSprite;
	Sprite      _warriorSprite;
	Sprite      _blackMageSprite;
	Sprite      _whiteMageSprite;
	Sprite      _ninjaSprite;

	EntityRef   _bg;

	EntityRef   _warriorHealthEmpty;
	EntityRef   _warriorHealthFull;
	EntityRef   _warrior;
	EntityRef   _blackMage;
	EntityRef   _whiteMage;
	EntityRef   _ninja;

	std::deque<std::string>
	            _messages;
	std::unique_ptr<Frame>
	            _messageFrame;
	float       _messageMargin;
	float       _messageOutMargin;
	float       _messageTextHeight;

	std::vector<Menu*>
	            _menuStack;
	std::unique_ptr<Menu>
	            _mainMenu;
	std::unique_ptr<Menu>
	            _switchMenu;
	std::unique_ptr<Menu>
	            _spellMenu;
	std::unique_ptr<Menu>
	            _summonMenu;
};


#endif
