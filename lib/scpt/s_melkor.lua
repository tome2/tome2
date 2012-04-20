-- handle the melkor school

MELKOR_CURSE = add_spell
{
	["name"] =      "Curse",
	["school"] =    {SCHOOL_MELKOR},
	["level"] =     1,
	["mana"] =      50,
	["mana_max"] =  300,
	["fail"] = 	20,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] = 	SKILL_SPIRITUALITY,
	["spell"] = 	function() return melkor_curse() end,
	["info"] = 	function() return melkor_curse_info() end,
	["desc"] =	{
			"It curses a monster, reducing its melee power",
			"At level 5 it can be auto-casted (with no piety cost) while fighting",
			"At level 15 it also reduces armor",
			"At level 25 it also reduces speed",
			"At level 35 it also reduces max life (but it is never fatal)",
	}
}

MELKOR_CORPSE_EXPLOSION = add_spell
{
	["name"] =      "Corpse Explosion",
	["school"] =    {SCHOOL_MELKOR},
	["level"] =     10,
	["mana"] =      100,
	["mana_max"] =  500,
	["fail"] = 	45,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] = 	SKILL_SPIRITUALITY,
	["spell"] = 	function() return melkor_corpse_explosion() end,
	["info"] = 	function() return melkor_corpse_explosion_info() end,
	["desc"] =	{
			"It makes corpses in an area around you explode for a percent of their",
			"hit points as damage",
	}
}

MELKOR_MIND_STEAL = add_spell
{
	["name"] =      "Mind Steal",
	["school"] =    {SCHOOL_MELKOR},
	["level"] =     20,
	["mana"] =      1000,
	["mana_max"] =  3000,
	["fail"] = 	90,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] = 	SKILL_SPIRITUALITY,
	["spell"] = 	function() return melkor_mind_steal() end,
	["info"] = 	function() return melkor_mind_steal_info() end,
	["desc"] =	{
			"It allows your spirit to temporarily leave your own body, which will",
			"be vulnerable, to control one of your enemies body."
	}
}
