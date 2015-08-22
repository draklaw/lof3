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

      _nextState(nullptr),
      _currentState(nullptr),

      _mainState(new MainState(this)) {
	_mlogger.addBackend(&_logBackend);
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
	return _sys;
}


Window* Game::window() {
	return _window;
}


RenderModule* Game::renderModule() {
	return _renderModule;
}


Renderer* Game::renderer() {
	return _renderer;
}


void Game::initialize() {
	_sys = new SysModule(&_mlogger, DEFAULT_LOG_LEVEL);
	_sys->initialize();
	_sys->onQuit = std::bind(&Game::quit, this);
	_sys->loader().setNThread(1);

	_window = _sys->createWindow("noname", 640, 360);
	log().info("VSync: ", _sys->isVSyncEnabled()? "on": "off");

	_renderModule = new RenderModule(_sys, &_mlogger, DEFAULT_LOG_LEVEL);
	_renderModule->initialize();
	_renderer = _renderModule->createRenderer();

	_mainState->initialize();
}


void Game::shutdown() {
	_mainState->shutdown();

	_renderModule->shutdown();
	delete _renderModule;

	_window->destroy();
	_sys->shutdown();
	delete _sys;
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
