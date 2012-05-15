-- Spells for Ulmo's school

BOOK_ULMO = 65

-- "Song of Belegaer" copied from Geyser
ULMO_BELEGAER = add_spell
{
	["name"] = "Song of Belegaer",
	["school"] = SCHOOL_ULMO,
	["level"] = 1,
	["mana"] = 1,
	["mana_max"] = 100,
	["fail"] = 25,
	["piety"] = TRUE,
	["stat"] =      A_WIS,
	["random"] =    SKILL_SPIRITUALITY,
	["spell"] = function() return ulmo_song_of_belegaer_spell() end,
	["info"] = function() return ulmo_song_of_belegaer_info() end,
	["desc"] =
	{
		"Channels the power of the Great Sea into your fingertips.",
		"Sometimes it can blast through its first target."
	},
}

-- "Draught of Ulmonan" copied with tweaks from T-Plus Nature spell "Restore Body"
ULMO_DRAUGHT_ULMONAN = add_spell
{
	["name"] = 	"Draught of Ulmonan",
	["school"] = 	{SCHOOL_ULMO},
	["level"] = 	15,
	["mana"] = 	25,
	["mana_max"] = 	200,
	["fail"] = 	50,
	["piety"] = TRUE,
	["stat"] =      A_WIS,
	["random"] =    SKILL_SPIRITUALITY,
	["spell"] = 	function() return ulmo_draught_of_ulmonan_spell() end,
	["info"] = 	function() return ulmo_draught_of_ulmonan_info() end,
	["desc"] =	{
			"Fills you with a draught with powerful curing effects,",
			"prepared by Ulmo himself.",
			"Level 1: blindness, poison, cuts and stunning",
			"Level 10: drained STR, DEX and CON",
			"Level 20: parasites and mimicry",
	},
}

-- "Call of the Ulumuri" based on Call Blessed Soul from T-Plus
ULMO_CALL_ULUMURI = add_spell

{
	["name"] = 	"Call of the Ulumuri",
	["school"] = 	{SCHOOL_ULMO},
	["level"] = 	20,
	["mana"] = 	50,
	["mana_max"] = 	300,
	["fail"] = 	75,
	["piety"] = TRUE,
	["stat"] =      A_WIS,
	["random"] =    SKILL_SPIRITUALITY,
	["spell"] =     function() return ulmo_call_of_the_ulumuri_spell() end,
	["info"] = 	function() return ulmo_call_of_the_ulumuri_info() end,
	["desc"] =	{
			"Summons a leveled water spirit or elemental",
			"to fight for you",

	},
}

-- "Wrath of Ulmo" based on Firewall
ULMO_WRATH = add_spell
{
	["name"] = 	"Wrath of Ulmo",
	["school"] = 	{SCHOOL_ULMO},
	["level"] = 	30,
	["mana"] = 	100,
	["mana_max"] = 	400,
	["fail"] = 	95,
	["piety"] = TRUE,
	["stat"] =      A_WIS,
	["random"] =    SKILL_SPIRITUALITY,
	["spell"] =     function() return ulmo_wrath_of_ulmo_spell() end,
	["info"] =      function() return ulmo_wrath_of_ulmo_info() end,
	["desc"] =      {
			"Conjures up a sea storm.",
			"At level 30 it turns into a more forceful storm."
	}
}