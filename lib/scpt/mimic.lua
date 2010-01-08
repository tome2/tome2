-- Define the various possible mimic shapes

-- Nature shapes
add_mimic_shape
{
	["name"] =      "Mouse",
	["obj_name"] =  "Mouse Fur",
	["desc"] = 	"Mice are small, fast and very stealthy",
	["realm"] =     "nature",
	["level"] =     1,
	["rarity"] =    10,
	["duration"] =  {20, 40},
	["calc"] =      function ()
			-- Mice run!
			player.pspeed = player.pspeed + 5 + (player.mimic_level / 7)

			-- They can crtawl under your armor to hit you ;)
			player.to_h = player.to_h + 10 + (player.mimic_level / 5)
			player.dis_to_h = player.dis_to_h + 10 + (player.mimic_level / 5)

			-- But they are not very powerfull
			player.to_d = player.to_d / 5
			player.dis_to_d = player.dis_to_d / 5

			-- But they are stealthy
			player.skill_stl = player.skill_stl + 10 + (player.mimic_level / 5)

			-- Stat mods
			player.modify_stat(A_STR, -5)
			player.modify_stat(A_DEX, 3)
			player.modify_stat(A_CON, 1)

	end,
	["power"] =     function()
			if player.mimic_level >= 30 then
				player.add_power(POWER_INVISIBILITY)
			end
	end,
}

add_mimic_shape
{
	["name"] =      "Eagle",
	["obj_name"] =  "Feathers Cloak",
	["desc"] = 	"Eagles are master of the air, good hunters with excellent vision.",
	["realm"] =     "nature",
	["level"] =     10,
	["rarity"] =    30,
	["duration"] =  {10, 50},
	["calc"] =      function ()
			player.ffall = TRUE
			player.pspeed = player.pspeed + 2 + (player.mimic_level / 6)

			player.modify_stat(A_STR, -3)
			player.modify_stat(A_DEX, 2 + (player.mimic_level / 15))
			player.modify_stat(A_CON, 4 + (player.mimic_level / 20))
			player.modify_stat(A_INT, -1)
			player.modify_stat(A_WIS, 1)
			player.modify_stat(A_CHR, -1)

			if player.mimic_level >= 20 then
                player.xtra_f4 = bor(player.xtra_f4, TR4_FLY)
                player.xtra_f3 = bor(player.xtra_f3, TR3_SEE_INVIS)
			end
			if player.mimic_level >= 25 then
                player.xtra_f2 = bor(player.xtra_f2, TR2_FREE_ACT)
			end
			if player.mimic_level >= 30 then
                player.xtra_f2 = bor(player.xtra_f2, TR2_RES_ELEC)
			end
			if player.mimic_level >= 30 then
                player.xtra_f3 = bor(player.xtra_f3, TR3_SH_ELEC)
			end
	end,
}

add_mimic_shape
{
	["name"] =      "Wolf",
	["obj_name"] =  "Wolf Pelt",
	["desc"] = 	"Wolves are masters of movement, strong and have excellent eyesight.",
	["realm"] =     "nature",
	["level"] =     20,
	["rarity"] =    40,
	["duration"] =  {10, 50},
	["calc"] =      function ()
			player.modify_stat(A_STR, 2 + (player.mimic_level / 20))
			player.modify_stat(A_DEX, 3 + (player.mimic_level / 20))
			player.modify_stat(A_INT, -3)
			player.modify_stat(A_CHR, -2)

			player.pspeed = player.pspeed + 10 + (player.mimic_level / 5)

            player.xtra_f2 = bor(player.xtra_f2, TR2_FREE_ACT)
            player.xtra_f2 = bor(player.xtra_f2, TR2_RES_FEAR)

			if player.mimic_level >= 10 then
				player.xtra_f2 = bor(player.xtra_f2, TR2_RES_COLD)
			end
			if player.mimic_level >= 15 then
                player.xtra_f3 = bor(player.xtra_f3, TR3_SEE_INVIS)
			end
			if player.mimic_level >= 30 then
				player.xtra_f2 = bor(player.xtra_f2, TR2_RES_DARK)
			end
			if player.mimic_level >= 35 then
				player.xtra_f2 = bor(player.xtra_f2, TR2_RES_CONF)
			end
	end,
}

