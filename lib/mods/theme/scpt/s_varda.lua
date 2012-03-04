-- Spells for Varda school (From Annals of Ea module)

BOOK_VARDA = 64

-- Holy light spell copied from Globe of Light
VARDA_LIGHT_VALINOR = add_spell 
{ 
	["name"] =	"Light of Valinor", 
	["school"] =	{SCHOOL_VARDA}, 
	["level"] =	1, 
	["mana"] =	1, 
	["mana_max"] =	100, 
	["fail"] =	20, 
	["piety"] =	TRUE, 
	["stat"] =	A_WIS, 
	["random"] =	SKILL_SPIRITUALITY, 
	["spell"] =	function() 
		local obvious
		if get_level(VARDA_LIGHT_VALINOR, 50) >= 3 then
			obvious = lite_area(10, 4)
		else
			lite_room(player.py, player.px)
			obvious = TRUE
		end
		if get_level(VARDA_LIGHT_VALINOR, 50) >= 15 then
			obvious = is_obvious(fire_ball(GF_LITE, 0, 10 + get_level(VARDA_LIGHT_VALINOR, 100), 5 + get_level(GLOBELIGHT, 6)), obvious)
		end
		return obvious
	end, 
	["info"] =	function() 
		if get_level(VARDA_LIGHT_VALINOR, 50) >= 15 then
			return "dam "..(10 + get_level(VARDA_LIGHT_VALINOR, 100)).." rad "..(5 + get_level(VARDA_LIGHT_VALINOR, 6))
		else
			return ""
		end
	end, 
	["desc"] =	{ 
		"Lights a room", 
		"At level 3 it starts damaging monsters",
		"At level 15 it starts creating a more powerful kind of light",
	} 
} 

VARDA_CALL_ALMAREN = add_spell 
{ 
	["name"] =	"Call of Almaren", 
	["school"] =	{SCHOOL_VARDA}, 
	["level"] =	10, 
	["mana"] =	5, 
	["mana_max"] =	150, 
	["fail"] =	20, 
	["piety"] =	TRUE, 
	["stat"] =	A_WIS, 
	["random"] =	SKILL_SPIRITUALITY, 
	["spell"] =	function() 
			local power = 5 * player.lev
			if (get_level(VARDA_CALL_ALMAREN) >= 20) then
				dispel_evil(power)
			else
				banish_evil(power)
			end
			return FALSE
	end, 
	["info"] = 	function()
			return ""
	end,
	["desc"] =	{ 
		"Banishes evil beings", 
		"At level 20 it dispels evil beings",
	} 
} 

VARDA_EVENSTAR = add_spell 
{ 
	["name"] =	"Evenstar", 
	["school"] =	{SCHOOL_VARDA}, 
	["level"] =	20, 
	["mana"] =	20, 
	["mana_max"] =	200, 
	["fail"] =	20, 
	["piety"] =	TRUE, 
	["stat"] =	A_WIS, 
	["random"] =	SKILL_SPIRITUALITY, 
	["spell"] =	function() 
			if (get_level(VARDA_EVENSTAR) >= 40) then
				-- Enlightenment
				wiz_lite_extra()
				-- Identify
				identify_pack()
				-- Self knowledge
				self_knowledge()
			else
				wiz_lite_extra()
			end
			return FALSE
	end,
	["info"] = 	function()
			return ""
	end,
	["desc"] =	{ 
		"Maps and lights the whole level.", 
		"At level 40 it maps and lights the whole level,",
		"in addition to letting you know yourself better",
		"and identifying your whole pack.",
	} 
} 

VARDA_STARKINDLER = add_spell 
{ 
	["name"] =	"Star Kindler", 
	["school"] =	{SCHOOL_VARDA}, 
	["level"] =	30, 
	["mana"] =	50, 
	["mana_max"] =	250, 
	["fail"] =	20, 
	["piety"] =	TRUE, 
	["stat"] =	A_WIS, 
	["random"] =	SKILL_SPIRITUALITY, 
	["spell"] =	function() 
			local power = player.lev / 5
			local ret, dir

			ret, dir = get_aim_dir()
			
			if ret == FALSE then return end
			for i = 1, power do
				fire_ball(GF_LITE, dir, 20 + get_level(VARDA_STARKINDLER, 100), 10)
			end
			
			return FALSE
	end,
	["info"] = 	function()
			local power = player.lev / 5
			return "dam "..(20 + get_level(VARDA_STARKINDLER, 100)).." rad 10"
	end,
	["desc"] =	{ 
		"Does multiple bursts of light damage.", 
		"The damage increases with level.",
	} 
} 