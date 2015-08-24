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


#include <functional>

#include "font.h"
#include "menu.h"
#include "game.h"
#include "fight.h"

#include "main_state.h"


MainState::MainState(Game* game)
	: _game(game),

      _entities(_game->log()),
      _sprites(_game->renderer()),
      _texts(),
      _inputs(_game->sys(), &_game->log()),

      _slotTracker(),

      _camera(),

      _initialized(false),
      _running(false),
      _loop(_game->sys()),
      _fpsTime(0),
      _fpsCount(0),

      _rules(log(), ""),
      _player{ {0}, {0}, {0}, 0, {0}, {0} },
      _fight(log(), _rules, _player),
      _state(PLAYING),

      _menuInputs(),

      _fontTex(nullptr),
      _fontJson(),
      _font(),

      _bgSprite(),
      _menuBgSprite(),

      _bg(),

      _pcName{ "Warrior", "Black mage", "White mage", "Ninja" },

      _messages(),
      _messageFrame(),
      _messageMargin(0),

      _menuStack(),
      _mainMenu(),
      _switchMenu(),
      _spellMenu(),
      _summonMenu() {
}


MainState::~MainState() {
}


Sprite MainState::loadSprite(const char* file, unsigned th, unsigned tv) {
	Texture* tex = _game->renderer()->getTexture(
	            file, Texture::NEAREST | Texture::CLAMP);
	return Sprite(tex, th, tv);
}


