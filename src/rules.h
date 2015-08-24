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


#ifndef _LOF3_RULES_H
#define _LOF3_RULES_H


#include <lair/core/lair.h>
#include <lair/core/log.h>

class Fight;


using namespace std;
using namespace lair;

#define NB_TIERS 3

enum Element { NONE, FIRE, ICE, SPARK, ACID, NB_ELEMS };

enum Effect { DAMAGE, HP_BOOST, MP_BOOST, ELEM_DAMAGE, TENACITY, RESILIENCE,
	AUTOREZ, HP_REGEN, MP_REGEN, NB_EFFECTS
};
struct Stuff {
	string name;                // Witty or epic name
	string description;         // Description of effects
	unsigned buffs[NB_EFFECTS]; // Actual effect
};

enum Job { FIGHTER, HEALER, WIZARD, NINJA, NB_JOBS };
enum Status { SLOW, DISABLE, SILENCE, NB_STATUS };
enum Spell { AA, SMITE, PROTECT, SLICE, SWIPE, HEAL, NURSE, REZ, NUKES,
	AOES = NUKES+NB_ELEMS, SHIELDS = AOES+NB_ELEMS, NB_SPELLS = SHIELDS+NB_ELEMS
};
inline constexpr Spell operator+ (Spell s, Element e)
{
	return Spell((unsigned) s + (unsigned) e);
}

struct PC {
	string name;                  // They're not just numbers
	Job job;                      // Character class^H^H^H^H^Hjob
	unsigned xp;                  // Level
	unsigned hp;                  // Health
	unsigned mp;                  // Mana
	Stuff* equip;                 // Equipment
	bool status[NB_STATUS];       // Active debuffs
	unsigned cooldown[NB_SPELLS]; // Spell cooldowns
	unsigned resist[NB_ELEMS];    // Current elemental resistances
	unsigned protector;           // Protection provider (PARTY_SIZE if none)
	unsigned init;                // Time to initiative
};

enum Spawn { SPRITES, MAGELING, TOMBERRY, NB_SPAWNS };
struct Minion {
	Spawn spawn;
	unsigned hp;   // Health bar
	Element elem;  // Minion-specific element
	unsigned init; // Time to initiative
};

enum Curse { PUNCH, SWITCH, SCAN, STRIKE, STORM, VORPAL, CRIPPLE, DRAIN, MUD,
	DISPEL, SUMMON, NB_CURSES = SUMMON + NB_SPAWNS
};
inline constexpr Curse operator+ (Curse c, Spawn s)
{ return Curse((unsigned) c + (unsigned) s); }
enum Qte { EASY, MEDIUM, HARD, NB_QTES };
struct Boss {
	unsigned hp;                   // Health bar
	Element elem;                  // Current active element
	unsigned curses_cd[NB_CURSES]; // Curses cooldowns
	unsigned qtes_cd[NB_QTES];     // QTEs cooldowns
	unsigned init;                 // Time to initiative
};

enum Item { POTION, ETHER, PHOENIX, NB_ITEMS };
enum Strat { SPELLS, AVOID_ELEM, PICK_ELEM, RAISE_DEAD, HEAL_UP, CLEAR_STATUS,
	RAISE_SHIELD, KILL_SPRITES, KILL_TOMBERRY, DPS_RUN, KEEP_HEAL, PROTECT_WEAK,
	CONSUMABLES, NB_STRATS
};
struct Player {
	unsigned inventory[NB_ITEMS];    // Party inventory
	unsigned practice[NB_QTES];      // QTE "skill"
	unsigned strat[NB_STRATS];       // Strats knowledge
	unsigned aa_favor;               // Fondness for auto-attacks
	unsigned spell_favor[NB_SPELLS]; // Favorite spells
	unsigned item_favor[NB_ITEMS];   // Favorite consumables
};

class Rules {
private:
	Logger _logger;
	Logger& log();

public:
	// Class-specific stats (per level)
	unsigned hd[NB_JOBS];        // Hit Dice
	unsigned mp[NB_JOBS];        // Mana Pool
	unsigned init[NB_JOBS];      // Time between each turn

	// Boss and minion stats
	unsigned boss_hp[NB_TIERS];       // Tier-trigering HP
	unsigned boss_init;               // Time between each turn
	unsigned minion_hp[NB_SPAWNS];    // Minions HP
	unsigned minion_init[NB_SPAWNS];  // Minions init
	unsigned minion_power[NB_SPAWNS]; // Heal/damage amount

	// Magic rules
	unsigned spell_power[NB_SPELLS];    // Spell power (%reduction, damage, heal)
	unsigned spell_utility[NB_SPELLS];  // Spell utility (%effect or duration)
	unsigned spell_cooldown[NB_SPELLS]; // Spell-specific cooldown
	unsigned spell_manacost[NB_SPELLS]; // Spell-specific mana cost
	unsigned powerup[NB_SPELLS];        // Spell power increase per level
	unsigned unlocks[NB_SPELLS];        // Spell unlock levels
	unsigned curse_power[NB_CURSES];    // Curse power (same as spells)
	unsigned curse_utility[NB_CURSES];  // Curse utility (same as spells)
	unsigned curse_cooldown[NB_CURSES]; // Curse-specific cooldown

	// Misc stats
	double elem_multiplier[3]; // Base, efficient and inefficient results.
	unsigned max_summons;      // Maximum number of concurrent minions.
	unsigned party_size;       // 4.

	Rules(Logger& logger, const string& rules_source);
	~Rules();

	void setFromJson(const Json::Value& json);
	void setJobFromJson(Job job, const Json::Value& json);
	void setSpellFromJson(Spell spell, const Json::Value& json);
	void setCurseFromJson(Curse curse, const Json::Value& json);

	// Return the efficiency factor of attacking defense with attack.
	double elem_factor (Element attack, Element defense);

	// Compute the actual power of s in the hands of u.
	unsigned spellpower (Spell s, const PC& u);

	// Returns a stat cap for c.
	unsigned max_hp (PC c);
	unsigned max_mp (PC c);

	// Returns the non-elemental version of s.
	Spell base_spell (Spell s);

	// Human-readable names for things.
	string name(Curse c, Element e);
	string name(Spell s);
};

#endif
