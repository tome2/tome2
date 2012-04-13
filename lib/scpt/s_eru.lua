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
	["spell"] = 	function() return eru_see_the_music() end,
	["info"] = 	function() return eru_see_the_music_info() end,
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
	["spell"] = 	function() return eru_listen_to_the_music() end,
	["info"] = 	function() return eru_listen_to_the_music_info() end,
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
	["spell"] = 	function() return eru_know_the_music() end,
	["info"] = 	function() return eru_know_the_music_info() end,
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
	["spell"] = 	function() return eru_lay_of_protection() end,
	["info"] = 	function() return eru_lay_of_protection_info() end,
	["desc"] =	{
			"Creates a circle of safety around you",
	}
}
