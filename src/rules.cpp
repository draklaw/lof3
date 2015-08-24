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


#include <lair/core/json.h>

#include "rules.h"

#include "fight.h"


void parseUnsigned(unsigned& value, const Json::Value& json) {
	if(json.isConvertibleTo(Json::ValueType::uintValue)) {
		value = json.asUInt();
	}
}


Rules::Rules(Logger& logger, const string& ruleset)
:	_logger(&logger),
	hd{30,15,10,20},
	mp{5,30,25,15},
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
	powerup{1,0},
	unlocks{},
	curse_power{20,0},
	curse_utility{},
	curse_cooldown{},
	elem_multiplier{1.5,3.0,0.0},
	max_summons(5),
	party_size(4)
{
	log().setLevel(LogLevel::Info);
}

Rules::~Rules()
{
}


void Rules::setFromJson(const Json::Value& json) {
	if(!json.isObject()) {
		return;
	}

	const Json::Value& boss = json["boss"];
	if(boss.isObject()) {
		const Json::Value& hp = boss["hp"];
		if(hp.isArray() && hp.size() == 3) {
			unsigned i = 0;
			for(const Json::Value v: hp) {
				parseUnsigned(boss_hp[i], v);
				++i;
			}
		}
		parseUnsigned(boss_init, boss["init"]);
	}
	log().error("Boss: ", boss_hp[0], "/", boss_hp[1], "/", boss_hp[2], ", ", boss_init);

	const Json::Value& jobs = json["jobs"];
	if(jobs.isObject()) {
		setJobFromJson(FIGHTER, jobs["fighter"]);
		setJobFromJson(HEALER,  jobs["healer"]);
		setJobFromJson(WIZARD,  jobs["wizard"]);
		setJobFromJson(NINJA,   jobs["ninja"]);
	}

	const Json::Value& spells = json["spells"];
	if(spells.isObject()) {
		setSpellFromJson(AA,              spells["attack"]);

		setSpellFromJson(SMITE,           spells["smite"]);

		setSpellFromJson(SLICE,           spells["slice"]);
		setSpellFromJson(SWIPE,           spells["swipe"]);

		setSpellFromJson(PROTECT,         spells["protect"]);

		setSpellFromJson(HEAL,            spells["heal"]);
		setSpellFromJson(NURSE,           spells["nurse"]);
		setSpellFromJson(REZ,             spells["rez"]);

		setSpellFromJson(NUKES + NONE,  spells["nuke"]);
		setSpellFromJson(NUKES + FIRE,  spells["nuke_elem"]);
		setSpellFromJson(NUKES + ICE,   spells["nuke_elem"]);
		setSpellFromJson(NUKES + SPARK, spells["nuke_elem"]);
		setSpellFromJson(NUKES + ACID,  spells["nuke_elem"]);

		setSpellFromJson(Spell(AOES + NONE),     spells["aoe"]);
		setSpellFromJson(Spell(AOES + FIRE),     spells["aoe_elem"]);
		setSpellFromJson(Spell(AOES + ICE),      spells["aoe_elem"]);
		setSpellFromJson(Spell(AOES + SPARK),    spells["aoe_elem"]);
		setSpellFromJson(Spell(AOES + ACID),     spells["aoe_elem"]);

		setSpellFromJson(Spell(SHIELDS + NONE),  spells["shield"]);
		setSpellFromJson(Spell(SHIELDS + FIRE),  spells["shield_elem"]);
		setSpellFromJson(Spell(SHIELDS + ICE),   spells["shield_elem"]);
		setSpellFromJson(Spell(SHIELDS + SPARK), spells["shield_elem"]);
		setSpellFromJson(Spell(SHIELDS + ACID),  spells["shield_elem"]);
	}

	const Json::Value& curses = json["curses"];
	if(curses.isObject()) {
		setCurseFromJson(PUNCH,   curses["punch"]);
		setCurseFromJson(STORM,   curses["storm"]);
		setCurseFromJson(STRIKE,  curses["strike"]);
		setCurseFromJson(VORPAL,  curses["vorpal"]);
		setCurseFromJson(CRIPPLE, curses["cripple"]);
		setCurseFromJson(DRAIN,   curses["drain"]);
		setCurseFromJson(MUD,     curses["mud"]);
		setCurseFromJson(DISPEL,  curses["dispel"]);

		setCurseFromJson(Curse(SUMMON + SPRITES),  curses["sprites"]);
		setCurseFromJson(Curse(SUMMON + TOMBERRY),  curses["tomberry"]);
		setCurseFromJson(Curse(SUMMON + MAGELING),  curses["mageling"]);
	}
}

/* Baseline :
Loser PC (20) :     400 HP,  400 MP, +  20x spells, AA  40, Heal 150
Killer PC (100) :  2000 HP, 2000 MP, + 100x spells, AA 120, Heal 550
Monster PC (200) : 4000 HP, 4000 MP, + 200x spells, AA 220, Heal 1050

 */


