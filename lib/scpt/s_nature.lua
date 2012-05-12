-- handle the nature school

GROWTREE = add_spell
{
	["name"] = 	"Grow Trees",
	["school"] = 	{SCHOOL_NATURE, SCHOOL_TEMPORAL},
	["level"] = 	6,
	["mana"] = 	6,
	["mana_max"] = 	30,
	["fail"] = 	35,
	["inertia"] = 	{ 5, 50 },
	["spell"] = 	function() return nature_grow_trees() end,
	["info"] = 	function() return nature_grow_trees_info() end,
	["desc"] =	{
			"Makes trees grow extremely quickly around you",
	}
}

HEALING = add_spell
{
	["name"] = 	"Healing",
	["school"] = 	{SCHOOL_NATURE},
	["level"] = 	10,
	["mana"] = 	15,
	["mana_max"] = 	50,
	["fail"] = 	45,
	["stick"] =
	{
			["charge"] =    { 2, 3 },
			[TV_STAFF] =
			{
				["rarity"] = 		90,
				["base_level"] =	{ 1, 5 },
				["max_level"] =		{ 20, 40 },
			},
	},
	["spell"] = 	function() return nature_healing() end,
	["info"] = 	function() return nature_healing_info() end,
	["desc"] =	{
			"Heals a percent of hitpoints",
	}
}

RECOVERY = add_spell
{
	["name"] = 	"Recovery",
	["school"] = 	{SCHOOL_NATURE},
	["level"] = 	15,
	["mana"] = 	10,
	["mana_max"] = 	25,
	["fail"] = 	60,
	["stick"] =
	{
			["charge"] =    { 5, 10 },
			[TV_STAFF] =
			{
				["rarity"] = 		50,
				["base_level"] =	{ 1, 5 },
				["max_level"] =		{ 10, 30 },
			},
	},
	["inertia"] = 	{ 2, 100 },
	["spell"] = 	function() return nature_recovery() end,
	["info"] = 	function() return nature_recovery_info() end,
	["desc"] =	{
			"Reduces the length of time that you are poisoned",
			"At level 5 it cures poison and cuts",
			"At level 10 it restores drained stats",
			"At level 15 it restores lost experience"
	}
}

REGENERATION = add_spell
{
	["name"] = 	"Regeneration",
	["school"] = 	{SCHOOL_NATURE},
	["level"] = 	20,
	["mana"] = 	30,
	["mana_max"] = 	55,
	["fail"] = 	70,
	["inertia"] = 	{ 4, 40 },
	["spell"] = 	function() return nature_regeneration() end,
	["info"] = 	function() return nature_regeneration_info() end,
	["desc"] =	{
			"Increases your body's regeneration rate",
	}
}


SUMMONANNIMAL = add_spell
{
	["name"] =      "Summon Animal",
	["school"] = 	{SCHOOL_NATURE},
	["level"] = 	25,
	["mana"] = 	25,
	["mana_max"] = 	50,
	["fail"] = 	90,
	["stick"] =
	{
			["charge"] =    { 1, 3 },
			[TV_WAND] =
			{
				["rarity"] = 		85,
				["base_level"] =	{ 1, 5 },
				["max_level"] =		{ 15, 45 },
			},
	},
	["spell"] = 	function() return nature_summon_animal() end,
	["info"] = 	function() return nature_summon_animal_info() end,
	["desc"] =	{
			"Summons a leveled animal to your aid",
	}
}
