-- The god quest: find randomly placed relic in a randomly placed dungeon!

-- set some global variables (stored in the save file via the ["data"] key)
god_quest = {}

-- increase this number to make god quests more common, to a max value of 100
god_quest.CHANCE_OF_GOD_QUEST = 21

-- increase this number to make more quests
god_quest.MAX_NUM_GOD_QUESTS = 5

-- d_idx of the god_quest (Lost Temple) dungeon
god_quest.DUNGEON_GOD = 30

-- Show directions given a function to print
function print_directions(feel_it, pfunc)
	local home, home_axis, home_distance, home2, home2_axis, home2_distance = get_god_quest_axes()

	local feel_it_str = "."
	if feel_it == TRUE then
		feel_it_str = ", I can feel it.'"
	end

	if home_axis ~= "close" then
		pfunc("The temple lies "..home_distance.." to the "..home_axis.." of "..home..", ")
	else
		pfunc("The temple lies very close to "..home..", ")
	end
	if home2_axis ~= "close" then
		pfunc( "and "..home2_distance.." to the "..home2_axis.." of "..home2..feel_it_str)
	else
		pfunc("and very close to "..home2..feel_it_str)
	end
end

function msg_directions()
	print_directions(TRUE, function (line)
		cmsg_print(TERM_YELLOW, line)
	end)
end

-- Set up relic number according to god
function setup_relic_number()
	if player.pgod == GOD_ERU then
		god_quest.relic_num = 7
	elseif player.pgod == GOD_MANWE then
		god_quest.relic_num = 8
	elseif player.pgod == GOD_TULKAS then
		god_quest.relic_num = 9
	elseif player.pgod == GOD_MELKOR then
		god_quest.relic_num = 10
	elseif player.pgod == GOD_YAVANNA then
		god_quest.relic_num = 11
	end
end

add_quest
{
	["global"] =    "GOD_QUEST",
	["name"] =      "God quest",
	["desc"] =      function()
			if quest(GOD_QUEST).status == QUEST_STATUS_TAKEN then
				print_hook("#####yGod quest "..god_quest.quests_given.."!\n")
				print_hook("Thou art to find the lost temple of thy God and\n");
				print_hook("to retrieve the lost part of the relic for thy God! \n")
				print_directions(FALSE, function (line)
					print_hook(line .. "\n")
				end)
				print_hook("\n")
			end
	end,
	["level"] =     -1,
	["data"] =      {
		["god_quest.relic_num"] = 1,
		["god_quest.quests_given"] = 0,
		["god_quest.relics_found"] = 0,
		["god_quest.dun_mindepth"] = 1,
		["god_quest.dun_maxdepth"] = 4,
		["god_quest.dun_minplev"] = 0,
		["god_quest.relic_gen_tries"] = 0,
		["god_quest.relic_generated"] = FALSE,
		["god_quest.dung_x"] = 1,
		["god_quest.dung_y"] = 1,
		["god_quest.player_x"] = 0,
		["god_quest.player_y"] = 0,
	},
	["hooks"] =     {
		-- Start the game without the quest, given it by chance
		[HOOK_BIRTH_OBJECTS] = function()
			quest(GOD_QUEST).status = QUEST_STATUS_UNTAKEN

			-- initialise save-file stored variables when new character is created
			god_quest.relic_num = 1
			god_quest.quests_given = 0
			god_quest.relics_found = 0
			god_quest.dun_mindepth = 1
			god_quest.dun_maxdepth = 4
			god_quest.dun_minplev = 0
			god_quest.relic_gen_tries = 0
			god_quest.relic_generated = FALSE
		end,
		[HOOK_PLAYER_LEVEL] = function(gained)
			quest_god_player_level_hook(gained)
		end,
		[HOOK_LEVEL_END_GEN] = function()
			quest_god_level_end_gen_hook()
		end,
		[HOOK_ENTER_DUNGEON] = function(d_idx)
			-- call the function to set the dungeon variables (dependant on pgod) the first time we enter the dungeon
			if d_idx ~= god_quest.DUNGEON_GOD then
				return
			else
				set_god_dungeon_attributes()
			end
		end,
		[HOOK_GEN_LEVEL_BEGIN] = function()
			-- call the function to set the dungeon variables (dependant on pgod) when we WoR back into the dungeon
			if current_dungeon_idx ~= god_quest.DUNGEON_GOD then
				return
			else
				set_god_dungeon_attributes()
			end
		end,
		[HOOK_STAIR] = function()
			-- call the function to set the dungeon variables (dependant on pgod) every time we go down a level
			if current_dungeon_idx ~= god_quest.DUNGEON_GOD then
				return
			else
				set_god_dungeon_attributes()
			end
		end,
		[HOOK_GET] = function(o_ptr, item)
				-- Is it the relic, and check to make sure the relic hasn't already been identified
			if (quest(GOD_QUEST).status == QUEST_STATUS_TAKEN) and (o_ptr.tval == TV_JUNK) and (o_ptr.sval == god_quest.relic_num)
			and (o_ptr.pval ~= TRUE)  and (god_quest.relics_found < god_quest.quests_given) then

				-- more God talky-talky
				cmsg_print(TERM_L_BLUE, deity(player.pgod).name.." speaks to you:")

				-- Is it the last piece of the relic?
				if (god_quest.quests_given == god_quest.MAX_NUM_GOD_QUESTS) then
					cmsg_print(TERM_YELLOW, "'At last! Thou hast found all of the relic pieces.")

					-- reward player by increasing prayer skill
					cmsg_print(TERM_YELLOW, "Thou hast done exceptionally well! I shall increase thy prayer skill even more!'")
					skill(SKILL_PRAY).value = skill(SKILL_PRAY).value + (10 * (skill(SKILL_PRAY).mod))

					-- Take the relic piece
					floor_item_increase(item, -1)
					floor_item_optimize(item)
				else
					cmsg_print(TERM_YELLOW, "'Well done! Thou hast found part of the relic.")
					cmsg_print(TERM_YELLOW, "I shall surely ask thee to find more of it later!")
					cmsg_print(TERM_YELLOW, "I will take it from thee for now'")

					-- Take the relic piece
					floor_item_increase(item, -1)
					floor_item_optimize(item)

					-- reward player by increasing prayer skill
					cmsg_print(TERM_YELLOW, "'As a reward, I shall teach thee how to pray better'")
					skill(SKILL_PRAY).value = skill(SKILL_PRAY).value + (5 * (skill(SKILL_PRAY).mod))
				end

				-- relic piece has been identified
				o_ptr.pval = TRUE
				god_quest.relics_found = god_quest.relics_found + 1

				-- Make sure quests can be given again if neccesary
				quest(GOD_QUEST).status = QUEST_STATUS_UNTAKEN
				return TRUE
			end
		end,
		[HOOK_CHAR_DUMP] = function()

			if (god_quest.quests_given > 0) then

				local relics = god_quest.relics_found
				local append_text = ""
				if (god_quest.relics_found == god_quest.MAX_NUM_GOD_QUESTS) then
					relics = "all"
					append_text = " and pleased your god"
				else
					if (god_quest.relics_found == 0) then
						relics = "none"
					end
					if (quest(GOD_QUEST).status == QUEST_STATUS_FAILED) then
						append_text = " and failed in your quest"
					end
				end

				print_hook("\n You found "..(relics).." of the relic pieces"..(append_text)..".")

			end
		end,
	},
}