void MainState::initialize() {
	_loop.reset();
	_loop.setTickDuration(    1000000000 /  60);
	_loop.setFrameDuration(   1000000000 /  60);
	_loop.setMaxFrameDuration(_loop.frameDuration() * 3);
	_loop.setFrameMargin(     _loop.frameDuration() / 2);

	_game->window()->onResize.connect(std::bind(&MainState::layoutScreen, this))
	        .track(_slotTracker);
	layoutScreen();


	_menuInputs.up     = _inputs.addInput("up");
	_menuInputs.down   = _inputs.addInput("down");
//	_menuInputs.left   = _inputs.addInput("left");
//	_menuInputs.right  = _inputs.addInput("right");
	_menuInputs.ok     = _inputs.addInput("ok");
	_menuInputs.cancel = _inputs.addInput("cancel");

	_inputs.mapScanCode(_menuInputs.up,     SDL_SCANCODE_UP);
	_inputs.mapScanCode(_menuInputs.down,   SDL_SCANCODE_DOWN);
//	_inputs.mapScanCode(_menuInputs.left,   SDL_SCANCODE_LEFT);
//	_inputs.mapScanCode(_menuInputs.right,  SDL_SCANCODE_RIGHT);
	_inputs.mapScanCode(_menuInputs.ok,     SDL_SCANCODE_RIGHT);
	_inputs.mapScanCode(_menuInputs.ok,     SDL_SCANCODE_RETURN);
	_inputs.mapScanCode(_menuInputs.ok,     SDL_SCANCODE_SPACE);
	_inputs.mapScanCode(_menuInputs.cancel, SDL_SCANCODE_LEFT);
	_inputs.mapScanCode(_menuInputs.cancel, SDL_SCANCODE_ESCAPE);


	_fontJson = _game->sys()->loader().getJson("8-bit_operator+_regular_23.json");
	_fontTex  = _game->renderer()->getTexture(_fontJson["file"].asString(),
	        Texture::NEAREST | Texture::REPEAT);
	_font.reset(new Font(_fontJson, _fontTex));
	_font->baselineToTop = 12;

	_bgSprite          = loadSprite("bg.png");
	_healthEmptySprite = loadSprite("health_bar_empty.png");
	_healthFullSprite  = loadSprite("health_bar_full.png");
	_menuBgSprite      = loadSprite("menu.png", 3, 3);
	_boss1Sprite       = loadSprite("BigBoss1.png");
	_pcSprite[0]       = loadSprite("GTP.png");
	_pcSprite[1]       = loadSprite("MN.png");
	_pcSprite[2]       = loadSprite("MB.png");
	_pcSprite[3]       = loadSprite("Ninja.png");


	_statusFrame.reset(new Frame(&_menuBgSprite, Vector2(640, 120)));
	_statusFrame->position = Vector3(0, 0, -.05);

	_messageMargin = 12;
	_messageOutMargin = 8;
	_messageFrame.reset(new Frame(&_menuBgSprite, Vector2(640 - 2 * _messageOutMargin, 0)));
	_messageFrame->position = Vector3(_messageOutMargin, 0, .9);


	_mainMenu.reset(new Menu(&_menuBgSprite, _font.get(), &_menuInputs));
	_switchMenu.reset(new Menu(&_menuBgSprite, _font.get(), &_menuInputs,
	                           closeMenuFunc()));
	_spellMenu.reset(new Menu(&_menuBgSprite, _font.get(), &_menuInputs,
	                          closeMenuFunc()));
	_summonMenu.reset(new Menu(&_menuBgSprite, _font.get(), &_menuInputs,
	                           closeMenuFunc()));
	_pcMenu.reset(new Menu(&_menuBgSprite, _font.get(), &_menuInputs,
	                       closeMenuFunc()));

	_mainMenu->addEntry("Attack", Menu::ENABLED,
	                    openMenuFunc(_pcMenu.get(), _mainMenu.get(), 0));
	_mainMenu->addEntry("Switch", Menu::ENABLED,
	                    openMenuFunc(_switchMenu.get(), _mainMenu.get(), 1));
	_mainMenu->addEntry("Spell", Menu::ENABLED,
	                    openMenuFunc(_spellMenu.get(),  _mainMenu.get(), 2));
	_mainMenu->addEntry("Summon", Menu::ENABLED,
	                    openMenuFunc(_summonMenu.get(), _mainMenu.get(), 3));
	_mainMenu->addEntry("Scan");
	_mainMenu->addEntry("QTE",     Menu::HIDDEN);
	_mainMenu->addEntry("Twist 1", Menu::HIDDEN);
	_mainMenu->layout();
	_mainMenu->show(Vector3(0, 0, 0));

	_switchMenu->addEntry("None", Menu::ENABLED, doActionFunc());
	_switchMenu->addEntry("Fire", Menu::ENABLED, doActionFunc());
	_switchMenu->addEntry("Ice", Menu::ENABLED, doActionFunc());
	_switchMenu->addEntry("Thunder", Menu::ENABLED, doActionFunc());
	_switchMenu->addEntry("Acid", Menu::ENABLED, doActionFunc());
	_switchMenu->layout();
	_switchMenu->show(Vector3(_mainMenu->width(), 16, .1));

	_spellMenu->addEntry("Storm", Menu::ENABLED, doActionFunc());
	_spellMenu->addEntry("Strike", Menu::ENABLED,
	                     openMenuFunc(_pcMenu.get(), _spellMenu.get(), 1));
	_spellMenu->addEntry("Crippling strike", Menu::ENABLED,
	                     openMenuFunc(_pcMenu.get(), _spellMenu.get(), 2));
	_spellMenu->addEntry("Soul drain", Menu::ENABLED,
	                     openMenuFunc(_pcMenu.get(), _spellMenu.get(), 3));
	_spellMenu->addEntry("Vorpal sword", Menu::ENABLED,
	                     openMenuFunc(_pcMenu.get(), _spellMenu.get(), 4));
	_spellMenu->addEntry("Mud pit", Menu::ENABLED, doActionFunc());
	_spellMenu->addEntry("Dispel magic", Menu::ENABLED,
	                     openMenuFunc(_pcMenu.get(), _spellMenu.get(), 6));
	_spellMenu->layout();
	_spellMenu->show(Vector3(_mainMenu->width(), 16, .1));

	_summonMenu->addEntry("Sprites", Menu::ENABLED, doActionFunc());
	_summonMenu->addEntry("Tomberry", Menu::ENABLED, doActionFunc());
	_summonMenu->addEntry("Mageling", Menu::ENABLED, doActionFunc());
	_summonMenu->layout();
	_summonMenu->show(Vector3(_mainMenu->width(), 16, .1));

	_pcMenu->addEntry(_pcName[0], Menu::ENABLED, doActionFunc());
	_pcMenu->addEntry(_pcName[1], Menu::ENABLED, doActionFunc());
	_pcMenu->addEntry(_pcName[2], Menu::ENABLED, doActionFunc());
	_pcMenu->addEntry(_pcName[3], Menu::ENABLED, doActionFunc());
	_pcMenu->layout();
	_pcMenu->show(Vector3(_mainMenu->width(), 16, .2));

	_initialized = true;
}


void MainState::shutdown() {
	_menuStack.clear();
	_mainMenu.reset();

	_slotTracker.disconnectAll();

	_initialized = false;
}


void MainState::run() {
	lairAssert(_initialized);

	_running = true;

	init();

	log().log("Starting main state...");
	_loop.start();
	_fpsTime  = _game->sys()->getTimeNs();
	_fpsCount = 0;
	while(_running) {
		switch(_loop.nextEvent()) {
		case InterpLoop::Tick:
			updateTick();
			break;
		case InterpLoop::Frame:
			updateFrame();
			break;
		}
	}
	log().log("Stopping main state...");
	_loop.stop();
}


void MainState::quit() {
	_running = false;
}


void MainState::layoutScreen() {
	int w = _game->window()->width();
	int h = _game->window()->height();
	glViewport(0, 0, w, h);
	_camera.setViewBox(Box3(
		Vector3(  0,           0, -1),
		Vector3(640, 640 * h / w,  1)
	));
}


