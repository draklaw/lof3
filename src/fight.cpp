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


#include "fight.h"

// Hard-coded data
unsigned ClassDPS[] = { 4, 1, 2, 5 };

Fight::Fight(Logger& logger, Player& p)
:	_logger(&logger),
	player(p),
	rules(logger, this, ""), //TODO: Provide ruleset.
	boss{rules.boss_hp[0], NONE, {0}, {0}, rules.boss_init},
	tier(0)
{
	log().setLevel(LogLevel::Warning);
	log().log("Knife fight : BEGIN !");

	party.push_back({
		FIGHTER, 12,
		rules.hd[FIGHTER] * 12,
		rules.mp[FIGHTER] * 12,
		NULL,
		{0}, {0}, {0},
		0,
		rules.init[FIGHTER]
	});
	party.push_back({
		HEALER, 11,
		rules.hd[HEALER] * 11,
		rules.mp[HEALER] * 11,
		NULL,
		{0}, {0}, {0},
		0,
		rules.init[HEALER]
	});
	party.push_back({
		WIZARD, 10,
		rules.hd[WIZARD] * 10,
		rules.mp[WIZARD] * 10,
		NULL,
		{0}, {0}, {0},
		0,
		rules.init[WIZARD]
	});
	party.push_back({
		NINJA, 11,
		rules.hd[NINJA] * 12,
		rules.mp[NINJA] * 12,
		NULL,
		{0}, {0}, {0},
		0,
		rules.init[NINJA]
	});
}

Fight::~Fight()
{
}

bool Fight::tick_fight ()
{
	log().log("Clock's ticking ! Status : ", boss.hp, " vs. ", party[0].hp, "/", party[1].hp, "/", party[2].hp, "/", party[3].hp);

	for (unsigned i = 0 ; i < party.size() ; i++)
		if (party[i].init-- == 0)
			play_party(i);

	if (boss.init-- == 0)
	{
		boss.init = rules.boss_init;
		return true;
	}

	return false;
}

bool Fight::game_over ()
{
	if (boss.hp == 0)
	{
		log().log("The Overlord has fallen.");
		return true;
	}

	bool survivors = false;
	for (PC& pc : party)
		if (pc.hp != 0)
			survivors = true;

	if (!survivors)
		log().log("Sadly, no trace of them was ever found.");
	return !survivors;
}

Logger& Fight::log()
{
	return _logger;
}

void Fight::play_party (Target character)
{
	Target c = character;

	// Attak the boss.
	log().log("Player ", c, " says : \"I smite thee, evil one !\"");
	rules.play(c, AA, rules.boss_target);
}
