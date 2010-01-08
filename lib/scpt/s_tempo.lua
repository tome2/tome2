-- Handles thhe temporal school


MAGELOCK = add_spell
{
	["name"] = 	"Magelock",
	["school"] = 	{SCHOOL_TEMPORAL},
	["level"] = 	1,
	["mana"] = 	1,
	["mana_max"] = 	35,
	["fail"] = 	10,
	["stick"] =
	{
			["charge"] =    { 7, 5 },
			[TV_WAND] =
			{
				["rarity"] = 		30,
				["base_level"] =	{ 1, 5 },
				["max_level"] =		{ 15, 45 },
			},
	},
	["spell"] = 	function()
			if get_level(MAGELOCK, 50) >= 30 then
				local ret, x, y, c_ptr

				if get_level(MAGELOCK, 50) >= 40 then
					ret, x, y = tgt_pt()
					if ret == FALSE then return end
					if cave_is(cave(y, x), FF1_FLOOR) == FALSE or cave_is(cave(y, x), FF1_PERMANENT) == TRUE or los(player.py, player.px, y, x) == FALSE then
						msg_print("You cannot place it there.")
						return TRUE
					end
				else
					y = player.py
					x = player.px
				end
				cave_set_feat(y, x, 3)
				return TRUE
			else
				ret, dir = get_aim_dir()
				if ret == FALSE then return end
				return wizard_lock(dir)
			end
	end,
	["info"] = 	function()
		       	return ""
	end,
	["desc"] =	{
			"Magically locks a door",
			"At level 30 it creates a glyph of warding",
			"At level 40 the glyph can be placed anywhere in the field of vision"
	}
}

SLOWMONSTER = add_spell
{
	["name"] = 	"Slow Monster",
	["school"] = 	{SCHOOL_TEMPORAL},
	["level"] = 	10,
	["mana"] = 	10,
	["mana_max"] = 	15,
	["fail"] = 	35,
	["stick"] =
	{
			["charge"] =    { 5, 5 },
			[TV_WAND] =
			{
				["rarity"] = 		23,
				["base_level"] =	{ 1, 15 },
				["max_level"] =		{ 20, 50 },
			},
	},
	["spell"] = 	function()
	       		local ret, dir

			ret, dir = get_aim_dir()
			if ret == FALSE then return end
			if get_level(SLOWMONSTER, 50) >= 20 then
				return fire_ball(GF_OLD_SLOW, dir, 40 + get_level(SLOWMONSTER, 160), 1)
			else
				return fire_bolt(GF_OLD_SLOW, dir, 40 + get_level(SLOWMONSTER, 160))
			end
	end,
	["info"] = 	function()
			if get_level(SLOWMONSTER, 50) >= 20 then
			       	return "power "..(40 + get_level(SLOWMONSTER, 160)).." rad 1"
			else
			       	return "power "..(40 + get_level(SLOWMONSTER, 160))
			end
	end,
	["desc"] =	{
			"Magically slows down the passing of time around a monster",
			"At level 20 it affects a zone"
	}
}

ESSENCESPEED = add_spell
{
	["name"] = 	"Essence of Speed",
	["school"] = 	{SCHOOL_TEMPORAL},
	["level"] = 	15,
	["mana"] = 	20,
	["mana_max"] = 	40,
	["fail"] = 	50,
	["stick"] =
	{
			["charge"] =    { 3, 3 },
			[TV_WAND] =
			{
				["rarity"] = 		80,
				["base_level"] =	{ 1, 1 },
				["max_level"] =		{ 10, 39 },
			},
	},
	["inertia"] = 	{ 5, 20 },
	["spell"] = 	function()
			if player.fast == 0 then return set_fast(10 + randint(10) + get_level(ESSENCESPEED, 50), 5 + get_level(ESSENCESPEED, 20)) end
	end,
	["info"] = 	function()
		       	return "dur "..(10 + get_level(ESSENCESPEED, 50)).."+d10 speed "..(5 + get_level(ESSENCESPEED, 20))
	end,
	["desc"] =	{
			"Magically decreases the passing of time around you, making you move faster with",
			"respect to the rest of the universe."
	}
}

BANISHMENT = add_spell
{
	["name"] = 	"Banishment",
	["school"] = 	{SCHOOL_TEMPORAL, SCHOOL_CONVEYANCE},
	["level"] = 	30,
	["mana"] = 	30,
	["mana_max"] = 	40,
	["fail"] = 	95,
	["stick"] =
	{
			["charge"] =    { 1, 3 },
			[TV_WAND] =
			{
				["rarity"] = 		98,
				["base_level"] =	{ 1, 15 },
				["max_level"] =		{ 10, 36 },
			},
	},
	["inertia"] = 	{ 5, 50 },
	["spell"] = 	function()
			local obvious
			obvious = project_los(GF_AWAY_ALL, 40 + get_level(BANISHMENT, 160))
			if get_level(BANISHMENT, 50) >= 15 then
				obvious = is_obvious(project_los(GF_STASIS, 20 + get_level(BANISHMENT, 120)), obvious)
			end
			return obvious
	end,
	["info"] = 	function()
		     	return "power "..(40 + get_level(BANISHMENT, 160))
	end,
	["desc"] =	{
			"Disrupts the space/time continuum in your area and teleports all monsters away.",
			"At level 15 it may also lock them in a time bubble for a while."
	}
}
