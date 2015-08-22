#ifndef _NONAME_MAIN_STATE_H
#define _NONAME_MAIN_STATE_H


#include <lair/core/lair.h>
#include <lair/core/log.h>

#include <lair/utils/interp_loop.h>

#include <lair/ec/entity.h>
#include <lair/ec/entity_manager.h>
#include <lair/ec/sprite_component.h>

#include "game_state.h"


using namespace lair;


class Game;


class MainState : public GameState {
public:
	MainState(Game* game);
	~MainState();

	virtual void initialize();
	virtual void shutdown();

	virtual void run();
	virtual void quit();

	void updateTick();
	void updateFrame();

	Logger& log();

protected:
	Game* _game;

	EntityManager          _entities;
	SpriteComponentManager _sprites;

	bool       _running;
	InterpLoop _loop;
};


#endif
