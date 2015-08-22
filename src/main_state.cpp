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

	_game->renderer()->preloadTexture("bg.jpg", Texture::NEAREST | Texture::CLAMP);
	Texture* bgTexture = _game->renderer()->getTexture(
	            "bg.jpg", Texture::NEAREST | Texture::CLAMP);
	_bgSprite = Sprite(bgTexture);
	log().warning("BG: ", _bgSprite.width(), "x", _bgSprite.height());

	_initialized = true;
}


void MainState::shutdown() {
	_initialized = false;
}


void MainState::run() {
	lairAssert(_initialized);

	_running = true;

	init();

	log().log("Starting main state...");
	_loop.start();
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
		Vector3(  0, -640 * h / w, -1),
		Vector3(640,            0,  1)
	));
}


void MainState::init() {
	log().log("Initialize main state.");

	_bg = _entities.createEntity(_entities.root(), "bg");
	_sprites.addComponent(_bg);
	_bg.sprite()->setSprite(&_bgSprite);

	log().warning("Bg transform:\n", _bg.transform().matrix());
}


void MainState::updateTick() {

}


void MainState::updateFrame() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_entities.updateWorldTransform();
	_sprites.render(_loop.frameInterp(), _camera);

	_game->renderer()->spriteShader()->use();
	_game->renderer()->spriteShader()->setTextureUnit(0);
	_game->renderer()->spriteShader()->setViewMatrix(_camera.transform());
	_game->renderer()->mainBatch().render();

	_game->window()->swapBuffers();

	LAIR_LOG_OPENGL_ERRORS_TO(log());
}


Logger& MainState::log() {
	return _game->log();
}
