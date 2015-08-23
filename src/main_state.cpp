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
      _inputs(_game->sys(), &_game->log()),

      _slotTracker(),

      _camera(),

      _initialized(false),
      _running(false),
      _loop(_game->sys()),
      _fpsTime(0),
      _fpsCount(0),

      _menuInputs(),

      _fontTex(nullptr),
      _fontJson(),
      _font(),

      _bgSprite(),
      _menuBgSprite(),

      _bg(),

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

	Texture* bgTexture = _game->renderer()->getTexture(
	            "bg.png", Texture::NEAREST | Texture::CLAMP);
	_bgSprite = Sprite(bgTexture);

	Texture* healthEmptyTexture = _game->renderer()->getTexture(
	            "health_bar_empty.png", Texture::NEAREST | Texture::CLAMP);
	_healthEmptySprite = Sprite(healthEmptyTexture);

	Texture* healthFullTexture = _game->renderer()->getTexture(
	            "health_bar_full.png", Texture::NEAREST | Texture::CLAMP);
	_healthFullSprite = Sprite(healthFullTexture);

	Texture* menuTexture = _game->renderer()->getTexture(
	            "menu.png", Texture::NEAREST | Texture::REPEAT);
	_menuBgSprite = Sprite(menuTexture, 3, 3);

	Texture* warriorTexture = _game->renderer()->getTexture(
	            "GTP.png", Texture::NEAREST | Texture::REPEAT);
	_warriorSprite = Sprite(warriorTexture, 1, 1);

	Texture* blackMageTexture = _game->renderer()->getTexture(
	            "MN.png", Texture::NEAREST | Texture::REPEAT);
	_blackMageSprite = Sprite(blackMageTexture, 1, 1);

	Texture* whiteMageTexture = _game->renderer()->getTexture(
	            "MB.png", Texture::NEAREST | Texture::REPEAT);
	_whiteMageSprite = Sprite(whiteMageTexture, 1, 1);

	Texture* ninjaTexture = _game->renderer()->getTexture(
	            "Ninja.png", Texture::NEAREST | Texture::REPEAT);
	_ninjaSprite = Sprite(ninjaTexture, 1, 1);


	_messageMargin = 12;
	_messageOutMargin = 8;
	_messageFrame.reset(new Frame(&_menuBgSprite, Vector2(640 - 2 * _messageOutMargin, 0)));
	_messageFrame->position = Vector3(_messageOutMargin, 0, .9);


	_mainMenu.reset(new Menu(&_menuBgSprite, _font.get(), &_menuInputs));
	_switchMenu.reset(new Menu(&_menuBgSprite, _font.get(), &_menuInputs,
	                           std::bind(&MainState::closeMenu, this)));
	_spellMenu.reset(new Menu(&_menuBgSprite, _font.get(), &_menuInputs,
	                          std::bind(&MainState::closeMenu, this)));
	_summonMenu.reset(new Menu(&_menuBgSprite, _font.get(), &_menuInputs,
	                           std::bind(&MainState::closeMenu, this)));

	_mainMenu->addEntry("Attack");
	_mainMenu->addEntry("Switch", Menu::ENABLED,
	                    std::bind(&MainState::openMenu, this, _switchMenu.get()));
	_mainMenu->addEntry("Spell", Menu::ENABLED,
	                    std::bind(&MainState::openMenu, this, _spellMenu.get()));
	_mainMenu->addEntry("Summon", Menu::ENABLED,
	                    std::bind(&MainState::openMenu, this, _summonMenu.get()));
	_mainMenu->addEntry("Scan");
	_mainMenu->addEntry("QTE",     Menu::HIDDEN);
	_mainMenu->addEntry("Twist 1", Menu::HIDDEN);
	_mainMenu->layout();
	_mainMenu->show(Vector3(0, 0, 0));

	_switchMenu->addEntry("None");
	_switchMenu->addEntry("Fire");
	_switchMenu->addEntry("Ice");
	_switchMenu->addEntry("Thunder");
	_switchMenu->addEntry("Acid");
	_switchMenu->layout();
	_switchMenu->show(Vector3(_mainMenu->width(), 16, .1));

	_spellMenu->addEntry("Storm");
	_spellMenu->addEntry("Strike");
	_spellMenu->addEntry("Crippling strike");
	_spellMenu->addEntry("Soul drain");
	_spellMenu->addEntry("Vorpal sword");
	_spellMenu->addEntry("Mud pit");
	_spellMenu->addEntry("Dispel magic");
	_spellMenu->layout();
	_spellMenu->show(Vector3(_mainMenu->width(), 16, .1));

	_summonMenu->addEntry("Sprites");
	_summonMenu->addEntry("Tomberry");
	_summonMenu->addEntry("Mageling");
	_summonMenu->layout();
	_summonMenu->show(Vector3(_mainMenu->width(), 16, .1));

	openMenu(_mainMenu.get());

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


