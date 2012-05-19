-- Geomancy school

CALL_THE_ELEMENTS = add_spell
{
	["name"] = 	"Call the Elements",
	["school"] = 	{SCHOOL_GEOMANCY},
	["level"] = 	1,
	["mana"] = 	2,
	["mana_max"] = 	20,
	["fail"] = 	10,
	-- Unnafected by blindness
	["blind"] =     FALSE,
	["random"] =    0,
	["spell"] = 	function() return geomancy_call_the_elements() end,
	["info"] = 	function() return geomancy_call_the_elements_info() end,
	["desc"] =	{
			"Randomly creates various elements around you",
			"Each type of element chance is controlled by your level",
			"in the corresponding skill",
			"At level 17 it can be targeted",
	}
}

CHANNEL_ELEMENTS = add_spell
{
	["name"] = 	"Channel Elements",
	["school"] = 	{SCHOOL_GEOMANCY},
	["level"] = 	3,
	["mana"] = 	3,
	["mana_max"] = 	30,
	["fail"] = 	20,
	-- Unnafected by blindness
	["blind"] =     FALSE,
	["random"] =    0,
	["spell"] = 	function() return geomancy_channel_elements() end,
	["info"] = 	function() return geomancy_channel_elements_info() end,
	["desc"] =	{
			"Draws on the caster's immediate environs to form an attack or other effect.",
			"Grass/Flower heals.",
			"Water creates water bolt attacks.",
			"Ice creates ice bolt attacks.",
			"Sand creates a wall of thick, blinding, burning sand around you.",
			"Lava creates fire bolt attacks.",
			"Deep lava creates fire ball attacks.",
			"Chasm creates darkness bolt attacks.",
			"At Earth level 18, darkness becomes nether.",
			"At Water level 8, water attacks become beams with a striking effect.",
			"At Water level 12, ice attacks become balls of ice shards.",
			"At Water level 18, water attacks push monsters back.",
			"At Fire level 15, fire become hellfire.",
	}
}

ELEMENTAL_WAVE = add_spell
{
	["name"] = 	"Elemental Wave",
	["school"] = 	{SCHOOL_GEOMANCY},
	["level"] = 	15,
	["mana"] = 	15,
	["mana_max"] = 	50,
	["fail"] = 	20,
	-- Unnafected by blindness
	["blind"] =     FALSE,
	["random"] =    0,
	["spell"] = 	function() return geomancy_elemental_wave() end,
	["info"] = 	function() return geomancy_elemental_wave_info() end,
	["desc"] =	{
			"Draws on an adjacent special square to project a slow-moving",
			"wave of that element in that direction",
			"Abyss squares cannot be channeled into a wave.",
	}
}

VAPORIZE = add_spell
{
	["name"] = 	"Vaporize",
	["school"] = 	{SCHOOL_GEOMANCY},
	["level"] = 	4,
	["mana"] = 	3,
	["mana_max"] = 	30,
	["fail"] = 	15,
	-- Unnafected by blindness
	["blind"] =     FALSE,
	-- Must have at least 4 Air
	["random"] =    0,
	["depend"] =    function() return geomancy_vaporize_depends() end,
	["spell"] = 	function() return geomancy_vaporize() end,
	["info"] = 	function() return geomancy_vaporize_info() end,
	["desc"] =	{
			"Draws upon your immediate environs to form a cloud of damaging vapors",
	}
}

GEOLYSIS = add_spell
{
	["name"] = 	"Geolysis",
	["school"] = 	{SCHOOL_GEOMANCY},
	["level"] = 	7,
	["mana"] = 	15,
	["mana_max"] = 	40,
	["fail"] = 	15,
	-- Unnafected by blindness
	["blind"] =     FALSE,
	["random"] =    0,
	-- Must have at least 7 Earth
	["depend"] =    function() return geomancy_geolysis_depends() end,
	["spell"] = 	function() return geomancy_geolysis() end,
	["info"] = 	function() return geomancy_geolysis_info() end,
	["desc"] =	{
			"Burrows deeply and slightly at random into a wall,",
			"leaving behind tailings of various different sorts of walls in the passage.",
	}
}

DRIPPING_TREAD = add_spell
{
	["name"] = 	"Dripping Tread",
	["school"] = 	{SCHOOL_GEOMANCY},
	["level"] = 	10,
	["mana"] = 	15,
	["mana_max"] = 	25,
	["fail"] = 	15,
	-- Unnafected by blindness
	["blind"] =     FALSE,
	["random"] =    0,
	-- Must have at least 10 Water
	["depend"] =    function() return geomancy_dripping_tread_depends() end,
	["spell"] =     function() return geomancy_dripping_tread() end,
	["info"] = 	function() return geomancy_dripping_tread_info() end,
	["desc"] =	{
			"Causes you to leave random elemental forms behind as you walk",
	}
}

GROW_BARRIER = add_spell
{
	["name"] = 	"Grow Barrier",
	["school"] = 	{SCHOOL_GEOMANCY},
	["level"] = 	12,
	["mana"] = 	30,
	["mana_max"] = 	40,
	["fail"] = 	15,
	-- Unnafected by blindness
	["blind"] =     FALSE,
	["random"] =    0,
	-- Must have at least 12 Earth
	["depend"] =    function() return geomancy_grow_barrier_depends() end,
	["spell"] = 	function() return geomancy_grow_barrier() end,
	["info"] = 	function() return geomancy_grow_barrier_info() end,
	["desc"] =	{
			"Creates impassable terrain (walls, trees, etc.) around you.",
			"At level 20 it can be projected around another area.",
	}
}

ELEMENTAL_MINION = add_spell
{
	["name"] = 	"Elemental Minion",
	["school"] = 	{SCHOOL_GEOMANCY},
	["level"] = 	20,
	["mana"] = 	40,
	["mana_max"] = 	80,
	["fail"] = 	25,
	-- Unnafected by blindness
	["random"] =    0,
	-- Must have at least 12 Earth
	["spell"] = 	function() return geomancy_elemental_minion() end,
	["info"] = 	function() return geomancy_elemental_minion_info() end,
	["desc"] =	{
			"Summons a minion from a nearby element.",
			"Walls can summon Earth elmentals, Xorns and Xarens",
			"Dark Pits can summon Air elementals, Ancient blue dragons, Great Storm Wyrms",
			"and Sky Drakes",
			"Sandwalls and lava can summon Fire elementals and Ancient red dragons",
			"Icewall, and water can summon Water elementals, Water trolls and Water demons",
	}
}
