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
	["spell"] =     function()
		local obvious
		if get_level(GLOBELIGHT, 50) >= 3 then
			obvious = lite_area(10, 4)
		else
			lite_room(player.py, player.px)
			obvious = TRUE
		end
		if get_level(GLOBELIGHT, 50) >= 15 then
			obvious = is_obvious(fire_ball(GF_LITE, 0, 10 + get_level(GLOBELIGHT, 100), 5 + get_level(GLOBELIGHT, 6)), obvious)
			player.update = bor(player.update, PU_VIEW)
		end
		return obvious
	end,
	["info"] =      function()
		if get_level(GLOBELIGHT, 50) >= 15 then
			return "dam "..(10 + get_level(GLOBELIGHT, 100)).." rad "..(5 + get_level(GLOBELIGHT, 6))
		else
			return ""
		end
	end,
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
	["spell"] =     function()
		local ret, dir, type
		if (get_level(FIREFLASH, 50) >= 20) then
			type = GF_HOLY_FIRE
		else
			type = GF_FIRE
		end
		ret, dir = get_aim_dir()
		if ret == FALSE then return end
		return fire_ball(type, dir, 20 + get_level(FIREFLASH, 500), 2 + get_level(FIREFLASH, 5))
	end,
	["info"] =      function()
		return "dam "..(20 + get_level(FIREFLASH, 500)).." rad "..(2 + get_level(FIREFLASH, 5))
	end,
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
	["spell"] =     function()
		local type
		if (get_level(FIERYAURA, 50) >= 8) then
			type = SHIELD_GREAT_FIRE
		else
			type = SHIELD_FIRE
		end
		return set_shield(randint(20) + 10 + get_level(FIERYAURA, 70), 10, type, 5 + get_level(FIERYAURA, 10), 5 + get_level(FIERYAURA, 7))
	end,
	["info"] =      function()
		return "dam "..(5 + get_level(FIERYAURA, 15)).."d"..(5 + get_level(FIERYAURA, 7)).." dur "..(10 + get_level(FIERYAURA, 70)).."+d20"
	end,
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
	["spell"] =     function()
		local ret, dir, type
		if (get_level(FIREWALL, 50) >= 6) then
			type = GF_HELL_FIRE
		else
			type = GF_FIRE
		end
		ret, dir = get_aim_dir()
		if ret == FALSE then return end
		fire_wall(type, dir, 40 + get_level(FIREWALL, 150), 10 + get_level(FIREWALL, 14))
		return TRUE
	end,
	["info"] =      function()
		return "dam "..(40 + get_level(FIREWALL, 150)).." dur "..(10 + get_level(FIREWALL, 14))
	end,
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
	["spell"] =     function()
			local m_idx, y, x, ret, item

			-- Can we reconnect ?
			if do_control_reconnect() == TRUE then
				msg_print("Control re-established.")
				return
			end

			ret, item = get_item("Which light source do you want to use to create the golem?",
					     "You have no light source for the golem",
					     bor(USE_INVEN, USE_EQUIP),
					     function (obj)
						if (obj.tval == TV_LITE) and ((obj.sval == SV_LITE_TORCH) or (obj.sval == SV_LITE_LANTERN)) then
							return TRUE
							end
							return FALSE
					     end
			)
			if ret == FALSE then return TRUE end
			inven_item_increase(item, -1)
			inven_item_describe(item)
			inven_item_optimize(item)

			-- Summon it
			m_allow_special[1043 + 1] = TRUE
			y, x = find_position(player.py, player.px)
			m_idx = place_monster_one(y, x, 1043, 0, FALSE, MSTATUS_FRIEND)
			m_allow_special[1043 + 1] = FALSE

			-- level it
			if m_idx ~= 0 then
				monster_set_level(m_idx, 7 + get_level(FIREGOLEM, 70))
				player.control = m_idx
				monster(m_idx).mflag = bor(monster(m_idx).mflag, MFLAG_CONTROL)
			end
			return TRUE
	end,
	["info"] =      function()
			return "golem level "..(7 + get_level(FIREGOLEM, 70))
	end,
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
