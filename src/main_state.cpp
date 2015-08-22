#include "game.h"

#include "main_state.h"


MainState::MainState(Game* game)
	: _game(game),
      _entities(_game->log()),
      _sprites(_game->renderer()),
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
}


void MainState::shutdown() {
}


void MainState::run() {
	_running = true;
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


void MainState::updateTick() {

}


void MainState::updateFrame() {

}


Logger& MainState::log() {
	return _game->log();
}
