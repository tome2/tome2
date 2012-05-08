-- handle the meta school

RECHARGE = add_spell
{
	["name"] = 	"Recharge",
	["school"] = 	{SCHOOL_META},
	["level"] = 	5,
	["mana"] = 	10,
	["mana_max"] = 	100,
	["fail"] = 	20,
	["spell"] = 	function() return meta_recharge() end,
	["info"] = 	function() return meta_recharge_info() end,
	["desc"] =	{
			"Taps on the ambient mana to recharge an object's power (charges or mana)",
	}
}

SPELLBINDER = add_spell
{
	["name"] = 	"Spellbinder",
	["school"] = 	{SCHOOL_META},
	["level"] = 	20,
	["mana"] = 	100,
	["mana_max"] = 	300,
	["fail"] = 	85,
	["spell"] = 	function() return meta_spellbinder() end,
	["info"] = 	function() return meta_spellbinder_info() end,
	["desc"] =	{
			"Stores spells in a trigger.",
			"When the condition is met all spells fire off at the same time",
			"This spell takes a long time to cast so you are advised to prepare it",
			"in a safe area.",
			"Also it will use the mana for the Spellbinder and the mana for the",
			"selected spells"
	}
}

DISPERSEMAGIC = add_spell
{
	["name"] = 	"Disperse Magic",
	["school"] = 	{SCHOOL_META},
	["level"] = 	15,
	["mana"] = 	30,
	["mana_max"] = 	60,
	["fail"] = 	40,
	-- Unnafected by blindness
	["blind"] =     FALSE,
	-- Unnafected by confusion
	["confusion"] = FALSE,
	["stick"] =
	{
			["charge"] =    { 5, 5 },
			[TV_WAND] =
			{
				["rarity"] = 		25,
				["base_level"] =	{ 1, 15 },
				["max_level"] =		{ 5, 40 },
			},
	},
	["inertia"] = 	{ 1, 5 },
	["spell"] = 	function() return meta_disperse_magic() end,
	["info"] = 	function() return meta_disperse_magic_info() end,
	["desc"] =	{
			"Dispels a lot of magic that can affect you, be it good or bad",
			"Level 1: blindness and light",
			"Level 5: confusion and hallucination",
			"Level 10: speed (both bad or good) and light speed",
			"Level 15: stunning, meditation, cuts",
			"Level 20: hero, super hero, bless, shields, afraid, parasites, mimicry",
	}
}

TRACKER = add_spell
{
	["name"] = 	"Tracker",
	["school"] = 	{SCHOOL_META, SCHOOL_CONVEYANCE},
	["level"] = 	30,
	["mana"] = 	50,
	["mana_max"] = 	50,
	["fail"] = 	95,
	["spell"] = 	function() return meta_tracker() end,
	["info"] = 	function() return meta_tracker_info() end,
	["desc"] =	{
			"Tracks down the last teleportation that happened on the level and teleports",
			"you to it",
	}
}

INERTIA_CONTROL = add_spell
{
	["name"] = 	"Inertia Control",
	["school"] = 	{SCHOOL_META},
	["level"] = 	37,
	["mana"] = 	300,
	["mana_max"] = 	700,
	["fail"] = 	95,
	["spell"] = 	function() return meta_inertia_control() end,
	["info"] = 	function() return meta_inertia_control_info() end,
	["desc"] = 	{
			"Changes the energy flow of a spell to be continuously recasted",
			"at a given interval. The inertia controlled spell reduces your",
			"maximum mana by four times its cost.",
	}
}
