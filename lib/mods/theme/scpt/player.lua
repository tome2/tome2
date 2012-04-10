------------------------------------------------------------------------------
----------------------- Hook to create birth objects -------------------------
------------------------------------------------------------------------------
function __birth_hook_objects()

	-- Start the undeads, as undeads with the corruptions
	if get_subrace_name() == "Vampire" then
		player_gain_corruption(CORRUPT_VAMPIRE_TEETH)
		player_gain_corruption(CORRUPT_VAMPIRE_STRENGTH)
		player_gain_corruption(CORRUPT_VAMPIRE_VAMPIRE)
	end

end

-- Register in the hook list
add_hook_script(HOOK_BIRTH_OBJECTS, "__birth_hook_objects", "__birth_hook_objects")