add_mimic_shape
{
	["name"] =      "Spider",
	["obj_name"] =  "Spider Web",
	["desc"] = 	"Spiders are clever and become good climbers.",
	["realm"] =     "nature",
	["level"] =     25,
	["rarity"] =    50,
	["duration"] =  {10, 50},
	["calc"] =      function ()
			player.modify_stat(A_STR, -4)
			player.modify_stat(A_DEX, 1 + (player.mimic_level / 8))
			player.modify_stat(A_INT, 1 + (player.mimic_level / 5))
			player.modify_stat(A_WIS, 1 + (player.mimic_level / 5))
			player.modify_stat(A_CON, -5)
			player.modify_stat(A_CHR, -10)

			player.pspeed = player.pspeed + 5

			player.xtra_f2 = bor(player.xtra_f2, TR2_RES_POIS)
			player.xtra_f2 = bor(player.xtra_f2, TR2_RES_FEAR)
			player.xtra_f2 = bor(player.xtra_f2, TR2_RES_DARK)

			if player.mimic_level >= 40 then
                player.xtra_f4 = bor(player.xtra_f4, TR4_CLIMB)
			end

	end,
	["power"] =     function()
			if player.mimic_level >= 25 then
				player.add_power(POWER_WEB)
			end
	end,
}

add_mimic_shape
{
	["name"] =      "Elder Ent",
	["obj_name"] =  "Entish Bark",
	["desc"] = 	"Ents are powerful tree-like beings dating from the dawn of time.",
	["realm"] =     "nature",
	["level"] =     40,
	["rarity"] =    60,
	["duration"] =  {10, 30},
	["limit"] =     TRUE,
	["calc"] =      function ()
			player.pspeed = player.pspeed - 5 - (player.mimic_level / 10)

			player.to_a = player.to_a + 10 + player.mimic_level
			player.dis_to_a = player.dis_to_a + 10 + player.mimic_level

			player.modify_stat(A_STR, player.mimic_level / 5)
			player.modify_stat(A_INT, - (player.mimic_level / 7))
			player.modify_stat(A_WIS, - (player.mimic_level / 7))
			player.modify_stat(A_DEX, -4)
			player.modify_stat(A_CON, player.mimic_level / 5)
			player.modify_stat(A_CHR, -7)

			player.xtra_f2 = bor(player.xtra_f2, TR2_RES_POIS)
			player.xtra_f2 = bor(player.xtra_f2, TR2_RES_COLD)
            player.xtra_f2 = bor(player.xtra_f2, TR2_FREE_ACT)
            player.xtra_f3 = bor(player.xtra_f3, TR3_REGEN)
            player.xtra_f3 = bor(player.xtra_f3, TR3_SEE_INVIS)
            player.xtra_f2 = bor(player.xtra_f2, TR2_SENS_FIRE)
	end,
	["power"] =     function ()
			player.add_power(PWR_GROW_TREE)
	end,
}

add_mimic_shape
{
	["name"] =      "Vapour",
	["obj_name"] =  "Cloak of Mist",
	["desc"] = 	"A sentient cloud, darting around",
	["realm"] =     "nature",
	["level"] =     15,
	["rarity"] =    10,
	["duration"] =  {10, 40},
	["calc"] =      function ()

			player.pspeed = player.pspeed + 5

			--Try to hit a cloud!
			player.to_a = player.to_a + 40 + player.mimic_level
			player.dis_to_a = player.dis_to_a + 40 + player.mimic_level

			--Try to hit WITH a cloud!
			player.to_h = player.to_h - 40
			player.dis_to_h = player.dis_to_h -40

			-- Stat mods
			player.modify_stat(A_STR, -4)
			player.modify_stat(A_DEX, 5)
			player.modify_stat(A_CON, -4)
			player.modify_stat(A_CHR, -10)

			-- But they are stealthy
			player.skill_stl = player.skill_stl + 10 + (player.mimic_level / 5)
			player.xtra_f2 = bor(player.xtra_f2, TR2_RES_POIS)
			player.xtra_f2 = bor(player.xtra_f2, TR2_RES_SHARDS)
			player.xtra_f2 = bor(player.xtra_f2, TR2_IM_COLD)
            player.xtra_f2 = bor(player.xtra_f2, TR2_FREE_ACT)
            player.xtra_f3 = bor(player.xtra_f3, TR3_REGEN)
            player.xtra_f3 = bor(player.xtra_f3, TR3_SEE_INVIS)
            player.xtra_f2 = bor(player.xtra_f2, TR2_SENS_FIRE)
            player.xtra_f3 = bor(player.xtra_f3, TR3_FEATHER)
	end,
}

