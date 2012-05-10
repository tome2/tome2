-- handle the mind school

CHARM = add_spell
{
	["name"] = 	"Charm",
	["school"] = 	{SCHOOL_MIND},
	["level"] = 	1,
	["mana"] = 	1,
	["mana_max"] = 	20,
	["fail"] = 	10,
	["stick"] =
	{
			["charge"] =    { 7, 5 },
			[TV_WAND] =
			{
				["rarity"] = 		35,
				["base_level"] =	{ 1, 15 },
				["max_level"] =		{ 20, 40 },
			},
	},
	["spell"] = 	function() return mind_charm() end,
	["info"] = 	function() return mind_charm_info() end,
	["desc"] =	{
			"Tries to manipulate the mind of a monster to make it friendly",
			"At level 15 it turns into a ball",
			"At level 35 it affects all monsters in sight"
	}
}

CONFUSE = add_spell
{
	["name"] = 	"Confuse",
	["school"] = 	{SCHOOL_MIND},
	["level"] = 	5,
	["mana"] = 	5,
	["mana_max"] = 	30,
	["fail"] = 	20,
	["stick"] =
	{
			["charge"] =    { 3, 4 },
			[TV_WAND] =
			{
				["rarity"] = 		45,
				["base_level"] =	{ 1, 5 },
				["max_level"] =		{ 20, 40 },
			},
	},
	["spell"] = 	function() return mind_confuse() end,
	["info"] = 	function() return mind_confuse_info() end,
	["desc"] =	{
			"Tries to manipulate the mind of a monster to confuse it",
			"At level 15 it turns into a ball",
			"At level 35 it affects all monsters in sight"
	}
}

ARMOROFFEAR = add_spell
{
	["name"] = 	"Armor of Fear",
	["school"] = 	SCHOOL_MIND,
	["level"] = 	10,
	["mana"] = 	10,
	["mana_max"] = 	50,
	["fail"] = 	35,
	["inertia"] = 	{ 2, 20 },
	["spell"] = 	function() return mind_armor_of_fear() end,
	["info"] = 	function() return mind_armor_of_fear_info() end,
	["desc"] =	{
			"Creates a shield of pure fear around you. Any monster attempting to hit you",
			"must save or flee",
		}
}

STUN = add_spell
{
	["name"] = 	"Stun",
	["school"] = 	{SCHOOL_MIND},
	["level"] = 	15,
	["mana"] = 	10,
	["mana_max"] = 	90,
	["fail"] = 	45,
	["spell"] = 	function() return mind_stun() end,
	["info"] = 	function() return mind_stun_info() end,
	["desc"] =	{
			"Tries to manipulate the mind of a monster to stun it",
			"At level 20 it turns into a ball",
	}
}
