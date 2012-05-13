-- handle the demonology school

-- Demonblade
DEMON_BLADE = add_spell
{
	["name"] =      "Demon Blade",
	["school"] =    {SCHOOL_DEMON},
	["level"] =     1,
	["mana"] =      4,
	["mana_max"] =  44,
	["fail"] =      10,
	["random"] =    0,
	["stick"] =
	{
			["charge"] =    { 3, 7 },
			[TV_WAND] =
			{
				["rarity"] =	    75,
				["base_level"] =	{ 1, 17 },
				["max_level"] =		{ 20, 40 },
			},
	},
	["spell"] =     function() return demonology_demon_blade() end,
	["info"] =      function() return demonology_demon_blade_info() end,
	["desc"] =      {
			"Imbues your blade with fire to deal more damage",
			"At level 30 it deals hellfire damage",
			"At level 45 it spreads over a 1 radius zone around your target",
	}
}

DEMON_MADNESS = add_spell
{
	["name"] =      "Demon Madness",
	["school"] =    {SCHOOL_DEMON},
	["level"] =     10,
	["mana"] =      5,
	["mana_max"] =  20,
	["fail"] =      25,
	["random"] =    0,
	["spell"] =     function() return demonology_demon_madness() end,
	["info"] =      function() return demonology_demon_madness_info() end,
	["desc"] =      {
			"Fire 2 balls in opposite directions of randomly chaos, confusion or charm",
	}
}

DEMON_FIELD = add_spell
{
	["name"] =      "Demon Field",
	["school"] =    {SCHOOL_DEMON},
	["level"] =     20,
	["mana"] =      20,
	["mana_max"] =  60,
	["fail"] =      60,
	["random"] =    0,
	["spell"] =     function() return demonology_demon_field() end,
	["info"] =      function() return demonology_demon_field_info() end,
	["desc"] =      {
			"Fires a cloud of deadly nexus over a radius of 7",
	}
}

-- Demonshield

DOOM_SHIELD = add_spell
{
	["name"] =      "Doom Shield",
	["school"] =    {SCHOOL_DEMON},
	["level"] =     1,
	["mana"] =      2,
	["mana_max"] =  30,
	["fail"] =      10,
	["random"] =    0,
	["spell"] =     function() return demonology_doom_shield() end,
	["info"] =      function() return demonology_doom_shield_info() end,
	["desc"] =      {
			"Raises a mirror of pain around you, doing very high damage to your foes",
			"that dare hit you, but greatly reduces your armour class",
	}
}

UNHOLY_WORD = add_spell
{
	["name"] =      "Unholy Word",
	["school"] =    {SCHOOL_DEMON},
	["level"] =     25,
	["mana"] =      15,
	["mana_max"] =  45,
	["fail"] =      55,
	["random"] =    0,
	["spell"] =     function() return demonology_unholy_word() end,
	["info"] =      function() return demonology_unholy_word_info() end,
	["desc"] =      {
			"Kills a pet to heal you",
			"There is a chance that the pet won't die but will turn against you",
			"it will decrease with higher level",
	}
}

DEMON_CLOAK = add_spell
{
	["name"] =      "Demon Cloak",
	["school"] =    {SCHOOL_DEMON},
	["level"] =     20,
	["mana"] =      10,
	["mana_max"] =  40,
	["fail"] =      70,
	["random"] =    0,
	["spell"] =     function() return demonology_demon_cloak() end,
	["info"] =      function() return demonology_demon_cloak_info() end,
	["desc"] =      {
			"Raises a mirror that can reflect bolts and arrows for a time",
	}
}


-- Demonhorn
DEMON_SUMMON = add_spell
{
	["name"] =      "Summon Demon",
	["school"] =    {SCHOOL_DEMON},
	["level"] =     5,
	["mana"] =      10,
	["mana_max"] =  50,
	["fail"] =      30,
	["random"] =    0,
	["spell"] =     function() return demonology_summon_demon() end,
	["info"] =      function() return demonology_summon_demon_info() end,
	["desc"] =      {
			"Summons a leveled demon to your side",
			"At level 35 it summons a high demon",
	}
}

DISCHARGE_MINION = add_spell
{
	["name"] =      "Discharge Minion",
	["school"] =    {SCHOOL_DEMON},
	["level"] =     10,
	["mana"] =      20,
	["mana_max"] =  50,
	["fail"] =      30,
	["random"] =    0,
	["spell"] =     function() return demonology_discharge_minion() end,
	["info"] =      function() return demonology_discharge_minion_info() end,
	["desc"] =      {
			"The targeted pet will explode in a burst of gravity",
	}
}

CONTROL_DEMON = add_spell
{
	["name"] =      "Control Demon",
	["school"] =    {SCHOOL_DEMON},
	["level"] =     25,
	["mana"] =      30,
	["mana_max"] =  70,
	["fail"] =      55,
	["random"] =    0,
	["spell"] =     function() return demonology_control_demon() end,
	["info"] =      function() return demonology_control_demon_info() end,
	["desc"] =      {
			"Attempts to control a demon",
	}
}