add_mimic_shape
{
	["name"] =      "Serpent",
	["obj_name"] =  "Snakeskin Cloak",
	["desc"] = 	"Serpents are fast, lethal predators.",
	["realm"] =     "nature",
	["level"] =     30,
	["rarity"] =    25,
	["duration"] =  {15, 20},
	["calc"] =      function ()
			player.pspeed = player.pspeed + 10 + (player.mimic_level / 6)
			player.to_a = player.to_a + 3 + (player.mimic_level / 8)
			player.dis_to_a = player.dis_to_a + 3 + (player.mimic_level / 8)

			player.modify_stat(A_STR, player.mimic_level / 8)
			player.modify_stat(A_INT, -6)
			player.modify_stat(A_WIS, -6)
			player.modify_stat(A_DEX, -4)
			player.modify_stat(A_CON, player.mimic_level / 7)
			player.modify_stat(A_CHR, -6)

			player.xtra_f2 = bor(player.xtra_f2, TR2_RES_POIS)
			if player.mimic_level >= 25 then
            	player.xtra_f2 = bor(player.xtra_f2, TR2_FREE_ACT)
			end
	end,
}

add_mimic_shape
{
	["name"] =      "Mumak",
	["obj_name"] =  "Mumak Hide",
	["desc"] = 	"A giant, elaphantine form.",
	["realm"] =     "nature",
	["level"] =     40,
	["rarity"] =    40,
	["duration"] =  {15, 20},
	["calc"] =      function ()
			player.pspeed = player.pspeed - 5 - (player.mimic_level / 10)
			player.to_a = player.to_a + 10 + (player.mimic_level / 6)
			player.dis_to_a = player.dis_to_a + 10 + (player.mimic_level / 6)
			player.to_d = player.to_d + 5 + ((player.mimic_level * 2) / 3)
			player.dis_to_d = player.dis_to_d + 5 + ((player.mimic_level * 2) / 3)

			player.modify_stat(A_STR, player.mimic_level / 4)
			player.modify_stat(A_INT, -8)
			player.modify_stat(A_WIS, -4)
			player.modify_stat(A_DEX, -5)
			player.modify_stat(A_CON, player.mimic_level / 3)
			player.modify_stat(A_CHR, -10)

			if player.mimic_level >= 10 then
				player.xtra_f2 = bor(player.xtra_f2, TR2_RES_FEAR)
			end
			if player.mimic_level >= 25 then
				player.xtra_f2 = bor(player.xtra_f2, TR2_RES_CONF)
			end
			if player.mimic_level >= 30 then
            	player.xtra_f2 = bor(player.xtra_f2, TR2_FREE_ACT)
			end
			if player.mimic_level >= 35 then
				player.xtra_f2 = bor(player.xtra_f2, TR2_RES_NEXUS)
			end
	end,
}

--------- Extra shapes -----------

-- For Beornings
add_mimic_shape
{
	["name"] =      "Bear",
	["desc"] = 	"A fierce, terrible bear.",
	["realm"] =     nil,
	["level"] =     1,
	["rarity"] =    101,
	["duration"] =  {50, 200},
	["limit"] =     TRUE,
	["calc"] =      function ()
			player.pspeed = player.pspeed - 5 + (player.mimic_level / 5)

			player.to_a = player.to_a + 5 + ((player.mimic_level * 2) / 3)
			player.dis_to_a = player.dis_to_a + 5 + ((player.mimic_level * 2) / 3)

			player.modify_stat(A_STR, player.mimic_level / 11)
			player.modify_stat(A_INT, player.mimic_level / 11)
			player.modify_stat(A_WIS, player.mimic_level / 11)
			player.modify_stat(A_DEX, -1)
			player.modify_stat(A_CON, player.mimic_level / 11)
			player.modify_stat(A_CHR, -10)

			if player.mimic_level >= 10 then
            	player.xtra_f2 = bor(player.xtra_f2, TR2_FREE_ACT)
			end
			if player.mimic_level >= 20 then
	            player.xtra_f3 = bor(player.xtra_f3, TR3_REGEN)
			end
			if player.mimic_level >= 30 then
				player.xtra_f2 = bor(player.xtra_f2, TR2_RES_CONF)
			end
			if player.mimic_level >= 35 then
				player.xtra_f2 = bor(player.xtra_f2, TR2_RES_NEXUS)
			end

			-- activate the skill
			skill(SKILL_BEAR).hidden = FALSE
	end,
}

