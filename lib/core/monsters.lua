-- SYSTEM FILE
--
-- Monster stuff, do not touch
--

function summon_monster(y, x, lev, friend, typ)
	if type(typ) == "number" then
		if friend == TRUE then
			return summon_specific_friendly(y, x, lev, typ, FALSE)
		else
			return summon_specific(y, x, lev, typ)
		end
	else
		return summon_monster_aux(y, x, lev, friend, typ)
	end
end
