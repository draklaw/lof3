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

		setSpellFromJson(SMITE,           spells["attack_spell"]);
		setSpellFromJson(SLICE,           spells["attack_spell"]);
		setSpellFromJson(NUKES,           spells["attack_spell"]);

		setSpellFromJson(PROTECT,         spells["protect"]);

		setSpellFromJson(HEAL,            spells["heal"]);

		setSpellFromJson(NURSE,           spells["nurse"]);

		setSpellFromJson(REZ,             spells["rez"]);

		setSpellFromJson(SWIPE,           spells["aoe"]);
		setSpellFromJson(Spell(AOES + NONE),     spells["aoe"]);
		setSpellFromJson(Spell(AOES + FIRE),     spells["aoe"]);
		setSpellFromJson(Spell(AOES + ICE),      spells["aoe"]);
		setSpellFromJson(Spell(AOES + SPARK),    spells["aoe"]);
		setSpellFromJson(Spell(AOES + ACID),     spells["aoe"]);

		setSpellFromJson(Spell(SHIELDS + NONE),  spells["shield"]);
		setSpellFromJson(Spell(SHIELDS + FIRE),  spells["shield"]);
		setSpellFromJson(Spell(SHIELDS + ICE),   spells["shield"]);
		setSpellFromJson(Spell(SHIELDS + SPARK), spells["shield"]);
		setSpellFromJson(Spell(SHIELDS + ACID),  spells["shield"]);
	}

	const Json::Value& curses = json["curses"];
	if(curses.isObject()) {
		setCurseFromJson(PUNCH,   curses["attack"]);
		setCurseFromJson(STORM,   curses["storm"]);
		setCurseFromJson(STRIKE,  curses["strike"]);
		setCurseFromJson(VORPAL,  curses["vorpal"]);
		setCurseFromJson(CRIPPLE, curses["cripple"]);
		setCurseFromJson(DRAIN,   curses["drain"]);
		setCurseFromJson(MUD,     curses["mud"]);
		setCurseFromJson(DISPEL,  curses["dispel"]);
	}
}


void Rules::setJobFromJson(Job job, const Json::Value& json) {
	if(!json.isObject()) {
		return;
	}

	parseUnsigned(hd[job],   json["health"]);
	parseUnsigned(mp[job],   json["mana"]);
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
