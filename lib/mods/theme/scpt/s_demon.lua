-- handle the demonology school

-- Demonblade
DEMON_BLADE = add_spell
{
	["name"] =      "Demon Blade",
	["school"] =    {SCHOOL_DEMON},
	["level"] =     1,
	["mana"] =      4,
	["mana_max"] =  44,
	["fail"] =      10,
	["random"] =    0,
	["stick"] =
	{
			["charge"] =    { 3, 7 },
			[TV_WAND] =
			{
				["rarity"] =	    75,
				["base_level"] =	{ 1, 17 },
				["max_level"] =		{ 20, 40 },
			},
	},
	["spell"] =     function()
			local type, rad

			type = GF_FIRE
			if get_level(DEMON_BLADE) >= 30 then type = GF_HELL_FIRE end

			rad = 0
			if get_level(DEMON_BLADE) >= 45 then rad = 1 end

			return set_project(randint(20) + get_level(DEMON_BLADE, 80),
				    type,
				    4 + get_level(DEMON_BLADE, 40),
				    rad,
				    bor(PROJECT_STOP, PROJECT_KILL))
	end,
	["info"] =      function()
			return "dur "..(get_level(DEMON_BLADE, 80)).."+d20 dam "..(4 + get_level(DEMON_BLADE, 40)).."/blow"
	end,
	["desc"] =      {
			"Imbues your blade with fire to deal more damage",
			"At level 30 it deals hellfire damage",
			"At level 45 it spreads over a 1 radius zone around your target",
	}
}

DEMON_MADNESS = add_spell
{
	["name"] =      "Demon Madness",
	["school"] =    {SCHOOL_DEMON},
	["level"] =     10,
	["mana"] =      5,
	["mana_max"] =  20,
	["fail"] =      25,
	["random"] =    0,
	["spell"] =     function()
			local ret, dir, type, y1, x1, y2, x2

			ret, dir = get_aim_dir()
			if ret == FALSE then return end

			type = GF_CHAOS
			if magik(33) == TRUE then type = GF_CONFUSION end
			if magik(33) == TRUE then type = GF_CHARM end

			-- Calc the coordinates of arrival
			y1, x1 = get_target(dir)
			y2 = player.py - (y1 - player.py)
			x2 = player.px - (x1 - player.px)

			local obvious = nil
			obvious = project(0, 1 + get_level(DEMON_MADNESS, 4, 0),
				y1, x1,
				20 + get_level(DEMON_MADNESS, 200),
				type, bor(PROJECT_STOP, PROJECT_GRID, PROJECT_ITEM, PROJECT_KILL))
			obvious = is_obvious(project(0, 1 + get_level(DEMON_MADNESS, 4, 0),
				y2, x2,
				20 + get_level(DEMON_MADNESS, 200),
				type, bor(PROJECT_STOP, PROJECT_GRID, PROJECT_ITEM, PROJECT_KILL)), obvious)
			return obvious
	end,
	["info"] =      function()
			return "dam "..(20 + get_level(DEMON_MADNESS, 200)).." rad "..(1 + get_level(DEMON_MADNESS, 4, 0))
	end,
	["desc"] =      {
			"Fire 2 balls in opposite directions of randomly chaos, confusion or charm",
	}
}

DEMON_FIELD = add_spell
{
	["name"] =      "Demon Field",
	["school"] =    {SCHOOL_DEMON},
	["level"] =     20,
	["mana"] =      20,
	["mana_max"] =  60,
	["fail"] =      60,
	["random"] =    0,
	["spell"] =     function()
			local ret, dir

			ret, dir = get_aim_dir()
			if ret == FALSE then return end
			return fire_cloud(GF_NEXUS, dir, 20 + get_level(DEMON_FIELD, 70), 7, 30 + get_level(DEMON_FIELD, 100))
	end,
	["info"] =      function()
			return "dam "..(20 + get_level(DEMON_FIELD, 70)).." dur "..(30 + get_level(DEMON_FIELD, 100))
	end,
	["desc"] =      {
			"Fires a cloud of deadly nexus over a radius of 7",
	}
}

