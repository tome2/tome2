-- The mana school

MANATHRUST = add_spell
{
	["name"] = 	"Manathrust",
	["school"] = 	SCHOOL_MANA,
	["level"] = 	1,
	["mana"] = 	1,
	["mana_max"] =  25,
	["fail"] = 	10,
	["stick"] =
	{
			["charge"] =    { 7, 10 },
			[TV_WAND] =
			{
				["rarity"] = 		5,
				["base_level"] =	{ 1, 20 },
				["max_level"] =		{ 15, 33 },
			},
	},
	["spell"] = 	function() return mana_manathrust() end,
	["info"] = 	function() return mana_manathrust_info() end,
	["desc"] =	{
			"Conjures up mana into a powerful bolt",
			"The damage is irresistible and will increase with level"
		}
}

DELCURSES = add_spell
{
	["name"] = 	"Remove Curses",
	["school"] = 	SCHOOL_MANA,
	["level"] = 	10,
	["mana"] = 	20,
	["mana_max"] = 	40,
	["fail"] = 	30,
	["stick"] =
	{
			["charge"] =    { 3, 8 },
			[TV_STAFF] =
			{
				["rarity"] = 		70,
				["base_level"] =	{ 1, 5 },
				["max_level"] =		{ 15, 50 },
			},
	},
	["inertia"] = 	{ 1, 10 },
	["spell"] = 	function() return mana_remove_curses() end,
	["info"] = 	function() return mana_remove_curses_info() end,
	["desc"] =	{
			"Remove curses of worn objects",
			"At level 20 switches to *remove curses*"
		}
}

RESISTS = add_spell
{
	["name"] = 	"Elemental Shield",
	["school"] = 	SCHOOL_MANA,
	["level"] = 	20,
	["mana"] = 	17,
	["mana_max"] = 	20,
	["fail"] = 	40,
	["inertia"] = 	{ 2, 25 },
	["spell"] = 	function() return mana_elemental_shield() end,
	["info"] = 	function() return mana_elemental_shield_info() end,
	["desc"] =	{
			"Provide resistances to the four basic elements",
		}
}

MANASHIELD = add_spell
{
	["name"] = 	"Disruption Shield",
	["school"] = 	SCHOOL_MANA,
	["level"] = 	45,
	["mana"] = 	50,
	["mana_max"] = 	50,
	["fail"] = 	90,
	["inertia"] = 	{ 9, 10},
	["spell"] = 	function() return mana_disruption_shield() end,
	["info"] = 	function() return mana_disruption_shield_info() end,
	["desc"] =	{
			"Uses mana instead of hp to take damage",
			"At level 5 switches to Globe of Invulnerability.",
			"The spell breaks as soon as a melee, shooting, throwing or magical",
			"skill action is attempted, and lasts only a short time."
		}
}
