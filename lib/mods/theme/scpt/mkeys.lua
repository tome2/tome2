-- Mkeys for skills & abilities

-- Far reaching attack of polearms
add_mkey
{
	["mkey"]	= 102,
	["fct"]	 = function()
			local weapon = get_object(INVEN_WIELD);
			if weapon.tval == TV_POLEARM then
			else
				msg_print("You will need a long polearm for this!")
				return
			end

			ret, dir = get_rep_dir()
			if ret == FALSE then return end

			local dy, dx = explode_dir(dir)
			dy = dy * 2
			dx = dx * 2
		    	targety = player.py + dy
		    	targetx = player.px + dx

			local max_blows = get_skill_scale(SKILL_POLEARM, player.num_blow / 2)
			if max_blows == 0 then max_blows = 1 end

			if get_skill(SKILL_POLEARM) >= 40 then
				energy_use = energy_use + 200
				return project(0, 0, targety, targetx, max_blows, GF_ATTACK, bor(PROJECT_BEAM, PROJECT_KILL))
			else
				energy_use = energy_use + 200
				return project(0, 0, targety, targetx, max_blows, GF_ATTACK, bor(PROJECT_BEAM, PROJECT_STOP, PROJECT_KILL))
			end
	end,
}
