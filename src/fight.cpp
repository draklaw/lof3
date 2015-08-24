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

double Fight::boss_hp_rate()
{
	return boss.hp / rules.boss_hp[tier];
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
	unsigned drain;
	Spawn s;

	//TODO: Trigger curse-specific cooldown.
	switch (c)
	{
		case PUNCH:
			damage(t, rules.curse_power[c], boss.elem);
			break;
		case SWITCH:
			boss.elem = Element(t);
			break;
		case SCAN:
			//TODO: Display party stats.
			break;
		case STORM:
			for (unsigned i = 0 ; i < rules.party_size ; i++)
				damage(i, rules.curse_power[c], boss.elem);
			break;
		case STRIKE:
			damage(t, rules.curse_power[c], boss.elem);
			break;
		case VORPAL:
			damage(t, rules.curse_power[c], NONE);
			if ((unsigned) rand()%100 < rules.curse_utility[c])
				damage(t, (unsigned) -1, NONE);
			break;
		case CRIPPLE:
			damage(t, rules.curse_power[c], NONE);
			if ((unsigned) rand()%100 < rules.curse_utility[c])
				control(t, DISABLE);
			break;
		case DRAIN:
			drain = rules.curse_power[c];
			if(party[t].mp > drain)
				party[t].mp -= drain;
			else
				party[t].mp = 0;
			if ((unsigned) rand()%100 < rules.curse_utility[c])
				control(t, SILENCE);
			break;
		case MUD:
			for (unsigned i = 0 ; i < rules.party_size ; i++)
				control(i, SLOW);
			break;
		case DISPEL:
			//TODO: Set resistance at what the party[t].equip provides.
			for (unsigned elem = 0 ; elem < NB_ELEMS ; elem++)
				party[t].resist[Element(elem)] = 0;
			break;
		case SUMMON+SPRITES:
		case SUMMON+MAGELING:
		case SUMMON+TOMBERRY:
			s = Spawn(c - SUMMON);
			horde.push_back({s,rules.minion_hp[s],boss.elem,rules.minion_init[s]});
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

	//TODO: Trigger spell-specific cooldown.
	switch (s)
	{
		case AA:
			//TODO: Add elemental damage for enchanted weapons.
			damage(boss_target, rules.spell_power[s], NONE);
			break;
		case SMITE:
		case SLICE:
			damage(boss_target, rules.spell_power[s], NONE);
			break;
		case PROTECT:
			party[t].protector = user;
			break;
		case SWIPE:
			damage(boss_target, rules.spell_power[s], NONE);
			for (unsigned i = 0 ; i < horde.size() ; i++)
				damage(boss_target + i, rules.spell_power[s], NONE);
			break;
		case HEAL:
			//TODO: Heal bad guys, if somehow targeted.
			if (t < boss_target)
			{
				party[t].hp += rules.spell_power[s];
				//TODO: Forbid overhealing.
				//party[t].hp = max(party[t].hp, rules.max_hp(party[t]));
			}
			break;
		case NURSE:
			for (unsigned i = 0 ; i < rules.party_size ; i++)
			{
				party[i].hp += rules.spell_power[s];
				//TODO: Forbid overhealing.
				//party[i].hp = max(party[i].hp, rules.max_hp(party[i]));
			}
			break;
		case REZ:
			if (party[t].hp == 0)
				party[t].hp = 50;
				//FIXME: party[i].hp = rules.max_hp(party[i]) / 4;
			break;
		case NUKES:
		case NUKES + FIRE:
		case NUKES + ICE:
		case NUKES + SPARK:
		case NUKES + ACID:
			damage(t, rules.spell_power[s], Element(s-NUKES));
			break;
		case AOES:
		case AOES + FIRE:
		case AOES + ICE:
		case AOES + SPARK:
		case AOES + ACID:
			damage(boss_target, rules.spell_power[s], Element(s-AOES));
			for (unsigned i = 0 ; i < horde.size() ; i++)
				damage(boss_target + i, rules.spell_power[s], Element(s-AOES));
			break;
		case SHIELDS:
		case SHIELDS + FIRE:
		case SHIELDS + ICE:
		case SHIELDS + SPARK:
		case SHIELDS + ACID:
			//TODO: Increment protection properly iff not currently shielded.
			if (t < boss_target)
				party[t].resist[Element(s-SHIELDS)] = rules.spell_power[s];
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

/* AI decision tree :
- Assess trouble (1-100)
> Improvise ()
> Strategy ()
> Enjoy


*/
}

void Fight::damage (Target t, unsigned amount, Element e)
{
	unsigned dmg = amount;
	// PC target
	if (t < boss_target)
	{
		dmg *= rules.elem_factor(e, NONE);
		unsigned& hp = party[t].hp;

		//TODO: Implement protectors.
		if (hp > dmg)
			hp -= dmg;
		else
		{
			//TODO: Implement resilience.
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

void Fight::control (Target t, Status s)
{
	//TODO: Implement tenacity.
	party[t].status[s] = true;
}