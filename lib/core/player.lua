-- SYSTEM FILE
--
-- Lua player funtions
--

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
