-- Spells for the school of Mandos

BOOK_MANDOS = 66

-- precognition timer for high-level spell [from T-Plus by Ingeborg S. Norden]

add_loadsave("tim_precognition",0) 

function set_precognition(v) 
   local notice = FALSE 
   if (v < 0) then v = 0 end 
   if (v > 10000) then v = 10000 end 

   -- Check if the state will change 
   if (v > 0) and (tim_precognition == 0) then 
      msg_print("You feel able to predict the future.") 
      notice = TRUE 
   elseif (v == 0) and (tim_precognition > 0) then 
      msg_print("You feel less able to predict the future.") 
      notice = TRUE 
   end 

   -- set the new value 
   tim_precognition = v 

   if (notice == TRUE) then 
      player.update = bor(player.update, PU_BONUS) 
      disturb(0,0) 
   end 
   return notice 
end 

-- related hooks

add_hooks{ 
   [HOOK_CALC_BONUS] = function() 
      if (tim_precognition > 0) then 
         --player.precognition = TRUE 
         apply_flags(0, 0, 0, TR4_PRECOGNITION, 0, 0, 0, 0, 0, 0, 0) 
      end 
   end, 

   [HOOK_PROCESS_WORLD] = function() 
      if (tim_precognition > 0) then 
         set_precognition(tim_precognition - 1) 
      end 
   end, 
} 

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
	["spell"] = function() 
      	local level = get_level(MANDOS_TEARS_LUTHIEN, 30) 
      	local obvious = hp_player(10 * level) 
        	obvious = is_obvious (set_stun(0), obvious) 
         	obvious = is_obvious (set_cut(0), obvious) 
         	obvious = is_obvious (set_afraid(0), obvious) 
         	return obvious 
   	end, 
	["info"] = 	function()
		local level = get_level(MANDOS_TEARS_LUTHIEN, 30)
		return "heals "..(10 * level)
	end,
	["desc"] =	{
			"Calls upon the spirit of Luthien to ask Mandos for healing and succour."
			}
}

-- "Spirit of the Feanturi" based on Restore Mind from T-Plus
MANDOS_SPIRIT_FEANTURI = add_spell { 
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
    ["spell"] =     function() 
            local level = get_level(MANDOS_SPIRIT_FEANTURI, 50) 
            local obvious 
            obvious = set_afraid(0) 
            obvious = is_obvious(set_confused(0), obvious) 

            if level >= 20 then 
                obvious = is_obvious(do_res_stat(A_WIS, TRUE), obvious) 
                obvious = is_obvious(do_res_stat(A_INT, TRUE), obvious) 
            end 
            
            if level >= 30 then 
                obvious = is_obvious(set_image(0), obvious) 
                obvious = is_obvious(heal_insanity(player.msane * level / 100), obvious) 
            end 
            
            return obvious 
    end,        

    ["info"] =  function() 
            local level = get_level(MANDOS_SPIRIT_FEANTURI, 50) 
            if level >= 20 then 
                return "heals "..level.."%" 
            else 
                return "" 
            end 
    end, 
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
   ["spell"] = function() 
      return set_precognition(5 + get_level(MANDOS_TALE_DOOM,10)) 
   end, 
   ["info"] = function() 
      return "dur "..(5 + get_level(MANDOS_TALE_DOOM,10)) 
   end, 
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
	["spell"] =     function()
			local y, x, m_idx
			local summons =
				{
				test_monster_name("Experienced spirit"),
				test_monster_name("Wise spirit"),
				}
			y, x = find_position(player.py, player.px)
			m_idx = place_monster_one(y, x, summons[rand_range(1, 2)], 0, FALSE, MSTATUS_FRIEND)
			if m_idx ~= 0 then
				monster_set_level(m_idx, 20 + get_level(MANDOS_CALL_HALLS, 70, 0))
				return TRUE
			end
	end,

	["info"] = 	function()
			return "level "..(get_level(MANDOS_CALL_HALLS, 70))
	end,
	["desc"] =	{
			"Summons a leveled spirit from the Halls of Mandos",
			"to fight for you."

	}
}