-- For balrog corruptions
add_mimic_shape
{
	["name"] =      "Balrog",
	["desc"] = 	"A corrupted maia.",
	["realm"] =     nil,
	["level"] =     1,
	["rarity"] =    101,
	["duration"] =  {30, 70},
	["limit"] =     TRUE,
	["calc"] =      function ()
			player.modify_stat(A_STR, 5 + player.mimic_level / 5)
			player.modify_stat(A_INT, player.mimic_level / 10)
			player.modify_stat(A_WIS, - ( 5 + player.mimic_level / 10))
			player.modify_stat(A_DEX, player.mimic_level / 10)
			player.modify_stat(A_CON, 5 + player.mimic_level / 5)
			player.modify_stat(A_CHR, - ( 5 + player.mimic_level / 10))

			player.xtra_f2 = bor(player.xtra_f2, TR2_IM_ACID)
			player.xtra_f2 = bor(player.xtra_f2, TR2_IM_FIRE)
			player.xtra_f2 = bor(player.xtra_f2, TR2_IM_ELEC)
			player.xtra_f2 = bor(player.xtra_f2, TR2_RES_DARK)
			player.xtra_f2 = bor(player.xtra_f2, TR2_RES_CHAOS)
			player.xtra_f2 = bor(player.xtra_f2, TR2_RES_POIS)
			player.xtra_f2 = bor(player.xtra_f2, TR2_HOLD_LIFE)
			player.xtra_f3 = bor(player.xtra_f3, TR3_FEATHER)
			player.xtra_f3 = bor(player.xtra_f3, TR3_REGEN)
			player.xtra_f3 = bor(player.xtra_f3, TR3_SH_FIRE)
			player.xtra_f3 = bor(player.xtra_f3, TR3_LITE1)
			return 1
	end,
}

-- For avatar spell
add_mimic_shape
{
	["name"] =      "Maia",
	["desc"] = 	"A near god-like being.",
	["realm"] =     nil,
	["level"] =     1,
	["rarity"] =    101,
	["duration"] =  {30, 70},
	["limit"] =     TRUE,
	["calc"] =      function ()
			player.modify_stat(A_STR, 5 + player.mimic_level / 5)
			player.modify_stat(A_INT, 5 + player.mimic_level / 5)
			player.modify_stat(A_WIS, 5 + player.mimic_level / 5)
			player.modify_stat(A_DEX, 5 + player.mimic_level / 5)
			player.modify_stat(A_CON, 5 + player.mimic_level / 5)
			player.modify_stat(A_CHR, 5 + player.mimic_level / 5)

			player.xtra_f2 = bor(player.xtra_f2, TR2_IM_FIRE)
			player.xtra_f2 = bor(player.xtra_f2, TR2_IM_ELEC)
			player.xtra_f2 = bor(player.xtra_f2, TR2_IM_ACID)
			player.xtra_f2 = bor(player.xtra_f2, TR2_IM_COLD)
			player.xtra_f2 = bor(player.xtra_f2, TR2_RES_POIS)
			player.xtra_f2 = bor(player.xtra_f2, TR2_RES_LITE)
			player.xtra_f2 = bor(player.xtra_f2, TR2_RES_DARK)
			player.xtra_f2 = bor(player.xtra_f2, TR2_RES_CHAOS)
			player.xtra_f2 = bor(player.xtra_f2, TR2_HOLD_LIFE)
			player.xtra_f3 = bor(player.xtra_f3, TR3_FEATHER)
			player.xtra_f3 = bor(player.xtra_f3, TR3_REGEN)
			return 2
	end,
}

-- For Geomancy
add_mimic_shape
{
	["name"] =      "Fire Elem.",
	["desc"] = 	"A towering column of flames",
	["realm"] =     nil,
	["level"] =     1,
	["rarity"] =    101,
	["duration"] =  {10, 10},
	["limit"] =     TRUE,
	["calc"] =      function ()
			player.modify_stat(A_STR, 5 + (player.mimic_level / 5))
			player.modify_stat(A_DEX, 5 + (player.mimic_level / 5))
			player.modify_stat(A_WIS, -5 - (player.mimic_level / 5))

			player.xtra_f2 = bor(player.xtra_f2, TR2_IM_FIRE)
			-- was immune to poison in the 3.0.0 version
			player.xtra_f2 = bor(player.xtra_f2, TR2_RES_POIS)
			player.xtra_f3 = bor(player.xtra_f3, TR3_SH_FIRE)
			player.xtra_f3 = bor(player.xtra_f3, TR3_LITE1)
			return 0
	end,
}
