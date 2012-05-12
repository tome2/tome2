-- handle the water school

TIDALWAVE = add_spell
{
	["name"] = 	"Tidal Wave",
	["school"] = 	{SCHOOL_WATER},
	["level"] = 	16,
	["mana"] = 	16,
	["mana_max"] = 	40,
	["fail"] = 	65,
	["stick"] =
	{
			["charge"] =    { 6, 5 },
			[TV_WAND] =
			{
				["rarity"] = 		54,
				["base_level"] =	{ 1, 10 },
				["max_level"] =		{ 20, 50 },
			},
	},
	["inertia"] = 	{ 4, 100 },
	["spell"] = 	function() return water_tidal_wave() end,
	["info"] = 	function() return water_tidal_wave_info() end,
	["desc"] =	{
			"Summons a monstrous tidal wave that will expand and crush the",
			"monsters under its mighty waves."
	}
}

ICESTORM = add_spell
{
	["name"] = 	"Ice Storm",
	["school"] = 	{SCHOOL_WATER},
	["level"] = 	22,
	["mana"] = 	30,
	["mana_max"] = 	60,
	["fail"] = 	80,
	["stick"] =
	{
			["charge"] =    { 3, 7 },
			[TV_WAND] =
			{
				["rarity"] = 		65,
				["base_level"] =	{ 1, 5 },
				["max_level"] =		{ 25, 45 },
			},
	},
	["inertia"] = 	{ 3, 40 },
	["spell"] = 	function() return water_ice_storm() end,
	["info"] = 	function() return water_ice_storm_info() end,
	["desc"] =	{
			"Engulfs you in a storm of roaring cold that strikes your foes.",
			"At level 10 it turns into shards of ice."
	}
}

ENTPOTION = add_spell
{
	["name"] = 	"Ent's Potion",
	["school"] = 	{SCHOOL_WATER},
	["level"] = 	6,
	["mana"] = 	7,
	["mana_max"] = 	15,
	["fail"] = 	35,
	["inertia"] = 	{ 1, 30 },
	["spell"] = 	function() return water_ent_potion() end,
	["info"] = 	function() return water_ent_potion_info() end,
	["desc"] =	{
			"Fills up your stomach.",
			"At level 5 it boldens your heart.",
			"At level 12 it makes you heroic."
	}
}

VAPOR = add_spell
{
	["name"] = 	"Vapor",
	["school"] = 	{SCHOOL_WATER},
	["level"] = 	2,
	["mana"] = 	2,
	["mana_max"] = 	12,
	["fail"] = 	20,
	["inertia"] = 	{ 1, 30 },
	["spell"] = 	function() return water_vapor() end,
	["info"] = 	function() return water_vapor_info() end,
	["desc"] =	{
			"Fills the air with toxic moisture to eradicate annoying critters."
	}
}

GEYSER = add_spell
{
	["name"] = "Geyser",
	["school"] = SCHOOL_WATER,
	["level"] = 1,
	["mana"] = 1,
	["mana_max"] = 35,
	["fail"] = 5,
	["spell"] = function() return water_geyser() end,
	["info"] = function() return water_geyser_info() end,
	["desc"] =
	{
		"Shoots a geyser of water from your fingertips.",
		"Sometimes it can blast through its first target."
	},
}
