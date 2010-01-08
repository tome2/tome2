-- Handle Manwe Sulimo magic school

MANWE_SHIELD = add_spell
{
	["name"] =      "Wind Shield",
	["school"] =    {SCHOOL_MANWE},
	["level"] =     10,
	["mana"] =      100,
	["mana_max"] =  500,
	["fail"] = 	30,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] = 	SKILL_SPIRITUALITY,
	["spell"] = 	function()
			local dur = get_level(MANWE_SHIELD, 50) + 10 + randint(20)
			local obvious

			obvious = set_protevil(dur)
			if get_level(MANWE_SHIELD) >= 10 then
				local type

				type = 0
				if get_level(MANWE_SHIELD) >= 20 then
					type = SHIELD_COUNTER
				end
				obvious = is_obvious(set_shield(dur, get_level(MANWE_SHIELD, 30), type, 1 + get_level(MANWE_SHIELD, 2), 1 + get_level(MANWE_SHIELD, 6)), obvious)
			end
			return obvious
	end,
	["info"] = 	function()
			local desc = "dur "..(get_level(MANWE_SHIELD, 50) + 10).."+d20"

			if get_level(MANWE_SHIELD) >= 10 then
				desc = desc.." AC "..(get_level(MANWE_SHIELD, 30))
			end
			if get_level(MANWE_SHIELD) >= 20 then
				desc = desc.." dam "..(1 + get_level(MANWE_SHIELD, 2)).."d"..(1 + get_level(MANWE_SHIELD, 6))
			end
			return desc
	end,
	["desc"] =	{
			"It surrounds you with a shield of wind that deflects blows from evil monsters",
			"At level 10 it increases your armour rating",
			"At level 20 it retaliates against monsters that melee you",
	}
}

MANWE_AVATAR = add_spell
{
	["name"] =      "Avatar",
	["school"] =    {SCHOOL_MANWE},
	["level"] =     35,
	["mana"] =      1000,
	["mana_max"] =  1000,
	["fail"] = 	80,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] = 	SKILL_SPIRITUALITY,
	["spell"] = 	function()
			return set_mimic(get_level(MANWE_AVATAR, 20) + randint(10), resolve_mimic_name("Maia"), player.lev)
	end,
	["info"] =      function()
			return "dur "..(get_level(MANWE_AVATAR, 20)).."+d10"
	end,
	["desc"] =	{
			"It turns you into a full grown Maia",
	}
}

MANWE_BLESS = add_spell
{
	["name"] =      "Manwe's Blessing",
	["school"] =    {SCHOOL_MANWE},
	["level"] =     1,
	["mana"] =      10,
	["mana_max"] =  100,
	["fail"] = 	20,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] = 	SKILL_SPIRITUALITY,
	["spell"] = 	function()
			local dur = get_level(MANWE_BLESS, 70) + 30 + randint(40)
			local obvious

			obvious = set_blessed(dur)
			obvious = is_obvious(set_afraid(0), obvious)
			obvious = is_obvious(set_lite(0), obvious)
			if get_level(MANWE_BLESS) >= 10 then
				obvious = is_obvious(set_hero(dur), obvious)
			end
			if get_level(MANWE_BLESS) >= 20 then
				obvious = is_obvious(set_shero(dur), obvious)
			end
			if get_level(MANWE_BLESS) >= 30 then
				obvious = is_obvious(set_holy(dur), obvious)
			end
			return obvious
	end,
	["info"] =      function()
			return "dur "..(get_level(MANWE_BLESS, 70) + 30).."+d40"
	end,
	["desc"] =	{
			"Manwe's Blessing removes your fears, blesses you and surrounds you with",
			"holy light",
			"At level 10 it also grants heroism",
			"At level 20 it also grants super heroism",
			"At level 30 it also grants holy luck and life protection",
	}
}

MANWE_CALL = add_spell
{
	["name"] =      "Manwe's Call",
	["school"] =    {SCHOOL_MANWE},
	["level"] =     20,
	["mana"] =      200,
	["mana_max"] =  500,
	["fail"] = 	40,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] = 	SKILL_SPIRITUALITY,
	["spell"] = 	function()
			local y, x, m_idx

			y, x = find_position(player.py, player.px)
			m_idx = place_monster_one(y, x, test_monster_name("Great eagle"), 0, FALSE, MSTATUS_FRIEND)

			if m_idx ~= 0 then
				monster_set_level(m_idx, 20 + get_level(MANWE_CALL, 70, 0))
				return TRUE
			end
	end,
	["info"] =      function()
			return "level "..(get_level(MANWE_CALL, 70) + 20)
	end,
	["desc"] =	{
			"Manwe's Call summons a Great Eagle to help you battle the forces",
			"of Morgoth"
	}
}
