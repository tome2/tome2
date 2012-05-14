-- Spells for the school of Mandos

BOOK_MANDOS = 66

-- "Tears of Luthien" based on Holy Word from T-Plus
MANDOS_TEARS_LUTHIEN = add_spell
{
	["name"] =      "Tears of Luthien",
	["school"] =    {SCHOOL_MANDOS},
	["level"] =     5,
	["mana"] =      10,
	["mana_max"] =  100,
	["fail"] = 	25,
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] = 	SKILL_SPIRITUALITY,
	["spell"] = 	function() return mandos_tears_of_luthien_spell() end,
	["info"] = 	function() return mandos_tears_of_luthien_info() end,
	["desc"] =	{
			"Calls upon the spirit of Luthien to ask Mandos for healing and succour."
			}
}

-- "Spirit of the Feanturi" based on Restore Mind from T-Plus
MANDOS_SPIRIT_FEANTURI = add_spell
{ 
    ["name"] =  "Feanturi", 
    ["school"] =    {SCHOOL_MANDOS}, 
    ["level"] =     10, 
    ["mana"] =      40, 
    ["mana_max"] = 200, 
    ["fail"] =     50, 
    -- Uses piety to cast
    ["piety"] =     TRUE,
    ["stat"] =      A_WIS,
    ["random"] = 	SKILL_SPIRITUALITY,
    ["spell"] = function() return mandos_spirit_of_the_feanturi_spell() end,
    ["info"] =  function() return mandos_spirit_of_the_feanturi_info() end,
    ["desc"] =  { 
            "Channels the power of Mandos to cure fear and confusion.", 
            "At level 20 it restores lost INT and WIS", 
            "At level 30 it cures hallucinations and restores a percentage of lost sanity" 
    } 
} 

-- "Tale of Doom" based on Foretell from T-Plus
MANDOS_TALE_DOOM = add_spell 
{ 
   ["name"] =    "Tale of Doom", 
   ["school"] =  {SCHOOL_MANDOS}, 
   ["level"] =    25, 
   ["mana"] =     60, 
   ["mana_max"] = 300, 
   ["stat"] =      A_WIS,
   ["fail"] =     75,
   -- Uses piety to cast
   ["piety"] =     TRUE,
   ["stat"] =      A_WIS,
   ["random"] = 	SKILL_SPIRITUALITY,
   ["spell"] = function() return mandos_tale_of_doom_spell() end,
   ["info"] = function() return mandos_tale_of_doom_info() end,
   ["desc"] = { 
      "Allows you to predict the future for a short time."
 } 
}

-- "Call to the Halls" based on Call Blessed Soul from T-Plus
MANDOS_CALL_HALLS= add_spell
{
	["name"] = 	"Call to the Halls",
	["school"] = 	{SCHOOL_MANDOS},
	["level"] = 	30,
	["mana"] = 	80,
	["mana_max"] = 	400,
	["fail"] = 	95,
      ["piety"] =     TRUE,
      ["stat"] =      A_WIS,
      ["random"] = 	SKILL_SPIRITUALITY,
	["spell"] =     function() return mandos_call_to_the_halls_spell() end,
	["info"] = 	function() return mandos_call_to_the_halls_info() end,
	["desc"] =	{
			"Summons a leveled spirit from the Halls of Mandos",
			"to fight for you."

	}
}