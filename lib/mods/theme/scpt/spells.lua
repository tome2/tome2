--
-- This file takes care of the schools of magic
--

-- Create the schools
SCHOOL_MANA = add_school
{
	["name"] = "Mana", 
	["skill"] = SKILL_MANA,
	["spell_power"] = TRUE,
	["sorcery"] = TRUE,
	["gods"] =
	{
		-- Varda provides the Mana school at 1/4 the prayer skill
		[GOD_VARDA] = 
		{
			["skill"] = SKILL_PRAY,
			["mul"] = 1,
			["div"] = 4,
		},
		-- Eru Iluvatar provides the Mana school at half the prayer skill
		[GOD_ERU] =
		{
			["skill"] = SKILL_PRAY,
			["mul"] = 1,
			["div"] = 2,
		},
	},
	["hooks"] =
	{
		[HOOK_CALC_MANA] = function(msp)
			if get_skill(SKILL_MANA) >= 35 then
				msp = msp + (msp * ((get_skill(SKILL_MANA) - 34)) / 100)
				return TRUE, msp
			end
		end
	},
}
SCHOOL_FIRE = add_school
{
	["name"] = "Fire", 
	["skill"] = SKILL_FIRE,
	["spell_power"] = TRUE,
	["sorcery"] = TRUE,
	["gods"] = 
	{
		-- Aule provides the Fire school at 3/5 the prayer skill
		[GOD_AULE] = 
		{
			["skill"] = SKILL_PRAY,
			["mul"] = 3,
			["div"] = 5,
		},
	},
}
SCHOOL_AIR = add_school
{
	["name"] = "Air", 
	["skill"] = SKILL_AIR,
	["spell_power"] = TRUE,
	["sorcery"] = TRUE,
	["gods"] =
	{
		-- Manwe Sulimo provides the Air school at 2/3 the prayer skill
		[GOD_MANWE] =
		{
			["skill"] = SKILL_PRAY,
			["mul"] = 2,
			["div"] = 3,
		},
	},
}
SCHOOL_WATER = add_school
{
	["name"] = "Water", 
	["skill"] = SKILL_WATER,
	["spell_power"] = TRUE,
	["sorcery"] = TRUE,
	["gods"] =
	{
		-- Yavanna Kementari provides the Water school at 1/2 the prayer skill
		[GOD_YAVANNA] =
		{
			["skill"] = SKILL_PRAY,
			["mul"] = 1,
			["div"] = 2,
		},
		-- Ulmo provides the Water school at 3/5 the prayer skill
		[GOD_ULMO] =
		{
			["skill"] = SKILL_PRAY,
			["mul"] = 3,
			["div"] = 5,
		},
	},
}
SCHOOL_EARTH = add_school
{
	["name"] = "Earth", 
	["skill"] = SKILL_EARTH,
	["spell_power"] = TRUE,
	["sorcery"] = TRUE,
	["gods"] =
	{
		-- Tulkas provides the Earth school at 4/5 the prayer skill
		[GOD_TULKAS] =
		{
			["skill"] = SKILL_PRAY,
			["mul"] = 4,
			["div"] = 5,
		},
		-- Yavanna Kementari provides the Earth school at 1/2 the prayer skill
		[GOD_YAVANNA] =
		{
			["skill"] = SKILL_PRAY,
			["mul"] = 1,
			["div"] = 2,
		},

		-- Aule provides the Earth school at 1/3 the prayer skill
		[GOD_AULE] = 
		{
			["skill"] = SKILL_PRAY,
			["mul"] = 1,
			["div"] = 3,
		},
	},
}
SCHOOL_CONVEYANCE = add_school
{
	["name"] = "Conveyance", 
	["skill"] = SKILL_CONVEYANCE,
	["spell_power"] = TRUE,
	["sorcery"] = TRUE,
	["gods"] =
	{
		-- Manwe Sulimo provides the Conveyance school at 1/2 the prayer skill
		[GOD_MANWE] =
		{
			["skill"] = SKILL_PRAY,
			["mul"] = 1,
			["div"] = 2,
		},
	},
}
SCHOOL_GEOMANCY = add_school
{
	["name"] = "Geomancy",
	["skill"] = SKILL_GEOMANCY,
	["spell_power"] = TRUE,
	-- Require to wield a Mage Staff, as the spells requries the caster to stomp the floor with it
	["depend"] = function()
		-- Require at least one point in each school
		if get_skill(SKILL_FIRE) == 0 then return end
		if get_skill(SKILL_AIR) == 0 then return end
		if get_skill(SKILL_EARTH) == 0 then return end
		if get_skill(SKILL_WATER) == 0 then return end

		local obj = get_object(INVEN_WIELD)
		if (obj.k_idx > 0) and (obj.tval == TV_MSTAFF) then return TRUE end
	end,
}
SCHOOL_DIVINATION = add_school
{
	["name"] = "Divination", 
	["skill"] = SKILL_DIVINATION,
	["spell_power"] = TRUE,
	["sorcery"] = TRUE,
	["gods"] =
	{
		-- Eru Iluvatar provides the Divination school at 2/3 the prayer skill
		[GOD_ERU] =
		{
			["skill"] = SKILL_PRAY,
			["mul"] = 2,
			["div"] = 3,
		},
		-- Mandos the Divination school at 1/3 the prayer skill
		[GOD_MANDOS] =
		{
			["skill"] = SKILL_PRAY,
			["mul"] = 1,
			["div"] = 3,
		},
	},
}
SCHOOL_TEMPORAL = add_school
{
	["name"] = "Temporal", 
	["skill"] = SKILL_TEMPORAL,
	["spell_power"] = TRUE,
	["sorcery"] = TRUE,
	["gods"] =
	{
		-- Yavanna Kementari provides the Temporal school at 1/6 the prayer skill
		[GOD_YAVANNA] =
		{
			["skill"] = SKILL_PRAY,
			["mul"] = 1,
			["div"] = 6,
		},
		-- Mandos provides the Temporal school at 1/4 the prayer skill
		[GOD_MANDOS] =
		{
			["skill"] = SKILL_PRAY,
			["mul"] = 1,
			["div"] = 4,
		},
	},
}
SCHOOL_NATURE = add_school
{
	["name"] = "Nature", 
	["skill"] = SKILL_NATURE,
	["spell_power"] = TRUE,
	["sorcery"] = TRUE,
	["gods"] =
	{
		-- Yavanna Kementari provides the Nature school at 1/2 the prayer skill
		[GOD_YAVANNA] =
		{
			["skill"] = SKILL_PRAY,
			["mul"] = 1,
			["div"] = 2,
		},
		-- Ulmo provides the Nature school at 1/2 the prayer skill
		[GOD_ULMO] =
		{
			["skill"] = SKILL_PRAY,
			["mul"] = 1,
			["div"] = 2,
		},
	},
}
SCHOOL_META = add_school
{
	["name"] = "Meta", 
	["skill"] = SKILL_META,
	["spell_power"] = TRUE,
	["sorcery"] = TRUE,
	["gods"] =
	{
		-- Manwe Sulimo provides the Meta school at 1/3 the prayer skill
		[GOD_MANWE] =
		{
			["skill"] = SKILL_PRAY,
			["mul"] = 1,
			["div"] = 3,
		},
		-- Varda provides the Meta school at 1/2 the prayer skill
		[GOD_VARDA] =
		{
			["skill"] = SKILL_PRAY,
			["mul"] = 1,
			["div"] = 2,
		},
	},
}
SCHOOL_MIND = add_school
{
	["name"] = "Mind",
	["skill"] = SKILL_MIND,
	["spell_power"] = TRUE,
	["sorcery"] = TRUE,
	["gods"] =
	{
		-- Eru Iluvatar provides the Mind school at 1/3 the prayer skill
		[GOD_ERU] =
		{
			["skill"] = SKILL_PRAY,
			["mul"] = 1,
			["div"] = 3,
		},
		-- Melkor Bauglir provides the Mind school at 1/3 the prayer skill
		[GOD_MELKOR] =
		{
			["skill"] = SKILL_PRAY,
			["mul"] = 1,
			["div"] = 3,
		},
	},
}
SCHOOL_UDUN = add_school
{
	["name"] = 		"Udun",
	["skill"] = 		SKILL_UDUN,
	["bonus_level"] = 	function()
					return ((player.lev * 2) / 3)
				end,
}
SCHOOL_DEMON = add_school
{
	["name"] = "Demon",
	["skill"] = SKILL_DAEMON,
	["no_random"] = TRUE,
}

