-- Spells for Ulmo's school

BOOK_ULMO = 65

-- "Song of Belegaer" copied from Geyser
ULMO_BELEGAER = add_spell
{
	["name"] = "Song of Belegaer",
	["school"] = SCHOOL_ULMO,
	["level"] = 1,
	["mana"] = 1,
	["mana_max"] = 100,
	["fail"] = 25,
	["piety"] = TRUE,
	["stat"] =      A_WIS,
	["random"] =    SKILL_SPIRITUALITY,
	["spell"] = function()
		local ret, dir
		ret, dir = get_aim_dir()
		if ret == FALSE then return end
		return fire_bolt_or_beam(2 * get_level(ULMO_BELEGAER, 85), GF_WATER, dir, damroll(get_geyser_damage()))
	end,
	["info"] = function()
		local n, d
		n, d = get_geyser_damage()
		return "dam "..n.."d"..d
	end,
	["desc"] =
	{
		"Channels the power of the Great Sea into your fingertips.",
		"Sometimes it can blast through its first target."
	},
}

-- "Draught of Ulmonan" copied with tweaks from T-Plus Nature spell "Restore Body"
ULMO_DRAUGHT_ULMONAN = add_spell
{
	["name"] = 	"Draught of Ulmonan",
	["school"] = 	{SCHOOL_ULMO},
	["level"] = 	15,
	["mana"] = 	25,
	["mana_max"] = 	200,
	["fail"] = 	50,
	["piety"] = TRUE,
	["stat"] =      A_WIS,
	["random"] =    SKILL_SPIRITUALITY,
	["spell"] = 	function()
			local level = get_level(ULMO_DRAUGHT_ULMONAN, 50)
			local obvious = hp_player(5 * level)
			obvious = is_obvious(set_poisoned(0), obvious)
			obvious = is_obvious(set_cut(0), obvious)
			obvious = is_obvious(set_stun(0), obvious)
			obvious = is_obvious(set_blind(0), obvious)
			if level >= 10 then
				obvious = is_obvious(do_res_stat(A_STR, TRUE), obvious)
				obvious = is_obvious(do_res_stat(A_CON, TRUE), obvious)
				obvious = is_obvious(do_res_stat(A_DEX, TRUE), obvious)
			end
			if level >= 20 then
				obvious =  is_obvious(set_parasite(0, 0), obvious)
				obvious = is_obvious(set_mimic(0, 0, 0), obvious)
			end
			return obvious
	end,		
	["info"] = 	function()
			local level = get_level(ULMO_DRAUGHT_ULMONAN, 50)
			return "cure "..(5 * level)
	end,
	["desc"] =	{
			"Fills you with a draught with powerful curing effects,",
			"prepared by Ulmo himself.",
			"Level 1: blindness, poison, cuts and stunning",
			"Level 10: drained STR, DEX and CON",
			"Level 20: parasites and mimicry",
	},
}

-- "Call of the Ulumuri" based on Call Blessed Soul from T-Plus
ULMO_CALL_ULUMURI = add_spell

{
	["name"] = 	"Call of the Ulumuri",
	["school"] = 	{SCHOOL_ULMO},
	["level"] = 	20,
	["mana"] = 	50,
	["mana_max"] = 	300,
	["fail"] = 	75,
	["piety"] = TRUE,
	["stat"] =      A_WIS,
	["random"] =    SKILL_SPIRITUALITY,
	["spell"] =     function()
			local y, x, m_idx
			local summons =
				{
				test_monster_name("Water spirit"),
				test_monster_name("Water elemental"),
				}
			y, x = find_position(player.py, player.px)
			m_idx = place_monster_one(y, x, summons[rand_range(1, 2)], 0, FALSE, MSTATUS_FRIEND)
			if m_idx ~= 0 then
				monster_set_level(m_idx, 30 + get_level(ULMO_CALL_ULUMURI, 70, 0))
				return TRUE
			end
	end,

	["info"] = 	function()
			return "level "..(get_level(ULMO_CALL_ULUMURI, 70))
	end,
	["desc"] =	{
			"Summons a leveled water spirit or elemental",
			"to fight for you",

	},
}

-- "Wrath of Ulmo" based on Firewall
ULMO_WRATH = add_spell
{
	["name"] = 	"Wrath of Ulmo",
	["school"] = 	{SCHOOL_ULMO},
	["level"] = 	30,
	["mana"] = 	100,
	["mana_max"] = 	400,
	["fail"] = 	95,
	["piety"] = TRUE,
	["stat"] =      A_WIS,
	["random"] =    SKILL_SPIRITUALITY,
	["spell"] =     function()
		local ret, dir, type
		if (get_level(ULMO_WRATH, 50) >= 30) then
			type = GF_WAVE
		else
			type = GF_WATER
		end
		ret, dir = get_aim_dir()
		if ret == FALSE then return end
		fire_wall(type, dir, 40 + get_level(ULMO_WRATH, 150), 10 + get_level(ULMO_WRATH, 14))
		return TRUE
	end,
	["info"] =      function()
		return "dam "..(40 + get_level(ULMO_WRATH, 150)).." dur "..(10 + get_level(ULMO_WRATH, 14))
	end,
	["desc"] =      {
			"Conjures up a sea storm.",
			"At level 30 it turns into a more forceful storm."
	}
}