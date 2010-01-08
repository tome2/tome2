-- Library quest in Minas Anor

-- Partially based on Fireproofing quest

library_quest = {}

-- The map definition itself
library_quest.MAP =
[[#!map

# Permanent wall
F:X:63:3

# Granite Wall
F:#:57:3

# Cobblestone Road
F:O:200:3

# Floor
F:.:1:3

# Lich
F:l:200:3:518

# Master lich
F:L:200:3:658

# Quest exit
F:<:6:3

D:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
D:X###############################################################X
D:X#<OlOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#O#.###.#.###.#.###.#.###.#.###.O.###.#.###.#.###.#.###.#.###O#X
D:X#OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOL#X
D:X###############################################################X
D:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

# Starting position
P:4:4
]]

-- Map helper
library_quest.place_random = function(minY, minX, maxY, maxX, monster)
	y = randint(maxY - minY + 1) + minY
	x = randint(maxX - minX + 1) + minX
	return place_monster_one(y, x, monster, 0, TRUE, MSTATUS_ENEMY)
end

-- Book creation helpers
library_quest.bookable_spells =
{
	MANATHRUST, DELCURSES,
	GLOBELIGHT, FIREGOLEM, FIREFLASH, FIREWALL,
	GEYSER, VAPOR, ENTPOTION,
	NOXIOUSCLOUD, POISONBLOOD,
	STONESKIN, DIG,
	RECHARGE, DISPERSEMAGIC,
	BLINK, DISARM, TELEPORT,
	SENSEMONSTERS, SENSEHIDDEN, REVEALWAYS, IDENTIFY, VISION,
	MAGELOCK, SLOWMONSTER, ESSENCESPEED,
	CHARM, CONFUSE, ARMOROFFEAR, STUN,
	GROWTREE, HEALING, RECOVERY,
	ERU_SEE, ERU_LISTEN,
	MANWE_BLESS, MANWE_SHIELD,
	YAVANNA_CHARM_ANIMAL, YAVANNA_GROW_GRASS, YAVANNA_TREE_ROOTS,
	TULKAS_AIM, TULKAS_SPIN,
	MELKOR_CURSE, MELKOR_CORPSE_EXPLOSION,
	DRAIN
}

library_quest.get_term_size = function()
	local width = 0
	local height = 0
	ret, width, height = Term_get_size(width, height)
	return width, height
end

library_quest.book_slots_left = function()
	if school_book[61][1] == -1 then
		return 3
	elseif school_book[61][2] == -1 then
		return 2
	elseif school_book[61][3] == -1 then
		return 1
	else
		return 0
	end
end

library_quest.book_contains_spell = function(spell)
	if school_book[61][1] == spell then
		return TRUE
	elseif school_book[61][2] == spell then
		return TRUE
	elseif school_book[61][3] == spell then
		return TRUE
	else
		return FALSE
	end
end

library_quest.add_spell = function(spell)
	if school_book[61][1] == -1 then
		school_book[61][1] = spell
		return TRUE
	elseif school_book[61][2] == -1 then
		school_book[61][2] = spell
		return TRUE
	elseif school_book[61][3] == -1 then
		school_book[61][3] = spell
		return TRUE
	else
		return FALSE
	end
end

library_quest.remove_spell = function(spell)
	if school_book[61][1] == spell then
		school_book[61][1] = school_book[61][2]
		school_book[61][2] = school_book[61][3]
		school_book[61][3] = -1
		return TRUE
	elseif school_book[61][2] == spell then
		school_book[61][2] = school_book[61][3]
		school_book[61][3] = -1
		return TRUE
	elseif school_book[61][3] == spell then
		school_book[61][3] = -1
		return TRUE
	else
		return FALSE
	end
end

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

		xx = nil
		sch_str = ""
		for index, sch in __spell_school[s] do
			if xx then
				sch_str = sch_str.."/"..school(sch).name
			else
				xx = 1
				sch_str = sch_str..school(sch).name
			end
		end

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

