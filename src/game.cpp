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


#include <iostream>
#include <functional>

#include "main_state.h"

#include "game.h"


#define DEFAULT_LOG_LEVEL LogLevel::Debug


Game::Game()
    : _mlogger(),
      _logBackend(std::cerr, true),
      _logger("game", &_mlogger, DEFAULT_LOG_LEVEL),

      _dataPath(),

      _sys(nullptr),
      _window(nullptr),

      _renderModule(nullptr),
      _renderer(nullptr),

      _audio(nullptr),

      _nextState(nullptr),
      _currentState(nullptr),

      _mainState(nullptr) {
	_mlogger.addBackend(&_logBackend);
	dbgLogger.setMaster(&_mlogger);
	dbgLogger.setDefaultModuleName("DEBUG");
	dbgLogger.setLevel(LogLevel::Debug);

	log().log("Starting game...");

#ifdef DATA_DIR
	_dataPath = boost::filesystem::canonical(DATA_DIR);
#else
	_dataPath = lair::exePath(argv[0]);
#endif

	log().log("Data directory: ", _dataPath.native());
}


Game::~Game() {
	log().log("Stopping game...");
}


Path Game::dataPath() const {
	return _dataPath;
}


SysModule* Game::sys() {
	return _sys.get();
}


Window* Game::window() {
	return _window;
}


RenderModule* Game::renderModule() {
	return _renderModule.get();
}


Renderer* Game::renderer() {
	return _renderer;
}


SoundPlayer* Game::audio() {
	return _audio.get();
}


void Game::initialize() {
	_sys.reset(new SysModule(&_mlogger, DEFAULT_LOG_LEVEL));
	_sys->initialize();
	_sys->onQuit = std::bind(&Game::quit, this);
	_sys->loader().setNThread(1);
	_sys->loader().setBasePath(dataPath());

	_window = _sys->createWindow("lof3", 1280, 720);
	//_window->setFullscreen(true);
	_sys->setVSyncEnabled(false);
	log().info("VSync: ", _sys->isVSyncEnabled()? "on": "off");

	_renderModule.reset(new RenderModule(sys(), &_mlogger, DEFAULT_LOG_LEVEL));
	_renderModule->initialize();
	_renderer = _renderModule->createRenderer();

	_audio.reset(new SoundPlayer(this));

	_mainState.reset(new MainState(this));
	_mainState->initialize();
}


void Game::shutdown() {
	_mainState->shutdown();
	_mainState.reset();

	_renderModule->shutdown();
	_renderModule.reset();

	_window->destroy();
	_sys->shutdown();
	_sys.reset();
}


void Game::setNextState(GameState* state) {
	if(_nextState) {
		log().warning("Setting next state while an other state is enqueued.");
	}
	_nextState = state;
}


void Game::run() {
	while(_nextState) {
		_currentState = _nextState;
		_nextState    = nullptr;
		_currentState->run();
	}
}


void Game::quit() {
	if(_currentState) {
		_currentState->quit();
	}
}


MainState* Game::mainState() {
	return _mainState.get();
}
