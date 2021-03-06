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

#include "rules.h"


using namespace std;
using namespace lair;

class MainState;

// 0 ... boss_target : target PC
//    boss_target    : target boss
// boss_target ... + : target minion
typedef unsigned Target;

class Fight {
private:
	Logger _logger;
	Logger& log();

	MainState& _mainState;

public:
	// Visible internal data
	Rules& rules;

	Player& player;
	vector<PC> party;
	Boss boss;
	unsigned tier;
	vector<Minion> horde;

	Target boss_target;

	// Engage a fresh fight with player p.
	Fight(Logger& logger, MainState& ms, Rules& r, Player& p, unsigned lvl);
	~Fight();

	// Returns true iff the boss has to play now.
	bool tick_fight();
	// Returns true if the fight is over (either side has been wiped out).
	bool game_over();

	// Returns the boss's current HP/max HP for the current tier.
	double boss_hp_rate();

	// Can curse c be used (on target t) ?
	bool can_haz (Curse c);
	bool can_haz (Curse c, Target t);
	// As the boss, use curse c on target t.
	void curse (Curse c, Target t);

private:
	unsigned imem;

	// Play as one of the PCs.
	void play_party (Target character);

	// Can user use ability s on target t ?
	bool can_do (Target user, Spell s, Target t);
	// As user, use ability s on target t.
	void play (Target user, Spell s, Target t);
	// As one of the PCs, use item i on target t.
	void use (Item i, Target t);

	// What it says on the tin.
	unsigned count_strategic_targets();
	unsigned count_weak_targets ();
	// Pick a bad guy to hit.
	Target pick_target();
	// Find a weak ally to help.
	Target find_weak();
	// Pick an element to strike target t with.
	Element pick_elem (Target t);

	// Roll the dice !
	unsigned rtd (unsigned max);
	unsigned rtd ();
	// Display message on UI.
	void msg (string s);
	// Inflict amount damage with element e to target t.
	void damage (Target t, unsigned amount, Element e);
	// Inflict status s to target t.
	void control (Target t, Status s);
};


#endif
