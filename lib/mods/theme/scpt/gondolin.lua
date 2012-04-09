-- This script makes the Save Gondolin quest take the player straight
-- to Gondolin instead of the Secret Valley.  Many thanks to TheFalcon
-- for the code.

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