-- The God specific schools, all tied to the prayer skill
SCHOOL_ERU = add_school
{
	["name"] = "Eru Iluvatar",
	["skill"] = SKILL_PRAY,
	["spell_power"] = TRUE,
	["god"] = GOD_ERU,
}
SCHOOL_MANWE = add_school
{
	["name"] = "Manwe Sulimo",
	["skill"] = SKILL_PRAY,
	["spell_power"] = TRUE,
	["god"] = GOD_MANWE,
}
SCHOOL_TULKAS = add_school
{
	["name"] = "Tulkas",
	["skill"] = SKILL_PRAY,
	["spell_power"] = TRUE,
	["god"] = GOD_TULKAS,
}
SCHOOL_MELKOR = add_school
{
	["name"] = "Melkor Bauglir",
	["skill"] = SKILL_PRAY,
	["spell_power"] = TRUE,
	["god"] = GOD_MELKOR,
}
SCHOOL_YAVANNA = add_school
{
	["name"] = "Yavanna Kementari",
	["skill"] = SKILL_PRAY,
	["spell_power"] = TRUE,
	["god"] = GOD_YAVANNA,
}

-- New schools
SCHOOL_AULE = add_school
{
	["name"] = "Aule the Smith",
	["skill"] = SKILL_PRAY,
	["spell_power"] = TRUE,
	["god"] = GOD_AULE,
}
SCHOOL_VARDA = add_school
{
	["name"] = "Varda Elentari",
	["skill"] = SKILL_PRAY,
	["spell_power"] = TRUE,
	["god"] = GOD_VARDA,
}

