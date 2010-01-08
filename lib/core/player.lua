-- SYSTEM FILE
--
-- Lua player funtions
--

-- Gods
function deity(i)
	return deity_info[1 + i]
end

-------- skill stuff ---------

-- Easy skill access
function skill(i)
	return s_info[i + 1]
end

-- Sart a lasting spell
function player.start_lasting_spell(spl)
	player.music_extra = -spl
end

-- stat mods
function player.modify_stat(stat, inc)
	player.stat_add[1 + stat] = player.stat_add[1 + stat] + inc
end

-- powers mods
function player.add_power(pow)
	player.powers[1 + pow] = TRUE
end

-- easier inventory access
function player.inventory(i)
	return player.inventory_real[i + 1]
end

-- modify mana
-- returns TRUE if there is a pb
function increase_mana(amt)
	player.csp = player.csp + amt
	player.redraw = bor(player.redraw, PR_MANA)
	if (player.csp < 0) then
		player.csp = 0
		return TRUE
	end
	if (player.csp > player.msp) then
		player.csp = player.msp
	end
	return FALSE
end


-- Return the coordinates of the player whether in wild or not
function player.get_wild_coord()
	if player.wild_mode == TRUE then
		return player.py, player.px
	else
		return player.wilderness_y, player.wilderness_x
	end
end

-- Create a new power
__power_fct = {}
function add_power(p)
	local i

	assert(p.name, "No power name!")
	assert(p.desc, "No power desc!")
	assert(p.desc_get, "No power desc get!")
	assert(p.desc_lose, "No power desc lose!")
	assert(p.stat, "No power stat!")
	assert(p.level, "No power level!")
	assert(p.cost, "No power cost!")
	assert(p.fail, "No power fail!")
	assert(p.power, "No power power!")

	i = add_new_power(p.name, p.desc, p.desc_get, p.desc_lose, p.level, p.cost, p.stat, p.fail)
	__power_fct[i] = p.power
	return i
end

function __power_fct_activate(power)
	if __power_fct[power] then
		__power_fct[power]()
		return TRUE
	else
		return FALSE
	end
end

-- Register in the hook list
add_hook_script(HOOK_ACTIVATE_POWER, "__power_fct_activate", "__power_fct_activate")


--- Mkeys

-- Create a new power
__mkey_fct = {}
function add_mkey(p)
	local i

	assert(p.mkey, "No mkey mkey!")
	assert(p.fct, "No mkeey fct!")

	__mkey_fct[p.mkey] = p.fct
end

function __mkey_fct_activate(power)
	if __mkey_fct[power] then
		__mkey_fct[power]()
		return TRUE
	else
		return FALSE
	end
end

-- Register in the hook list
add_hook_script(HOOK_MKEY, "__mkey_fct_activate", "__mkey_fct_activate")


-- Subraces
function subrace(racem)
	return race_mod_info[racem + 1]
end

function subrace_add_power(subrace, power)
	for i = 1, 4 do
		if subrace.powers[i] == -1 then
			subrace.powers[i] = power
			return not nil
		end
	end
	return nil
end

-- Body parts
function player.add_body_part(part, nb)
	player.extra_body_parts[part + 1] = player.extra_body_parts[part + 1] + nb
end