void MainState::init() {
	log().log("Initialize main state.");

	_bg = _entities.createEntity(_entities.root(), "bg");
	_sprites.addComponent(_bg);
	_bg.sprite()->setSprite(&_bgSprite);
	_bg.setTransform(Transform(Translation(
	               Vector3(0, _camera.viewBox().max().y() - 480, -.99))));


	_warriorHealthEmpty = _entities.createEntity(_entities.root(), "warriorHealthEmpty");
	_sprites.addComponent(_warriorHealthEmpty);
	_warriorHealthEmpty.sprite()->setSprite(&_healthEmptySprite);
	_warriorHealthEmpty.setTransform(Transform(Translation(
	               Vector3(20, _camera.viewBox().max().y() - 20, -.10))));

	_warriorHealthFull = _entities.createEntity(_entities.root(), "warriorHealthFull");
	_sprites.addComponent(_warriorHealthFull);
	_warriorHealthFull.sprite()->setSprite(&_healthFullSprite);
	_warriorHealthFull.setTransform(Transform(Translation(
	               Vector3(20, _camera.viewBox().max().y() - 20, -.05))));
	_warriorHealthFull.sprite()->setView(Box2(Vector2(0, 0), Vector2(.66, 1)));


	Vector3 closestPos(_camera.viewBox().max().x() - 30,
	                   _camera.viewBox().max().y() - 260, -.8);
	Vector3 offset(-50, 18, -.01);

	_warrior = _entities.createEntity(_entities.root(), "warrior");
	_sprites.addComponent(_warrior);
	_warrior.sprite()->setSprite(&_warriorSprite);
	_warrior.setTransform(Translation(closestPos + 0 * offset) * Eigen::Scaling(-1, 1, 1));

	_blackMage = _entities.createEntity(_entities.root(), "blackMage");
	_sprites.addComponent(_blackMage);
	_blackMage.sprite()->setSprite(&_blackMageSprite);
	_blackMage.setTransform(Translation(closestPos + 1 * offset) * Eigen::Scaling(-1, 1, 1));

	_whiteMage = _entities.createEntity(_entities.root(), "whiteMage");
	_sprites.addComponent(_whiteMage);
	_whiteMage.sprite()->setSprite(&_whiteMageSprite);
	_whiteMage.setTransform(Translation(closestPos + 2 * offset) * Eigen::Scaling(-1, 1, 1));

	_ninja = _entities.createEntity(_entities.root(), "ninja");
	_sprites.addComponent(_ninja);
	_ninja.sprite()->setSprite(&_ninjaSprite);
	_ninja.setTransform(Translation(closestPos + 3 * offset) * Eigen::Scaling(-1, 1, 1));

//	EntityRef test = _entities.createEntity(_entities.root(), "test");
//	_sprites.addComponent(test);
//	test.sprite()->setSprite(&_menuBgSprite);

	showMessage("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec a diam lectus. Sed sit amet ipsum mauris. Maecenas congue ligula ac quam viverra nec consectetur ante hendrerit. Donec et mollis dolor. Praesent et diam eget libero egestas mattis sit amet vitae augue.");
	showMessage("Nam tincidunt congue enim, ut porta lorem lacinia consectetur.");
	showMessage("Donec ut libero sed arcu vehicula ultricies a non tortor. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean ut gravida lorem. Ut turpis felis, pulvinar a semper sed, adipiscing id dolor. Pellentesque auctor nisi id magna consequat sagittis. Curabitur dapibus enim sit amet elit pharetra tincidunt feugiat nisl imperdiet. Ut convallis libero in urna ultrices accumsan. Donec sed odio eros.");
}


void MainState::updateTick() {

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
	log().warning("NLines: ", nLines);
	_messageFrame->size.y() = _messageMargin * 2 + nLines * _font->height();
	_messageFrame->position.y() = _camera.viewBox().max().y() - 8 - _messageFrame->size.y();
	_messageTextHeight = _camera.viewBox().max().y() - _messageOutMargin
	        - _messageMargin - _font->baselineToTop;
}


void MainState::openMenu(Menu* menu) {
	_menuStack.push_back(menu);
}


void MainState::closeMenu() {
	_menuStack.pop_back();
}


Logger& MainState::log() {
	return _game->log();
}
