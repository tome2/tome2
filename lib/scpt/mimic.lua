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

			if player.mimic_level >= 20 then player.fly = TRUE end
			if player.mimic_level >= 20 then player.see_inv = TRUE end
			if player.mimic_level >= 25 then player.free_act = TRUE end
			if player.mimic_level >= 30 then player.resist_elec = TRUE end
			if player.mimic_level >= 40 then player.sh_elec = TRUE end

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

			player.free_act = TRUE
			player.resist_fear = TRUE

			if player.mimic_level >= 10 then player.resist_cold = TRUE end
			if player.mimic_level >= 15 then player.see_inv = TRUE end
			if player.mimic_level >= 30 then player.resist_dark = TRUE end
			if player.mimic_level >= 35 then player.resist_conf = TRUE end

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

			player.resist_pois = TRUE
			player.resist_fear = TRUE
			player.resist_dark = TRUE

			if player.mimic_level >= 40 then player.climb = TRUE end

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

			player.resist_pois = TRUE
			player.resist_cold = TRUE
			player.free_act = TRUE
			player.regenerate = TRUE
			player.see_inv = TRUE
			player.sensible_fire = TRUE

			player.modify_stat(A_STR, player.mimic_level / 5)
			player.modify_stat(A_INT, - (player.mimic_level / 7))
			player.modify_stat(A_WIS, - (player.mimic_level / 7))
			player.modify_stat(A_DEX, -4)
			player.modify_stat(A_CON, player.mimic_level / 5)
			player.modify_stat(A_CHR, -7)

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

			-- But they are stealthy
			player.skill_stl = player.skill_stl + 10 + (player.mimic_level / 5)
			player.resist_pois = TRUE
			player.immune_cold = TRUE
			player.resist_shard = TRUE
			player.free_act = TRUE
			player.regenerate = TRUE
			player.see_inv = TRUE
			player.sensible_fire = TRUE
			player.levitate = TRUE

			-- Stat mods
			player.modify_stat(A_STR, -4)
			player.modify_stat(A_DEX, 5)
			player.modify_stat(A_CON, -4)
			player.modify_stat(A_CHR, -10)
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

			player.resist_pois = TRUE
			if player.mimic_level >= 25 then player.free_act = TRUE end
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

			if player.mimic_level >= 10 then player.resist_fear = TRUE end
			if player.mimic_level >= 25 then player.resist_conf = TRUE end
			if player.mimic_level >= 30 then player.free_act = TRUE end
			if player.mimic_level >= 35 then player.resist_nexus = TRUE end
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

			if player.mimic_level >= 10 then player.free_act = TRUE end
			if player.mimic_level >= 20 then player.regenerate = TRUE end
			if player.mimic_level >= 30 then player.resist_conf = TRUE end
			if player.mimic_level >= 35 then player.resist_nexus = TRUE end

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

			player.immune_fire = TRUE
			player.immune_elec = TRUE
			player.immune_acid = TRUE
			player.resist_pois = TRUE
			player.resist_dark = TRUE
			player.resist_chaos = TRUE
			player.hold_life = TRUE
			player.ffall =  TRUE
			player.regenerate = TRUE
			player.sh_fire = TRUE
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

			player.immune_fire = TRUE
			player.immune_elec = TRUE
			player.immune_acid = TRUE
			player.immune_cold = TRUE
			player.resist_pois = TRUE
			player.resist_lite = TRUE
			player.resist_dark = TRUE
			player.resist_chaos = TRUE
			player.hold_life = TRUE
			player.ffall = TRUE
			player.regenerate = TRUE
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

			player.immune_fire = TRUE
			-- was immune to poison in the 3.0.0 version
			player.resist_pois = TRUE
			player.sh_fire = TRUE
			player.lite = TRUE
			return 0
	end,
}
