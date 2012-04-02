------------------------------------------------------------------------------
----------------------- Hook to create birth objects -------------------------
------------------------------------------------------------------------------
function __birth_hook_objects()

	-- Grace delay for adding piety
	GRACE_DELAY = 0

	-- Start the undeads, as undeads with the corruptions
	if get_subrace_name() == "Vampire" then
		player.corruption(CORRUPT_VAMPIRE_TEETH, TRUE)
		player.corruption(CORRUPT_VAMPIRE_STRENGTH, TRUE)
		player.corruption(CORRUPT_VAMPIRE_VAMPIRE, TRUE)
	end

end

-- Register in the hook list
add_hook_script(HOOK_BIRTH_OBJECTS, "__birth_hook_objects", "__birth_hook_objects")
