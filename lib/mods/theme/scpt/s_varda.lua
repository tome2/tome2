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
	["spell"] =	function() return varda_light_of_valinor_spell() end,
	["info"] =	function() return varda_light_of_valinor_info() end,
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
	["spell"] =	function() return varda_call_of_almaren_spell() end,
	["info"] = 	function() return varda_call_of_almaren_info() end,
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
	["spell"] =	function() return varda_evenstar_spell() end,
	["info"] = 	function() return varda_evenstar_info() end,
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
	["spell"] =	function() return varda_star_kindler_spell() end,
	["info"] = 	function() return varda_star_kindler_info() end,
	["desc"] =	{ 
		"Does multiple bursts of light damage.", 
		"The damage increases with level.",
	} 
}