EntityRef MainState::createSprite(Sprite* sprite, const Vector3& pos,
                                  const char* name) {
	return createSprite(sprite, pos, Vector2(1, 1), name);
}


EntityRef MainState::createSprite(Sprite* sprite, const Vector3& pos,
                                  const Vector2& scale, const char* name) {
	EntityRef entity = _entities.createEntity(_entities.root(), name);
	_sprites.addComponent(entity);
	entity.sprite()->setSprite(sprite);
	entity.setTransform(Translation(pos) * Eigen::Scaling(scale.x(), scale.y(), 1.f));
	return entity;
}


EntityRef MainState::createText(const std::string& text, const Vector3& pos,
                                      const Vector4& color) {
	EntityRef entity = _entities.createEntity(_entities.root(), "text");
	_texts.addComponent(entity);
	TextComponent* comp = _texts.get(entity);
	comp->font = _font.get();
	comp->text = text;
	comp->color = color;
	Transform t = Transform::Identity();
	t.translate(pos);
	entity.setTransform(t);
	return entity;
}


EntityRef MainState::createDamageText(const std::string& text, const Vector3& pos,
                                      const Vector4& color) {
	return createText(text, pos, color);
}


EntityRef MainState::createHealthBar(const Vector3& pos, float size) {
	EntityRef parent = _entities.createEntity(_entities.root());
	parent.setTransform(Transform(Translation(pos)));

	Box2 view(Vector2(0, 0), Vector2(size, 1));
	EntityRef empty = _entities.createEntity(parent, "healthEmpty");
	_sprites.addComponent(empty);
	empty.sprite()->setSprite(&_healthEmptySprite);
	empty.sprite()->setView(view);

	EntityRef full = _entities.createEntity(parent, "healthFull");
	_sprites.addComponent(full);
	full.sprite()->setSprite(&_healthFullSprite);
	full.sprite()->setView(view);
	full.setTransform(Transform(Translation(Vector3(0, 0, 0.01))));

	return full;
}


void MainState::init() {
	log().log("Initialize main state.");

	_bg = _entities.createEntity(_entities.root(), "bg");
	_sprites.addComponent(_bg);
	_bg.sprite()->setSprite(&_bgSprite);
	_bg.setTransform(Transform(Translation(
	               Vector3(0, _camera.viewBox().max().y() - 480, -.99))));


	_boss = createSprite(&_boss1Sprite,
	                     Vector3(240, _camera.viewBox().max().y() - 240, -.815),
	                     Vector2(-1, 1), "Boss");

	Vector3 bhpPos(10, _camera.viewBox().max().y() - 22, 0);
	Vector3 bhpOffset(0, -22, 0);
	for(unsigned i = 0; i < 3; ++i) {
		_bossHealthFull[i] = createHealthBar(bhpPos + i * bhpOffset, 1);
	}

	Vector3 closestPos(_camera.viewBox().max().x() - 30,
	                   _camera.viewBox().max().y() - 240, -.8);
	Vector3 offset(-50, 14, -.01);

	Vector3 namePos(300, 94, -0.01);
	Vector3 nameOffset(0, -24, 0);
	Vector3 hpPos = namePos + Vector3(110, -8, 0);

	_maxPcHp = 0;
	for(PC& pc: _fight.party) {
		_maxPcHp = std::max(_maxPcHp, pc.hp);
	}
	for(unsigned pc = 0; pc < 4; ++pc) {
		_pc[pc] = createSprite(&_pcSprite[pc], closestPos + pc * offset,
		                       Vector2(-1, 1), _pcName[pc].c_str());
		createText(_pcName[pc], namePos + pc * nameOffset);
		_pcHealthFull[pc] = createHealthBar(hpPos + pc * nameOffset,
		                                    float(_fight.party[pc].hp) / _maxPcHp);
	}

//	EntityRef test = _entities.createEntity(_entities.root(), "test");
//	_sprites.addComponent(test);
//	test.sprite()->setSprite(&_menuBgSprite);

	showMessage("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec a diam lectus. Sed sit amet ipsum mauris. Maecenas congue ligula ac quam viverra nec consectetur ante hendrerit. Donec et mollis dolor. Praesent et diam eget libero egestas mattis sit amet vitae augue.");
	showMessage("Nam tincidunt congue enim, ut porta lorem lacinia consectetur.");
	showMessage("Donec ut libero sed arcu vehicula ultricies a non tortor. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean ut gravida lorem. Ut turpis felis, pulvinar a semper sed, adipiscing id dolor. Pellentesque auctor nisi id magna consequat sagittis. Curabitur dapibus enim sit amet elit pharetra tincidunt feugiat nisl imperdiet. Ut convallis libero in urna ultrices accumsan. Donec sed odio eros.");
}


