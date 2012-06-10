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