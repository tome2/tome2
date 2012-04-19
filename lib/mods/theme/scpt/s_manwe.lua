-- Handle Manwe Sulimo magic school

MANWE_SHIELD = add_spell
{
	["name"] =      "Wind Shield",
	["school"] =    {SCHOOL_MANWE},
	["level"] =     10,
	["mana"] =      100,
	["mana_max"] =  500,
	["fail"] = 	30,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] = 	SKILL_SPIRITUALITY,
	["spell"] = 	function() return manwe_wind_shield() end,
	["info"] = 	function() return manwe_wind_shield_info() end,
	["desc"] =	{
			"It surrounds you with a shield of wind that deflects blows from evil monsters",
			"At level 10 it increases your armour rating",
			"At level 20 it retaliates against monsters that melee you",
	}
}

MANWE_AVATAR = add_spell
{
	["name"] =      "Avatar",
	["school"] =    {SCHOOL_MANWE},
	["level"] =     35,
	["mana"] =      1000,
	["mana_max"] =  1000,
	["fail"] = 	80,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] = 	SKILL_SPIRITUALITY,
	["spell"] = 	function() return manwe_avatar() end,
	["info"] =      function() return manwe_avatar_info() end,
	["desc"] =	{
			"It turns you into a full grown Maia",
	}
}

MANWE_BLESS = add_spell
{
	["name"] =      "Manwe's Blessing",
	["school"] =    {SCHOOL_MANWE},
	["level"] =     1,
	["mana"] =      10,
	["mana_max"] =  100,
	["fail"] = 	20,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] = 	SKILL_SPIRITUALITY,
	["spell"] = 	function() return manwe_blessing() end,
	["info"] =      function() return manwe_blessing_info() end,
	["desc"] =	{
			"Manwe's Blessing removes your fears, blesses you and surrounds you with",
			"holy light",
			"At level 10 it also grants heroism",
			"At level 20 it also grants super heroism",
			"At level 30 it also grants holy luck and life protection",
	}
}

MANWE_CALL = add_spell
{
	["name"] =      "Manwe's Call",
	["school"] =    {SCHOOL_MANWE},
	["level"] =     20,
	["mana"] =      200,
	["mana_max"] =  500,
	["fail"] = 	40,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] = 	SKILL_SPIRITUALITY,
	["spell"] = 	function() return manwe_call() end,
	["info"] =      function() return manwe_call_info() end,
	["desc"] =	{
			"Manwe's Call summons a Great Eagle to help you battle the forces",
			"of Morgoth"
	}
}