void MainState::updateTick() {
	if(_state == PLAYING
	&& (_loop.tickCount() % 30) == 0) { // 2 turns per second
		log().warning("Turn");
		if(_fight.game_over()) {
			_state = GAME_OVER;
		} else if(_fight.tick_fight()) {
			_state = BOSS_TURN;
			for(unsigned i = 0; i < 4; ++i) {
				if(_fight.can_haz(PUNCH, i)) {
					_pcMenu->enableEntry(i);
				} else {
					_pcMenu->disableEntry(i);
				}
			}
			openMenu(_mainMenu.get());
		}

		for(unsigned pc = 0; pc < 4; ++pc) {
			_pcHealthFull[pc].sprite()->setView(
			            Box2(Vector2(0, 0), Vector2(float(_fight.party[pc].hp) / _maxPcHp, 1)));
		}
	}
}


void MainState::updateFrame() {
	_inputs.sync();
	if(!_messages.empty()) {
		if(_menuInputs.ok->justPressed()
		|| _menuInputs.cancel->justPressed()) {
			nextMessage();
		}
	} else if(!_menuStack.empty()) {
		_menuStack.back()->update();
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_game->renderer()->mainBatch().clearBuffers();

	_entities.updateWorldTransform();
	_sprites.render(_loop.frameInterp(), _camera);
	_texts.render(  _loop.frameInterp(), _game->renderer());

	_statusFrame->render(_game->renderer());

	if(_messages.empty()) {
		for(Menu* menu: _menuStack) {
			menu->render(_game->renderer());
		}
	} else {
		_messageFrame->render(_game->renderer());

		_font->render(_game->renderer(),
		              Vector3(_messageOutMargin + _messageMargin,
		                      _messageTextHeight, 0.95),
		              Vector4(1, 1, 1, 1),
		              _messages.front());
	}

	_game->renderer()->spriteShader()->use();
	_game->renderer()->spriteShader()->setTextureUnit(0);
	_game->renderer()->spriteShader()->setViewMatrix(_camera.transform());
	_game->renderer()->mainBatch().render();

	_game->window()->swapBuffers();

	uint64 now = _game->sys()->getTimeNs();
	++_fpsCount;
	if(_fpsCount == 60) {
		log().info("Fps: ", _fpsCount * 1000000000. / (now - _fpsTime));
		_fpsTime  = now;
		_fpsCount = 0;
	}

	LAIR_LOG_OPENGL_ERRORS_TO(log());
}


void MainState::showMessage(const std::string& message) {
	_messages.push_back(_font->layoutText(
	                        message, 640 - (_messageMargin + _messageOutMargin) * 2));
	if(_messages.size() == 1) {
		layoutMessage();
	}
}


void MainState::nextMessage() {
	lairAssert(!_messages.empty());
	_messages.pop_front();
	if(!_messages.empty()) {
		layoutMessage();
	}
}


void MainState::layoutMessage() {
	unsigned nLines = std::count(_messages.front().begin(), _messages.front().end(), '\n') + 1;
	_messageFrame->size.y() = _messageMargin * 2 + nLines * _font->height();
	_messageFrame->position.y() = _camera.viewBox().max().y() - 8 - _messageFrame->size.y();
	_messageTextHeight = _camera.viewBox().max().y() - _messageOutMargin
	        - _messageMargin - _font->baselineToTop;
}


void MainState::openMenu(Menu* menu, Menu* parent, unsigned entry) {
	if(parent) {
		menu->_frame.position.head<2>() = parent->_frame.position.head<2>()
		                                + parent->_frame.size;
		menu->_frame.position.y() -=
		        menu->height() + _font->height() * entry;
		menu->_frame.position.y() = std::max(menu->_frame.position.y(), 0.f);
	}
	_menuStack.push_back(menu);
}


Menu::Callback MainState::openMenuFunc(Menu* menu, Menu* parent, unsigned entry) {
	return std::bind(&MainState::openMenu, this, menu, parent, entry);
}


void MainState::closeMenu() {
	_menuStack.pop_back();
}


Menu::Callback MainState::closeMenuFunc() {
	return std::bind(&MainState::closeMenu, this);
}


Logger& MainState::log() {
	return _game->log();
}


void MainState::doAction() {
	lairAssert(!_menuStack.empty());
	switch(_menuStack[0]->selected()) {
	case 0: { // attack
		_fight.curse(PUNCH, _menuStack.back()->selected());
		break;
	}
	case 1: { // spell
		break;
	}
	case 2: { // summon
		break;
	}
	}

	_state = PLAYING;
	while(!_menuStack.empty()) {
		closeMenu();
	}
}


Menu::Callback MainState::doActionFunc() {
	return std::bind(&MainState::doAction, this);
}
