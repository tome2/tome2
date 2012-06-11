-- This file holds various things that govern monster behaviour with respect to the player

add_hooks{ 
[HOOK_GAME_START] = function() 

  	if ((get_race_name() == "Maia") and 
	(player_has_corruption(CORRUPT_BALROG_AURA) ~= TRUE) and
	(player_has_corruption(CORRUPT_BALROG_WINGS) ~= TRUE) and
	(player_has_corruption(CORRUPT_BALROG_STRENGTH) ~= TRUE) and
	(player_has_corruption(CORRUPT_BALROG_FORM) ~= TRUE)) then
	-- "Proper" Maiar aggravate evil beings
        timer_aggravate_evil_enable()
        end
end,
}
