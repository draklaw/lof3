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

#include "rules.h"

#include "fight.h"

Rules::Rules(Logger& logger, const string& ruleset)
:	_logger(&logger),
	bab{3,1,1,5},
	hd{30,15,10,20},
	mp{},
	unlocks{},
	powerup{1,0},
	init{4,5,5,3},
	boss_hp{5000,3500,1000},
	boss_init(2),
	minion_hp{30,150,200},
	minion_init{2,6,15},
	minion_power {50,30,(unsigned)-1},
	spell_power{10,0},
	spell_utility{},
	spell_cooldown{},
	spell_manacost{},
	curse_power{20,0},
	curse_utility{},
	curse_cooldown{},
	elem_multiplier{1.5,3.0,-0.5},
	max_summons(5),
	party_size(4)
{
	//log().setLevel(LogLevel::Warning);
}

Rules::~Rules()
{
}

Logger& Rules::log()
{
	return _logger;
}

double Rules::elem_factor (Element attack, Element defense)
{
	// No elemental defense.
	if (defense == NONE)
		return (attack == NONE)?1.0:elem_multiplier[0];

	// Trying to attack the same element.
	if (attack == defense)
	{
		log().info("It's not very effective...");
		return elem_multiplier[2];
	}

	//TODO: Double-check this hastily-written test.
	// Attacking the opposite element.
	if (defense+1 / 2 == attack+1 / 2)
	{
		log().info("It's super effective !");
		return elem_multiplier[1];
	}

	// No match in elemental defense.
	return elem_multiplier[0];
}

unsigned Rules::max_hp (PC c)
{
	return hd[c.job] * c.xp;
}

unsigned Rules::max_mp (PC c)
{
	return mp[c.job] * c.xp;
}