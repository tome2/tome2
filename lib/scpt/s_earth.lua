-- The earth school

STONESKIN = add_spell
{
	["name"] = 	"Stone Skin",
	["school"] = 	SCHOOL_EARTH,
	["level"] = 	1,
	["mana"] = 	1,
	["mana_max"] = 	50,
	["fail"] = 	10,
	["inertia"] = 	{ 2, 50 },
	["spell"] = 	function()
			local type
			if get_level(STONESKIN, 50) >= 25 then
				type = SHIELD_COUNTER
			else
				type = 0
			end
			return set_shield(randint(10) + 10 + get_level(STONESKIN, 100), 10 + get_level(STONESKIN, 50), type, 2 + get_level(STONESKIN, 5), 3 + get_level(STONESKIN, 5))
	end,
	["info"] = 	function()
			if get_level(STONESKIN, 50) >= 25 then
				return "dam "..(2 + get_level(STONESKIN, 5)).."d"..(3 + get_level(STONESKIN, 5)).." dur "..(10 + get_level(STONESKIN, 100)).."+d10 AC "..(10 + get_level(STONESKIN, 50))
			else
				return "dur "..(10 + get_level(STONESKIN, 100)).."+d10 AC "..(10 + get_level(STONESKIN, 50))
			end
	end,
	["desc"] =	{
			"Creates a shield of earth around you to protect you",
			"At level 25 it starts dealing damage to attackers"
		}
}

DIG = add_spell
{
	["name"] = 	"Dig",
	["school"] = 	SCHOOL_EARTH,
	["level"] = 	12,
	["mana"] = 	14,
	["mana_max"] = 	14,
	["fail"] = 	20,
	["stick"] =
	{
			["charge"] =    { 15, 5 },
			[TV_WAND] =
			{
				["rarity"] = 		25,
				["base_level"] =	{ 1, 1 },
				["max_level"] =		{ 1, 1 },
			},
	},
	["spell"] = 	function()
			local ret, dir
			ret, dir = get_aim_dir()
			if ret == FALSE then return end
			return wall_to_mud(dir)
	end,
	["info"] = 	function()
			return ""
	end,
	["desc"] =	{
			"Digs a hole in a wall much faster than any shovels",
		}
}

STONEPRISON = add_spell
{
	["name"] = 	"Stone Prison",
	["school"] = 	SCHOOL_EARTH,
	["level"] = 	25,
	["mana"] = 	30,
	["mana_max"] = 	50,
	["fail"] = 	65,
	["stick"] =
	{
			["charge"] =    { 5, 3 },
			[TV_WAND] =
			{
				["rarity"] = 		57,
				["base_level"] =	{ 1, 3 },
				["max_level"] =		{ 5, 20 },
			},
	},
	["spell"] = 	function()
			local ret, x, y
			if get_level(STONEPRISON, 50) >= 10 then
				ret, x, y = tgt_pt()
			else
				y = player.py
				x = player.px
			end
			wall_stone(y, x)
			return TRUE
	end,
	["info"] = 	function()
			return ""
	end,
	["desc"] =	{
			"Creates a prison of walls around you",
			"At level 10 it allows you to target a monster"
		}
}

STRIKE = add_spell
{
	["name"] = 	"Strike",
	["school"] = 	{SCHOOL_EARTH},
	["level"] = 	30,
	["mana"] = 	30,
	["mana_max"] = 	50,
	["fail"] = 	60,
	["stick"] =
	{
			["charge"] =    { 2, 6 },
			[TV_WAND] =
			{
				["rarity"] = 		635,
				["base_level"] =	{ 1, 5 },
				["max_level"] =		{ 10, 50 },
			},
	},
	["spell"] = 	function()
			local ret, dir, rad
			ret, dir = get_aim_dir()
			if ret == FALSE then return end
			if get_level(STRIKE, 50) >= 12 then
				return fire_ball(GF_FORCE, dir, 50 + get_level(STRIKE, 50), 1)
			else
				return fire_ball(GF_FORCE, dir, 50 + get_level(STRIKE, 50), 0)
			end
	end,
	["info"] = 	function()
			if get_level(STRIKE, 50) >= 12 then
	       			return "dam "..(50 + get_level(STRIKE, 50)).." rad 1"
			else
				return "dam "..(50 + get_level(STRIKE, 50))
			end
	end,
	["desc"] =	{
			"Creates a micro-ball of force that will push monsters backwards",
			"If the monster is caught near a wall, it'll be crushed against it",
			"At level 12 it turns into a ball of radius 1"
	}
}

SHAKE = add_spell
{
	["name"] = 	"Shake",
	["school"] = 	{SCHOOL_EARTH},
	["level"] = 	27,
	["mana"] = 	25,
	["mana_max"] = 	30,
	["fail"] = 	60,
	["stick"] =
	{
			["charge"] =    { 5, 10 },
			[TV_STAFF] =
			{
				["rarity"] = 		75,
				["base_level"] =	{ 1, 3 },
				["max_level"] =		{ 9, 20 },
			},
	},
	["inertia"] = 	{ 2, 50 },
	["spell"] = 	function()
			local ret, x, y
			if get_level(SHAKE, 50) >= 10 then
			       	ret, x, y = tgt_pt()
				if ret == FALSE then return end
			else
				x = player.px
				y = player.py
			end
			earthquake(y, x, 4 + get_level(SHAKE, 10));
			return TRUE
	end,
	["info"] = 	function()
			return "rad "..(4 + get_level(SHAKE, 10))
	end,
	["desc"] =	{
			"Creates a localised earthquake",
			"At level 10 it can be targeted at any location"
	}
}
