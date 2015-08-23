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

Rules::Rules(Logger& logger, Fight* fight, const string& ruleset)
:	_logger(&logger),
	_fight(fight),
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
	boss_target = party_size;
}

Rules::~Rules()
{
}

Logger& Rules::log()
{
	return _logger;
}

bool Rules::can_haz (Curse curse, Target t)
{
	// Tier test.
	switch (curse)
	{
		case VORPAL:
		case DISPEL:
			if (_fight->tier < 2) return false;
		case SUMMON + TOMBERRY:
		case SUMMON + SPRITES:
		case STORM:
		case MUD:
		case SWITCH:
			if (_fight->tier < 1) return false;
		case SUMMON + MAGELING:
		case CRIPPLE:
		case DRAIN:
		case STRIKE:
		case PUNCH:
		case SCAN:
			break;
		default:
			assert(curse < NB_CURSES);
			return false;
	}

	// Target test.
	switch (curse)
	{
		// Untargeted spells.
		case SUMMON + SPRITES:
		case SUMMON + MAGELING:
		case SUMMON + TOMBERRY:
			// Don't get summonning sickness.
			if (_fight->horde.size() == max_summons)
				return false;
		case STORM:
		case SCAN:
		case MUD:
			if (t == -1)
				return true;
			else return false;

		// Target must be a live PC.
		case PUNCH:
		case STRIKE:
		case VORPAL:
		case CRIPPLE:
		case DRAIN:
		case DISPEL:
			if (t < party_size && _fight->party[t].hp != 0)
				return true;
			else return false;

		// Target must be a valid element.
		case SWITCH:
			if (t < NB_ELEMS && _fight->boss.elem != t)
				return true;
			else return false;

		// Nope.
		default:
			return false;
	}
}

void Rules::curse (Curse c, Target t)
{
	// Sanity check.
	assert (can_haz(c, t));
	
	switch (c)
	{
		case PUNCH:
			damage(t, curse_power[c], _fight->boss.elem);
			break;

		// Unimplemented.
		default://TODO
			log().log("I'm sorry, Dave. I'm afraid I can't do that.");
	}
}

void Rules::play (Target user, Spell s, Target t)
{
	// Sanity check.
	assert (s < NB_SPELLS);
	assert (user != boss_target);
	assert (user < boss_target + 1 + _fight->horde.size());

	switch (s)
	{
		case AA:
			//TODO: Add elemental damage for enchanted weapons.
			damage(boss_target, spell_power[s], NONE);
			break;

		// Unimplemented.
		default://TODO
			log().log("I cannot obey this command because I'm not a wombat.");
	}

	// User is a PC.
	if (user < boss_target)
	{
		PC& pc = _fight->party[user];
		pc.init = init[pc.job];
	}
	else // User is a minion.
	{
		Minion& m = _fight->horde[user - boss_target - 1];
		m.init = minion_init[m.spawn];
	}
}

double Rules::elem_factor (Element attack, Element defense)
{
	// No elemental defense.
	if (defense == NONE)
		return (attack == NONE)?1.0:elem_multiplier[0];

	// Trying to attack the same element.
	if (attack == defense)
	{
		log().log("It's not very effective...");
		return elem_multiplier[2];
	}

	//TODO: Double-check this hastily-written test.
	// Attacking the opposite element.
	if (defense+1 / 2 == attack+1 / 2)
	{
		log().log("It's super effective !");
		return elem_multiplier[1];
	}

	// No match in elemental defense.
	return elem_multiplier[0];
}

void Rules::damage (Target t, unsigned amount, Element e)
{
	unsigned dmg = amount;
	// PC target
	if (t < boss_target)
	{
		dmg *= elem_factor(e, NONE);
		unsigned& hp = _fight->party[t].hp;

		if (hp > dmg)
			hp -= dmg;
		else
		{
			hp = 0;
			//TODO: Kill the PC.
		}
	}
	else if (t == boss_target)
	{
		Boss& b = _fight->boss;
		dmg *= elem_factor(e, b.elem);

		if (b.hp > dmg)
			b.hp -= dmg;
		else
		{
			b.hp = 0;
			//TODO: Kill the boss and his minions.
		}
	}
	else // Minion target
	{
		Minion& m = _fight->horde[t - boss_target - 1];
		dmg *= elem_factor(e, m.elem);

		if (m.hp > dmg)
			m.hp -= dmg;
		else
		{
			m.hp = 0;
			_fight->horde.erase(_fight->horde.begin() + t - boss_target - 1);
			//TODO: Kill a minion.
		}
	}
}
