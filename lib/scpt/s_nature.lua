-- handle the nature school

GROWTREE = add_spell
{
	["name"] = 	"Grow Trees",
	["school"] = 	{SCHOOL_NATURE, SCHOOL_TEMPORAL},
	["level"] = 	6,
	["mana"] = 	6,
	["mana_max"] = 	30,
	["fail"] = 	35,
	["inertia"] = 	{ 5, 50 },
	["spell"] = 	function()
			grow_trees(2 + get_level(GROWTREE, 7))
			return TRUE
	end,
	["info"] = 	function()
			return "rad "..(2 + get_level(GROWTREE, 7))
	end,
	["desc"] =	{
			"Makes trees grow extremely quickly around you",
	}
}

HEALING = add_spell
{
	["name"] = 	"Healing",
	["school"] = 	{SCHOOL_NATURE},
	["level"] = 	10,
	["mana"] = 	15,
	["mana_max"] = 	50,
	["fail"] = 	45,
	["stick"] =
	{
			["charge"] =    { 2, 3 },
			[TV_STAFF] =
			{
				["rarity"] = 		90,
				["base_level"] =	{ 1, 5 },
				["max_level"] =		{ 20, 40 },
			},
	},
	["spell"] = 	function()
			return hp_player(player.mhp * (15 + get_level(HEALING, 35)) / 100)
	end,
	["info"] = 	function()
			return "heal "..(15 + get_level(HEALING, 35)).."% = "..(player.mhp * (15 + get_level(HEALING, 35)) / 100).."hp"
	end,
	["desc"] =	{
			"Heals a percent of hitpoints",
	}
}

RECOVERY = add_spell
{
	["name"] = 	"Recovery",
	["school"] = 	{SCHOOL_NATURE},
	["level"] = 	15,
	["mana"] = 	10,
	["mana_max"] = 	25,
	["fail"] = 	60,
	["stick"] =
	{
			["charge"] =    { 5, 10 },
			[TV_STAFF] =
			{
				["rarity"] = 		50,
				["base_level"] =	{ 1, 5 },
				["max_level"] =		{ 10, 30 },
			},
	},
	["inertia"] = 	{ 2, 100 },
	["spell"] = 	function()
			local obvious
			obvious = set_poisoned(player.poisoned / 2)
			if get_level(RECOVERY, 50) >= 5 then
				obvious = is_obvious(set_poisoned(0), obvious)
				obvious = is_obvious(set_cut(0), obvious)
			end
			if get_level(RECOVERY, 50) >= 10 then
				obvious = is_obvious(do_res_stat(A_STR, TRUE), obvious)
				obvious = is_obvious(do_res_stat(A_CON, TRUE), obvious)
				obvious = is_obvious(do_res_stat(A_DEX, TRUE), obvious)
				obvious = is_obvious(do_res_stat(A_WIS, TRUE), obvious)
				obvious = is_obvious(do_res_stat(A_INT, TRUE), obvious)
				obvious = is_obvious(do_res_stat(A_CHR, TRUE), obvious)
			end
			if get_level(RECOVERY, 50) >= 15 then
				obvious = is_obvious(restore_level(), obvious)
			end
			return obvious
	end,
	["info"] = 	function()
			return ""
	end,
	["desc"] =	{
			"Reduces the length of time that you are poisoned",
			"At level 5 it cures poison and cuts",
			"At level 10 it restores drained stats",
			"At level 15 it restores lost experience"
	}
}

REGENERATION = add_spell
{
	["name"] = 	"Regeneration",
	["school"] = 	{SCHOOL_NATURE},
	["level"] = 	20,
	["mana"] = 	30,
	["mana_max"] = 	55,
	["fail"] = 	70,
	["inertia"] = 	{ 4, 40 },
	["spell"] = 	function()
			if player.tim_regen == 0 then return set_tim_regen(randint(10) + 5 + get_level(REGENERATION, 50), 300 + get_level(REGENERATION, 700)) end
	end,
	["info"] = 	function()
			return "dur "..(5 + get_level(REGENERATION, 50)).."+d10 power "..(300 + get_level(REGENERATION, 700))
	end,
	["desc"] =	{
			"Increases your body's regeneration rate",
	}
}


SUMMONANNIMAL = add_spell
{
	["name"] =      "Summon Animal",
	["school"] = 	{SCHOOL_NATURE},
	["level"] = 	25,
	["mana"] = 	25,
	["mana_max"] = 	50,
	["fail"] = 	90,
	["stick"] =
	{
			["charge"] =    { 1, 3 },
			[TV_WAND] =
			{
				["rarity"] = 		85,
				["base_level"] =	{ 1, 5 },
				["max_level"] =		{ 15, 45 },
			},
	},
	["spell"] = 	function()
			summon_specific_level = 25 + get_level(SUMMONANNIMAL, 50)
			return summon_monster(player.py, player.px, dun_level, TRUE, SUMMON_ANIMAL)
	end,
	["info"] = 	function()
			return "level "..(25 + get_level(SUMMONANNIMAL, 50))
	end,
	["desc"] =	{
			"Summons a leveled animal to your aid",
	}
}