-- spell selection routines inspired by skills.c
library_quest.print_spells = function(first, current)
	Term_clear()
	width, height = library_quest.get_term_size()
	slots = library_quest.book_slots_left()

	c_prt(TERM_WHITE, "Book Creation Screen", 0, 0);
	c_prt(TERM_WHITE, "Up/Down to move, Right/Left to modify, I to describe, Esc to Save/Cancel", 1, 0);

	if slots == 0 then
		c_prt(TERM_L_RED, "The book can hold no more spells.", 2, 0);
	elseif slots == 1 then
		c_prt(TERM_L_BLUE, "The book can hold 1 more spell.", 2, 0);
	else
		c_prt(TERM_L_BLUE, "The book can hold "..slots.." more spells.", 2, 0);
	end

	row = 3;
	for index, spell in library_quest.bookable_spells do
		if index >= first then
			if index == current then
				color = TERM_GREEN
			elseif library_quest.book_contains_spell(spell) == TRUE then
				color = TERM_WHITE
			else
				color = TERM_ORANGE
			end
			library_quest.print_spell(color, row, spell)
	
			if row == height - 1 then
				return
			end
			row = row + 1
		end
	end
end

library_quest.fill_book = function()
	-- Always start with a cleared book
	school_book[61] = {-1, -1, -1}

	screen_save()
	width, height = library_quest.get_term_size()
	-- room for legend
	margin = 3

	first = 1
	current = 1
	done = FALSE

	while done == FALSE do
		library_quest.print_spells(first, current)

		inkey_scan = FALSE
		inkey_base = TRUE
		char = inkey()
		dir = get_keymap_dir(char)
		if char == ESCAPE then
			if library_quest.book_slots_left() == 0 then
				flush()
				done = get_check("Really create the book?")
			else
				done = TRUE
			end
		elseif char == strbyte('\r') then
			-- TODO: make tree of schools
		elseif char == strbyte('n') then
			current = current + height
		elseif char == strbyte('p') then
			current = current - height
		elseif char == strbyte('I') then
			print_spell_desc(library_quest.bookable_spells[current], 0)
			inkey()
		elseif dir == 2 then
			current = current + 1
		elseif dir == 8 then
			current = current - 1
		elseif dir == 6 then
			if library_quest.book_contains_spell(library_quest.bookable_spells[current]) == FALSE then
				library_quest.add_spell(library_quest.bookable_spells[current])
			end
		elseif dir == 4 then
			library_quest.remove_spell(library_quest.bookable_spells[current])
		end
		total = getn(library_quest.bookable_spells)
		if current > total then
			current = total
		elseif current < 1 then
			current = 1
		end
		
		if current > (first + height - margin - 1) then
			first = current - height + margin + 1
		elseif first > current then
			first = current
		end
	end

	screen_load()
end

