-- Spells that are stick or artifacts/... only

DEVICE_HEAL_MONSTER = add_spell
{
	["name"] = 	"Heal Monster",
	["school"] = 	{SCHOOL_DEVICE},
	["level"] = 	3,
	["mana"] = 	5,
	["mana_max"] = 	20,
	["fail"] = 	15,
	["random"] =    -1,
	["stick"] =
	{
			["charge"] =    { 10, 10 },
			[TV_WAND] =
			{
				["rarity"] = 		17,
				["base_level"] =	{ 1, 15 },
				["max_level"] =		{ 20, 50 },
			},
	},
	["spell"] = 	function()
			local ret, dir = get_aim_dir()
			if ret == FALSE then return end

			return fire_ball(GF_OLD_HEAL, dir, 20 + get_level(DEVICE_HEAL_MONSTER, 380), 0)
	end,
	["info"] = 	function()
			return "heal "..(20 + get_level(DEVICE_HEAL_MONSTER, 380))
	end,
	["desc"] =	{
			"Heals a monster",
	}
}

DEVICE_SPEED_MONSTER = add_spell
{
	["name"] = 	"Haste Monster",
	["school"] = 	{SCHOOL_DEVICE},
	["level"] = 	10,
	["mana"] = 	10,
	["mana_max"] = 	10,
	["fail"] = 	30,
	["random"] =    -1,
	["stick"] =
	{
			["charge"] =    { 10, 5 },
			[TV_WAND] =
			{
				["rarity"] = 		7,
				["base_level"] =	{ 1, 1 },
				["max_level"] =		{ 20, 50 },
			},
	},
	["spell"] = 	function()
			local ret, dir = get_aim_dir()
			if ret == FALSE then return end

			return fire_ball(GF_OLD_SPEED, dir, 1, 0)
	end,
	["info"] = 	function()
			return "speed +10"
	end,
	["desc"] =	{
			"Haste a monster",
	}
}

DEVICE_WISH = add_spell
{
	["name"] = 	"Wish",
	["school"] = 	{SCHOOL_DEVICE},
	["level"] = 	50,
	["mana"] = 	400,
	["mana_max"] = 	400,
	["fail"] = 	99,
	["random"] =    -1,
	["stick"] =
	{
			["charge"] =    { 1, 2 },
			[TV_STAFF] =
			{
				["rarity"] = 		98,
				["base_level"] =	{ 1, 1 },
				["max_level"] =		{ 1, 1 },
			},
	},
	["spell"] = 	function()
			make_wish()
			return TRUE
	end,
	["info"] = 	function()
			return ""
	end,
	["desc"] =	{
			"This grants you a wish, beware of what you ask for!",
	}
}

DEVICE_SUMMON = add_spell
{
	["name"] = 	"Summon",
	["school"] = 	{SCHOOL_DEVICE},
	["level"] = 	5,
	["mana"] = 	5,
	["mana_max"] = 	25,
	["fail"] = 	20,
	["random"] =    -1,
	["stick"] =
	{
			["charge"] =    { 1, 20 },
			[TV_STAFF] =
			{
				["rarity"] = 		13,
				["base_level"] =	{ 1, 40 },
				["max_level"] =		{ 25, 50 },
			},
	},
	["spell"] = 	function()
			local i, obvious
			obvious = nil
			for i = 1, 4 + get_level(DEVICE_SUMMON, 30) do
				obvious = is_obvious(summon_specific(player.py, player.px, dun_level, 0), obvious)
			end
			return obvious
	end,
	["info"] = 	function()
			return ""
	end,
	["desc"] =	{
			"Summons hostile monsters near you",
	}
}

DEVICE_MANA = add_spell
{
	["name"] = 	"Mana",
	["school"] = 	{SCHOOL_DEVICE},
	["level"] = 	30,
	["mana"] = 	1,
	["mana_max"] = 	1,
	["fail"] = 	80,
	["random"] =    -1,
	["stick"] =
	{
			["charge"] =    { 2, 3 },
			[TV_STAFF] =
			{
				["rarity"] = 		78,
				["base_level"] =	{ 1, 5 },
				["max_level"] =		{ 20, 35 },
			},
	},
	["spell"] = 	function()
			increase_mana((player.msp * (20 + get_level(DEVICE_MANA, 50))) / 100)
			return TRUE
	end,
	["info"] = 	function()
			return "restore "..(20 + get_level(DEVICE_MANA, 50)).."%"
	end,
	["desc"] =	{
			"Restores a part(or all) of your mana",
	}
}

