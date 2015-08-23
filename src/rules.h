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
enum Status { NORMAL, SLOW, DISABLE, SILENCE, NB_STATUS };
enum Spell { AA, SMITE, SWIPE, PROTECT, HEAL, NURSE, REZ, NUKES,
	AOES = NUKES+NB_ELEMS, SHIELDS = AOES+NB_ELEMS, NB_SPELLS = SHIELDS+NB_ELEMS
};
struct PC {
	Job job;                       // Character class^H^H^H^H^Hjob
	unsigned xp;                   // Level
	unsigned hp;                   // Health
	unsigned mp;                   // Mana
	Stuff* equip;                  // Equipment
	bool status[NB_STATUS];        // Active debuffs
	unsigned cooldowns[NB_SPELLS]; // Spell cooldowns
	unsigned resists[NB_ELEMS];    // Current elemental resistances
	unsigned protector;            // Protection provider (PARTY_SIZE if none)
	unsigned init;                 // Time to initiative
};

enum Spawn { SPRITES, MAGELING, TOMBERRY, NB_SPAWNS };
struct Minion {
	Spawn spawn;
	unsigned hp;   // Health bar
	Element elem;  // Minion-specific element
	unsigned init; // Time to initiative
};

enum Curse { PUNCH, SWITCH, SCAN, STORM, STRIKE, VORPAL, CRIPPLE, DRAIN, MUD,
	DISPEL, SUMMON, NB_CURSES = SUMMON + NB_SPAWNS
};
enum Qte { EASY, MEDIUM, HARD, NB_QTES };
struct Boss {
	unsigned hp;                   // Health bar
	Element elem;                  // Current active element
	unsigned curses_cd[NB_CURSES]; // Curses cooldowns
	unsigned qtes_cd[NB_QTES];     // QTEs cooldowns
	unsigned init;                 // Time to initiative
};

enum Item { POTION, ETHER, PHOENIX, NB_ITEMS };
enum Strat { SPELLS, AVOID_ELEM, PICK_ELEM, RAISE_SHIELD, RAISE_DEAD,
	KILL_SPRITES, KILL_TOMBERRY, CLEAR_STATUS, CONSUMABLES, KEEP_HEAL,
	PROTECT_WEAK, DPS_RUN, NB_STRATS
};
struct Player {
	unsigned inventory[NB_ITEMS];     // Party inventory
	unsigned practice[NB_QTES];       // QTE "skill"
	unsigned strats[NB_STRATS];       // Strats knowledge
	unsigned aa_favor;                // Fondness for auto-attacks
	unsigned spell_favors[NB_SPELLS]; // Favorite spells
	unsigned item_favors[NB_ITEMS];   // Favorite consumables
};

class Rules {
public:
	// Class-specific stats (per level)
	unsigned bab[NB_JOBS];       // Base Attack Bonus
	unsigned hd[NB_JOBS];        // Hit Dice
	unsigned mp[NB_JOBS];        // Mana Pool
	unsigned unlocks[NB_SPELLS]; // Spell levels
	unsigned powerup[NB_SPELLS]; // Specific spell power per level
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
	unsigned curse_power[NB_CURSES];    // Curse power (same as spells)
	unsigned curse_utility[NB_CURSES];  // Curse utility (same as spells)
	unsigned curse_cooldown[NB_CURSES]; // Curse-specific cooldown

	// Misc stats
	double elem_multiplier[3]; // Base, efficient and inefficient results.
	unsigned max_summons;       // Maximum number of concurrent minions.
	unsigned party_size;        // 4.

	Rules(Logger& logger, const string& rules_source);
	~Rules();

	// Return the efficiency factor of attacking defense with attack.
	double elem_factor (Element attack, Element defense);

private:
	Logger _logger;
	Logger& log();
};

#endif
