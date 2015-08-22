#ifndef _NONAME_GAME_H
#define _NONAME_GAME_H


#include <lair/core/lair.h>
#include <lair/core/log.h>

#include <lair/utils/path.h>

#include <lair/sys_sdl2/sys_module.h>
#include <lair/sys_sdl2/window.h>

#include <lair/render_gl2/render_module.h>
#include <lair/render_gl2/renderer.h>

#include "main_state.h"

#include "game_state.h"


using namespace lair;


class Game {
public:
	Game();
	~Game();

	Path dataPath() const;

	SysModule*    sys();
	Window*       window();

	RenderModule* renderModule();
	Renderer*     renderer();

	void initialize();
	void shutdown();

	void setNextState(GameState* state);
	void run();
	void quit();

	MainState* mainState();

	Logger& log() { return _logger; }


protected:
	MasterLogger  _mlogger;
	OStreamLogger _logBackend;
	Logger        _logger;

	Path          _dataPath;

	SysModule*    _sys;
	Window*       _window;

	RenderModule* _renderModule;
	Renderer*     _renderer;

	GameState*    _nextState;
	GameState*    _currentState;

	std::unique_ptr<MainState> _mainState;
};


#endif
