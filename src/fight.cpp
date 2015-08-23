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

Fight::Fight(Logger& logger, void* knowledge)
: _logger(&logger),
  party({
		{HEALER,50,40,6,{},{},{},4},
		{FIGHTER,100,0,5,{},{},{},3},
		{WIZARD,30,60,4,{},{},{},2},
		{NINJA,80,30,5,{},{},{},1},
	})
{
	assert (knowledge == nullptr);

	log().log("Knife fight : BEGIN !");

	// Our antihero.
	boss = {3000,{0},5};
}

Fight::~Fight()
{
}

bool Fight::tick_fight ()
{
	log().log("Clock's ticking !");

	if (boss.init-- == 0)
		return true;

	for (unsigned i = 0 ; i < PARTY_SIZE ; i++)
		if (party[i].init-- == 0)
			play_party(i);

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
	for (unsigned i = 0 ; i < PARTY_SIZE ; i++)
		if (party[i].hp != 0)
			survivors = true;

	if (!survivors)
		log().log("Sadly, no trace of them was ever found.");
	return !survivors;
}

bool Fight::can_haz (Curse curse, unsigned target)
{
	log().log("Checking my mighty ", curse, ".");
	if (curse != PUNCH)
		return false;
	
	if (target < PARTY_SIZE)
	{
		log().log("Punching the face of ", target, ".");
		damage (party[target].hp, 20);
		boss.init = 5;
	}
	return true;
}

Logger& Fight::log()
{
	return _logger;
}

void Fight::play_party (unsigned character)
{
	unsigned c = character;

	// Attak the boss.
	log().log("Player ", c, " says : \"I smite thee, evil one !\"");
	damage (boss.hp, 42);

	// Reset initiative.
	switch (party[c].job)
	{
		case NINJA:
			party[c].init = 3;
			break;
		case FIGHTER:
			party[c].init = 4;
			break;
		case HEALER:
		case WIZARD:
		default:
			party[c].init = 5;
	}
}

bool Fight::damage (unsigned& hitpoints, unsigned amount)
{
	if (amount < hitpoints)
		hitpoints -= amount;
	else
		hitpoints = 0;

	log().log("HP : ", hitpoints, "(-", amount, ")");

	return hitpoints == 0;	
}