DEVICE_NOTHING = add_spell
{
	["name"] = 	"Nothing",
	["school"] = 	{SCHOOL_DEVICE},
	["level"] = 	1,
	["mana"] = 	0,
	["mana_max"] = 	0,
	["fail"] = 	0,
	["random"] =    -1,
	["stick"] =
	{
			["charge"] =    { 0, 0 },
			[TV_WAND] =
			{
				["rarity"] = 		3,
				["base_level"] =	{ 1, 1 },
				["max_level"] =		{ 1, 1 },
			},
			[TV_STAFF] =
			{
				["rarity"] = 		3,
				["base_level"] =	{ 1, 1 },
				["max_level"] =		{ 1, 1},
			},
	},
	["spell"] = 	function()
			return FALSE
	end,
	["info"] = 	function()
			return ""
	end,
	["desc"] =	{
			"It does nothing.",
	}
}

DEVICE_LEBOHAUM = add_spell
{
	["name"] = 	"Artifact Lebauhaum",
	["school"] = 	{SCHOOL_DEVICE},
	["level"] = 	1,
	["mana"] = 	0,
	["mana_max"] = 	0,
	["fail"] = 	0,
	["random"] =    -1,
	["activate"] =  3,
	["spell"] = 	function()
			msg_print("You hear a little song in your head in some unknown tongue:")
			msg_print("'Avec le casque Lebohaum y a jamais d'anicroches, je parcours les dongeons,")
			msg_print("j'en prend plein la caboche. Avec le casque Lebohaum, tout ces monstres a la")
			msg_print("con, je leur met bien profond: c'est moi le maitre du dongeon!'")
	end,
	["info"] = 	function()
			return ""
	end,
	["desc"] =	{
			"sing a cheerful song",
	}
}

DEVICE_MAGGOT = add_spell
{
	["name"] = 	"Artifact Maggot",
	["school"] = 	{SCHOOL_DEVICE},
	["level"] = 	1,
	["mana"] = 	7,
	["mana_max"] = 	7,
	["fail"] = 	20,
	["random"] =    -1,
	["activate"] =  { 10, 50 },
	["spell"] = 	function()
			local ret, dir = get_aim_dir()
			if ret == FALSE then return end
			return fire_ball(GF_TURN_ALL, dir, 40, 2)
	end,
	["info"] = 	function()
			return "power 40 rad 2"
	end,
	["desc"] =	{
			"terrify",
	}
}

DEVICE_HOLY_FIRE = add_spell
{
	["name"] = 	"Holy Fire of Mithrandir",
	["school"] = 	{SCHOOL_DEVICE},
	["level"] = 	30,
	["mana"] = 	50,
	["mana_max"] = 	150,
	["fail"] = 	75,
	["random"] =    -1,
	["stick"] =
	{
			["charge"] =    { 2, 5 },
			[TV_STAFF] =
			{
				-- Rarity higher than 100 to be sure to not have it generated randomly
				["rarity"] = 		999,
				["base_level"] =	{ 1, 1 },
				["max_level"] =		{ 35, 35 },
			},
	},
	["spell"] = 	function()
			return project_los(GF_HOLY_FIRE, 50 + get_level(DEVICE_HOLY_FIRE, 300))
	end,
	["info"] = 	function()
			return "dam "..(50 + get_level(DEVICE_HOLY_FIRE, 250))
	end,
	["desc"] =	{
			"The Holy Fire created by this staff will deeply(double damage) burn",
			"all that is evil.",
	}
}

