-- SYSTEM FILE
--
-- Lua player funtions
--

-------- skill stuff ---------

-- Easy skill access
function skill(i)
	return s_info[i + 1]
end

-- easier inventory access
function player.inventory(i)
	return player.inventory_real[i + 1]
end
