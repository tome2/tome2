-- Definition of the corruptions

-- The Balrog corruptions
CORRUPT_BALROG_AURA = add_corruption
{
	["color"]       = TERM_ORANGE,
	["name"]	= "Balrog Aura",
	["get_text"]    = "A corrupted wall of flames surrounds you.",
	["lose_text"]   = "The wall of corrupted flames abandons you.",
	["desc"]	=
	{
			  "  Surrounds you with a fiery aura",
			  "  But it can burn scrolls when you read them"
	},
	["hooks"]       =
	{
		[HOOK_CALC_BONUS] = function()
			player.xtra_f3 = bor(player.xtra_f3, TR3_SH_FIRE)
			player.xtra_f3 = bor(player.xtra_f3, TR3_LITE1)
		end,
		[HOOK_READ] = function(obj)
			if magik(5) == TRUE then
				msg_print("Your demon aura burns the scroll before you read it!")
				return TRUE, TRUE, FALSE
			else
				return FALSE
			end
		end,
	},
}

CORRUPT_BALROG_WINGS = add_corruption
{
	["color"]       = TERM_ORANGE,
	["name"]	= "Balrog Wings",
	["get_text"]    = "Wings of shadow grow in your back.",
	["lose_text"]   = "The wings in your back fall apart.",
	["desc"]	=
	{
			  "  Creates ugly, but working, wings allowing you to fly",
			  "  But it reduces charisma by 4 and dexterity by 2"
	},
	["hooks"]       =
	{
		[HOOK_CALC_BONUS] = function()
			player.xtra_f4 = bor(player.xtra_f4, TR4_FLY)
			player.modify_stat(A_CHR, -4)
			player.modify_stat(A_DEX, -2)
		end,
	},
}

CORRUPT_BALROG_STRENGTH = add_corruption
{
	["color"]       = TERM_ORANGE,
	["name"]	= "Balrog Strength",
	["get_text"]    = "Your muscles get unnatural strength.",
	["lose_text"]   = "Your muscles get weaker again.",
	["desc"]	=
	{
			  "  Provides 3 strength and 1 constitution",
			  "  But it reduces charisma by 1 and dexterity by 3"
	},
	["hooks"]       =
	{
		[HOOK_CALC_BONUS] = function()
			player.modify_stat(A_STR, 3)
			player.modify_stat(A_CON, 1)
			player.modify_stat(A_DEX, -3)
			player.modify_stat(A_CHR, -1)
		end,
	},
}

CORRUPT_BALROG_FORM = add_corruption
{
	["color"]       = TERM_YELLOW,
	["name"]	= "Balrog Form",
	["get_text"]    = "You feel the might of a Balrog inside you.",
	["lose_text"]   = "The presence of the Balrog seems to abandon you.",
	["desc"]	=
	{
			  "  Allows you to turn into a Balrog at will",
			  "  You need Balrog Wings, Balrog Aura and Balrog Strength to activate it"
	},
	["depends"]     =
	{
			[CORRUPT_BALROG_AURA] = TRUE,
			[CORRUPT_BALROG_WINGS] = TRUE,
			[CORRUPT_BALROG_STRENGTH] = TRUE
	},
	["hooks"]       =
	{
		[HOOK_CALC_BONUS] = function()
			player.xtra_f2 = bor(player.xtra_f2, TR2_IM_ACID)
			player.xtra_f2 = bor(player.xtra_f2, TR2_IM_FIRE)
			player.xtra_f2 = bor(player.xtra_f2, TR2_IM_ELEC)
			player.xtra_f2 = bor(player.xtra_f2, TR2_RES_DARK)
			player.xtra_f2 = bor(player.xtra_f2, TR2_RES_CHAOS)
		end,
		[HOOK_CALC_POWERS] = function()
			player.add_power(PWR_BALROG)
		end,
	},
}


