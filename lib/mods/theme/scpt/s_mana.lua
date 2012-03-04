-- The mana school

function get_manathrust_dam()
	return 3 + get_level(MANATHRUST, 50), 1 + get_level(MANATHRUST, 20)
end

MANATHRUST = add_spell
{
	["name"] = 	"Manathrust",
	["school"] = 	SCHOOL_MANA,
	["level"] = 	1,
	["mana"] = 	1,
	["mana_max"] =  25,
	["fail"] = 	10,
	["stick"] =
	{
			["charge"] =    { 7, 10 },
			[TV_WAND] =
			{
				["rarity"] = 		5,
				["base_level"] =	{ 1, 20 },
				["max_level"] =		{ 15, 33 },
			},
	},
	["spell"] = 	function()
			local ret, dir	

			ret, dir = get_aim_dir()
			if ret == FALSE then return end
			return fire_bolt(GF_MANA, dir, damroll(get_manathrust_dam()))
	end,
	["info"] = 	function()
			local x, y

			x, y = get_manathrust_dam()
			return "dam "..x.."d"..y
	end,
	["desc"] =	{
			"Conjures up mana into a powerful bolt",
			"The damage is irresistible and will increase with level"
		}
}

DELCURSES = add_spell
{
	["name"] = 	"Remove Curses",
	["school"] = 	SCHOOL_MANA,
	["level"] = 	10,
	["mana"] = 	20,
	["mana_max"] = 	40,
	["fail"] = 	30,
	["stick"] =
	{
			["charge"] =    { 3, 8 },
			[TV_STAFF] =
			{
				["rarity"] = 		70,
				["base_level"] =	{ 1, 5 },
				["max_level"] =		{ 15, 50 },
			},
	},
	["inertia"] = 	{ 1, 10 },
	["spell"] = 	function()
			local done

			if get_level(DELCURSES, 50) >= 20 then done = remove_all_curse()
			else done = remove_curse() end
			if done == TRUE then msg_print("The curse is broken!") end
			return done
	end,
	["info"] = 	function()
			return ""
	end,
	["desc"] =	{
			"Remove curses of worn objects",
			"At level 20 switches to *remove curses*"
		}
}

RESISTS = add_spell
{
	["name"] = 	"Elemental Shield",
	["school"] = 	SCHOOL_MANA,
	["level"] = 	20,
	["mana"] = 	17,
	["mana_max"] = 	20,
	["fail"] = 	40,
	["inertia"] = 	{ 2, 25 },
	["spell"] = 	function()
			local obvious
		       	if player.oppose_fire == 0 then obvious = set_oppose_fire(randint(10) + 15 + get_level(RESISTS, 50)) end
		       	if player.oppose_cold == 0 then obvious = is_obvious(set_oppose_cold(randint(10) + 15 + get_level(RESISTS, 50)), obvious) end
		       	if player.oppose_elec == 0 then obvious = is_obvious(set_oppose_elec(randint(10) + 15 + get_level(RESISTS, 50)), obvious) end
		       	if player.oppose_acid == 0 then obvious = is_obvious(set_oppose_acid(randint(10) + 15 + get_level(RESISTS, 50)), obvious) end
			return obvious
	end,
	["info"] = 	function()
			return "dur "..(15 + get_level(RESISTS, 50)).."+d10"
	end,
	["desc"] =	{
			"Provide resistances to the four basic elements",
		}
}

MANASHIELD = add_spell
{
	["name"] = 	"Disruption Shield",
	["school"] = 	SCHOOL_MANA,
	["level"] = 	45,
	["mana"] = 	50,
	["mana_max"] = 	50,
	["fail"] = 	90,
	["inertia"] = 	{ 9, 10},
	["spell"] = 	function()
			if get_level(MANASHIELD, 50) >= 5 then
			       	if (player.invuln == 0) then
					return set_invuln(randint(5) + 3 + get_level(MANASHIELD, 10))
				end
			else
			       	if (player.disrupt_shield == 0) then return set_disrupt_shield(randint(5) + 3 + get_level(MANASHIELD, 10)) end
			end
	end,
	["info"] = 	function()
			return "dur "..(3 + get_level(MANASHIELD, 10)).."+d5"
	end,
	["desc"] =	{
			"Uses mana instead of hp to take damage",
			"At level 5 switches to Globe of Invulnerability.",
			"The spell breaks as soon as a melee, shooting, throwing or magical",
			"skill action is attempted, and lasts only a short time."
		}
}
