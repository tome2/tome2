-- handle the water school

TIDALWAVE = add_spell
{
	["name"] = 	"Tidal Wave",
	["school"] = 	{SCHOOL_WATER},
	["level"] = 	16,
	["mana"] = 	16,
	["mana_max"] = 	40,
	["fail"] = 	65,
	["stick"] =
	{
			["charge"] =    { 6, 5 },
			[TV_WAND] =
			{
				["rarity"] = 		54,
				["base_level"] =	{ 1, 10 },
				["max_level"] =		{ 20, 50 },
			},
	},
	["inertia"] = 	{ 4, 100 },
	["spell"] = 	function()
			fire_wave(GF_WAVE, 0, 40 + get_level(TIDALWAVE, 200), 0, 6 + get_level(TIDALWAVE, 10), EFF_WAVE)
			return TRUE
	end,
	["info"] = 	function()
			return "dam "..(40 + get_level(TIDALWAVE,  200)).." rad "..(6 + get_level(TIDALWAVE,  10))
	end,
	["desc"] =	{
			"Summons a monstrous tidal wave that will expand and crush the",
			"monsters under its mighty waves."
	}
}

ICESTORM = add_spell
{
	["name"] = 	"Ice Storm",
	["school"] = 	{SCHOOL_WATER},
	["level"] = 	22,
	["mana"] = 	30,
	["mana_max"] = 	60,
	["fail"] = 	80,
	["stick"] =
	{
			["charge"] =    { 3, 7 },
			[TV_WAND] =
			{
				["rarity"] = 		65,
				["base_level"] =	{ 1, 5 },
				["max_level"] =		{ 25, 45 },
			},
	},
	["inertia"] = 	{ 3, 40 },
	["spell"] = 	function()
			local type
	
			if get_level(ICESTORM, 50) >= 10 then type = GF_ICE
			else type = GF_COLD end
			fire_wave(type, 0, 80 + get_level(ICESTORM, 200), 1 + get_level(ICESTORM, 3, 0), 20 + get_level(ICESTORM, 70), EFF_STORM)
			return TRUE
	end,
	["info"] = 	function()
			return "dam "..(80 + get_level(ICESTORM, 200)).." rad "..(1 + get_level(ICESTORM, 3, 0)).." dur "..(20 + get_level(ICESTORM, 70))
	end,
	["desc"] =	{
			"Engulfs you in a storm of roaring cold that strikes your foes.",
			"At level 10 it turns into shards of ice."
	}
}

ENTPOTION = add_spell
{
	["name"] = 	"Ent's Potion",
	["school"] = 	{SCHOOL_WATER},
	["level"] = 	6,
	["mana"] = 	7,
	["mana_max"] = 	15,
	["fail"] = 	35,
	["inertia"] = 	{ 1, 30 },
	["spell"] = 	function()
			set_food(PY_FOOD_MAX - 1)
			msg_print("The Ent's Potion fills your stomach.")
			if get_level(ENTPOTION, 50) >= 5 then
				set_afraid(0)
			end
			if get_level(ENTPOTION, 50) >= 12 then
				set_hero(player.hero + randint(25) + 25 + get_level(ENTPOTION, 40))
			end
			return TRUE
	end,
	["info"] = 	function()
			if get_level(ENTPOTION, 50) >= 12 then
				return "dur "..(25 + get_level(ENTPOTION, 40)).."+d25"
			else
				return ""
			end
	end,
	["desc"] =	{
			"Fills up your stomach.",
			"At level 5 it boldens your heart.",
			"At level 12 it make you heroic."
	}
}

VAPOR = add_spell
{
	["name"] = 	"Vapor",
	["school"] = 	{SCHOOL_WATER},
	["level"] = 	2,
	["mana"] = 	2,
	["mana_max"] = 	12,
	["fail"] = 	20,
	["inertia"] = 	{ 1, 30 },
	["spell"] = 	function()
			fire_cloud(GF_WATER, 0, 3 + get_level(VAPOR, 20), 3 + get_level(VAPOR, 9, 0), 5)
			return TRUE
	end,
	["info"] = 	function()
       			return "dam "..(3 + get_level(VAPOR, 20)).." rad "..(3 + get_level(VAPOR, 9, 0)).." dur 5"
	end,
	["desc"] =	{
			"Fills the air with toxic moisture to eradicate annoying critters."
	}
}

function get_geyser_damage()
	return get_level(GEYSER, 10), 3 + get_level(GEYSER, 35)
end

GEYSER = add_spell
{
	["name"] = "Geyser",
	["school"] = SCHOOL_WATER,
	["level"] = 1,
	["mana"] = 1,
	["mana_max"] = 35,
	["fail"] = 5,
	["spell"] = function()
		local ret, dir
		ret, dir = get_aim_dir()
		if ret == FALSE then return end
		return fire_bolt_or_beam(2 * get_level(GEYSER, 85), GF_WATER, dir, damroll(get_geyser_damage()))
	end,
	["info"] = function()
		local n, d
		n, d = get_geyser_damage()
		return "dam "..n.."d"..d
	end,
	["desc"] =
	{
		"Shoots a geyser of water from your fingertips.",
		"Sometimes it can blast through its first target."
	},
}
