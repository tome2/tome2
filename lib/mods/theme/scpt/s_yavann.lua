-- Handle Yavanna kementari magic school

YAVANNA_CHARM_ANIMAL = add_spell
{
	["name"] =      "Charm Animal",
	["school"] =    {SCHOOL_YAVANNA},
	["level"] =     1,
	["mana"] =      10,
	["mana_max"] =  100,
	["fail"] =      30,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] =    SKILL_SPIRITUALITY,
	["spell"] =     function() return yavanna_charm_animal() end,
	["info"] =      function() return yavanna_charm_animal_info() end,
	["desc"] =      {
			"It tries to tame an animal",
	}
}

YAVANNA_GROW_GRASS = add_spell
{
	["name"] =      "Grow Grass",
	["school"] =    {SCHOOL_YAVANNA},
	["level"] =     10,
	["mana"] =      70,
	["mana_max"] =  150,
	["fail"] =      65,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] =    SKILL_SPIRITUALITY,
	["spell"] =     function() return yavanna_grow_grass() end,
	["info"] =      function() return yavanna_grow_grass_info() end,
	["desc"] =      {
			"Create a floor of grass around you. While on grass and praying",
			"a worshipper of Yavanna will know a greater regeneration rate"
	}
}

YAVANNA_TREE_ROOTS = add_spell
{
	["name"] =      "Tree Roots",
	["school"] =    {SCHOOL_YAVANNA},
	["level"] =     15,
	["mana"] =      50,
	["mana_max"] =  1000,
	["fail"] =      70,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] =    SKILL_SPIRITUALITY,
	["spell"] =     function() return yavanna_tree_roots() end,
	["info"] =      function() return yavanna_tree_roots_info() end,
	["desc"] =      {
			"Creates roots deep in the floor from your feet, making you more stable and able",
			"to make stronger attacks, but prevents any movement (even teleportation).",
			"It also makes you recover from stunning almost immediately."
	}
}

YAVANNA_WATER_BITE = add_spell
{
	["name"] =      "Water Bite",
	["school"] =    {SCHOOL_YAVANNA},
	["level"] =     20,
	["mana"] =      150,
	["mana_max"] =  300,
	["fail"] =      90,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] =    SKILL_SPIRITUALITY,
	["spell"] =     function() return yavanna_water_bite() end,
	["info"] =      function() return yavanna_water_bite_info() end,
	["desc"] =      {
			"Imbues your melee weapon with a natural stream of water",
			"At level 25, it spreads over a 1 radius zone around your target"
	}
}

YAVANNA_UPROOT = add_spell
{
	["name"] =      "Uproot",
	["school"] =    {SCHOOL_YAVANNA},
	["level"] =     35,
	["mana"] =      250,
	["mana_max"] =  350,
	["fail"] =      95,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] =    SKILL_SPIRITUALITY,
	["spell"] =     function() return yavanna_uproot() end,
	["info"] =      function() return yavanna_uproot_info() end,
	["desc"] =      {
			"Awakes a tree to help you battle the forces of Morgoth",
	}
}
