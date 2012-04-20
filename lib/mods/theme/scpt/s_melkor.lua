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
	["spell"] = 	function() return melkor_curse() end,
	["info"] = 	function() return melkor_curse_info() end,
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
	["spell"] = 	function() return melkor_corpse_explosion() end,
	["info"] = 	function() return melkor_corpse_explosion_info() end,
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
	["spell"] = 	function() return melkor_mind_steal() end,
	["info"] = 	function() return melkor_mind_steal_info() end,
	["desc"] =	{
			"It allows your spirit to temporarily leave your own body, which will",
			"be vulnerable, to control one of your enemies body."
	}
}