function set_god_dungeon_attributes()

	-- dungeon properties altered according to which god player is worshipping,
	if player.pgod == GOD_ERU then
		quest_god_set_god_dungeon_attributes_eru()
	elseif player.pgod == GOD_MANWE then
		quest_god_set_god_dungeon_attributes_manwe()
	elseif player.pgod == GOD_TULKAS then
		quest_god_set_god_dungeon_attributes_tulkas()
	elseif player.pgod == GOD_MELKOR then
		quest_god_set_god_dungeon_attributes_melkor()
	elseif player.pgod == GOD_YAVANNA then
		quest_god_set_god_dungeon_attributes_yavanna()
	end

	-- W: All dungeons are 5 levels deep, and created at 2/3 of the player clvl when the quest is given
	dungeon(god_quest.DUNGEON_GOD).mindepth = god_quest.dun_mindepth
	dungeon(god_quest.DUNGEON_GOD).maxdepth = god_quest.dun_maxdepth
	dungeon(god_quest.DUNGEON_GOD).minplev = god_quest.dun_minplev

end

-- Calling this function returns the direction the dungeon is in from the players position at the time
-- the quest was given, and also the direction from angband (if the player is worshipping Melkor) or lothlorien.
function get_god_quest_axes()
	local home, home_y_coord, home_x_coord, home_axis, home2, home2_y_coord, home2_x_coord, home2_axis, mydistance

	-- different values for different gods...
	if player.pgod ~= GOD_MELKOR then

		-- one of the valar, "home" is lothlorien, home2 is Minas Arnor
		home = "Bree"
		home_y_coord = 21
		home_x_coord = 34
		home2 = "Minas Anor"
		home2_y_coord = 56
		home2_x_coord = 60
	else
		-- Melkor, "home" is angband, home2 is Barad-dur
		home = "the Pits of Angband"
		home_y_coord = 7
		home_x_coord = 34
		home2 = "the Land of Mordor"
		home2_y_coord = 58
		home2_x_coord = 65	
	end

	home_axis = compass(home_y_coord, home_x_coord, god_quest.dung_y, god_quest.dung_x)
	home2_axis = compass(home2_y_coord, home2_x_coord, god_quest.dung_y, god_quest.dung_x)

	home_distance = approximate_distance(home_y_coord, home_x_coord, god_quest.dung_y, god_quest.dung_x)
	home2_distance = approximate_distance(home2_y_coord, home2_x_coord, god_quest.dung_y, god_quest.dung_x)

	return home, home_axis, home_distance, home2, home2_axis, home2_distance
end