SCHOOL_ULMO = add_school
{
	["name"] = "Ulmo",
	["skill"] = SKILL_PRAY,
	["spell_power"] = TRUE,
	["god"] = GOD_ULMO,
}

SCHOOL_MANDOS = add_school
{
	["name"] = "Mandos",
	["skill"] = SKILL_PRAY,
	["spell_power"] = TRUE,
	["god"] = GOD_MANDOS,
}

-- Not a real school, rather a palcehodler for stick only spells
SCHOOL_DEVICE = add_school
{
	["name"] = "Device",
	["skill"] = SKILL_DEVICE,
}

-- Music "spells"
SCHOOL_MUSIC = add_school
{
	["name"] = "Music",
	["skill"] = SKILL_MUSIC,
}

-- Put some spells
tome_dofile("s_fire.lua")
tome_dofile("s_mana.lua")
tome_dofile("s_water.lua")
tome_dofile("s_air.lua")
tome_dofile("s_earth.lua")
tome_dofile("s_convey.lua")
tome_dofile("s_nature.lua")
tome_dofile("s_divin.lua")
tome_dofile("s_tempo.lua")
tome_dofile("s_meta.lua")
tome_dofile("s_mind.lua")
tome_dofile("s_udun.lua")
tome_dofile("s_geom.lua")

-- God's specific spells
tome_dofile("s_eru.lua")
tome_dofile("s_manwe.lua")
tome_dofile("s_tulkas.lua")
tome_dofile("s_melkor.lua")
tome_dofile("s_yavann.lua")

-- New gods' spells
tome_dofile("s_aule.lua")
tome_dofile("s_varda.lua")
tome_dofile("s_ulmo.lua")
tome_dofile("s_mandos.lua")

-- Specific schools
tome_dofile("s_demon.lua")

-- Device spells
tome_dofile("s_stick.lua")

-- Musics
tome_dofile("s_music.lua")

-- Initialize spellbooks
init_school_books()
