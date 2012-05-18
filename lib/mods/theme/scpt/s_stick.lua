-- Spells that are stick or artifacts/... only

DEVICE_HEAL_MONSTER = add_spell
{
	["name"] = 	"Heal Monster",
	["school"] = 	{SCHOOL_DEVICE},
	["level"] = 	3,
	["mana"] = 	5,
	["mana_max"] = 	20,
	["fail"] = 	15,
	["random"] =    -1,
	["stick"] =
	{
			["charge"] =    { 10, 10 },
			[TV_WAND] =
			{
				["rarity"] = 		17,
				["base_level"] =	{ 1, 15 },
				["max_level"] =		{ 20, 50 },
			},
	},
	["spell"] = 	function() return device_heal_monster() end,
	["info"] = 	function() return device_heal_monster_info() end,
	["desc"] =	{
			"Heals a monster",
	}
}

DEVICE_SPEED_MONSTER = add_spell
{
	["name"] = 	"Haste Monster",
	["school"] = 	{SCHOOL_DEVICE},
	["level"] = 	10,
	["mana"] = 	10,
	["mana_max"] = 	10,
	["fail"] = 	30,
	["random"] =    -1,
	["stick"] =
	{
			["charge"] =    { 10, 5 },
			[TV_WAND] =
			{
				["rarity"] = 		7,
				["base_level"] =	{ 1, 1 },
				["max_level"] =		{ 20, 50 },
			},
	},
	["spell"] = 	function() return device_haste_monster() end,
	["info"] = 	function() return device_haste_monster_info() end,
	["desc"] =	{
			"Haste a monster",
	}
}

DEVICE_WISH = add_spell
{
	["name"] = 	"Wish",
	["school"] = 	{SCHOOL_DEVICE},
	["level"] = 	50,
	["mana"] = 	400,
	["mana_max"] = 	400,
	["fail"] = 	99,
	["random"] =    -1,
	["stick"] =
	{
			["charge"] =    { 1, 2 },
			[TV_STAFF] =
			{
				["rarity"] = 		98,
				["base_level"] =	{ 1, 1 },
				["max_level"] =		{ 1, 1 },
			},
	},
	["spell"] = 	function() return device_wish() end,
	["info"] = 	function() return device_wish_info() end,
	["desc"] =	{
			"This grants you a wish, beware of what you ask for!",
	}
}

DEVICE_SUMMON = add_spell
{
	["name"] = 	"Summon",
	["school"] = 	{SCHOOL_DEVICE},
	["level"] = 	5,
	["mana"] = 	5,
	["mana_max"] = 	25,
	["fail"] = 	20,
	["random"] =    -1,
	["stick"] =
	{
			["charge"] =    { 1, 20 },
			[TV_STAFF] =
			{
				["rarity"] = 		13,
				["base_level"] =	{ 1, 40 },
				["max_level"] =		{ 25, 50 },
			},
	},
	["spell"] = 	function() return device_summon_monster() end,
	["info"] = 	function() return device_summon_monster_info() end,
	["desc"] =	{
			"Summons hostile monsters near you",
	}
}

DEVICE_MANA = add_spell
{
	["name"] = 	"Mana",
	["school"] = 	{SCHOOL_DEVICE},
	["level"] = 	30,
	["mana"] = 	1,
	["mana_max"] = 	1,
	["fail"] = 	80,
	["random"] =    -1,
	["stick"] =
	{
			["charge"] =    { 2, 3 },
			[TV_STAFF] =
			{
				["rarity"] = 		78,
				["base_level"] =	{ 1, 5 },
				["max_level"] =		{ 20, 35 },
			},
	},
	["spell"] = 	function() return device_mana() end,
	["info"] = 	function() return device_mana_info() end,
	["desc"] =	{
			"Restores a part(or all) of your mana",
	}
}

DEVICE_NOTHING = add_spell
{
	["name"] = 	"Nothing",
	["school"] = 	{SCHOOL_DEVICE},
	["level"] = 	1,
	["mana"] = 	0,
	["mana_max"] = 	0,
	["fail"] = 	0,
	["random"] =    -1,
	["stick"] =
	{
			["charge"] =    { 0, 0 },
			[TV_WAND] =
			{
				["rarity"] = 		3,
				["base_level"] =	{ 1, 1 },
				["max_level"] =		{ 1, 1 },
			},
			[TV_STAFF] =
			{
				["rarity"] = 		3,
				["base_level"] =	{ 1, 1 },
				["max_level"] =		{ 1, 1},
			},
	},
	["spell"] = 	function() return device_nothing() end,
	["info"] = 	function() return device_nothing_info() end,
	["desc"] =	{
			"It does nothing.",
	}
}

