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


-- Return the coordinates of the player whether in wild or not
function player.get_wild_coord()
	if player.wild_mode == TRUE then
		return player.py, player.px
	else
		return player.wilderness_y, player.wilderness_x
	end
end


-- Subraces
function subrace(racem)
	return race_mod_info[racem + 1]
end
