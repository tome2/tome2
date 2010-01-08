-- Handles thhe divination school


STARIDENTIFY = add_spell
{
	["name"] = 	"Greater Identify",
	["school"] = 	{SCHOOL_DIVINATION},
	["level"] = 	35,
	["mana"] = 	30,
	["mana_max"] = 	30,
	["fail"] = 	80,
	["spell"] = 	function()
			if get_check("Cast on yourself?") == TRUE then
				self_knowledge()
			else
				identify_fully()
			end
			return TRUE
	end,
	["info"] = 	function()
		       	return ""
	end,
	["desc"] =	{
			"Asks for an object and fully identify it, providing the full list of powers",
			"Cast at yourself it will reveal your powers"
	}
}

IDENTIFY = add_spell
{
	["name"] = 	"Identify",
	["school"] = 	{SCHOOL_DIVINATION},
	["level"] = 	8,
	["mana"] = 	10,
	["mana_max"] = 	50,
	["fail"] = 	40,
	["stick"] =
	{
			["charge"] =    { 7, 10 },
			[TV_STAFF] =
			{
				["rarity"] = 		45,
				["base_level"] =	{ 1, 15 },
				["max_level"] =		{ 15, 40 },
			},
	},
	["spell"] = 	function()
			if get_level(IDENTIFY, 50) >= 27 then
				local obvious
				obvious = identify_pack()
				obvious = is_obvious(fire_ball(GF_IDENTIFY, 0, 1, get_level(IDENTIFY, 3)), obvious)
				if obvious == TRUE then
					player.notice = bor(player.notice, PN_COMBINE, PN_REORDER)
				end
				return obvious
			elseif get_level(IDENTIFY, 50) >= 17 then
				local obvious
				obvious = identify_pack()
				obvious = is_obvious(fire_ball(GF_IDENTIFY, 0, 1, 0), obvious)
				if obvious == TRUE then
					player.notice = bor(player.notice, PN_COMBINE, PN_REORDER)
				end
				return obvious
			else
				if ident_spell() == TRUE then return TRUE else return end
			end
	end,
	["info"] = 	function()
			if get_level(IDENTIFY, 50) >= 27 then
				return "rad "..(get_level(IDENTIFY, 3))
			else
				return ""
			end
	end,
	["desc"] =	{
			"Asks for an object and identifies it",
			"At level 17 it identifies all objects in the inventory",
			"At level 27 it identifies all objects in the inventory and in a",
			"radius on the floor, as well as probing monsters in that radius"
	}
}

VISION = add_spell
{
	["name"] = 	"Vision",
	["school"] = 	{SCHOOL_DIVINATION},
	["level"] = 	15,
	["mana"] = 	7,
	["mana_max"] = 	55,
	["fail"] = 	45,
	["stick"] =
	{
			["charge"] =    { 4, 6 },
			[TV_STAFF] =
			{
				["rarity"] = 		60,
				["base_level"] =	{ 1, 5 },
				["max_level"] =		{ 10, 30 },
			},
	},
	["inertia"] = 	{ 2, 200 },
	["spell"] = 	function()
			if get_level(VISION, 50) >= 25 then
				wiz_lite_extra()
			else
				map_area()
			end
			return TRUE
	end,
	["info"] = 	function()
			return ""
	end,
	["desc"] =	{
			"Detects the layout of the surrounding area",
			"At level 25 it maps and lights the whole level",
	}
}

SENSEHIDDEN = add_spell
{
	["name"] = 	"Sense Hidden",
	["school"] = 	{SCHOOL_DIVINATION},
	["level"] = 	5,
	["mana"] = 	2,
	["mana_max"] = 	10,
	["fail"] = 	25,
	["stick"] =
	{
			["charge"] =    { 1, 15 },
			[TV_STAFF] =
			{
				["rarity"] = 		20,
				["base_level"] =	{ 1, 15 },
				["max_level"] =		{ 10, 50 },
			},
	},
	["inertia"] = 	{ 1, 10 },
	["spell"] = 	function()
			local obvious = nil
			obvious = detect_traps(15 + get_level(SENSEHIDDEN, 40, 0))
			if get_level(SENSEHIDDEN, 50) >= 15 then
				obvious = is_obvious(set_tim_invis(10 + randint(20) + get_level(SENSEHIDDEN, 40)), obvious)
			end
			return obvious
	end,
	["info"] = 	function()
			if get_level(SENSEHIDDEN, 50) >= 15 then
				return "rad "..(15 + get_level(SENSEHIDDEN, 40)).." dur "..(10 + get_level(SENSEHIDDEN, 40)).."+d20"
			else
				return "rad "..(15 + get_level(SENSEHIDDEN, 40))
			end
	end,
	["desc"] =	{
			"Detects the traps in a certain radius around you",
			"At level 15 it allows you to sense invisible for a while"
	}
}

REVEALWAYS = add_spell
{
	["name"] = 	"Reveal Ways",
	["school"] = 	{SCHOOL_DIVINATION},
	["level"] = 	9,
	["mana"] = 	3,
	["mana_max"] = 	15,
	["fail"] = 	20,
	["stick"] =
	{
			["charge"] =    { 6, 6 },
			[TV_STAFF] =
			{
				["rarity"] = 		35,
				["base_level"] =	{ 1, 15 },
				["max_level"] =		{ 25, 50 },
			},
	},
	["inertia"] = 	{ 1, 10 },
	["spell"] = 	function()
			local obvious
			obvious = detect_doors(10 + get_level(REVEALWAYS, 40, 0))
			obvious = is_obvious(detect_stairs(10 + get_level(REVEALWAYS, 40, 0)), obvious)
			return obvious
	end,
	["info"] = 	function()
			return "rad "..(10 + get_level(REVEALWAYS, 40))
	end,
	["desc"] =	{
			"Detects the doors/stairs/ways in a certain radius around you",
	}
}

SENSEMONSTERS = add_spell
{
	["name"] = 	"Sense Monsters",
	["school"] = 	{SCHOOL_DIVINATION},
	["level"] = 	1,
	["mana"] =      1,
	["mana_max"] =  20,
	["fail"] = 	10,
	["stick"] =
	{
			["charge"] =    { 5, 10 },
			[TV_STAFF] =
			{
				["rarity"] = 		37,
				["base_level"] =	{ 1, 10 },
				["max_level"] =		{ 15, 40 },
			},
	},
	["inertia"] = 	{ 1, 10 },
	["spell"] = 	function()
			local obvious
			obvious = detect_monsters_normal(10 + get_level(SENSEMONSTERS, 40, 0))
			if get_level(SENSEMONSTERS, 50) >= 30 then
				obvious = is_obvious(set_tim_esp(10 + randint(10) + get_level(SENSEMONSTERS, 20)), obvious)
			end
			return obvious
	end,
	["info"] = 	function()
			if get_level(SENSEMONSTERS, 50) >= 30 then
				return "rad "..(10 + get_level(SENSEMONSTERS, 40)).." dur "..(10 + get_level(SENSEMONSTERS, 20)).."+d10"
			else
				return "rad "..(10 + get_level(SENSEMONSTERS, 40))
			end
	end,
	["desc"] =	{
			"Detects all monsters near you",
			"At level 30 it allows you to sense monster minds for a while"
	}
}
