-- handle the air school

NOXIOUSCLOUD = add_spell
{
	["name"] = 	"Noxious Cloud",
	["school"] = 	{SCHOOL_AIR},
	["level"] = 	3,
	["mana"] = 	3,
	["mana_max"] = 	30,
	["fail"] = 	20,
	["stick"] =
	{
			["charge"] =    { 5, 7 },
			[TV_WAND] =
			{
				["rarity"] = 		15,
				["base_level"] =	{ 1, 15 },
				["max_level"] =		{ 25, 50 },
			},
	},
	["spell"] = 	function() return air_noxious_cloud() end,
	["info"] = 	function() return air_noxious_cloud_info() end,
	["desc"] =	{
			"Creates a cloud of poison",
			"The cloud will persist for some turns, damaging all monsters passing by",
			"At spell level 30 it turns into a thick gas attacking all living beings"
	}
}

AIRWINGS = add_spell
{
	["name"] = 	"Wings of Winds",
	["school"] = 	{SCHOOL_AIR, SCHOOL_CONVEYANCE},
	["level"] = 	22,
	["mana"] = 	30,
	["mana_max"] = 	40,
	["fail"] = 	60,
	["stick"] =
	{
			["charge"] =    { 7, 5 },
			[TV_STAFF] =
			{
				["rarity"] = 		27,
				["base_level"] =	{ 1, 10 },
				["max_level"] =		{ 20, 50 },
			},
	},
	["inertia"] = 	{ 1, 10 },
	["spell"] = 	function() return air_wings_of_winds() end,
	["info"] = 	function() return air_wings_of_winds_info() end,
	["desc"] =	{
			"Grants the power of levitation",
			"At level 16 it grants the power of controlled flight"
	}
}

INVISIBILITY = add_spell
{
	["name"] = 	"Invisibility",
	["school"] = 	{SCHOOL_AIR},
	["level"] = 	16,
	["mana"] = 	10,
	["mana_max"] = 	20,
	["fail"] = 	50,
	["inertia"] = 	{ 1, 30 },
	["spell"] = 	function() return air_invisibility() end,
	["info"] = 	function() return air_invisibility_info() end,
	["desc"] =	{
			"Grants invisibility"
	}
}

POISONBLOOD = add_spell
{
	["name"] = 	"Poison Blood",
	["school"] = 	{SCHOOL_AIR},
	["level"] = 	12,
	["mana"] = 	10,
	["mana_max"] = 	20,
	["fail"] = 	30,
	["stick"] =
	{
			["charge"] =    { 10, 15 },
			[TV_WAND] =
			{
				["rarity"] = 		45,
				["base_level"] =	{ 1, 25 },
				["max_level"] =		{ 35, 50 },
			},
	},
	["inertia"] = 	{ 1, 35 },
	["spell"] = 	function() return air_poison_blood() end,
	["info"] = 	function() return air_poison_blood_info() end,
	["desc"] =	{
			"Grants resist poison",
			"At level 15 it provides poison branding to wielded weapon"
	}
}

THUNDERSTORM = add_spell
{
	["name"] = 	"Thunderstorm",
	["school"] = 	{SCHOOL_AIR, SCHOOL_NATURE},
	["level"] = 	25,
	["mana"] = 	40,
	["mana_max"] = 	60,
	["fail"] = 	60,
	["stick"] =
	{
			["charge"] =    { 5, 5 },
			[TV_WAND] =
			{
				["rarity"] = 		85,
				["base_level"] =	{ 1, 5 },
				["max_level"] =		{ 25, 50 },
			},
	},
	["inertia"] = 	{ 2, 15 },
	["spell"] = 	function() return air_thunderstorm() end,
	["info"] = 	function() return air_thunderstorm_info() end,
	["desc"] =	{
			"Charges up the air around you with electricity",
			"Each turn it will throw a thunder bolt at a random monster in sight",
			"The thunder does 3 types of damage, one third of lightning",
			"one third of sound and one third of light"
	}
}

STERILIZE = add_spell
{
	["name"] = 	"Sterilize",
	["school"] = 	{SCHOOL_AIR},
	["level"] = 	20,
	["mana"] = 	10,
	["mana_max"] = 	100,
	["fail"] = 	50,
	["stick"] =
	{
			["charge"] =    { 7, 5 },
			[TV_STAFF] =
			{
				["rarity"] = 		20,
				["base_level"] =	{ 1, 10 },
				["max_level"] =		{ 20, 50 },
			},
	},
	["spell"] = 	function() return air_sterilize() end,
	["info"] = 	function() return air_sterilize_info() end,
	["desc"] =	{
			"Prevents explosive breeding for a while."
	}
}
