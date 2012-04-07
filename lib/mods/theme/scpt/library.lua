-- Library quest in Minas Anor

-- Partially based on Fireproofing quest

library_quest = {}

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
	MELKOR_CURSE, MELKOR_CORPSE_EXPLOSION, DRAIN,
	AULE_FIREBRAND, AULE_CHILD,
	VARDA_LIGHT_VALINOR, VARDA_EVENSTAR,
	ULMO_BELEGAER, ULMO_WRATH,
	MANDOS_TEARS_LUTHIEN, MANDOS_TALE_DOOM
}

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

			load_map("library.map", 2, 2)
			level_flags2 = DF2_NO_GENO

                        -- generate monsters
                        quest_library_gen_hook()

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