void Rules::setJobFromJson(Job job, const Json::Value& json) {
	if(!json.isObject()) {
		return;
	}

	parseUnsigned(hd[job],   json["hit_dice"]);
	parseUnsigned(mp[job],   json["mana_pool"]);
	parseUnsigned(init[job], json["init"]);
}


void Rules::setSpellFromJson(Spell spell, const Json::Value& json) {
	if(!json.isObject()) {
		return;
	}

	parseUnsigned(spell_power[spell],    json["power"]);
	parseUnsigned(spell_utility[spell],  json["utility"]);
	parseUnsigned(spell_cooldown[spell], json["cooldown"]);
	parseUnsigned(spell_manacost[spell], json["manacost"]);
	parseUnsigned(powerup[spell],        json["powerup"]);
}


void Rules::setCurseFromJson(Curse curse, const Json::Value& json) {
	if(!json.isObject()) {
		return;
	}

	parseUnsigned(curse_power[curse],    json["power"]);
	parseUnsigned(curse_utility[curse],  json["utility"]);
	parseUnsigned(curse_cooldown[curse], json["cooldown"]);
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

unsigned Rules::spellpower (Spell s, const PC& u)
{
	log().info("Spell power of ",s," for ",u.job," is ",spell_power[s] + powerup[s] * u.xp,".");
	return spell_power[s] + powerup[s] * u.xp;
}

unsigned Rules::max_hp (PC c)
{
	return hd[c.job] * c.xp;
}

unsigned Rules::max_mp (PC c)
{
	return mp[c.job] * c.xp;
}

Spell Rules::base_spell (Spell s)
{
	switch (s)
	{
		case NUKES:
		case NUKES + FIRE:
		case NUKES + ICE:
		case NUKES + SPARK:
		case NUKES + ACID:
			return NUKES;
		case AOES:
		case AOES + FIRE:
		case AOES + ICE:
		case AOES + SPARK:
		case AOES + ACID:
			return AOES;
		case SHIELDS:
		case SHIELDS + FIRE:
		case SHIELDS + ICE:
		case SHIELDS + SPARK:
		case SHIELDS + ACID:
			return SHIELDS;
		default:
			return s;
	}
}

string Rules::name(Curse c, Element e)
{
	string adj = "";
	
	switch (e)
	{
		case NONE:
			adj = "unholy";
			break;
		case FIRE:
			adj = "fiery";
			break;
		case ICE:
			adj = "frozen";
			break;
		case SPARK:
			adj = "electrifying";
			break;
		case ACID:
			adj = "corrosive";
			break;
		default:
			adj = "frankly unexplainable";
	}
	
	switch (c)
	{
		case PUNCH:
			return adj + " punch";
		case SWITCH:
			return "switch";
		case SCAN:
			return "scan";
		case STRIKE:
			return adj + " curse";
		case STORM:
			return adj + " storm";
		case CRIPPLE:
			return "crippling blow";
		case DRAIN:
			return "soul-draining curse";
		case MUD:
			return "flood of mud";
		case VORPAL:
			return "lethal strike";
		case DISPEL:
			return "magical disjunction";
		case SUMMON + SPRITES:
			return "swarm of healing sprites";
		case SUMMON + TOMBERRY:
			return "deadly knifeling";
		case SUMMON + MAGELING:
			return "mageling minion against your foes";
		default:
			return adj + " MISSINGNO";
	}
}

string Rules::name(Spell s)
{
	string adj = "";
	
	switch (Element(s - base_spell(s)))
	{
		case NONE:
			adj = "holy";
			break;
		case FIRE:
			adj = "fire";
			break;
		case ICE:
			adj = "ice";
			break;
		case SPARK:
			adj = "lightning";
			break;
		case ACID:
			adj = "acid";
			break;
		default:
			adj = "a never-seen-before";
	}

	switch (s)
	{
		case AA:
			return "his weapon";
		case SMITE:
			return "mighty force";
		case PROTECT:
			return "a protecting shield";
		case SLICE:
			return "deep-cut-no-jutsu";
		case SWIPE:
			return "thousand-cuts-no-jutsu";
		case HEAL:
			return "heal";
		case NURSE:
			return "\"summon wave of prancing nurses\"";
		case REZ:
			return "resurrection";
		case NUKES:
		case NUKES + FIRE:
		case NUKES + ICE:
		case NUKES + SPARK:
		case NUKES + ACID:
			return adj + " ball";
		case AOES:
		case AOES + FIRE:
		case AOES + ICE:
		case AOES + SPARK:
		case AOES + ACID:
			return adj + " storm";
		case SHIELDS:
		case SHIELDS + FIRE:
		case SHIELDS + ICE:
		case SHIELDS + SPARK:
		case SHIELDS + ACID:
			return adj + " shield";
		default:
			return "an unknown but possibly reality-threatening spell";
	}
	
}