-- The Demon corruptions
CORRUPT_DEMON_SPIRIT = add_corruption
{
	["color"]       = TERM_RED,
	["name"]	= "Demon Spirit",
	["get_text"]    = "Your spirit opens to corrupted thoughts.",
	["lose_text"]   = "Your spirit closes again to the corrupted thoughts.",
	["desc"]	=
	{
			  "  Increases your intelligence by 1",
			  "  But reduce your charisma by 2",
	},
	["hooks"]       =
	{
		[HOOK_CALC_BONUS] = function()
			player.modify_stat(A_INT, 1)
			player.modify_stat(A_CHR, -2)
		end,
	},
}

CORRUPT_DEMON_HIDE = add_corruption
{
	["color"]       = TERM_RED,
	["name"]	= "Demon Hide",
	["get_text"]    = "Your skin grows into a thick hide.",
	["lose_text"]   = "Your skin returns to a natural state.",
	["desc"]	=
	{
			  "  Increases your armour class by your level",
			  "  Provides immunity to fire at level 40",
			  "  But reduces speed by your level / 7",
	},
	["hooks"]       =
	{
		[HOOK_CALC_BONUS] = function()
			player.to_a = player.to_a + player.lev
			player.dis_to_a = player.dis_to_a + player.lev
			player.pspeed = player.pspeed - (player.lev / 7)
			if player.lev >= 40 then player.xtra_f2 = bor(player.xtra_f2, TR2_IM_FIRE) end
		end,
	},
}

CORRUPT_DEMON_BREATH = add_corruption
{
	["color"]       = TERM_RED,
	["name"]	= "Demon Breath",
	["get_text"]    = "Your breath becomes mephitic.",
	["lose_text"]   = "Your breath is once again normal.",
	["desc"]	=
	{
			  "  Provides fire breath",
			  "  But gives a small chance to spoil potions when you quaff them",
	},
	["hooks"]       =
	{
		[HOOK_CALC_POWERS] = function()
			player.add_power(PWR_BR_FIRE)
		end,
		[HOOK_QUAFF] = function(obj)
			if magik(9) == TRUE then
				msg_print("Your demon breath spoils the potion!")
				return TRUE, FALSE
			else
				return FALSE
			end
		end,
	},
}

CORRUPT_DEMON_REALM = add_corruption
{
	["color"]       = TERM_L_RED,
	["name"]	= "Demon Realm",
	["get_text"]    = "You feel more attuned to the demon realm.",
	["lose_text"]   = "You lose your attunement to the demon realm.",
	["desc"]	=
	{
			  "  Provides access to the demon school skill and the use of demonic equipment",
			  "  You need Demon Spirit, Demon Hide and Demon Breath to activate it"
	},
	["depends"]     =
	{
			[CORRUPT_DEMON_SPIRIT] = TRUE,
			[CORRUPT_DEMON_HIDE] = TRUE,
			[CORRUPT_DEMON_BREATH] = TRUE
	},
	["hooks"]       =
	{
		[HOOK_CALC_BONUS] = function()
			-- 1500 may seem a lot, but people are rather unlikely to get the corruption very soon
			-- due to the dependencies
			if s_info[SKILL_DAEMON + 1].mod == 0 then s_info[SKILL_DAEMON + 1].mod = 1500 end
			s_info[SKILL_DAEMON + 1].hidden = FALSE;
		end,
	},
}


-- Teleportation corruptions

-- Random teleportation will ask for confirmation 70% of the time
-- But 30% of the time it will teleport, without asking
CORRUPT_RANDOM_TELEPORT = add_corruption
{
	["color"]       = TERM_GREEN,
	["name"]	= "Random teleportation",
	["get_text"]    = "Space seems to fizzle around you.",
	["lose_text"]   = "Space solidify again around you.",
	["desc"]	=
	{
			  "  Randomly teleports you around",
	},
	-- No oppose field, it will be automatically set when we declare the anti-telep corruption to oppose us
	["hooks"]       =
	{
		[HOOK_CALC_BONUS] = function()
			player.xtra_f3 = bor(player.xtra_f3, TR3_TELEPORT)
		end,
		[HOOK_PROCESS_WORLD] = function()
			if rand_int(300) == 1 then
				if magik(70) == TRUE then
					if get_check("Teleport?") == TRUE then
						teleport_player(50)
					end
				else
					disturb(0, 0)
					msg_print("Your corruption takes over you, you teleport!")
					teleport_player(50)
				end
			end
		end,
	},
}

