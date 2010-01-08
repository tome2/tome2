-- The bounty quest! bring back corpses to increase your monster lore skill

add_quest
{
	["global"] =    "BOUNTY_QUEST",
	["name"] =      "Bounty quest",
	["desc"] =      function()
		if quest(BOUNTY_QUEST).status == QUEST_STATUS_TAKEN then
			print_hook("#####yBounty quest!\n")
			print_hook("You must bring back "..monster_race_desc(bounty_quest_monster, 0).." corpse to the beastmaster.\n")
			print_hook("\n")
		end
	end,
	["level"] =     -1,
	["data"] =      {
			["bounty_quest_monster"] = 0,
	},
	["hooks"] =     {
		-- Start the game without the quest, need to request it
		[HOOK_BIRTH_OBJECTS] = function()
			quest(BOUNTY_QUEST).status = QUEST_STATUS_UNTAKEN
		end,
	},
}

add_building_action
{
	-- Index is used in ba_info.txt to set the actions
	["index"] =     54,
	["action"] =    function()
		if quest(BOUNTY_QUEST).status == QUEST_STATUS_UNTAKEN then
			quest(BOUNTY_QUEST).status = QUEST_STATUS_TAKEN
			bounty_quest_monster = get_new_bounty_monster(3 + ((player.lev * 3) / 2))

			msg_print("You must bring me back "..monster_race_desc(bounty_quest_monster, 0).." corpse.")
		else
			msg_print("You still must bring me back "..monster_race_desc(bounty_quest_monster, 0).." corpse.")
		end
	end
}

add_building_action
{
	-- Index is used in ba_info.txt to set the actions
	["index"] =     55,
	["action"] =    function()
		if quest(BOUNTY_QUEST).status == QUEST_STATUS_TAKEN then
			local ret, item
	
			-- Ask for an item
			ret, item = get_item("What corpse to return?",
					     "You have no corpse to return.",
					     bor(USE_INVEN),
					     function (obj)
					     	if (obj.tval == TV_CORPSE) and (obj.pval2 == bounty_quest_monster) then
							return TRUE
						end
						return FALSE
					     end
			)

			-- Ok we got the corpse!
			if ret == TRUE then
				-- Take the corpse from the inventory
				inven_item_increase(item, -1)
				inven_item_optimize(item)

				msg_print("Ah well done adventurer!")
				msg_print("As a reward I will teach you a bit of monster lore.")

				if skill(SKILL_LORE).mod == 0 then
					skill(SKILL_LORE).mod = 900
					skill(SKILL_LORE).dev = TRUE
				end
				skill(SKILL_LORE).value = skill(SKILL_LORE).value + skill(SKILL_LORE).mod
				if skill(SKILL_PRESERVATION).mod == 0 then
					skill(SKILL_PRESERVATION).value = 800
					skill(SKILL_PRESERVATION).mod = 800
					skill(SKILL_PRESERVATION).dev = TRUE
					msg_print("I see you don't know the corpse preservation skill, I shall teach you it too.")
				end

				quest(BOUNTY_QUEST).status = QUEST_STATUS_UNTAKEN
				bounty_quest_monster = 0
			end
		else
			msg_print("You do not have any bounty quest yet.")
		end
	end
}
