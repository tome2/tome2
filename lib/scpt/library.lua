-- Library quest in Minas Anor

-- Partially based on Fireproofing quest

library_quest = {}

-- Print a spell (taken from s_aux)
function library_quest.print_spell(color, y, spl)
	local x, index, sch, size, s

	x = 0
	size = 0
	book = 255
	obj = nil

	-- Hack if the book is 255 it is a random book
	if book == 255 then
		school_book[book] = {spl}
	end

	-- Parse all spells
	for index, s in school_book[book] do
		local lvl, na = get_level_school(s, 50, -50)
		local xx, sch_str

		sch_str = spell_school_name(s)

		if s == spl then
			if na then
				c_prt(color, format("%-20s%-16s   %3s %4s %3d%s %s", spell(s).name, sch_str, na, get_mana(s), spell_chance(s), "%", __spell_info[s]()), y, x)
			else
				c_prt(color, format("%-20s%-16s   %3d %4s %3d%s %s", spell(s).name, sch_str, lvl, get_mana(s), spell_chance(s), "%", __spell_info[s]()), y, x)
			end
			y = y + 1
			size = size + 1
		end
	end
	return y
end