-- Anti-teleportation corruption, can be stopped with this power
CORRUPT_ANTI_TELEPORT = add_corruption
{
	["color"]       = TERM_GREEN,
	["name"]	= "Anti-teleportation",
	["get_text"]    = "Space continuum freezes around you.",
	["lose_text"]   = "Space continuum can once more be altered around you.",
	["desc"]	=
	{
			  "  Prevents all teleportations, be it of you or monsters",
	},
	["oppose"]     	=
	{
			[CORRUPT_RANDOM_TELEPORT] = TRUE
	},
	["hooks"]       =
	{
		[HOOK_BIRTH_OBJECTS] = function()
			player.corrupt_anti_teleport_stopped = FALSE
		end,
		[HOOK_CALC_POWERS] = function()
			player.add_power(POWER_COR_SPACE_TIME)
		end,
		[HOOK_CALC_BONUS] = function()
			if player.corrupt_anti_teleport_stopped == FALSE then
				player.resist_continuum = TRUE
			end
		end,
		[HOOK_PROCESS_WORLD] = function()
			if player.corrupt_anti_teleport_stopped == TRUE then
				local amt = player.msp + player.csp
				amt = amt / 100
				if (amt < 1) then amt = 1 end
				increase_mana(-amt)
				if player.csp == 0 then
					player.corrupt_anti_teleport_stopped = FALSE
					msg_print("You stop controlling your corruption.")
					player.update = bor(player.update, PU_BONUS)
				end
			end
		end,
	},
}


-- Troll blood
CORRUPT_TROLL_BLOOD = add_corruption
{
	["color"]       = TERM_GREEN,
	["name"]	= "Troll Blood",
	["get_text"]    = "Your blood thickens, you sense corruption in it.",
	["lose_text"]   = "Your blood returns to a normal state.",
	["desc"]	=
	{
			  "  Troll blood flows in your veins, granting increased regeneration",
			  "  It also enables you to feel the presence of other troll beings",
			  "  But it will make your presence more noticeable and aggravating",
	},
	["can_gain"] =	function()
			-- Ok trolls should not get this one. never.
			if get_race_name() == "Troll" then
				return nil
			else
				return not nil
			end
	end,
	["hooks"]       =
	{
		[HOOK_CALC_BONUS] = function()
			player.xtra_f3 = bor(player.xtra_f3, TR3_REGEN, TR3_AGGRAVATE)
			player.xtra_esp = bor(player.xtra_esp, ESP_TROLL)
		end,
	},
}

