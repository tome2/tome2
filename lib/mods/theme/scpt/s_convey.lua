-- handle the conveyance school

BLINK = add_spell
{
	["name"] = 	"Phase Door",
	["school"] = 	{SCHOOL_CONVEYANCE},
	["level"] = 	1,
	["mana"] = 	1,
	["mana_max"] =  3,
	["fail"] = 	10,
	["inertia"] = 	{ 1, 5 },
	["spell"] = 	function() return convey_blink() end,
	["info"] = 	function() return convey_blink_info() end,
	["desc"] =	{
			"Teleports you on a small scale range",
			"At level 30 it creates void jumpgates",
	}
}

DISARM = add_spell
{
	["name"] = 	"Disarm",
	["school"] = 	{SCHOOL_CONVEYANCE},
	["level"] = 	3,
	["mana"] = 	2,
	["mana_max"] = 	4,
	["fail"] = 	15,
	["stick"] =
	{
			["charge"] =    { 10, 15 },
			[TV_STAFF] =
			{
				["rarity"] = 		4,
				["base_level"] =	{ 1, 10 },
				["max_level"] =		{ 10, 50 },
			},
	},
	["spell"] = 	function() return convey_disarm() end,
	["info"] = 	function() return convey_disarm_info() end,
	["desc"] =	{
			"Destroys doors and disarms traps",
			"At level 10 it unlocks doors and disarms traps",
	}
}

TELEPORT = add_spell
{
	["name"] = 	"Teleportation",
	["school"] = 	{SCHOOL_CONVEYANCE},
	["level"] = 	10,
	["mana"] = 	8,
	["mana_max"] = 	14,
	["fail"] = 	30,
	["stick"] =
	{
			["charge"] =    { 7, 7 },
			[TV_STAFF] =
			{
				["rarity"] = 		50,
				["base_level"] =	{ 1, 20 },
				["max_level"] =		{ 20, 50 },
			},
	},
	["inertia"] = 	{ 1, 10 },
	["spell"] = 	function() return convey_teleport() end,
	["info"] = 	function() return convey_teleport_info() end,
	["desc"] =	{
			"Teleports you around the level. The casting time decreases with level",
	}
}

TELEAWAY = add_spell
{
	["name"] = 	"Teleport Away",
	["school"] = 	{SCHOOL_CONVEYANCE},
	["level"] = 	23,
	["mana"] = 	15,
	["mana_max"] = 	40,
	["fail"] = 	60,
	["stick"] =
	{
			["charge"] =    { 3, 5 },
			[TV_WAND] =
			{
				["rarity"] = 		75,
				["base_level"] =	{ 1, 20 },
				["max_level"] =		{ 20, 50 },
			},
	},
	["spell"] = 	function() return convey_teleport_away() end,
	["info"] = 	function() return convey_teleport_away_info() end,
	["desc"] =	{
			"Teleports a line of monsters away",
			"At level 10 it turns into a ball",
			"At level 20 it teleports all monsters in sight"
	}
}

RECALL = add_spell
{
	["name"] = 	"Recall",
	["school"] = 	{SCHOOL_CONVEYANCE},
	["level"] = 	30,
	["mana"] = 	25,
	["mana_max"] = 	25,
	["fail"] =      60,
	["spell"] = 	function() return convey_recall() end,
	["info"] = 	function() return convey_recall_info() end,
	["desc"] =	{
			"Cast on yourself it will recall you to the surface/dungeon.",
			"Cast at a monster you will swap positions with the monster.",
			"Cast at an object it will fetch the object to you."
	}
}

PROBABILITY_TRAVEL = add_spell
{
	["name"] = 	"Probability Travel",
	["school"] = 	{SCHOOL_CONVEYANCE},
	["level"] = 	35,
	["mana"] = 	30,
	["mana_max"] = 	50,
	["fail"] = 	90,
	["stick"] =
	{
			["charge"] =    { 1, 2 },
			[TV_STAFF] =
			{
				["rarity"] = 		97,
				["base_level"] =	{ 1, 5 },
				["max_level"] =		{ 8, 25 },
			},
	},
	["inertia"] = 	{ 6, 40 },
	["spell"] = 	function() return convey_probability_travel() end,
	["info"] = 	function() return convey_probability_travel_info() end,
	["desc"] =	{
			"Renders you immaterial, when you hit a wall you travel through it and",
			"instantly appear on the other side of it. You can also float up and down",
			"at will"
	}
}