-- Demonshield

DOOM_SHIELD = add_spell
{
	["name"] =      "Doom Shield",
	["school"] =    {SCHOOL_DEMON},
	["level"] =     1,
	["mana"] =      2,
	["mana_max"] =  30,
	["fail"] =      10,
	["random"] =    0,
	["spell"] =     function()
			return set_shield(randint(10) + 20 + get_level(DOOM_SHIELD, 100), -300 + get_level(DOOM_SHIELD, 100), SHIELD_COUNTER, 1 + get_level(DOOM_SHIELD, 14), 10 + get_level(DOOM_SHIELD, 15))
	end,
	["info"] =      function()
			return "dur "..(20 + get_level(DOOM_SHIELD, 100)).."+d10 dam "..(1 + get_level(DOOM_SHIELD, 14)).."d"..(10 + get_level(DOOM_SHIELD, 15))
	end,
	["desc"] =      {
			"Raises a mirror of pain around you, doing very high damage to your foes",
			"that dare hit you, but greatly reduces your armour class",
	}
}

UNHOLY_WORD = add_spell
{
	["name"] =      "Unholy Word",
	["school"] =    {SCHOOL_DEMON},
	["level"] =     25,
	["mana"] =      15,
	["mana_max"] =  45,
	["fail"] =      55,
	["random"] =    0,
	["spell"] =     function()
			local ret, x, y, c_ptr
			ret, x, y = tgt_pt()
			if ret == FALSE then return end
			c_ptr = cave(y, x)

			-- ok that is a monster
			if c_ptr.m_idx > 0 then
				local m_ptr = monster(c_ptr.m_idx)
				if m_ptr.status ~= MSTATUS_PET then
					msg_print("You can only target a pet.")
					return
				end

				-- Oups he is angry now
				if magik(30 - get_level(UNHOLY_WORD, 25, 0)) == TRUE then
					local m_name = monster_desc(m_ptr, 0).." turns against you."
					msg_print(strupper(strsub(m_name, 0, 1))..strsub(m_name, 2))
				else
					local m_name = monster_desc(m_ptr, 0)
					msg_print("You consume "..m_name..".")

					local heal = (m_ptr.hp * 100) / m_ptr.maxhp
					heal = ((30 + get_level(UNHOLY_WORD, 50, 0)) * heal) / 100

					hp_player(heal)

					delete_monster_idx(c_ptr.m_idx)
				end
				return TRUE
			end
	end,
	["info"] =      function()
			return "heal mhp% of "..(30 + get_level(UNHOLY_WORD, 50, 0)).."%"
	end,
	["desc"] =      {
			"Kills a pet to heal you",
			"There is a chance that the pet won't die but will turn against you",
			"it will decrease with higher level",
	}
}

DEMON_CLOAK = add_spell
{
	["name"] =      "Demon Cloak",
	["school"] =    {SCHOOL_DEMON},
	["level"] =     20,
	["mana"] =      10,
	["mana_max"] =  40,
	["fail"] =      70,
	["random"] =    0,
	["spell"] =     function()
			return set_tim_reflect(randint(5) + 5 + get_level(DEMON_CLOAK, 15, 0))
	end,
	["info"] =      function()
			return "dur "..(5 + get_level(DEMON_CLOAK, 15, 0)).."+d5"
	end,
	["desc"] =      {
			"Raises a mirror that can reflect bolts and arrows for a time",
	}
}


