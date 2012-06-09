-- A not-too-scummy way of generating junk for ammo
function food_vessel(object)
   if ((object.tval == 80) and (object.sval == 43)) or
	((object.tval == 80) and (object.sval == 44)) then
      local obj = create_object(TV_JUNK, 3)
	obj.ident = bor(obj.ident, IDENT_MENTAL, IDENT_KNOWN)
	inven_carry(obj, FALSE)
	end_object(obj)
	return FALSE
   end
end

add_hook_script(HOOK_EAT, "food_vessel", "food_vessel")

-- Longbottom Leaf *is* a great stress reliever:
function longbottom_leaf(object)
	if (object.tval == 80) and (object.sval == 45) then
		msg_print("What a stress reliever!")
		heal_insanity(1000)
		return FALSE
	end
end
add_hook_script(HOOK_EAT, "longbottom_leaf", "longbottom_leaf")

-- Hobbits like food
function hobbit_food(m_idx, item)

	m_ptr = monster(m_idx)
	o_ptr = get_object(item)

	if (m_ptr.r_idx == test_monster_name("Scruffy-looking hobbit")) 
	and (o_ptr.tval == TV_FOOD) then
		cmsg_print(TERM_YELLOW, "'Yum!'")
		inven_item_increase(item, -1)
		inven_item_optimize(item)
		return TRUE
	else
		return FALSE
	end
end

add_hook_script(HOOK_GIVE, "hobbit_food", "hobbit_food")

-- Smeagol likes rings
function smeagol_ring(m_idx, item)

   m_ptr = monster(m_idx)
   o_ptr = get_object(item)

   if (m_ptr.r_idx == test_monster_name("Smeagol"))
         and (o_ptr.tval == TV_RING) then

      cmsg_print(TERM_YELLOW, "'MY... PRECIOUSSSSS!!!'")

      inven_item_increase(item, -1)
      inven_item_optimize(item)
      return TRUE
   else
      return FALSE
   end
end

add_hook_script(HOOK_GIVE, "smeagol_ring", "smeagol_ring") 

-- functions to check for Map and Key of Thror before proceeding in Erebor
-- Thank you, Massimiliano Marangio :-)
add_hooks
{
   [HOOK_STAIR] = function(direction)
      if ((current_dungeon_idx == 20) and (dun_level == 60) and (direction == "down")) then
            local i
            local mapkey = 0
            for i = 0, INVEN_TOTAL - 1 do
               if ((player.inventory(i).name1 == 209) or (player.inventory(i).name1 == 210)) then
                  mapkey = mapkey + 1
               end
            end

            if (mapkey == 2) then
                msg_print("The moon-letters on the map show you the keyhole! You use the key to enter.")
                return FALSE
            else
                msg_print("You have found a door, but you cannot find a way to enter. Ask in Dale, perhaps?")
                return TRUE
            end
      end
      return FALSE
   end,
}

-- function to make the Dale mayor tell you about how to get to Erebor 61
add_building_action
{
	["index"] =     66,
	["action"] =    function()
		msg_print("You will need Thorin's Key and Thrain's Map to get anywhere in Erebor. One may be found in the Barrow-Downs. The other, in Mirkwood.")
	end
}

-- function to check for Key of Orthanc before proceeding to the final level in Isengard
add_hooks
{
   [HOOK_STAIR] = function(direction)
      if ((current_dungeon_idx == 36) and (dun_level == 39) and (direction == "down")) then
            local i
            local orthkey = 0
            for i = 0, INVEN_TOTAL - 1 do
               if (player.inventory(i).name1 == 15) then
                  orthkey = orthkey + 1
               end
            end

            if (orthkey == 1) then
                msg_print("#BYou have the key to the tower of Orthanc! You may proceed.#w")
                return FALSE
            else
                msg_print("#yYou may not enter Orthanc without the key to the gates!#w Rumours say the key was lost in the Mines of Moria...")
                return TRUE
            end
      end
      return FALSE
   end,
}