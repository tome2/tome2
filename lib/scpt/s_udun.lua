-- handle the udun school

DRAIN = add_spell
{
	["name"] = 	"Drain",
	["school"] = 	{SCHOOL_UDUN, SCHOOL_MANA},
	["level"] = 	1,
	["mana"] = 	0,
	["mana_max"] = 	0,
	["fail"] = 	20,
	["spell"] = 	function() return udun_drain() end,
	["info"] = 	function() return udun_drain_info() end,
	["desc"] =	{
			"Drains the mana contained in wands, staves and rods to increase yours",
	}
}

GENOCIDE = add_spell
{
	["name"] = 	"Genocide",
	["school"] = 	{SCHOOL_UDUN, SCHOOL_NATURE},
	["level"] = 	25,
	["mana"] = 	50,
	["mana_max"] = 	50,
	["fail"] = 	90,
	["stick"] =
	{
			["charge"] =    { 2, 2 },
			[TV_STAFF] =
			{
				["rarity"] = 		85,
				["base_level"] =	{ 1, 1 },
				["max_level"] =		{ 5, 15 },
			},
	},
	["spell"] = 	function() return udun_genocide() end,
	["info"] = 	function() return udun_genocide_info() end,
	["desc"] =	{
			"Genocides all monsters of a race on the level",
			"At level 10 it can genocide all monsters near you"
	}
}

WRAITHFORM = add_spell
{
	["name"] = 	"Wraithform",
	["school"] = 	{SCHOOL_UDUN, SCHOOL_CONVEYANCE},
	["level"] = 	30,
	["mana"] = 	20,
	["mana_max"] = 	40,
	["fail"] = 	95,
	["inertia"] = 	{ 4, 30 },
	["spell"] = 	function() return udun_wraithform() end,
	["info"] = 	function() return udun_wraithform_info() end,
	["desc"] =	{
			"Turns you into an immaterial being",
	}
}

FLAMEOFUDUN = add_spell
{
	["name"] = 	"Flame of Udun",
	["school"] = 	{SCHOOL_UDUN, SCHOOL_FIRE},
	["level"] = 	35,
	["mana"] = 	70,
	["mana_max"] = 	100,
	["fail"] = 	95,
	["inertia"] = 	{ 7, 15 },
	["spell"] = 	function() return udun_flame_of_udun() end,
	["info"] = 	function() return udun_flame_of_udun_info() end,
	["desc"] =	{
			"Turns you into a powerful Balrog",
	}
}


-- Return the number of Udun/Melkor spells in a given book
function udun_in_book(sval, pval)
	local i, y, index, sch, s

	i = 0

	-- Hack if the book is 255 it is a random book
	if sval == 255 then
		school_book[sval] = {pval}
	end
	-- Parse all spells
	for index, s in school_book[sval] do
		for index, sch in __spell_school[s] do
			if sch == SCHOOL_UDUN then i = i + 1 end
			if sch == SCHOOL_MELKOR then i = i + 1 end
		end
	end
	return i
end

-- Return the total level of spells
function levels_in_book(sval, pval)
	local i, y, index, sch, s

	i = 0

	-- Hack if the book is 255 it is a random book
	if sval == 255 then
		school_book[sval] = {pval}
	end

	-- Parse all spells
	for index, s in school_book[sval] do
		i = i + __tmp_spells[s].level
	end
	return i
end
