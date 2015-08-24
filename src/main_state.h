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
#include "text_component.h"
#include "rules.h"
#include "fight.h"

#include "game_state.h"


using namespace lair;


class Game;

class Font;


class MainState : public GameState {
public:
	MainState(Game* game);
	~MainState();

	Sprite loadSprite(const char* file, unsigned th = 1, unsigned tv = 1);

	virtual void initialize();
	virtual void shutdown();

	virtual void run();
	virtual void quit();

	void layoutScreen();

	EntityRef createSprite(Sprite* sprite, const Vector3& pos,
	                       const char* name = nullptr);
	EntityRef createSprite(Sprite* sprite, const Vector3& pos,
	                       const Vector2& scale,
	                       const char* name = nullptr);
	EntityRef createText(const std::string& msg, const Vector3& pos,
	                     const Vector4& color = Vector4(1, 1, 1, 1));
	EntityRef createDamageText(const std::string& msg, const Vector3& pos,
	                           const Vector4& color = Vector4(1, 1, 1, 1));
	EntityRef createHealthBar(const Vector3& pos, float size);
	void init();

	void updateTick();
	void updateFrame();

	void showMessage(const std::string& message);
	void nextMessage();
	void layoutMessage();

	void updateMenu();
	void openMenu(Menu* menu, Menu* parent = nullptr, unsigned entry = 0);
	Menu::Callback openMenuFunc(Menu* menu, Menu* parent = nullptr,
	                            unsigned entry = 0);
	void closeMenu();
	Menu::Callback closeMenuFunc();

	void doAction();
	Menu::Callback doActionFunc();

	Logger& log();

protected:
	enum FightState {
		PLAYING,
		BOSS_TURN,
		GAME_OVER
	};

	enum {
		MAIN_ATTACK,
		MAIN_SWITCH,
		MAIN_SPELL,
		MAIN_SUMMON,
		MAIN_SCAN,
		MAIN_QTE,

		SPELL_STORM = 0,
		SPELL_STRIKE,
		SPELL_CRIPPLE,
		SPELL_DRAIN,
		SPELL_VORPAL,
		SPELL_MUD,
		SPELL_DISPEL
	};



protected:
	Game* _game;

	EntityManager          _entities;
	SpriteComponentManager _sprites;
	TextComponentManager   _texts;
	InputManager           _inputs;

	SlotTracker _slotTracker;

	OrthographicCamera _camera;

	bool        _initialized;
	bool        _running;
	InterpLoop  _loop;
	int64       _fpsTime;
	unsigned    _fpsCount;

	Rules       _rules;
	Player      _player;
	Fight       _fight;
	FightState  _state;
	unsigned    _maxPcHp;

	MenuInputs  _menuInputs;

	Texture*    _fontTex;
	Json::Value _fontJson;
	std::unique_ptr<Font>
	            _font;

	Sprite      _bgSprite;
	Sprite      _healthEmptySprite;
	Sprite      _healthFullSprite;
	Sprite      _menuBgSprite;
	Sprite      _boss1Sprite;
	Sprite      _pcSprite[4];

	EntityRef   _bg;

	EntityRef   _boss;
	EntityRef   _bossHealthFull[4];

	EntityRef   _pc[4];
	EntityRef   _pcHealthFull[4];
	std::string _pcName[4];

	std::unique_ptr<Frame>
	            _statusFrame;

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
	std::unique_ptr<Menu>
	            _pcMenu;
};


#endif
