-- Handle Tulkas magic school

TULKAS_AIM = add_spell
{
	["name"] =      "Divine Aim",
	["school"] =    {SCHOOL_TULKAS},
	["level"] =     1,
	["mana"] =      30,
	["mana_max"] =  500,
	["fail"] = 	20,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] = 	SKILL_SPIRITUALITY,
	["spell"] = 	function() return tulkas_divine_aim() end,
	["info"] = 	function() return tulkas_divine_aim_info() end,
	["desc"] =	{
			"It makes you more accurate in combat",
			"At level 20 all your blows are critical hits",
	}
}

TULKAS_WAVE = add_spell
{
	["name"] =      "Wave of Power",
	["school"] =    {SCHOOL_TULKAS},
	["level"] =     20,
	["mana"] =      200,
	["mana_max"] =  200,
	["fail"] = 	75,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] = 	SKILL_SPIRITUALITY,
	["spell"] = 	function() return tulkas_wave_of_power() end,
	["info"] = 	function() return tulkas_wave_of_power_info() end,
	["desc"] =	{
			"It allows you to project a number of melee blows across a distance",
	}
}

TULKAS_SPIN = add_spell
{
	["name"] =      "Whirlwind",
	["school"] =    {SCHOOL_TULKAS},
	["level"] =     10,
	["mana"] =      100,
	["mana_max"] =  100,
	["fail"] = 	45,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] = 	SKILL_SPIRITUALITY,
	["spell"] = 	function() return tulkas_whirlwind() end,
	["info"] = 	function() return tulkas_whirlwind_info() end,
	["desc"] =	{
			"It allows you to spin around and hit all monsters nearby",
	}
}
