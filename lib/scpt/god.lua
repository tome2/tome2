-- The god quest: find randomly placed relic in a randomly placed dungeon!

-- set some global variables (stored in the save file via the ["data"] key)
god_quest = {}

-- increase this number to make god quests more common, to a max value of 100
god_quest.CHANCE_OF_GOD_QUEST = 21

-- increase this number to make more quests
god_quest.MAX_NUM_GOD_QUESTS = 5

-- d_idx of the god_quest (Lost Temple) dungeon
god_quest.DUNGEON_GOD = 30

add_quest
{
	["global"] =    "GOD_QUEST",
	["name"] =      "God quest",
	["desc"] =      function() quest_god_describe() end,
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
			quest_god_enter_dungeon_hook(d_idx)
		end,
		[HOOK_GEN_LEVEL_BEGIN] = function()
			quest_god_enter_dungeon_hook(current_dungeon_idx)
		end,
		[HOOK_STAIR] = function()
			quest_god_enter_dungeon_hook(current_dungeon_idx)
		end,
		[HOOK_GET] = function(o_ptr, item)
			return quest_god_get_hook(item)
		end,
		[HOOK_CHAR_DUMP] = function()
			return quest_god_char_dump()
		end,
	},
}
