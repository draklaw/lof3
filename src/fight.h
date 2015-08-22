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


#ifndef _LOF3_FIGHT_H
#define _LOF3_FIGHT_H


#include <lair/core/lair.h>
#include <lair/core/log.h>


using namespace lair;


#define PARTY_SIZE 4

enum Class { FIGHTER, HEALER, WIZARD, NINJA };
enum Element { NONE, FIRE, WATER, EARTH, AIR, NB_ELEMS };
enum Status { NORMAL, SHIELD, SLOW, DISABLE, SILENCE, DEAD, NB_STATUS };
enum Spell { SMITE, HEAL, REZ, NUKES,
	AOES = NUKES+NB_ELEMS, SHIELDS = AOES+NB_ELEMS, NB_SPELLS = SHIELDS+NB_ELEMS
};
enum QTE { EASY, MEDIUM, HARD, NB_QTES };
enum Curse { PUNCH, SWITCH, SCAN, STORM, STRIKE, VORPAL, CRIPPLE, DRAIN, MUD,
	DISPEL, SPRITES, MAGELING, TOMBERRY, PROMPT, NB_CURSES = PROMPT+NB_QTES
};

struct PC {
	Class job;                     // Character class^H^H^H^H^Hjob
	unsigned xp;                   // Level
	unsigned hp;                   // Health
	unsigned mp;                   // Mana
	unsigned resists[NB_ELEMS];    // Current elemental resistances
	bool status[NB_STATUS];     // Active (de)buffs
	unsigned cooldowns[NB_SPELLS]; // Spell cooldowns
	unsigned init;                 // Time to initiative
};

struct Boss {
	unsigned hp;                   // Health bar
	unsigned cooldowns[NB_CURSES]; // Curses cooldowns
	unsigned init;                 // Time to initiative
};

class Fight {
public:
	// Engage a fresh fight using NULL knowledge.
	Fight(Logger& logger, void* knowledge);
	~Fight();

	// Returns true iff the boss has to play now.
	bool tick_fight();

	// Can the bad guy do X ? If so, do it on target (if a valid one is provided).
	bool can_haz (Curse curse, unsigned target);

	// Returns true if the fight is over (either side has been wiped out).
	bool game_over ();

private:
	Logger _logger;
	Logger& log();

	PC party[PARTY_SIZE];
	Boss boss;

	// Play as one of the PCs.
	void play_party (unsigned character);

	// Substract amount from hitpoints total, return true iff death ensues.
	bool damage (unsigned& hitpoints, unsigned amount);
};


#endif
