#ifndef _NONAME_GAME_STATE_H
#define _NONAME_GAME_STATE_H


#include <lair/core/lair.h>
#include <lair/core/log.h>


using namespace lair;


class GameState {
public:
	virtual void initialize() = 0;
	virtual void shutdown() = 0;

	virtual void run() = 0;
	virtual void quit() = 0;

protected:
};


#endif
