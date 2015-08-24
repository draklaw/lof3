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

#include "main_state.h"

// Dummy target
#define NOTARGET (Target) -1

// Confidence values
#define ALIVE_CONF  50.0
#define SLOW_CONF    0.9
#define DISABLE_CONF 0.8
#define SILENCE_CONF 0.8

// Multi-target spells thresholds
#define AOE_THRESHOLD 2
#define NURSE_THRESHOLD 2

// Subjective HP factor of healers
#define HEALER_BIAS 0.8

Fight::Fight(Logger& logger, MainState& ms, Rules& r, Player& p, unsigned lvl)
:	_logger(&logger),
	_mainState(ms),
	rules(r),
	player(p),
	boss{rules.boss_hp[0], NONE, {0}, {0}, rules.boss_init},
	tier(0),
	imem(0)
{
	log().setLevel(LogLevel::Info);
	log().info("Knife fight : BEGIN !");

	boss_target = rules.party_size;

	party.push_back({"Alpha",
		FIGHTER, lvl+rtd(4),
		rules.hd[FIGHTER] * lvl+rtd(4),
		rules.mp[FIGHTER] * lvl+rtd(4),
		NULL,
		{0}, {0}, {0},
		0,
		rules.init[FIGHTER]
	});
	party.push_back({"Beta",
		HEALER, lvl+rtd(4),
		rules.hd[HEALER] * lvl+rtd(4),
		rules.mp[HEALER] * lvl+rtd(4),
		NULL,
		{0}, {0}, {0},
		0,
		rules.init[HEALER]
	});
	party.push_back({"Gamma",
		WIZARD, lvl+rtd(4),
		rules.hd[WIZARD] * lvl+rtd(4),
		rules.mp[WIZARD] * lvl+rtd(4),
		NULL,
		{0}, {0}, {0},
		0,
		rules.init[WIZARD]
	});
	party.push_back({"Delta",
		NINJA, lvl+rtd(4),
		rules.hd[NINJA] * lvl+rtd(4),
		rules.mp[NINJA] * lvl+rtd(4),
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

	for (unsigned& i = imem ; i < party.size() ; i++)
	{
		PC& pc = party[i];

		if (!pc.hp)
			continue;

		if (pc.init == 0)
		{
			for (unsigned s = 0 ; s < NB_SPELLS ; s++)
				if (pc.cooldown[s] != 0)
					pc.cooldown[s]--;

			play_party(i);

			pc.init = rules.init[pc.job] + (pc.status[SLOW] ? 1 : 0);

			return false;
		}
		else
			pc.init--;
	}

	imem = 0;

	if (boss.init-- == 0)
	{
		for (unsigned c = 0 ; c < NB_CURSES ; c++)
			if (boss.curses_cd[c] != 0)
				boss.curses_cd[c]--;

		if (tier < 2 && boss.hp < rules.boss_hp[tier+1])
		{
			string status = (tier == 0) ? "somewhat irritated." : "REALLY PISSED OFF NOW !";
			tier++;
			msg("You are... " + status);
		}
		boss.init = rules.boss_init;
		return true;
	}

	/* Minion.
	{
		Minion& m = horde[user - boss_target - 1];
		m.init = rules.minion_init[m.spawn];
	}*/

	return false;
}

bool Fight::game_over ()
{
	if (boss.hp == 0)
	{
		msg("The Overlord has fallen...");
		return true;
	}

	bool survivors = false;
	for (PC& pc : party)
		if (pc.hp != 0)
			survivors = true;

	if (!survivors)
		msg("Sadly, no trace of them was ever found.");
	return !survivors;
}

double Fight::boss_hp_rate()
{
	return double( boss.hp - (tier==NB_TIERS-1) ? 0 : rules.boss_hp[tier+1] )
	     / rules.boss_hp[tier];
}

bool Fight::can_haz (Curse c)
{
	// Cooldown test.
	if (boss.curses_cd[c] != 0)
		return false;

	// Tier test.
	switch (c)
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
			assert(c < NB_CURSES);
			return false;
	}

	return true;
}

bool Fight::can_haz (Curse c, Target t)
{
	if (!can_haz(c))
		return false;

	// Target test.
	switch (c)
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

	Element e = boss.elem;

	if (c == SWITCH)
		switch (Element(t))
		{
			case NONE:
				msg("You return to your unholy form !");
				break;
			case FIRE:
				msg("You engulf yourself in flames !");
				break;
			case ICE:
				msg("You become a chilling threat !");
				break;
			case SPARK:
				msg("You crackle with tremendous power !");
				break;
			case ACID:
				msg("You start oozing corrosive fluids !");
				break;
			default:
				msg("You become a horrid mess of tentacles ?!");
		}
	else if (c == SCAN)
		msg("Studying the enemy threat...");
	else if (t == NOTARGET)
		msg("You unleash a " + rules.name(c,e) + " !");
	else
		msg("You inflict a " + rules.name(c,e) + " on " + party[t].name + " !");

	boss.curses_cd[c] = rules.curse_cooldown[c];

	switch (c)
	{
		case PUNCH:
			damage(t, rules.curse_power[c], e);
			break;
		case SWITCH:
			boss.elem = Element(t);
			break;
		case SCAN:
			//TODO: Display party stats.
			break;
		case STORM:
			for (unsigned i = 0 ; i < rules.party_size ; i++)
				damage(i, rules.curse_power[c], e);
			break;
		case STRIKE:
			damage(t, rules.curse_power[c], e);
			break;
		case VORPAL:
			damage(t, rules.curse_power[c], NONE);
			if (rtd() < rules.curse_utility[c])
				damage(t, (unsigned) -1, NONE);
			break;
		case CRIPPLE:
			damage(t, rules.curse_power[c], NONE);
			if (rtd() < rules.curse_utility[c])
				control(t, DISABLE);
			break;
		case DRAIN:
			drain = rules.curse_power[c];
			if (party[t].mp > drain)
				party[t].mp -= drain;
			else
				party[t].mp = 0;
			if (rtd() < rules.curse_utility[c])
				control(t, SILENCE);
			break;
		case MUD:
			for (unsigned i = 0 ; i < rules.party_size ; i++)
				if (rtd() < rules.curse_utility[c])
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
			horde.push_back({s,rules.minion_hp[s],e,rules.minion_init[s]});
			break;
		// Unimplemented.
		default://TODO
			log().info("I'm sorry, Dave. I'm afraid I can't do that.");
	}
}

void Fight::play_party (Target character)
{
	Target c = character;

	Element e = NONE;
	Target t = pick_target(), w = find_weak(), d = boss_target;
	unsigned nbt = count_strategic_targets(), nbw = count_weak_targets();

	// Each %HP left on a character gives 1 confidence
	// Confidence is reduced linearly by each status effect
	// Being alive gives additional confidence
	//FIXME: Worry about silence/disable does not take job into account.
	unsigned confidence = 0, max_confidence = 0;
	for (unsigned i = 0 ; i < rules.party_size ; i++)
	{
		max_confidence += ALIVE_CONF + 100;
		if (!party[i].hp)
			d = i;
		else
			confidence += ALIVE_CONF + 100.0 * party[i].hp / rules.max_hp(party[i]);
	}

	//TODO: Cache confidence and stratcount in the fight/player.
	unsigned known_strats = 0;
	for (unsigned i = 0 ; i < NB_STRATS ; i++)
		if (player.strat[i] != 0)
			known_strats++;

	log().info("Confidence value : ",confidence,"/",max_confidence,".");
	log().info("Known strats : ",known_strats,".");

	if (rtd(max_confidence) > confidence)
	{ // Panic !
		//TODO: Improvise when panicking.
		// - List all available actions, pick one.
		// - List all available targets, pick one.
		// - Do that.
		// (- Learn.)

		// Attack the boss.
		log().info("Player ", c, " punches the boss in a panic !");
		play(c, AA, boss_target);
	}
	else if (rtd(NB_STRATS) < known_strats)
	{ // Keep calm and use strategy.
		log().info("Player ", c, " ponders his options.");
		if (player.strat[SPELLS] && !party[c].status[SILENCE])
		{ // I put on my robe and wizard hat.
			log().info("Player ", c, " acts smart.");
			switch (party[c].job)
			{
				case FIGHTER:
					// Protect the weakest, or smite a bad guy, or just punch one.
					if (party[w].protector == rules.party_size && can_do(c, PROTECT, w))
						play (c, PROTECT, w);
					else if (can_do(c, SMITE, t))
						play (c, SMITE, t);
					else
						play (c, AA, t);
					break;
				case NINJA:
					// Cut them down to size or cut him in two, or just punch.
					if (nbt > AOE_THRESHOLD && can_do(c, SWIPE, NOTARGET))
						play (c, SWIPE, NOTARGET);
					else if (can_do(c, SLICE, t))
						play (c, SLICE, t);
					else
						play (c, AA, t);
					break;
				case HEALER:
					// Raise dead, nurse, heal, clear status, raise elem shield or punch.
					if (player.strat[RAISE_DEAD] && d < boss_target && can_do(c, REZ, d))
						play (c, REZ, d);
					else if (player.strat[HEAL_UP]
					      && nbw > NURSE_THRESHOLD
					      && can_do(c, NURSE, w))
						play (c, NURSE, NOTARGET);
					else if (player.strat[HEAL_UP]
					      && party[w].hp < rules.max_hp(party[w])
					      && can_do(c, HEAL, w))
						play(c, HEAL, w);
					/* else if (bad status)
						clear status; */
					//TODO: Improve shield targeting and element picking.
					else if (player.strat[RAISE_SHIELD]
					      && can_do(c, SHIELDS + (e = Element(rtd(NB_ELEMS))), w))
						play(c, SHIELDS + e, w);
					else
						play (c, AA, t);
					break;
				case WIZARD:
					if (nbt > AOE_THRESHOLD
					 && can_do(c, AOES + (e = pick_elem(NOTARGET)), NOTARGET))
						play (c, AOES + e, NOTARGET);
					else if (can_do(c, NUKES + (e = pick_elem(t)), t))
						play (c, NUKES + e, t);
					else
						play (c, AA, t);
					break;
				default:
					log().warning("Whatsisname ", c, "should get a job.");
			}
		}
		else // Euh bah... j'auto-attack ?
			play(c, AA, t);
	}
	else
	{
		// Do what you want 'cause a pirate is free.
		//TODO: Enjoy the ride.
		// - Sum all possible favors and roll against that.
		// - Substract each favor in turn, and pick whatever reaches zero.

		// Attack the boss.
		log().info("Player ", c, " enjoys punching the bad guy.");
		play(c, AA, boss_target);
	}
}

bool Fight::can_do (Target user, Spell s, Target t)
{
	//TODO: Handle minion users.
	
	PC& u = party[user];
	
	//TODO: Check user job.
	
	// Check status using cute little "safety parentheses" around &&.
	if ((u.status[SILENCE] && (u.job == HEALER || u.job == WIZARD))
	 || (u.status[DISABLE] && (u.job == FIGHTER || u.job == NINJA)))
		return s == AA;
	
	// Check mana.
	if (rules.spell_manacost[s] > u.mp)
		return false;
	
	// Check cooldowns.
	if (u.cooldown[s])
		return false;
	
	// Check target liveliness.
	unsigned thp = (unsigned) -1;
	if (t < boss_target)
		thp = party[t].hp;
	else if (t == boss_target)
		thp = boss.hp;
	else
		thp = horde[t - boss_target - 1].hp;
	
	if (thp && s == REZ)
		return false;
	if (!thp && s != REZ)
		return false;
	
	return true;
}

void Fight::play (Target user, Spell s, Target t)
{
	// Sanity check.
	assert (s < NB_SPELLS);
	assert (user != boss_target);
	assert (user < boss_target + 1 + horde.size());

	string verb_msg = " casts ";
	string preposition = " on ";
	if (s == PROTECT)
	{
		verb_msg = " lends ";
		preposition = " to ";
	}
	else if (s == AA || s == SMITE || s == SLICE || s == SWIPE)
	{
		verb_msg = " strikes with ";
		preposition = " against ";
	}

	string target_msg = (t == NOTARGET ? "" :
		(preposition + (t == boss_target ? "you" : party[t].name)));

	msg(party[user].name + verb_msg + rules.name(s) + target_msg + ".");

	// Mana expenditure & cooldown trigger
	party[user].mp -= rules.spell_manacost[s];
	party[user].cooldown[s] = rules.spell_cooldown[s];

	log().info(party[user].name, " spent ", rules.spell_manacost[s], " - left : ", party[user].mp);
	log().info(rules.name(s), " in cooldown for ", party[user].cooldown[s], "turns.");

	// Effect
	switch (s)
	{
		case AA:
			//TODO: Add elemental damage for enchanted weapons.
			damage(boss_target, rules.spellpower(s, party[user]), NONE);
			break;
		case SMITE:
		case SLICE:
			damage(boss_target, rules.spellpower(s, party[user]), NONE);
			break;
		case PROTECT:
			party[t].protector = user;
			break;
		case SWIPE:
			damage(boss_target, rules.spellpower(s, party[user]), NONE);
			for (unsigned i = 0 ; i < horde.size() ; i++)
				damage(boss_target + i, rules.spellpower(s, party[user]), NONE);
			break;
		case HEAL:
			//TODO: Heal bad guys, if somehow targeted.
			if (t < boss_target)
			{
				party[t].hp += rules.spellpower(s, party[user]);
				party[t].hp = min(party[t].hp, rules.max_hp(party[t]));
			}
			break;
		case NURSE:
			for (unsigned i = 0 ; i < rules.party_size ; i++)
			{
				party[i].hp += rules.spellpower(s, party[user]);
				party[i].hp = min(party[i].hp, rules.max_hp(party[i]));
			}
			break;
		case REZ:
			if (party[t].hp == 0) {
				party[t].hp = rules.max_hp(party[t]) / 4;
				_mainState.playRezAnim(t);
			}
			break;
		case NUKES:
		case NUKES + FIRE:
		case NUKES + ICE:
		case NUKES + SPARK:
		case NUKES + ACID:
			damage(t, rules.spellpower(s, party[user]), Element(s-NUKES));
			break;
		case AOES:
		case AOES + FIRE:
		case AOES + ICE:
		case AOES + SPARK:
		case AOES + ACID:
			damage(boss_target, rules.spellpower(s, party[user]), Element(s-AOES));
			for (unsigned i = 0 ; i < horde.size() ; i++)
				damage(boss_target + i, rules.spellpower(s, party[user]), Element(s-AOES));
			break;
		case SHIELDS:
		case SHIELDS + FIRE:
		case SHIELDS + ICE:
		case SHIELDS + SPARK:
		case SHIELDS + ACID:
			//TODO: Increment protection properly iff not currently shielded.
			if (t < boss_target)
				party[t].resist[Element(s-SHIELDS)] = rules.spellpower(s, party[user]);
			break;
		// Unimplemented.
		default://TODO
			log().info("I cannot obey this command because I'm not a wombat.");
	}
}

unsigned Fight::count_strategic_targets ()
{
	unsigned c = 1;

	for (unsigned i = 0 ; i < horde.size() ; i++)
		if ((player.strat[KILL_SPRITES] && horde[i].spawn == SPRITES)
		 || (player.strat[KILL_TOMBERRY] && horde[i].spawn == TOMBERRY))
			c++;

	log().info("Counted ", c, " strategic targets.");
	return c;
}

Target Fight::pick_target()
{
	//TODO: KILL_SPRITES, KILL_TOMBERRY, DPS_RUN or random
	log().info("Targeting... boss.");
	return boss_target;
}

unsigned Fight::count_weak_targets ()
{
	unsigned wc = 0;

	//TODO: Double check.
	for (unsigned i = 0 ; i < rules.party_size ; i++)
		if (party[i].hp < rules.max_hp(party[i]) / 2 )
			wc++;

	log().info("Counted ", wc, " weaklings.");
	return wc;
}

Target Fight::find_weak ()
{
	unsigned w = rules.party_size;

	//TODO: Bias towards squishies and bad status.
	if (player.strat[PROTECT_WEAK])
	{
		unsigned lowest_hp = (unsigned) -1;
		for (unsigned i = 0 ; i < rules.party_size ; i++)
		{
			unsigned subjective_hp = (player.strat[KEEP_HEAL]?HEALER_BIAS:1.0) * party[i].hp;
			if (party[i].hp && subjective_hp < lowest_hp)
			{
				lowest_hp = subjective_hp;
				w = i;
			}
		}
	}
	else
		w = rtd(rules.party_size);

	log().info("Chose ", w, " as my weakling.");
	return w;
}

Element Fight::pick_elem (Target t)
{
	log().info("Picking random element against ", t, ".");
	//TODO: AoE
	//TODO: AVOID_ELEM, PICK_ELEM
	return Element(rtd(NB_ELEMS));
}

unsigned Fight::rtd ()
{
	log().info("!rtd");
	return (Element) rtd(100);
}

unsigned Fight::rtd (unsigned max)
{
	//TODO: Improve distribution.
	return rand() % max;
}

void Fight::msg (string s)
{
	_mainState.showMessage(s);
}

void Fight::damage (Target t, unsigned amount, Element e)
{
	log().info("Damaging ",t," for ",amount," of type ", e,".");
	unsigned dmg = amount;

	// PC target
	if (t < boss_target)
	{
		dmg *= rules.elem_factor(e, NONE);
		unsigned& hp = party[t].hp;

		_mainState.displayDamages(t, amount);

		//TODO: Implement protectors.
		if (hp > dmg)
			hp -= dmg;
		else
		{
			//TODO: Implement resilience.
			hp = 0;
			_mainState.playDeathAnim(t);
		}
	}
	else if (t == boss_target)
	{
		Boss& b = boss;
		dmg *= rules.elem_factor(e, b.elem);

		_mainState.displayDamages(t, amount);

		if (b.hp > dmg)
			b.hp -= dmg;
		else
		{
			b.hp = 0;
			_mainState.playDeathAnim(t);
			//TODO: Kill his minions.
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
	log().info("Status ",s," inflicted on ",t,".");
	//TODO: Implement tenacity.
	party[t].status[s] = true;
}