DEVICE_MAGGOT = add_spell
{
	["name"] = 	"Artifact Maggot",
	["school"] = 	{SCHOOL_DEVICE},
	["level"] = 	1,
	["mana"] = 	7,
	["mana_max"] = 	7,
	["fail"] = 	20,
	["random"] =    -1,
	["activate"] =  { 10, 50 },
	["spell"] = 	function() return device_maggot() end,
	["info"] = 	function() return device_maggot_info() end,
	["desc"] =	{
			"terrify",
	}
}

DEVICE_HOLY_FIRE = add_spell
{
	["name"] = 	"Holy Fire of Mithrandir",
	["school"] = 	{SCHOOL_DEVICE},
	["level"] = 	30,
	["mana"] = 	50,
	["mana_max"] = 	150,
	["fail"] = 	75,
	["random"] =    -1,
	["stick"] =
	{
			["charge"] =    { 2, 5 },
			[TV_STAFF] =
			{
				-- Rarity higher than 100 to be sure to not have it generated randomly
				["rarity"] = 		999,
				["base_level"] =	{ 1, 1 },
				["max_level"] =		{ 35, 35 },
			},
	},
	["spell"] = 	function() return device_holy_fire() end,
	["info"] = 	function() return device_holy_fire_info() end,
	["desc"] =	{
			"The Holy Fire created by this staff will deeply(double damage) burn",
			"all that is evil.",
	}
}

-- Ok the Eternal Flame, to craete one of the 4 ultimate arts
-- needed to enter the last level of the Void
DEVICE_ETERNAL_FLAME = add_spell
{
	["name"] = 	"Artifact Eternal Flame",
	["school"] = 	{SCHOOL_DEVICE},
	["level"] = 	1,
	["mana"] = 	0,
	["mana_max"] = 	0,
	["fail"] = 	0,
	["random"] =    -1,
	["activate"] =  { 0, 0 },
	["spell"] = 	function(flame_item) return device_eternal_flame(flame_item) end,
	["info"] = 	function() return "" end,
	["desc"] =	{
			"Imbuing an object with the eternal fire",
	}
}

DEVICE_THUNDERLORDS = add_spell
{
	["name"] = 	"Artifact Thunderlords",
	["school"] = 	{SCHOOL_DEVICE},
	["level"] = 	1,
	["mana"] = 	1,
	["mana_max"] = 	1,
	["fail"] = 	20,
	["random"] =    -1,
	["stick"] =
	{
			["charge"] =    { 5, 5 },
			[TV_STAFF] =
			{
				-- Rarity higher than 100 to be sure to not have it generated randomly
				["rarity"] = 		999,
				["base_level"] =	{ 1, 1 },
				["max_level"] =		{ 1, 1 },
			},
	},
	["spell"] = 	function() return device_thunderlords() end,
	["info"] = 	function() return device_thunderlords_info() end,
	["desc"] =	{
			"An Eagle of Manwe will appear to transport you quickly to the town.",
	}
}

-- Two new spells from T-Plus by Ingeborg S. Norden, for artifact activations:

DEVICE_RADAGAST = add_spell
{
	["name"] = "Artifact Radagast",
	["school"] = {SCHOOL_DEVICE},
	["level"] = 1,
	["mana"] = 0,
	["mana_max"] = 0,
	["fail"] = 10,
	["random"] = -1,
	["activate"] = { 15000, 0 },
	["spell"] = function() return device_radagast() end,
	["info"] = function() return device_radagast_info() end,
	["desc"] = {
	"purity and health",
	}
}

DEVICE_VALAROMA = add_spell
{
	["name"] = 	"Artifact Valaroma",
	["school"] = 	{SCHOOL_DEVICE},
	["level"] = 	1,
	["mana"] = 	0,
	["mana_max"] = 	0,
	["fail"] = 	25,
	["random"] =    -1,
	["activate"] =  { 250, 0 },
	["spell"] = 	function() return device_valaroma() end,
	["info"] = 	function() return device_valaroma_info() end,
	["desc"] =	{
			"banish evil (level x5)",
	}
}
