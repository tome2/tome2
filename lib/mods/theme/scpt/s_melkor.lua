-- handle the melkor school

-- Not included in the spell code directly because I need to call it from somewhere else too
function do_melkor_curse(who)
	local m_ptr = monster(who)

	if get_level(MELKOR_CURSE) >= 35 then
		local r_ptr = race_info_idx(m_ptr.r_idx, m_ptr.ego)

		m_ptr.maxhp = m_ptr.maxhp - r_ptr.hside;
		if m_ptr.maxhp < 1 then m_ptr.maxhp = 1 end
		if m_ptr.hp > m_ptr.maxhp then m_ptr.hp = m_ptr.maxhp end
		player.redraw = bor(player.redraw, PR_HEALTH)
	end
	if get_level(MELKOR_CURSE) >= 25 then
		m_ptr.speed = m_ptr.speed - get_level(MELKOR_CURSE, 7)
		m_ptr.mspeed = m_ptr.mspeed - get_level(MELKOR_CURSE, 7)

		if m_ptr.speed < 70 then m_ptr.speed = 70 end
		if m_ptr.mspeed < 70 then m_ptr.mspeed = 70 end
	end
	if get_level(MELKOR_CURSE) >= 15 then
		m_ptr.ac = m_ptr.ac - get_level(MELKOR_CURSE, 50)

		if m_ptr.ac < -70 then m_ptr.ac = -70 end
	end

	local i, pow
	i = 1
	pow = get_level(MELKOR_CURSE, 2)
	while (i <= 4) do
		if m_ptr.blow[i].d_dice > 0 then
			if m_ptr.blow[i].d_dice < pow then
				pow = m_ptr.blow[i].d_dice
			end
			if m_ptr.blow[i].d_side < pow then
				pow = m_ptr.blow[i].d_side
			end
			m_ptr.blow[i].d_dice = m_ptr.blow[i].d_dice - pow
		end
		i = i + 1
	end

	local m_name = monster_desc(m_ptr, 0).." looks weaker."
	msg_print(strupper(strsub(m_name, 0, 1))..strsub(m_name, 2))

	-- wake it
	m_ptr.csleep = 0;
end

MELKOR_CURSE = add_spell
{
	["name"] =      "Curse",
	["school"] =    {SCHOOL_MELKOR},
	["level"] =     1,
	["mana"] =      50,
	["mana_max"] =  300,
	["fail"] = 	20,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] = 	SKILL_SPIRITUALITY,
	["spell"] = 	function()
				local ret, dir = get_aim_dir()
				if ret == FALSE then return end

				if target_who == -1 then
					msg_print("You must target a monster.")
				else
					do_melkor_curse(target_who)
				end
				return TRUE
	end,
	["info"] = 	function()
			return ""
	end,
	["desc"] =	{
			"It curses a monster, reducing its melee power",
			"At level 5 it can be auto-casted (with no piety cost) while fighting",
			"At level 15 it also reduces armor",
			"At level 25 it also reduces speed",
			"At level 35 it also reduces max life (but it is never fatal)",
	}
}

MELKOR_CORPSE_EXPLOSION = add_spell
{
	["name"] =      "Corpse Explosion",
	["school"] =    {SCHOOL_MELKOR},
	["level"] =     10,
	["mana"] =      100,
	["mana_max"] =  500,
	["fail"] = 	45,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] = 	SKILL_SPIRITUALITY,
	["spell"] = 	function()
			return fire_ball(GF_CORPSE_EXPL, 0, 20 + get_level(MELKOR_CORPSE_EXPLOSION, 70), 2 + get_level(MELKOR_CORPSE_EXPLOSION, 5))
	end,
	["info"] = 	function()
			return "dam "..(20 + get_level(MELKOR_CORPSE_EXPLOSION, 70)).."%"
	end,
	["desc"] =	{
			"It makes corpses in an area around you explode for a percent of their",
			"hit points as damage",
	}
}

MELKOR_MIND_STEAL = add_spell
{
	["name"] =      "Mind Steal",
	["school"] =    {SCHOOL_MELKOR},
	["level"] =     20,
	["mana"] =      1000,
	["mana_max"] =  3000,
	["fail"] = 	90,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] = 	SKILL_SPIRITUALITY,
	["spell"] = 	function()
				local ret, dir = get_aim_dir()
				if ret == FALSE then return end

				if target_who == -1 then
					msg_print("You must target a monster.")
				else
					local chance, m_ptr, r_ptr

					m_ptr = monster(target_who)
					r_ptr = race_info_idx(m_ptr.r_idx, m_ptr.ego)
					chance = get_level(MELKOR_MIND_STEAL)
					if (randint(m_ptr.level) < chance) and (band(r_ptr.flags1, RF1_UNIQUE) == 0) then
						player.control = target_who
						m_ptr.mflag = bor(m_ptr.mflag, MFLAG_CONTROL)

						local m_name = monster_desc(m_ptr, 0).." falls under your control."
						msg_print(strupper(strsub(m_name, 0, 1))..strsub(m_name, 2))
					else
						local m_name = monster_desc(m_ptr, 0).." resists."
						msg_print(strupper(strsub(m_name, 0, 1))..strsub(m_name, 2))
					end
					return TRUE
				end
	end,
	["info"] = 	function()
			return "chance 1d(mlvl)<"..(get_level(MELKOR_MIND_STEAL))
	end,
	["desc"] =	{
			"It allows your spirit to temporarily leave your own body, which will",
			"be vulnerable, to control one of your enemies body."
	}
}
