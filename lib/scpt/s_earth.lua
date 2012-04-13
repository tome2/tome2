-- The earth school

STONESKIN = add_spell
{
	["name"] = 	"Stone Skin",
	["school"] = 	SCHOOL_EARTH,
	["level"] = 	1,
	["mana"] = 	1,
	["mana_max"] = 	50,
	["fail"] = 	10,
	["inertia"] = 	{ 2, 50 },
	["spell"] = 	function() return earth_stone_skin() end,
	["info"] = 	function() return earth_stone_skin_info() end,
	["desc"] =	{
			"Creates a shield of earth around you to protect you",
			"At level 25 it starts dealing damage to attackers"
		}
}

DIG = add_spell
{
	["name"] = 	"Dig",
	["school"] = 	SCHOOL_EARTH,
	["level"] = 	12,
	["mana"] = 	14,
	["mana_max"] = 	14,
	["fail"] = 	20,
	["stick"] =
	{
			["charge"] =    { 15, 5 },
			[TV_WAND] =
			{
				["rarity"] = 		25,
				["base_level"] =	{ 1, 1 },
				["max_level"] =		{ 1, 1 },
			},
	},
	["spell"] = 	function() return earth_dig() end,
	["info"] = 	function() return earth_dig_info() end,
	["desc"] =	{
			"Digs a hole in a wall much faster than any shovels",
		}
}

STONEPRISON = add_spell
{
	["name"] = 	"Stone Prison",
	["school"] = 	SCHOOL_EARTH,
	["level"] = 	25,
	["mana"] = 	30,
	["mana_max"] = 	50,
	["fail"] = 	65,
	["stick"] =
	{
			["charge"] =    { 5, 3 },
			[TV_WAND] =
			{
				["rarity"] = 		57,
				["base_level"] =	{ 1, 3 },
				["max_level"] =		{ 5, 20 },
			},
	},
	["spell"] = 	function() return earth_stone_prison() end,
	["info"] = 	function() return earth_stone_prison_info() end,
	["desc"] =	{
			"Creates a prison of walls around you",
			"At level 10 it allows you to target a monster"
		}
}

STRIKE = add_spell
{
	["name"] = 	"Strike",
	["school"] = 	{SCHOOL_EARTH},
	["level"] = 	30,
	["mana"] = 	30,
	["mana_max"] = 	50,
	["fail"] = 	60,
	["stick"] =
	{
			["charge"] =    { 2, 6 },
			[TV_WAND] =
			{
				["rarity"] = 		635,
				["base_level"] =	{ 1, 5 },
				["max_level"] =		{ 10, 50 },
			},
	},
	["spell"] = 	function() return earth_strike() end,
	["info"] = 	function() return earth_strike_info() end,
	["desc"] =	{
			"Creates a micro-ball of force that will push monsters backwards",
			"If the monster is caught near a wall, it'll be crushed against it",
			"At level 12 it turns into a ball of radius 1"
	}
}

SHAKE = add_spell
{
	["name"] = 	"Shake",
	["school"] = 	{SCHOOL_EARTH},
	["level"] = 	27,
	["mana"] = 	25,
	["mana_max"] = 	30,
	["fail"] = 	60,
	["stick"] =
	{
			["charge"] =    { 5, 10 },
			[TV_STAFF] =
			{
				["rarity"] = 		75,
				["base_level"] =	{ 1, 3 },
				["max_level"] =		{ 9, 20 },
			},
	},
	["inertia"] = 	{ 2, 50 },
	["spell"] = 	function() return earth_shake() end,
	["info"] = 	function() return earth_shake_info() end,
	["desc"] =	{
			"Creates a localised earthquake",
			"At level 10 it can be targeted at any location"
	}
}
