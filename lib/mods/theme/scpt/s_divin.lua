-- Handles thhe divination school


STARIDENTIFY = add_spell
{
	["name"] = 	"Greater Identify",
	["school"] = 	{SCHOOL_DIVINATION},
	["level"] = 	35,
	["mana"] = 	30,
	["mana_max"] = 	30,
	["fail"] = 	80,
	["spell"] = 	function() return divination_greater_identify() end,
	["info"] = 	function() return divination_greater_identify_info() end,
	["desc"] =	{
			"Asks for an object and fully identify it, providing the full list of powers",
			"Cast at yourself it will reveal your powers"
	}
}

IDENTIFY = add_spell
{
	["name"] = 	"Identify",
	["school"] = 	{SCHOOL_DIVINATION},
	["level"] = 	8,
	["mana"] = 	10,
	["mana_max"] = 	50,
	["fail"] = 	40,
	["stick"] =
	{
			["charge"] =    { 7, 10 },
			[TV_STAFF] =
			{
				["rarity"] = 		45,
				["base_level"] =	{ 1, 15 },
				["max_level"] =		{ 15, 40 },
			},
	},
	["spell"] = 	function() return divination_identify() end,
	["info"] = 	function() return divination_identify_info() end,
	["desc"] =	{
			"Asks for an object and identifies it",
			"At level 17 it identifies all objects in the inventory",
			"At level 27 it identifies all objects in the inventory and in a",
			"radius on the floor, as well as probing monsters in that radius"
	}
}

VISION = add_spell
{
	["name"] = 	"Vision",
	["school"] = 	{SCHOOL_DIVINATION},
	["level"] = 	15,
	["mana"] = 	7,
	["mana_max"] = 	55,
	["fail"] = 	45,
	["stick"] =
	{
			["charge"] =    { 4, 6 },
			[TV_STAFF] =
			{
				["rarity"] = 		60,
				["base_level"] =	{ 1, 5 },
				["max_level"] =		{ 10, 30 },
			},
	},
	["inertia"] = 	{ 2, 200 },
	["spell"] = 	function() return divination_vision() end,
	["info"] = 	function() return divination_vision_info() end,
	["desc"] =	{
			"Detects the layout of the surrounding area",
			"At level 25 it maps and lights the whole level",
	}
}

SENSEHIDDEN = add_spell
{
	["name"] = 	"Sense Hidden",
	["school"] = 	{SCHOOL_DIVINATION},
	["level"] = 	5,
	["mana"] = 	2,
	["mana_max"] = 	10,
	["fail"] = 	25,
	["stick"] =
	{
			["charge"] =    { 1, 15 },
			[TV_STAFF] =
			{
				["rarity"] = 		20,
				["base_level"] =	{ 1, 15 },
				["max_level"] =		{ 10, 50 },
			},
	},
	["inertia"] = 	{ 1, 10 },
	["spell"] = 	function() return divination_sense_hidden() end,
	["info"] = 	function() return divination_sense_hidden_info() end,
	["desc"] =	{
			"Detects the traps in a certain radius around you",
			"At level 15 it allows you to sense invisible for a while"
	}
}

REVEALWAYS = add_spell
{
	["name"] = 	"Reveal Ways",
	["school"] = 	{SCHOOL_DIVINATION},
	["level"] = 	9,
	["mana"] = 	3,
	["mana_max"] = 	15,
	["fail"] = 	20,
	["stick"] =
	{
			["charge"] =    { 6, 6 },
			[TV_STAFF] =
			{
				["rarity"] = 		35,
				["base_level"] =	{ 1, 15 },
				["max_level"] =		{ 25, 50 },
			},
	},
	["inertia"] = 	{ 1, 10 },
	["spell"] = 	function() return divination_reveal_ways() end,
	["info"] = 	function() return divination_reveal_ways_info() end,
	["desc"] =	{
			"Detects the doors/stairs/ways in a certain radius around you",
	}
}

SENSEMONSTERS = add_spell
{
	["name"] = 	"Sense Monsters",
	["school"] = 	{SCHOOL_DIVINATION},
	["level"] = 	1,
	["mana"] =      1,
	["mana_max"] =  20,
	["fail"] = 	10,
	["stick"] =
	{
			["charge"] =    { 5, 10 },
			[TV_STAFF] =
			{
				["rarity"] = 		37,
				["base_level"] =	{ 1, 10 },
				["max_level"] =		{ 15, 40 },
			},
	},
	["inertia"] = 	{ 1, 10 },
	["spell"] = 	function() return divination_sense_monsters() end,
	["info"] = 	function() return divination_sense_monsters_info() end,
	["desc"] =	{
			"Detects all monsters near you",
			"At level 30 it allows you to sense monster minds for a while"
	}
}
