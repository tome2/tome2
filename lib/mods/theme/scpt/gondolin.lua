-- This script makes the void jumpgates between Minas Anor and Gondolin appear in Gondolin rather than in a weird wilderness spot
-- as well as making the Save Gondolin quest take the player straight to Gondolin instead of the Secret Valley.
-- Many thanks to TheFalcon for the code.

function minas_gate()
	if (quest(16).status == QUEST_STATUS_FINISHED) and (player.wilderness_y == 56) and (player.wilderness_x == 60) and (player.wild_mode == FALSE) then
		cave(35,10).feat = 159
	end
end

add_hook_script(HOOK_QUEST_FINISH, "minas_gate", "minas_gate")
add_hook_script(HOOK_WILD_GEN, "minas_gate", "minas_gate")

function minas_jump(direction)
	if (quest(16).status == QUEST_STATUS_FINISHED) and (player.wilderness_y == 56) and (player.wilderness_x == 60) and (player.wild_mode == FALSE) then
		if (player.px == 10) and (player.py == 35) then
			if (direction == "down") then
				player.wilderness_x = 3
				player.wilderness_y = 11
				player.wild_mode = FALSE
				player.px = 119
				player.py = 25
				player.oldpx = player.px
				player.oldpy = player.py
				dun_level = 0
				player.leaving = TRUE
				return TRUE
			end
		end
	end
end

add_hook_script(HOOK_STAIR, "minas_jump", "minas_jump")

add_loadsave("tolan_count", 0)

function tolan_travel()
	if (quest(15).status == QUEST_STATUS_TAKEN) and (tolan_count == 0) then
		player.wilderness_x = 3
		player.wilderness_y = 11
		player.wild_mode = FALSE
		player.px = 117
		player.py = 25
		player.oldpx = player.px
		player.oldpy = player.py
		dun_level = 0
		player.leaving = TRUE
		tolan_count = 1
		return TRUE
	end
end

add_hook_script(HOOK_END_TURN, "tolan_travel", "tolan_travel")

add_hooks
{
   [HOOK_BIRTH] = function()
      if tolan_count >=1
         then tolan_count = 0
      else
      end
   end
}