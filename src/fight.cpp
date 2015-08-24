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

Fight::Fight(Logger& logger, Rules& r, Player& p)
:	_logger(&logger),
	rules(r),
	player(p),
	boss{rules.boss_hp[0], NONE, {0}, {0}, rules.boss_init},
	tier(0)
{
	//log().setLevel(LogLevel::Warning);
	log().info("Knife fight : BEGIN !");

	boss_target = rules.party_size;

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

Logger& Fight::log()
{
	return _logger;
}

bool Fight::tick_fight ()
{
	log().info("Clock's ticking ! Status : ", boss.hp, " vs. ", party[0].hp, "/", party[1].hp, "/", party[2].hp, "/", party[3].hp);

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
		log().info("The Overlord has fallen.");
		return true;
	}

	bool survivors = false;
	for (PC& pc : party)
		if (pc.hp != 0)
			survivors = true;

	if (!survivors)
		log().info("Sadly, no trace of them was ever found.");
	return !survivors;
}

bool Fight::can_haz (Curse curse, Target t)
{
	// Tier test.
	switch (curse)
	{
		case VORPAL:
		case DISPEL:
			if (tier < 2) return false;
		case SUMMON + TOMBERRY:
		case SUMMON + SPRITES:
		case STORM:
		case MUD:
		case SWITCH:
			if (tier < 1) return false;
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
			if (horde.size() == rules.max_summons)
				return false;
		case STORM:
		case SCAN:
		case MUD:
			if (t == (unsigned) -1)
				return true;
			else return false;

		// Target must be a live PC.
		case PUNCH:
		case STRIKE:
		case VORPAL:
		case CRIPPLE:
		case DRAIN:
		case DISPEL:
			if (t < rules.party_size && party[t].hp != 0)
				return true;
			else return false;

		// Target must be a valid element.
		case SWITCH:
			if (t < NB_ELEMS && boss.elem != t)
				return true;
			else return false;

		// Nope.
		default:
			return false;
	}
}

void Fight::curse (Curse c, Target t)
{
	// Sanity check.
	assert (can_haz(c, t));
	
	switch (c)
	{
		case PUNCH:
			damage(t, rules.curse_power[c], boss.elem);
			break;

		// Unimplemented.
		default://TODO
			log().info("I'm sorry, Dave. I'm afraid I can't do that.");
	}
}

void Fight::play (Target user, Spell s, Target t)
{
	// Sanity check.
	assert (s < NB_SPELLS);
	assert (user != boss_target);
	assert (user < boss_target + 1 + horde.size());

	switch (s)
	{
		case AA:
			//TODO: Add elemental damage for enchanted weapons.
			damage(boss_target, rules.spell_power[s], NONE);
			break;

		// Unimplemented.
		default://TODO
			log().info("I cannot obey this command because I'm not a wombat.");
	}

	// User is a PC.
	if (user < boss_target)
	{
		PC& pc = party[user];
		pc.init = rules.init[pc.job];
	}
	else // User is a minion.
	{
		Minion& m = horde[user - boss_target - 1];
		m.init = rules.minion_init[m.spawn];
	}
}

void Fight::play_party (Target character)
{
	Target c = character;

	// Attak the boss.
	log().info("Player ", c, " says : \"I smite thee, evil one !\"");
	play(c, AA, boss_target);
}

void Fight::damage (Target t, unsigned amount, Element e)
{
	unsigned dmg = amount;
	// PC target
	if (t < boss_target)
	{
		dmg *= rules.elem_factor(e, NONE);
		unsigned& hp = party[t].hp;

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
		Boss& b = boss;
		dmg *= rules.elem_factor(e, b.elem);

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
		Minion& m = horde[t - boss_target - 1];
		dmg *= rules.elem_factor(e, m.elem);

		if (m.hp > dmg)
			m.hp -= dmg;
		else
		{
			m.hp = 0;
			horde.erase(horde.begin() + t - boss_target - 1);
			//TODO: Kill a minion.
		}
	}
}