-- Demonhorn
DEMON_SUMMON = add_spell
{
	["name"] =      "Summon Demon",
	["school"] =    {SCHOOL_DEMON},
	["level"] =     5,
	["mana"] =      10,
	["mana_max"] =  50,
	["fail"] =      30,
	["random"] =    0,
	["spell"] =     function()
			local type = SUMMON_DEMON
			local level = dun_level
			local minlevel = 4
			if level < minlevel then level=minlevel end
			summon_specific_level = 5 + get_level(DEMON_SUMMON, 100)
			if get_level(DEMON_SUMMON) >= 35 then type = SUMMON_HI_DEMON end
			if summon_monster(player.py, player.px, level, TRUE, type) == TRUE then
				return TRUE
			else
				msg_print("Something blocks your summoning!")
				return FALSE
			end
	end,
	["info"] =      function()
			return "level "..(5 + get_level(DEMON_SUMMON, 100))
	end,
	["desc"] =      {
			"Summons a leveled demon to your side",
			"At level 35 it summons a high demon",
	}
}

DISCHARGE_MINION = add_spell
{
	["name"] =      "Discharge Minion",
	["school"] =    {SCHOOL_DEMON},
	["level"] =     10,
	["mana"] =      20,
	["mana_max"] =  50,
	["fail"] =      30,
	["random"] =    0,
	["spell"] =     function()
			local ret, x, y, c_ptr
			ret, x, y = tgt_pt()
			if ret == FALSE then return end
			c_ptr = cave(y, x)

			-- ok that is a monster
			if c_ptr.m_idx > 0 then
				local m_ptr = monster(c_ptr.m_idx)
				if m_ptr.status ~= MSTATUS_PET then
					msg_print("You can only target a pet.")
					return
				end

				local dam = m_ptr.hp
				delete_monster_idx(c_ptr.m_idx)
				dam = (dam * (20 + get_level(DISCHARGE_MINION, 60, 0))) / 100
				if dam > 100 + get_level(DISCHARGE_MINION, 500, 0) then
					dam = 100 + get_level(DISCHARGE_MINION, 500, 0)
				end

				-- We use project instead of fire_ball because we must tell it exactly where to land
				return project(0, 2,
					y, x,
					dam,
					GF_GRAVITY, bor(PROJECT_STOP, PROJECT_GRID, PROJECT_ITEM, PROJECT_KILL))
			end
	end,
	["info"] =      function()
			return "dam "..(20 + get_level(DISCHARGE_MINION, 60, 0)).."% max "..(100 + get_level(DISCHARGE_MINION, 500, 0))
	end,
	["desc"] =      {
			"The targeted pet will explode in a burst of gravity",
	}
}

CONTROL_DEMON = add_spell
{
	["name"] =      "Control Demon",
	["school"] =    {SCHOOL_DEMON},
	["level"] =     25,
	["mana"] =      30,
	["mana_max"] =  70,
	["fail"] =      55,
	["random"] =    0,
	["spell"] =     function()
			local ret, dir = get_aim_dir()
			return fire_ball(GF_CONTROL_DEMON, dir, 50 + get_level(CONTROL_DEMON, 250), 0)
	end,
	["info"] =      function()
			return "power "..(50 + get_level(CONTROL_DEMON, 250))
	end,
	["desc"] =      {
			"Attempts to control a demon",
	}
}

-- ok we need to have different wield slots
add_hooks
{
	[HOOK_WIELD_SLOT] =     function (obj, ideal)
			if (obj.tval == TV_DAEMON_BOOK) then
				local slot
				if (obj.sval == SV_DEMONBLADE) then
					if(ideal == TRUE) then
						slot = INVEN_WIELD
					else
						slot = get_slot(INVEN_WIELD)
					end
				elseif (obj.sval == SV_DEMONSHIELD) then
					if(ideal == TRUE) then
						slot = INVEN_ARM
					else
						slot = get_slot(INVEN_ARM)
					end
				elseif (obj.sval == SV_DEMONHORN) then
					if(ideal == TRUE) then
						slot = INVEN_HEAD
					else
						slot = get_slot(INVEN_HEAD)
					end
				end
				return TRUE, slot
			end
	end,
}
