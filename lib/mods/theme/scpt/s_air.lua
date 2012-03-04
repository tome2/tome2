-- handle the air school

NOXIOUSCLOUD = add_spell
{
	["name"] = 	"Noxious Cloud",
	["school"] = 	{SCHOOL_AIR},
	["level"] = 	3,
	["mana"] = 	3,
	["mana_max"] = 	30,
	["fail"] = 	20,
	["stick"] =
	{
			["charge"] =    { 5, 7 },
			[TV_WAND] =
			{
				["rarity"] = 		15,
				["base_level"] =	{ 1, 15 },
				["max_level"] =		{ 25, 50 },
			},
	},
	["spell"] = 	function()
			local ret, dir, type

			ret, dir = get_aim_dir()
			if ret == FALSE then return end
			if get_level(NOXIOUSCLOUD, 50) >= 30 then type = GF_UNBREATH
			else type = GF_POIS end
			fire_cloud(type, dir, 7 + get_level(NOXIOUSCLOUD, 150), 3, 5 + get_level(NOXIOUSCLOUD, 40))
			return TRUE
	end,
	["info"] = 	function()
			return "dam "..(7 + get_level(NOXIOUSCLOUD, 150)).." rad 3 dur "..(5 + get_level(NOXIOUSCLOUD, 40))
	end,
	["desc"] =	{
			"Creates a cloud of poison",
			"The cloud will persist for some turns, damaging all monsters passing by",
			"At spell level 30 it turns into a thick gas attacking all living beings"
	}
}

AIRWINGS = add_spell
{
	["name"] = 	"Wings of Winds",
	["school"] = 	{SCHOOL_AIR, SCHOOL_CONVEYANCE},
	["level"] = 	22,
	["mana"] = 	30,
	["mana_max"] = 	40,
	["fail"] = 	60,
	["stick"] =
	{
			["charge"] =    { 7, 5 },
			[TV_STAFF] =
			{
				["rarity"] = 		27,
				["base_level"] =	{ 1, 10 },
				["max_level"] =		{ 20, 50 },
			},
	},
	["inertia"] = 	{ 1, 10 },
	["spell"] = 	function()
			if get_level(AIRWINGS, 50) >= 16 then
				if player.tim_fly == 0 then return set_tim_fly(randint(10) + 5 + get_level(AIRWINGS, 25)) end
			else
				if player.tim_ffall == 0 then return set_tim_ffall(randint(10) + 5 + get_level(AIRWINGS, 25)) end
			end
			return FALSE
	end,
	["info"] = 	function()
			return "dur "..(5 + get_level(AIRWINGS, 25)).."+d10"
	end,
	["desc"] =	{
			"Grants the power of levitation",
			"At level 16 it grants the power of controlled flight"
	}
}

INVISIBILITY = add_spell
{
	["name"] = 	"Invisibility",
	["school"] = 	{SCHOOL_AIR},
	["level"] = 	16,
	["mana"] = 	10,
	["mana_max"] = 	20,
	["fail"] = 	50,
	["inertia"] = 	{ 1, 30 },
	["spell"] = 	function()
			if player.tim_invisible == 0 then return set_invis(randint(20) + 15 + get_level(INVISIBILITY, 50), 20 + get_level(INVISIBILITY, 50)) end
	end,
	["info"] = 	function()
			return "dur "..(15 + get_level(INVISIBILITY, 50)).."+d20 power "..(20 + get_level(INVISIBILITY, 50))
	end,
	["desc"] =	{
			"Grants invisibility"
	}
}

POISONBLOOD = add_spell
{
	["name"] = 	"Poison Blood",
	["school"] = 	{SCHOOL_AIR},
	["level"] = 	12,
	["mana"] = 	10,
	["mana_max"] = 	20,
	["fail"] = 	30,
	["stick"] =
	{
			["charge"] =    { 10, 15 },
			[TV_WAND] =
			{
				["rarity"] = 		45,
				["base_level"] =	{ 1, 25 },
				["max_level"] =		{ 35, 50 },
			},
	},
	["inertia"] = 	{ 1, 35 },
	["spell"] = 	function()
			local obvious = nil
		       	if player.oppose_pois == 0 then obvious = set_oppose_pois(randint(30) + 25 + get_level(POISONBLOOD, 25)) end
		       	if (player.tim_poison == 0) and (get_level(POISONBLOOD, 50) >= 15) then obvious = is_obvious(set_poison(randint(30) + 25 + get_level(POISONBLOOD, 25)), obvious) end
			return obvious
       	end,
	["info"] = 	function()
			return "dur "..(25 + get_level(POISONBLOOD, 25)).."+d30"
	end,
	["desc"] =	{
			"Grants resist poison",
			"At level 15 it provides poison branding to wielded weapon"
	}
}

THUNDERSTORM = add_spell
{
	["name"] = 	"Thunderstorm",
	["school"] = 	{SCHOOL_AIR, SCHOOL_NATURE},
	["level"] = 	25,
	["mana"] = 	40,
	["mana_max"] = 	60,
	["fail"] = 	60,
	["stick"] =
	{
			["charge"] =    { 5, 5 },
			[TV_WAND] =
			{
				["rarity"] = 		85,
				["base_level"] =	{ 1, 5 },
				["max_level"] =		{ 25, 50 },
			},
	},
	["inertia"] = 	{ 2, 15 },
	["spell"] = 	function()
			if player.tim_thunder == 0 then return set_tim_thunder(randint(10) + 10 + get_level(THUNDERSTORM, 25), 5 + get_level(THUNDERSTORM, 10), 10 + get_level(THUNDERSTORM, 25)) end
			return FALSE
	end,
	["info"] = 	function()
			return "dam "..(5 + get_level(THUNDERSTORM, 10)).."d"..(10 + get_level(THUNDERSTORM, 25)).." dur "..(10 + get_level(THUNDERSTORM, 25)).."+d10"
	end,
	["desc"] =	{
			"Charges up the air around you with electricity",
			"Each turn it will throw a thunder bolt at a random monster in sight",
			"The thunder does 3 types of damage, one third of lightning",
			"one third of sound and one third of light"
	}
}

STERILIZE = add_spell
{
	["name"] = 	"Sterilize",
	["school"] = 	{SCHOOL_AIR},
	["level"] = 	20,
	["mana"] = 	10,
	["mana_max"] = 	100,
	["fail"] = 	50,
	["stick"] =
	{
			["charge"] =    { 7, 5 },
			[TV_STAFF] =
			{
				["rarity"] = 		20,
				["base_level"] =	{ 1, 10 },
				["max_level"] =		{ 20, 50 },
			},
	},
	["spell"] = 	function()
			set_no_breeders((30) + 20 + get_level(STERILIZE, 70))
			return TRUE
	end,
	["info"] = 	function()
			return "dur "..(20 + get_level(STERILIZE, 70)).."+d30"
	end,
	["desc"] =	{
			"Prevents explosive breeding for a while."
	}
}