-- Quest data and hooks
add_quest
{
	["global"] = "LIBRARY_QUEST",
	["name"] = "Library quest",
	["desc"] = function()
		-- Quest taken
		if (quest(LIBRARY_QUEST).status == QUEST_STATUS_TAKEN) then
			print_hook("#####yAn Old Mages Quest! (Danger Level: 35)\n")
			print_hook("Make the library safe for the old mage in Minas Anor.\n")
			print_hook("\n")
		-- Quest done, book not gotten yet
		elseif (quest(LIBRARY_QUEST).status == QUEST_STATUS_COMPLETED) then
			print_hook("#####yAn Old Mages Quest!\n")
			print_hook("You have made the library safe for the old mage in Minas Anor.\n")
			print_hook("Perhaps you should see about a reward.\n")
			print_hook("\n")
		end
	end,
	["level"] = 35,
	["data"] =
	{
		["school_book[61][1]"] = -1,
		["school_book[61][2]"] = -1,
		["school_book[61][3]"] = -1
	},
	["hooks"] =
	{
		-- Start the game without the quest, need to request it
		[HOOK_BIRTH_OBJECTS] = function()
			quest(LIBRARY_QUEST).status = QUEST_STATUS_UNTAKEN
			school_book[61] = {-1, -1, -1}
		end,

		[HOOK_GEN_QUEST] = function()
			-- Only if player doing this quest
			if (player.inside_quest ~= LIBRARY_QUEST) then
				return FALSE
			end

			load_map(library_quest.MAP, 2, 2)
			level_flags2 = DF2_NO_GENO

			-- generate the Liches 518
			liches = damroll(4, 2) -- plus one on the map
			while(liches > 0) do
				if 0 < library_quest.place_random(4, 4, 14, 37, 518) then
					liches = liches - 1
				end
			end

			-- generate the Monastic liches 611
			liches = damroll(1, 2)
			while(liches > 0) do
				if 0 < library_quest.place_random(14, 34, 37, 67, 611) then
					liches = liches - 1
				end
			end

			-- generate more Monastic liches 611
			liches = damroll(1, 2) - 1
			while(liches > 0) do
				if 0 < library_quest.place_random(4, 34, 14, 67, 611) then
					liches = liches - 1
				end
			end

			-- generate even more Monastic liches 611
			liches = damroll(1, 2) - 1
			while(liches > 0) do
				if 0 < library_quest.place_random(14, 4, 37, 34, 611) then
					liches = liches - 1
				end
			end

			-- Flesh golem 256
			golems = 2
			while(golems > 0) do
				if 0 < library_quest.place_random(10, 10, 37, 67, 256) then
					golems = golems - 1
				end
			end

			-- Clay golem 261
			golems = 2
			while(golems > 0) do
				if 0 < library_quest.place_random(10, 10, 37, 67, 261) then
					golems = golems - 1
				end
			end

			-- Iron golem 367
			golems = 2
			while(golems > 0) do
				if 0 < library_quest.place_random(10, 10, 37, 67, 367) then
					golems = golems - 1
				end
			end

			-- Mithril Golem 464
			golems = 1
			while(golems > 0) do
				if 0 < library_quest.place_random(10, 10, 37, 67, 464) then
					golems = golems - 1
				end
			end

			-- one Master lich is on the map

			return TRUE
		end,
		[HOOK_STAIR] = function()
			local ret

			-- only ask this if player about to go up stairs of quest and hasn't won yet
			if (player.inside_quest ~= LIBRARY_QUEST) or (quest(LIBRARY_QUEST).status == QUEST_STATUS_COMPLETED) then
				return FALSE
			end

			if cave(player.py, player.px).feat ~= FEAT_LESS then return end

			-- flush all pending input
			flush()

			-- confirm
			ret = get_check("Really abandon the quest?")

			-- if yes, then
			if ret == TRUE then
				-- fail the quest
				quest(LIBRARY_QUEST).status = QUEST_STATUS_FAILED
				return FALSE
			else 
				-- if no, they stay in the quest
				return TRUE
			end
		end,
		[HOOK_MONSTER_DEATH] = function()
			-- if they're in the quest and haven't won, continue
			if (player.inside_quest ~= LIBRARY_QUEST) or (quest(LIBRARY_QUEST).status == QUEST_STATUS_COMPLETED) then
				return FALSE
			end

			i = 1
			count = -1
			while i <= m_max do
				local monster = m_list[i]
				if (monster.r_idx > 0) and (monster.status <= MSTATUS_ENEMY) then
					count = count + 1
				end
				i = i + 1
			end

			if count == 0 then
				quest(LIBRARY_QUEST).status = QUEST_STATUS_COMPLETED
				msg_print(TERM_YELLOW, "The library is safe now.")
			end
		end,
	},
}

-- Library store action
add_building_action
{
	["index"] = 61,
	["action"] = function()
		-- the quest hasn't been requested already, right?
		if quest(LIBRARY_QUEST).status == QUEST_STATUS_UNTAKEN then
			-- quest has been taken now
			quest(LIBRARY_QUEST).status = QUEST_STATUS_TAKEN

			-- issue instructions
			msg_print("I need get some stock from my main library, but it is infested with monsters!")
			msg_print("Please use the side entrance and vanquish the intruders for me.")

			return TRUE, FALSE, TRUE
		-- if quest completed
		elseif (quest(LIBRARY_QUEST).status == QUEST_STATUS_COMPLETED) then
			msg_print("Thank you!  Let me make a special book for you.")
			msg_print("Tell me three spells and I will write them in the book.")
			library_quest.fill_book()
			if library_quest.book_slots_left() == 0 then
				quest(LIBRARY_QUEST).status = QUEST_STATUS_REWARDED
				book = create_object(TV_BOOK, 61)
				book.art_name = quark_add(player_name())
				book.found = OBJ_FOUND_REWARD
				set_aware(book)
				set_known(book)
				inven_carry(book, FALSE)
			end

		-- if the player asks for a quest when they already have it, but haven't failed it, give them some extra instructions
		elseif (quest(LIBRARY_QUEST).status == QUEST_STATUS_TAKEN) then
			msg_print("Please use the side entrance and vanquish the intruders for me.")

		-- quest failed or completed, then give no more quests
		elseif (quest(LIBRARY_QUEST).status == QUEST_STATUS_FAILED) or (quest(LIBRARY_QUEST).status == QUEST_STATUS_REWARDED) then
			msg_print("I have no more quests for you.")
		end
		return TRUE
	end,
}