-- The vampire corruption set
CORRUPT_VAMPIRE_TEETH = add_corruption
{
	["group"]       = "Vampire",
	["removable"]   = FALSE,
	["color"]       = TERM_L_DARK,
	["name"]	= "Vampiric Teeth",
	["get_text"]    = "You grow vampiric teeth!",
	["lose_text"]   = "BUG! this should not happen",
	["desc"]	=
	{
			  "  Your teeth allow you to drain blood to feed yourself",
			  "  However your stomach now only accepts blood.",
	},
	["allow"]       = function()
		if test_race_flags(1, PR1_NO_SUBRACE_CHANGE) == FALSE then return not nil else return nil end
	end,
	["gain"]	= function()
		switch_subrace(SUBRACE_SAVE, TRUE);

		subrace_add_power(subrace(SUBRACE_SAVE), PWR_VAMPIRISM)
		subrace(SUBRACE_SAVE).flags1 = bor(subrace(SUBRACE_SAVE).flags1, PR1_VAMPIRE, PR1_UNDEAD, PR1_NO_SUBRACE_CHANGE)
	end,
	["hooks"]       =
	{
	},
}
CORRUPT_VAMPIRE_STRENGTH = add_corruption
{
	["group"]       = "Vampire",
	["removable"]   = FALSE,
	["color"]       = TERM_L_DARK,
	["name"]	= "Vampiric Strength",
	["get_text"]    = "Your body seems more dead than alive.",
	["lose_text"]   = "BUG! this should not happen",
	["desc"]	=
	{
			  "  Your body seems somewhat dead",
			  "  In this near undead state it has improved strength, constitution and intelligence",
			  "  But reduced dexterity, wisdom and charisma.",
	},
	["depends"]     =
	{
			[CORRUPT_VAMPIRE_TEETH] = TRUE,
	},
	["gain"]	= function()
		-- Apply the bonuses/penalities
		subrace(SUBRACE_SAVE).r_mhp = subrace(SUBRACE_SAVE).r_mhp + 1
		subrace(SUBRACE_SAVE).r_exp = subrace(SUBRACE_SAVE).r_exp + 100

		subrace(SUBRACE_SAVE).r_adj[A_STR + 1] = subrace(SUBRACE_SAVE).r_adj[A_STR + 1] + 3
		subrace(SUBRACE_SAVE).r_adj[A_INT + 1] = subrace(SUBRACE_SAVE).r_adj[A_INT + 1] + 2
		subrace(SUBRACE_SAVE).r_adj[A_WIS + 1] = subrace(SUBRACE_SAVE).r_adj[A_WIS + 1] - 3
		subrace(SUBRACE_SAVE).r_adj[A_DEX + 1] = subrace(SUBRACE_SAVE).r_adj[A_DEX + 1] - 2
		subrace(SUBRACE_SAVE).r_adj[A_CON + 1] = subrace(SUBRACE_SAVE).r_adj[A_CON + 1] + 1
		subrace(SUBRACE_SAVE).r_adj[A_CHR + 1] = subrace(SUBRACE_SAVE).r_adj[A_CHR + 1] - 4

		-- be reborn!
		do_rebirth()
		cmsg_print(TERM_L_DARK, "You feel death slipping inside.")
	end,
	["hooks"]       =
	{
	},
}
CORRUPT_VAMPIRE_VAMPIRE = add_corruption
{
	["group"]       = "Vampire",
	["removable"]   = FALSE,
	["color"]       = TERM_L_DARK,
	["name"]	= "Vampire",
	["get_text"]    = "You die to be reborn in a Vampire form.",
	["lose_text"]   = "BUG! this should not happen",
	["desc"]	=
	{
			  "  You are a Vampire. As such you resist cold, poison, darkness and nether.",
			  "  Your life is sustained, but you cannot stand the light of the sun."
	},
	["depends"]     =
	{
			[CORRUPT_VAMPIRE_STRENGTH] = TRUE,
	},
	["gain"]	= function()
		-- Be a Vampire and be proud of it
		local title = get_subrace_title(SUBRACE_SAVE)
		if title == " " or title == "Vampire" then
			title = "Vampire"
			subrace(SUBRACE_SAVE).place = FALSE
		else
			title = "Vampire "..title
		end
		set_subrace_title(SUBRACE_SAVE, title)

		-- Bonus/and .. not bonus :)
		subrace(SUBRACE_SAVE).flags1 = bor(subrace(SUBRACE_SAVE).flags1, PR1_HURT_LITE)
		subrace(SUBRACE_SAVE).oflags2[2] = bor(subrace(SUBRACE_SAVE).oflags2[2], TR2_RES_POIS, TR2_RES_NETHER, TR2_RES_COLD, TR2_RES_DARK, TR2_HOLD_LIFE)
		subrace(SUBRACE_SAVE).oflags3[2] = bor(subrace(SUBRACE_SAVE).oflags3[2], TR3_LITE1)
	end,
	["hooks"]       =
	{
	},
}


--[[
CORRUPT_ = add_corruption
{
	["color"]       = TERM_GREEN,
	["name"]	= "",
	["get_text"]    = "",
	["lose_text"]   = "",
	["desc"]	=
	{
			  "  ",
	},
	["hooks"]       =
	{
		[HOOK_CALC_BONUS] = function()
		end,
	},
}
]]
