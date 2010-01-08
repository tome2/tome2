-- handle the conveyance school

BLINK = add_spell
{
	["name"] = 	"Phase Door",
	["school"] = 	{SCHOOL_CONVEYANCE},
	["level"] = 	1,
	["mana"] = 	1,
	["mana_max"] =  3,
	["fail"] = 	10,
	["inertia"] = 	{ 1, 5 },
	["spell"] = 	function()
			if get_level(BLINK, 50) >= 30 then
				local oy, ox = player.py, player.px

				teleport_player(10 + get_level(BLINK, 8))
				create_between_gate(0, oy, ox)
				return TRUE
			else
				teleport_player(10 + get_level(BLINK, 8))
				return TRUE
			end
	end,
	["info"] = 	function()
	       		return "distance "..(10 + get_level(BLINK, 8))
	end,
	["desc"] =	{
			"Teleports you on a small scale range",
			"At level 30 it creates void jumpgates",
	}
}

DISARM = add_spell
{
	["name"] = 	"Disarm",
	["school"] = 	{SCHOOL_CONVEYANCE},
	["level"] = 	3,
	["mana"] = 	2,
	["mana_max"] = 	4,
	["fail"] = 	15,
	["stick"] =
	{
			["charge"] =    { 10, 15 },
			[TV_STAFF] =
			{
				["rarity"] = 		4,
				["base_level"] =	{ 1, 10 },
				["max_level"] =		{ 10, 50 },
			},
	},
	["spell"] = 	function()
			local obvious
			obvious = destroy_doors_touch()
			if get_level(DISARM, 50) >= 10 then obvious = is_obvious(destroy_traps_touch(), obvious) end
			return obvious
	end,
	["info"] = 	function()
			return ""
	end,
	["desc"] =	{
			"Destroys doors and traps",
			"At level 10 it destroys doors and traps, then reveals and unlocks any secret",
			"doors"
	}
}

TELEPORT = add_spell
{
	["name"] = 	"Teleportation",
	["school"] = 	{SCHOOL_CONVEYANCE},
	["level"] = 	10,
	["mana"] = 	8,
	["mana_max"] = 	14,
	["fail"] = 	30,
	["stick"] =
	{
			["charge"] =    { 7, 7 },
			[TV_STAFF] =
			{
				["rarity"] = 		50,
				["base_level"] =	{ 1, 20 },
				["max_level"] =		{ 20, 50 },
			},
	},
	["inertia"] = 	{ 1, 10 },
	["spell"] = 	function()
			player.energy = player.energy - (25 - get_level(TELEPORT, 50))
			teleport_player(100 + get_level(TELEPORT, 100))
			return TRUE
	end,
	["info"] = 	function()
			return "distance "..(100 + get_level(TELEPORT, 100))
	end,
	["desc"] =	{
			"Teleports you around the level. The casting time decreases with level",
	}
}

TELEAWAY = add_spell
{
	["name"] = 	"Teleport Away",
	["school"] = 	{SCHOOL_CONVEYANCE},
	["level"] = 	23,
	["mana"] = 	15,
	["mana_max"] = 	40,
	["fail"] = 	60,
	["stick"] =
	{
			["charge"] =    { 3, 5 },
			[TV_WAND] =
			{
				["rarity"] = 		75,
				["base_level"] =	{ 1, 20 },
				["max_level"] =		{ 20, 50 },
			},
	},
	["spell"] = 	function()
	       		local ret, dir

			if get_level(TELEAWAY, 50) >= 20 then
				return project_los(GF_AWAY_ALL, 100)
			elseif get_level(TELEAWAY, 50) >= 10 then
				ret, dir = get_aim_dir()
				if ret == FALSE then return end
				return fire_ball(GF_AWAY_ALL, dir, 100, 3 + get_level(TELEAWAY, 4))
			else
				ret, dir = get_aim_dir()
				if ret == FALSE then return end
				return teleport_monster(dir)
			end
	end,
	["info"] = 	function()
			return ""
	end,
	["desc"] =	{
			"Teleports a line of monsters away",
			"At level 10 it turns into a ball",
			"At level 20 it teleports all monsters in sight"
	}
}

RECALL = add_spell
{
	["name"] = 	"Recall",
	["school"] = 	{SCHOOL_CONVEYANCE},
	["level"] = 	30,
	["mana"] = 	25,
	["mana_max"] = 	25,
	["fail"] =      60,
	["spell"] = 	function()
			local ret, x, y, c_ptr
			ret, x, y = tgt_pt()
			if ret == FALSE then return end
			c_ptr = cave(y, x)
			if (y == player.py) and (x == player.px) then
				local d = 21 - get_level(RECALL, 15)
				if d < 0 then
					d = 0
				end
				local f = 15 - get_level(RECALL, 10)
				if f < 1 then
					f = 1
				end
				recall_player(d, f)
				return TRUE
			elseif c_ptr.m_idx > 0 then
				swap_position(y, x)
				return TRUE
			elseif c_ptr.o_idx > 0 then
				set_target(y, x)
				if get_level(RECALL, 50) >= 15 then
					fetch(5, 10 + get_level(RECALL, 150), FALSE)
				else
					fetch(5, 10 + get_level(RECALL, 150), TRUE)
				end
				return TRUE
			end
	end,
	["info"] = 	function()
			local d = 21 - get_level(RECALL, 15)
			if d < 0 then
				d = 0
			end
			local f = 15 - get_level(RECALL, 10)
			if f < 1 then
				f = 1
			end
			return "dur "..f.."+d"..d.." weight "..(1 + get_level(RECALL, 15)).."lb"
	end,
	["desc"] =	{
			"Cast on yourself it will recall you to the surface/dungeon.",
			"Cast at a monster you will swap positions with the monster.",
			"Cast at an object it will fetch the object to you."
	}
}

PROBABILITY_TRAVEL = add_spell
{
	["name"] = 	"Probability Travel",
	["school"] = 	{SCHOOL_CONVEYANCE},
	["level"] = 	35,
	["mana"] = 	30,
	["mana_max"] = 	50,
	["fail"] = 	90,
	["stick"] =
	{
			["charge"] =    { 1, 2 },
			[TV_STAFF] =
			{
				["rarity"] = 		97,
				["base_level"] =	{ 1, 5 },
				["max_level"] =		{ 8, 25 },
			},
	},
	["inertia"] = 	{ 6, 40 },
	["spell"] = 	function()
			return set_prob_travel(randint(20) + get_level(PROBABILITY_TRAVEL, 60))
	end,
	["info"] = 	function()
			return "dur "..get_level(PROBABILITY_TRAVEL, 60).."+d20"
	end,
	["desc"] =	{
			"Renders you immaterial, when you hit a wall you travel through it and",
			"instantly appear on the other side of it. You can also float up and down",
			"at will"
	}
}
