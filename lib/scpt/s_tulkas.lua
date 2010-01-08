-- Handle Tulkas magic school

TULKAS_AIM = add_spell
{
	["name"] =      "Divine Aim",
	["school"] =    {SCHOOL_TULKAS},
	["level"] =     1,
	["mana"] =      30,
	["mana_max"] =  500,
	["fail"] = 	20,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] = 	SKILL_SPIRITUALITY,
	["spell"] = 	function()
			local dur = get_level(TULKAS_AIM, 50) + randint(10)
			local obvious

			obvious = set_strike(dur)
			if get_level(TULKAS_AIM) >= 20 then
				obvious = is_obvious(set_tim_deadly(dur), obvious)
			end
			return obvious
	end,
	["info"] = 	function()
			return "dur "..(get_level(TULKAS_AIM, 50)).."+d10"
	end,
	["desc"] =	{
			"It makes you more accurate in combat",
			"At level 20 all your blows are critical hits",
	}
}

TULKAS_WAVE = add_spell
{
	["name"] =      "Wave of Power",
	["school"] =    {SCHOOL_TULKAS},
	["level"] =     20,
	["mana"] =      200,
	["mana_max"] =  200,
	["fail"] = 	75,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] = 	SKILL_SPIRITUALITY,
	["spell"] = 	function()
			local ret, dir = get_aim_dir()
			if ret == FALSE then return end

			return fire_bolt(GF_ATTACK, dir, get_level(TULKAS_WAVE, player.num_blow))
	end,
	["info"] = 	function()
			return "blows "..(get_level(TULKAS_WAVE, player.num_blow))
	end,
	["desc"] =	{
			"It allows you to project a number of melee blows across a distance",
	}
}

TULKAS_SPIN = add_spell
{
	["name"] =      "Whirlwind",
	["school"] =    {SCHOOL_TULKAS},
	["level"] =     10,
	["mana"] =      100,
	["mana_max"] =  100,
	["fail"] = 	45,
	-- Uses piety to cast
	["piety"] =     TRUE,
	["stat"] =      A_WIS,
	["random"] = 	SKILL_SPIRITUALITY,
	["spell"] = 	function()
			return fire_ball(GF_ATTACK, 0, 1, 1)
	end,
	["info"] = 	function()
			return ""
	end,
	["desc"] =	{
			"It allows you to spin around and hit all monsters nearby",
	}
}