-- Ok the Eternal Flame, to craete one of the 4 ultimate arts
-- needed to enter the last level of the Void
DEVICE_ETERNAL_FLAME = add_spell
{
	["name"] = 	"Artifact Eternal Flame",
	["school"] = 	{SCHOOL_DEVICE},
	["level"] = 	1,
	["mana"] = 	0,
	["mana_max"] = 	0,
	["fail"] = 	0,
	["random"] =    -1,
	["activate"] =  { 0, 0 },
	["spell"] = 	function(flame_item)
			local ret, item, obj

			ret, item = get_item("Which object do you want to imbue?",
					     "You have no objects to imbue.",
					     bor(USE_INVEN),
					     function (obj)
						if obj.name1 > 0 or obj.name2 > 0 then return FALSE end
					     	if (obj.tval == TV_SWORD) and (obj.sval == SV_LONG_SWORD) then
							return TRUE
					     	elseif (obj.tval == TV_MSTAFF) and (obj.sval == SV_MSTAFF) then
							return TRUE
					     	elseif (obj.tval == TV_BOW) and (obj.sval == SV_HEAVY_XBOW) then
							return TRUE
					     	elseif (obj.tval == TV_DRAG_ARMOR) and (obj.sval == SV_DRAGON_POWER) then
							return TRUE
					     	end
						return FALSE
					     end
			)
			if ret == FALSE then return FALSE end

			obj = get_object(item)

			if (obj.tval == TV_SWORD) and (obj.sval == SV_LONG_SWORD) then
				obj.name1 = 147
			elseif (obj.tval == TV_MSTAFF) and (obj.sval == SV_MSTAFF) then
				obj.name1 = 127
			elseif (obj.tval == TV_BOW) and (obj.sval == SV_HEAVY_XBOW) then
				obj.name1 = 152
			elseif (obj.tval == TV_DRAG_ARMOR) and (obj.sval == SV_DRAGON_POWER) then
				obj.name1 = 17
			end
			apply_magic(obj, -1, TRUE, TRUE, TRUE)

			obj.found = OBJ_FOUND_SELFMADE

			inven_item_increase(flame_item, -1)
			inven_item_describe(flame_item)
			inven_item_optimize(flame_item)

			return TRUE
	end,
	["info"] = 	function()
			return ""
	end,
	["desc"] =	{
			"imbuing an object with the eternal fire",
	}
}

-- And one more silly activation :)
DEVICE_DURANDIL = add_spell
{
	["name"] = 	"Artifact Durandil",
	["school"] = 	{SCHOOL_DEVICE},
	["level"] = 	1,
	["mana"] = 	0,
	["mana_max"] = 	0,
	["fail"] = 	0,
	["random"] =    -1,
	["activate"] =  3,
	["spell"] = 	function()
			msg_print("You hear a little song in your head in some unknown tongue:")
			msg_print("'Les epees Durandils sont forgees dans les mines par des nains.")
			msg_print("Avec ca c'est facile de tuer un troll avec une seule main. Pas besoin")
			msg_print("de super entrainement nis de niveau 28. Quand tu sors l'instrument")
			msg_print("c'est l'ennemi qui prend la fuite! Avec ton epee Durandil quand tu")
			msg_print("parcours les chemins, tu massacre sans peine les brigands et les gobelins,")
			msg_print("les rats geants, les ogres mutants, les zombies et les liches, tu les")
			msg_print("decoupe en tranches comme si c'etait des parts de quiches.")
			msg_print("Les epees Durandil! Les epees Durandil!")
			msg_print("Quand tu la sort dans un dongeon au moins t'as pas l'air debile.")
			msg_print("C'est l'arme des bourins qui savent etre subtils.")
			msg_print("Ne partez pas a l'aventure sans votre epee Durandil!'")
	end,
	["info"] = 	function()
			return ""
	end,
	["desc"] =	{
			"sing a cheerful song",
	}
}

DEVICE_THUNDERLORDS = add_spell
{
	["name"] = 	"Artifact Thunderlords",
	["school"] = 	{SCHOOL_DEVICE},
	["level"] = 	1,
	["mana"] = 	1,
	["mana_max"] = 	1,
	["fail"] = 	20,
	["random"] =    -1,
	["stick"] =
	{
			["charge"] =    { 3, 3 },
			[TV_STAFF] =
			{
				-- Rarity higher than 100 to be sure to not have it generated randomly
				["rarity"] = 		999,
				["base_level"] =	{ 1, 1 },
				["max_level"] =		{ 1, 1 },
			},
	},
	["spell"] = 	function()
			if dun_level > 0 then
				msg_print("As you blow the horn a thunderlord pops out of nowhere and grabs you.")
				recall_player(0, 1)
			else
				msg_print("You cannot use it there.")
			end
			return TRUE
	end,
	["info"] = 	function()
			return ""
	end,
	["desc"] =	{
			"A thunderlord will appear to transport you quickly to the surface.",
	}
}

--[[ Template
DEVICE_ = add_spell
{
	["name"] = 	"",
	["school"] = 	{SCHOOL_DEVICE},
	["level"] = 	1,
	["mana"] = 	2,
	["mana_max"] = 	15,
	["fail"] = 	10,
	["random"] =    -1,
	["stick"] =
	{
			["charge"] =    { 10, 5 },
			[TV_STAFF] =
			{
				["rarity"] = 		7,
				["base_level"] =	{ 1, 15 },
				["max_level"] =		{ 25, 50 },
			},
	},
	["spell"] = 	function()
			return FALSE
	end,
	["info"] = 	function()
			return ""
	end,
	["desc"] =	{
			"",
	}
}
]]
