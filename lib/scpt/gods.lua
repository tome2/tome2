add_hooks
{
	[HOOK_FOLLOW_GOD] = function(god, action)
		if action == "ask" then
			if not (god == GOD_MELKOR) then
				local i = INVEN_WIELD
				while i < INVEN_TOTAL do
					-- 13 is ART_POWER
					if player.inventory(i).name1 == 13 then
						msg_print("The One Ring has corrupted you, and you are rejected.")
						return TRUE
					end
					i = i + 1
				end
			end
		end
		return FALSE
	end,
	[HOOK_RECALC_SKILLS] = function()
		if not (player.pgod == GOD_NONE) and (get_skill(SKILL_ANTIMAGIC) > 0) then
			msg_print("You no longer believe.")
			abandon_god(GOD_ALL)
		end
		return FALSE
	end,
}
