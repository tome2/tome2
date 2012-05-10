-- Handles thhe temporal school


MAGELOCK = add_spell
{
	["name"] = 	"Magelock",
	["school"] = 	{SCHOOL_TEMPORAL},
	["level"] = 	1,
	["mana"] = 	1,
	["mana_max"] = 	35,
	["fail"] = 	10,
	["stick"] =
	{
			["charge"] =    { 7, 5 },
			[TV_WAND] =
			{
				["rarity"] = 		30,
				["base_level"] =	{ 1, 5 },
				["max_level"] =		{ 15, 45 },
			},
	},
	["spell"] = 	function() return tempo_magelock() end,
	["info"] = 	function() return tempo_magelock_info() end,
	["desc"] =	{
			"Magically locks a door",
			"At level 30 it creates a glyph of warding",
			"At level 40 the glyph can be placed anywhere in the field of vision"
	}
}

SLOWMONSTER = add_spell
{
	["name"] = 	"Slow Monster",
	["school"] = 	{SCHOOL_TEMPORAL},
	["level"] = 	10,
	["mana"] = 	10,
	["mana_max"] = 	15,
	["fail"] = 	35,
	["stick"] =
	{
			["charge"] =    { 5, 5 },
			[TV_WAND] =
			{
				["rarity"] = 		23,
				["base_level"] =	{ 1, 15 },
				["max_level"] =		{ 20, 50 },
			},
	},
	["spell"] = 	function() return tempo_slow_monster() end,
	["info"] = 	function() return tempo_slow_monster_info() end,
	["desc"] =	{
			"Magically slows down the passing of time around a monster",
			"At level 20 it affects a zone"
	}
}

ESSENCESPEED = add_spell
{
	["name"] = 	"Essence of Speed",
	["school"] = 	{SCHOOL_TEMPORAL},
	["level"] = 	15,
	["mana"] = 	20,
	["mana_max"] = 	40,
	["fail"] = 	50,
	["stick"] =
	{
			["charge"] =    { 3, 3 },
			[TV_WAND] =
			{
				["rarity"] = 		80,
				["base_level"] =	{ 1, 1 },
				["max_level"] =		{ 10, 39 },
			},
	},
	["inertia"] = 	{ 5, 20 },
	["spell"] = 	function() return tempo_essence_of_speed() end,
	["info"] = 	function() return tempo_essence_of_speed_info() end,
	["desc"] =	{
			"Magically decreases the passing of time around you, making you move faster with",
			"respect to the rest of the universe."
	}
}

BANISHMENT = add_spell
{
	["name"] = 	"Banishment",
	["school"] = 	{SCHOOL_TEMPORAL, SCHOOL_CONVEYANCE},
	["level"] = 	30,
	["mana"] = 	30,
	["mana_max"] = 	40,
	["fail"] = 	95,
	["stick"] =
	{
			["charge"] =    { 1, 3 },
			[TV_WAND] =
			{
				["rarity"] = 		98,
				["base_level"] =	{ 1, 15 },
				["max_level"] =		{ 10, 36 },
			},
	},
	["inertia"] = 	{ 5, 50 },
	["spell"] = 	function() return tempo_banishment() end,
	["info"] = 	function() return tempo_banishment_info() end,
	["desc"] =	{
			"Disrupts the space/time continuum in your area and teleports all monsters away.",
			"At level 15 it may also lock them in a time bubble for a while."
	}
}
