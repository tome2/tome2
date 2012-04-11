add_hooks
{
	[HOOK_RECALC_SKILLS] = function()
		if not (player.pgod == GOD_NONE) and (get_skill(SKILL_ANTIMAGIC) > 0) then
			msg_print("You no longer believe.")
			abandon_god(GOD_ALL)
		end
		return FALSE
	end,
}
