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


#include "font.h"
#include "menu.h"
#include "game.h"

#include "main_state.h"


MainState::MainState(Game* game)
	: _game(game),
      _entities(_game->log()),
      _sprites(_game->renderer()),
      _camera(),
      _running(false),
      _loop(_game->sys()) {
}


MainState::~MainState() {
}


void MainState::initialize() {
	_loop.reset();
	_loop.setTickDuration(    1000000000 /  60);
	_loop.setFrameDuration(   1000000000 /  60);
	_loop.setMaxFrameDuration(_loop.frameDuration() * 3);
	_loop.setFrameMargin(     _loop.frameDuration() / 2);

	layoutScreen();


	_fontJson = _game->sys()->loader().getJson("8-bit_operator+_bold_23.json");
	_fontTex  = _game->renderer()->getTexture(_fontJson["file"].asString(),
	        Texture::NEAREST | Texture::REPEAT);
	_font.reset(new Font(_fontJson, _fontTex));

	Texture* bgTexture = _game->renderer()->getTexture(
	            "bg.jpg", Texture::NEAREST | Texture::CLAMP);
	_bgSprite = Sprite(bgTexture);

	Texture* menuTexture = _game->renderer()->getTexture(
	            "menu.png", Texture::NEAREST | Texture::REPEAT);
	_menuBgSprite = Sprite(menuTexture, 3, 3);


	_menu.reset(new Menu(&_menuBgSprite, Vector2(128, 128)));
	_menu->show(Vector3(64, 64, 0));

	_initialized = true;
}


void MainState::shutdown() {
	_menu.reset();

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
	_bg.setTransform(Transform(Translation(Vector3(0, _camera.viewBox().max().y() - 480, -.99))));

	EntityRef test = _entities.createEntity(_entities.root(), "test");
	_sprites.addComponent(test);
	test.sprite()->setSprite(&_menuBgSprite);
}


void MainState::updateTick() {

}


void MainState::updateFrame() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_game->renderer()->mainBatch().clearBuffers();

	_entities.updateWorldTransform();
	_sprites.render(_loop.frameInterp(), _camera);

	_menu->render(_game->renderer());

	_font->color = Vector4(1, .5, 0, 1);
	_font->render(_game->renderer(), Vector3(64, 64 + 128 - _font->height(), .99),
	              "Test\naoeu_ht nspq. !?\nThe quick brown fox jumps over the lazy dog",
	              128);

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


Logger& MainState::log() {
	return _game->log();
}
