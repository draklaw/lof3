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

	_bgSprite = _game->renderer()->getSprite("bg.jpg");

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
	unsigned w = _game->window()->width();
	unsigned h = _game->window()->height();
	_camera.setViewBox(Box3(
		Vector3(  0, -640 * h / w, -1),
		Vector3(640,            0,  1)
	));
}


void MainState::init() {
	log().log("Initialize main state.");

	_bg = _entities.createEntity(_entities.root(), "bg");
	_sprites.addComponent(_bg);
	_bg.sprite()->setSprite(_bgSprite);
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
