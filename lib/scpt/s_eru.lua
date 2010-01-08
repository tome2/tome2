-- Handle Eru Iluvatar magic school

ERU_SEE = add_spell
{
	["name"] = 	"See the Music",
	["school"] = 	{SCHOOL_ERU},
	["level"] = 	1,
	["mana"] = 	1,
	["mana_max"] = 	50,
	["fail"] = 	20,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	-- Unnafected by blindness
	["blind"] =     FALSE,
	["random"] = 	SKILL_SPIRITUALITY,
	["spell"] = 	function()
			local obvious
			obvious = set_tim_invis(randint(20) + 10 + get_level(ERU_SEE, 100))
			if get_level(ERU_SEE) >= 30 then
				wiz_lite_extra()
				obvious = TRUE
			elseif get_level(ERU_SEE) >= 10 then
				map_area()
				obvious = TRUE
			end
			if get_level(ERU_SEE) >= 20 then
				obvious = is_obvious(set_blind(0), obvious)
			end
			return obvious
	end,
	["info"] = 	function()
			return "dur "..(10 + get_level(ERU_SEE, 100)).."+d20"
	end,
	["desc"] =	{
			"Allows you to 'see' the Great Music from which the world",
			"originates, allowing you to see unseen things",
			"At level 10 it allows you to see your surroundings",
			"At level 20 it allows you to cure blindness",
			"At level 30 it allows you to fully see all the level"
	}
}

ERU_LISTEN = add_spell
{
	["name"] = 	"Listen to the Music",
	["school"] = 	{SCHOOL_ERU},
	["level"] = 	7,
	["mana"] = 	15,
	["mana_max"] = 	200,
	["fail"] = 	25,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] = 	SKILL_SPIRITUALITY,
	["spell"] = 	function()
			if get_level(ERU_LISTEN) >= 30 then
				ident_all()
				identify_pack()
				return TRUE
			elseif get_level(ERU_LISTEN) >= 14 then
				identify_pack()
				return TRUE
			else
				return ident_spell()
			end
	end,
	["info"] = 	function()
			return ""
	end,
	["desc"] =	{
			"Allows you to listen to the Great Music from which the world",
			"originates, allowing you to understand the meaning of things",
			"At level 14 it allows you to identify all your pack",
			"At level 30 it allows you to identify all items on the level",
	}
}

ERU_UNDERSTAND = add_spell
{
	["name"] = 	"Know the Music",
	["school"] = 	{SCHOOL_ERU},
	["level"] = 	30,
	["mana"] = 	200,
	["mana_max"] = 	600,
	["fail"] = 	50,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] = 	SKILL_SPIRITUALITY,
	["spell"] = 	function()
			if get_level(ERU_UNDERSTAND) >= 10 then
				identify_pack_fully()
				return TRUE
			else
				return identify_fully()
			end
	end,
	["info"] = 	function()
			return ""
	end,
	["desc"] =	{
			"Allows you to understand the Great Music from which the world",
			"originates, allowing you to know the full abilities of things",
			"At level 10 it allows you to *identify* all your pack",
	}
}

ERU_PROT = add_spell
{
	["name"] = 	"Lay of Protection",
	["school"] = 	{SCHOOL_ERU},
	["level"] = 	35,
	["mana"] = 	400,
	["mana_max"] = 	400,
	["fail"] = 	80,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] = 	SKILL_SPIRITUALITY,
	["spell"] = 	function()
			return fire_ball(GF_MAKE_GLYPH, 0, 1, 1 + get_level(ERU_PROT, 2, 0))
	end,
	["info"] = 	function()
			return "rad "..(1 + get_level(ERU_PROT, 2, 0))
	end,
	["desc"] =	{
			"Creates a circle of safety around you",
	}
}
