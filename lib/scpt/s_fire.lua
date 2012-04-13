-- handle the fire school

GLOBELIGHT = add_spell
{
	["name"] =      "Globe of Light",
	["school"] =    {SCHOOL_FIRE},
	["level"] =     1,
	["mana"] =      2,
	["mana_max"] =  15,
	["fail"] =      10,
	["stick"] =
	{
			["charge"] =    { 10, 5 },
			[TV_STAFF] =
			{
				["rarity"] =	    7,
				["base_level"] =	{ 1, 15 },
				["max_level"] =		{ 10, 45 },
			},
	},
	["inertia"] = 	{ 1, 40 },
	["spell"] =     function() return fire_globe_of_light() end,
	["info"] =      function() return fire_globe_of_light_info() end,
	["desc"] =      {
			"Creates a globe of pure light",
			"At level 3 it starts damaging monsters",
			"At level 15 it starts creating a more powerful kind of light",
	}
}

FIREFLASH = add_spell
{
	["name"] =      "Fireflash",
	["school"] =    {SCHOOL_FIRE},
	["level"] =     10,
	["mana"] =      5,
	["mana_max"] =  70,
	["fail"] =      35,
	["stick"] =
	{
			["charge"] =    { 5, 5 },
			[TV_WAND] =
			{
				["rarity"] =	    35,
				["base_level"] =	{ 1, 15 },
				["max_level"] =		{ 15, 35 },
			},
	},
	["spell"] =     function() return fire_fireflash() end,
	["info"] =      function() return fire_fireflash_info() end,
	["desc"] =      {
			"Conjures a ball of fire to burn your foes to ashes",
			"At level 20 it turns into a ball of holy fire"
	}
}

FIERYAURA = add_spell
{
	["name"] =      "Fiery Shield",
	["school"] =    {SCHOOL_FIRE},
	["level"] =     20,
	["mana"] =      20,
	["mana_max"] =  60,
	["fail"] =      50,
	["stick"] =
	{
			["charge"] =    { 3, 5 },
			[TV_STAFF] =
			{
				["rarity"] =	    50,
				["base_level"] =	{ 1, 10 },
				["max_level"] =		{ 5, 40 },
			},
	},
	["inertia"] = 	{ 2, 15 },
	["spell"] =     function() return fire_fiery_shield() end,
	["info"] =      function() return fire_fiery_shield_info() end,
	["desc"] =      {
			"Creates a shield of fierce flames around you",
			"At level 8 it turns into a greater kind of flame that can not be resisted"
	}
}

FIREWALL = add_spell
{
	["name"] =      "Firewall",
	["school"] =    {SCHOOL_FIRE},
	["level"] =     15,
	["mana"] =      25,
	["mana_max"] =  100,
	["fail"] =      40,
	["stick"] =
	{
			["charge"] =    { 4, 5 },
			[TV_WAND] =
			{
				["rarity"] =	    55,
				["base_level"] =	{ 1, 10 },
				["max_level"] =		{ 5, 40 },
			},
	},
	["spell"] =     function() return fire_firewall() end,
	["info"] =      function() return fire_firewall_info() end,
	["desc"] =      {
			"Creates a fiery wall to incinerate monsters stupid enough to attack you",
			"At level 6 it turns into a wall of hell fire"
	}
}

FIREGOLEM = add_spell
{
	["name"] =      "Fire Golem",
	["school"] =    {SCHOOL_FIRE, SCHOOL_MIND},
	["level"] =     7,
	["mana"] =      16,
	["mana_max"] =  70,
	["fail"] =      40,
	["spell"] =     function() return fire_golem() end,
	["info"] =      function() return fire_golem_info() end,
	["desc"] =      {
			"Creates a fiery golem and controls it",
			"During the control the available keylist is:",
			"Movement keys: movement of the golem(depending on its speed",
			"               it can move more than one square)",
			", : pickup all items on the floor",
			"d : drop all carried items",
			"i : list all carried items",
			"m : end the possession/use golem powers",
			"Most of the other keys are disabled, you cannot interact with your",
			"real body while controlling the golem",
			"But to cast the spell you will need a lantern or a wooden torch to",
			"Create the golem from"
	}
}
