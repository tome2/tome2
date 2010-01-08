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
	["spell"] =     function()
			local ret, dir = get_aim_dir()
			if ret == FALSE then return end

			return fire_ball(GF_CONTROL_ANIMAL, dir, 10 + get_level(YAVANNA_CHARM_ANIMAL, 170), get_level(YAVANNA_CHARM_ANIMAL, 2))
	end,
	["info"] =      function()
			return "power "..(10 + get_level(YAVANNA_CHARM_ANIMAL, 170)).." rad "..(get_level(YAVANNA_CHARM_ANIMAL, 2))
	end,
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
	["spell"] =     function()
			grow_grass(get_level(YAVANNA_GROW_GRASS, 4))
			return TRUE
	end,
	["info"] =      function()
			return "rad "..(get_level(YAVANNA_GROW_GRASS, 4))
	end,
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
	["spell"] =     function()
			return set_roots(10 + get_level(YAVANNA_TREE_ROOTS, 30), 10 + get_level(YAVANNA_TREE_ROOTS, 60), 10 + get_level(YAVANNA_TREE_ROOTS, 20))
	end,
	["info"] =      function()
			return "dur "..(10 + get_level(YAVANNA_TREE_ROOTS, 30)).." AC "..(10 + get_level(YAVANNA_TREE_ROOTS, 60)).." dam "..(10 + get_level(YAVANNA_TREE_ROOTS, 20))
	end,
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
	["spell"] =     function()
			local rad

			rad = 0
			if get_level(YAVANNA_WATER_BITE) >= 25 then rad = 1 end

			return set_project(randint(30) + 30 + get_level(YAVANNA_WATER_BITE, 150),
				    GF_WATER,
				    10 + get_level(YAVANNA_WATER_BITE),
				    rad,
				    bor(PROJECT_STOP, PROJECT_KILL))
	end,
	["info"] =      function()
			return "dur "..(30 + get_level(YAVANNA_WATER_BITE, 150)).."+d30 dam "..(10 + get_level(YAVANNA_WATER_BITE)).."/blow"
	end,
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
	["spell"] =     function()
			local m_idx, x, y, c_ptr, ret, dir

			ret, dir = get_rep_dir()
			if ret == FALSE then return end
			y, x = explode_dir(dir)
			y, x = y + player.py, x + player.px
			c_ptr = cave(y, x)

			if c_ptr.feat == FEAT_TREES then
				cave_set_feat(y, x, FEAT_GRASS);

				-- Summon it
				y, x = find_position(y, x)
				m_idx = place_monster_one(y, x, test_monster_name("Ent"), 0, FALSE, MSTATUS_FRIEND)

				-- level it
				if m_idx ~= 0 then
					monster_set_level(m_idx, 30 + get_level(YAVANNA_UPROOT, 70))
				end

				msg_print("The tree awakes!");
			else
				msg_print("There is no tree there.")
			end
			return TRUE
	end,
	["info"] =      function()
			return "lev "..(30 + get_level(YAVANNA_UPROOT, 70))
	end,
	["desc"] =      {
			"Awakes a tree to help you battle the forces of Morgoth",
	}
}
