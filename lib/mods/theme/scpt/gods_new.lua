-- This file contains all the new gods

add_loadsave("GRACE_DELAY",0)

function aule_stone_skin()
local type
	if player.grace >= 10000 then
		type = SHIELD_COUNTER
	else
		type = 0
	end

	set_shield(randint(10) + 10 + (player.grace / 100), 10 + (player.grace / 100), type, 2 + (player.grace / 200), 3 + (player.grace / 400))
end

GOD_AULE = add_god
{
	["name"] = "Aule the Smith",
	["desc"] =
	{
		"Aule is a smith, and the creator of the Dwarves."
	},
	["hooks"] =
	{
		[HOOK_PROCESS_WORLD] = function()
			if (player.pgod == GOD_AULE) then
				GRACE_DELAY = GRACE_DELAY + 1
				if GRACE_DELAY >= 15 then
					-- Aule likes Dwarves and Dark Elves (Eol's influence here)
					if 
					(get_race_name() ~= "Dwarf") and 
					(get_race_name() ~= "Petty-dwarf") and 
					(get_race_name() ~= "Gnome") and 
					(get_race_name() ~= "Dark-Elf") then
						set_grace(player.grace - 1)
					end
					
					-- Search inventory for axe or hammer - Gain 1 point of grace for each hammer or axe
					for i = 0, INVEN_TOTAL - 1 do
						if ((player.inventory(i).tval) == TV_AXE) then 
							set_grace(player.grace + 1)
						end
						if ((player.inventory(i).tval) == TV_HAFTED) then
							if (((player.inventory(i).sval) == SV_WAR_HAMMER) or ((player.inventory(i).sval) == SV_LUCERN_HAMMER) or ((player.inventory(i).sval) == SV_GREAT_HAMMER)) then
								set_grace(player.grace + 1)
							end
						end
					end
					
					if (player.praying == TRUE) then
						set_grace(player.grace - 2)
						
						-- Chance of casting Stoneskin if praying
						local chance
						if (player.grace >= 50000) then
							chance = 50000
						else
							chance = 50000 - player.grace
						end

						if (randint(100000) <= 100000 / chance) then
							aule_stone_skin()
							msg_print("Aule casts Stone Skin on you.")
						end
						
					end
					GRACE_DELAY = 0
				end
				
			end
		end,
		[HOOK_SACRIFICE_GOD] = function() 
			if (player.pgod == GOD_AULE) then 
				local ret, item, obj, value 
				ret, item = get_item( 
					"Sacrifice which item? ", 
					"You have nothing to sacrifice.", 
					USE_INVEN, 
					function(obj) 
						-- perhaps restrict this only to metal armour and weapons 
						if (obj.found == OBJ_FOUND_SELFMADE) then 
							return TRUE 
						end 
						return FALSE 
					end 
				) 

				-- Item selected 
				if ret == TRUE then 
					-- Increase piety by the value of the item / 10 
					-- object_value is not available in Lua, therefore I used the 
					-- cost of the base item, without magical boni 
					obj = get_object(item) 
					-- value = object_value(obj)/10 
					value = k_info[obj.k_idx + 1].cost/10

					set_grace(player.grace + value) 

					-- remove the object 
					inven_item_increase(item, -1) 
					inven_item_optimize(item) 
				end 
			end 
		end,
		[HOOK_MONSTER_DEATH] = function(m_idx)
			if (player.pgod == GOD_AULE) then
				m_ptr = monster(m_idx)
				if 
				(m_ptr.r_idx == test_monster_name("Petty-dwarf"))  or 
				(m_ptr.r_idx == test_monster_name("Petty-dwarf mage"))  or 
				(m_ptr.r_idx == test_monster_name("Dark dwarven warrior"))  or 
				(m_ptr.r_idx == test_monster_name("Dark dwarven smith"))  or 
				(m_ptr.r_idx == test_monster_name("Dark dwarven lord"))  or 
				(m_ptr.r_idx == test_monster_name("Dark dwarven priest"))  or 
				(m_ptr.r_idx == test_monster_name("Dwarven warrior")) then
					-- Aule dislikes you killing dwarves
					set_grace(player.grace - 20)
				end
				if 
				(m_ptr.r_idx == test_monster_name("Nar, the Dwarf"))  or 
				(m_ptr.r_idx == test_monster_name("Naugladur, Lord of Nogrod"))  or 
				(m_ptr.r_idx == test_monster_name("Telchar the Smith"))  or 
				(m_ptr.r_idx == test_monster_name("Fundin Bluecloak"))  or 
				(m_ptr.r_idx == test_monster_name("Khim, Son of Mim")) or 
				(m_ptr.r_idx == test_monster_name("Ibun, Son of Mim")) or 
				(m_ptr.r_idx == test_monster_name("Mim, Betrayer of Turin")) then
					-- These uniques earn a bigger penalty
					set_grace(player.grace - 500)
				end
			end
		end,
	}
}

GOD_VARDA = add_god
{
	["name"] = "Varda Elentari",
	["desc"] =
	{
		"The Queen of the Stars. In light is her power and joy."
	},
	["hooks"] =
	{
		[HOOK_CALC_LITE] = function()
			if (player.pgod == GOD_VARDA) then
				-- increase lite radius
				player.cur_lite = player.cur_lite + 1
			end
		end,
	},
}

GOD_ULMO = add_god
{
	["name"] = "Ulmo",
	["desc"] =
	{
		"Ulmo is called Lord of Waters, he rules all that is water on Arda."
	},
	["hooks"] =
	{
		[HOOK_MONSTER_DEATH] = function(m_idx)
			if (player.pgod == GOD_ULMO) then
				m_ptr = monster(m_idx)
				if 
				(m_ptr.r_idx == test_monster_name("Swordfish"))  or 
				(m_ptr.r_idx == test_monster_name("Barracuda"))  or 
				(m_ptr.r_idx == test_monster_name("Globefish"))  or 
				(m_ptr.r_idx == test_monster_name("Aquatic bear"))  or 
				(m_ptr.r_idx == test_monster_name("Pike"))  or 
				(m_ptr.r_idx == test_monster_name("Electric eel"))  or 
				(m_ptr.r_idx == test_monster_name("Giant crayfish"))  or 
				(m_ptr.r_idx == test_monster_name("Mermaid"))  or 
				(m_ptr.r_idx == test_monster_name("Leviathan"))  or 
				(m_ptr.r_idx == test_monster_name("Box jellyfish"))  or 
				(m_ptr.r_idx == test_monster_name("Giant piranha"))  or 
				(m_ptr.r_idx == test_monster_name("Piranha"))  or 
				(m_ptr.r_idx == test_monster_name("Ocean naga"))  or 
				(m_ptr.r_idx == test_monster_name("Whale"))  or 
				(m_ptr.r_idx == test_monster_name("Octopus"))  or 
				(m_ptr.r_idx == test_monster_name("Giant octopus"))  or 
				(m_ptr.r_idx == test_monster_name("Drowned soul"))  or 
				(m_ptr.r_idx == test_monster_name("Tiger shark"))  or 
				(m_ptr.r_idx == test_monster_name("Hammerhead shark"))  or 
				(m_ptr.r_idx == test_monster_name("Great white shark"))  or 
				(m_ptr.r_idx == test_monster_name("White shark"))  or 
				(m_ptr.r_idx == test_monster_name("Stargazer"))  or 
				(m_ptr.r_idx == test_monster_name("Flounder"))  or 
				(m_ptr.r_idx == test_monster_name("Giant turtle"))  or 
				(m_ptr.r_idx == test_monster_name("Killer whale"))  or 
				(m_ptr.r_idx == test_monster_name("Water naga"))  or 
				(m_ptr.r_idx == test_monster_name("Behemoth")) then
					-- He doesn't like it if you kill these monsters
					set_grace(player.grace - 20)
				end
				if 
				(m_ptr.r_idx == test_monster_name("Seahorse"))  or 
				(m_ptr.r_idx == test_monster_name("Aquatic elven warrior")) or
				(m_ptr.r_idx == test_monster_name("Aquatic elven mage")) or
				(m_ptr.r_idx == test_monster_name("Wavelord")) or
				(m_ptr.r_idx == test_monster_name("The Watcher in the Water")) then
					-- These monsters earn higher penalties
					set_grace(player.grace - 500)
				end
			end
		end,
		[HOOK_PROCESS_WORLD] = function()
			if (player.pgod == GOD_ULMO) then
				GRACE_DELAY = GRACE_DELAY + 1
				if GRACE_DELAY >= 15 then
					-- Ulmo likes the Edain (except Easterlings)
					if 
					(get_race_name() == "Human") or 
					(get_race_name() == "Dunadan") or 
					(get_race_name() == "Druadan") or 
					(get_race_name() == "RohanKnight") then 
						set_grace(player.grace + 1)
					
					elseif (
					(get_race_name() == "Easterling") or 
					(get_race_name() == "Demon") or
					(get_race_name() == "Orc")) then
					-- hated races
					set_grace(player.grace - 2)
					else
						set_grace(player.grace + 1)
					end
					
					if (player.praying == TRUE) then
						set_grace(player.grace - 1)
					end					
					-- Search inventory for axe or hammer - Gain 1 point of grace for each hammer or axe
					for i = 0, INVEN_TOTAL - 1 do
						if ((player.inventory(i).tval) == TV_POLEARM) then
							if ((player.inventory(i).sval) == SV_TRIDENT) then
								set_grace(player.grace + 1)
							end
						end
					end

					GRACE_DELAY = 0
				end
				
			end
		end,
	},
}

GOD_MANDOS = add_god
{
	["name"] = "Mandos",
	["desc"] =
	{
		"The Doomsman of the Valar and keeper of the slain."
	},
	["hooks"] =
	{
		[HOOK_PROCESS_WORLD] = function()
			if (player.pgod == GOD_MANDOS) then
				GRACE_DELAY = GRACE_DELAY + 1
				if GRACE_DELAY >= 15 then
					-- He loves astral beings 
					if (get_subrace_name() == "LostSoul") then
						set_grace(player.grace + 1)
					end

					-- He likes High Elves only, though, as races
					if (get_race_name() ~= "High-Elf") then 
						set_grace(player.grace - 1)
					end
				end
				-- piety increase if (condition)
				if (GRACE_DELAY >= 15) then
					if (
					(get_subrace_name() == "Vampire") or 
					(get_race_name() == "Demon")) then
					-- hated races
					set_grace(player.grace - 10)
					else
						set_grace(player.grace + 2)
					end
					-- he really doesn't like to be disturbed
					if (player.praying == TRUE) then
						set_grace(player.grace - 5)
					end					
					GRACE_DELAY = 0
				end
				
			end
		end,
		[HOOK_MONSTER_DEATH] = function(m_idx)
			if (player.pgod == GOD_MANDOS) then
				m_ptr = monster(m_idx)
				if
				(m_ptr.r_idx == test_monster_name("Vampire")) or 
				(m_ptr.r_idx == test_monster_name("Master vampire")) or 
				(m_ptr.r_idx == test_monster_name("Oriental vampire")) or 
				(m_ptr.r_idx == test_monster_name("Vampire lord")) or 
				(m_ptr.r_idx == test_monster_name("Vampire orc")) or 
				(m_ptr.r_idx == test_monster_name("Vampire yeek")) or 
				(m_ptr.r_idx == test_monster_name("Vampire ogre")) or 
				(m_ptr.r_idx == test_monster_name("Vampire troll")) or 
				(m_ptr.r_idx == test_monster_name("Vampire dwarf")) or 
				(m_ptr.r_idx == test_monster_name("Vampire gnome")) or
				(m_ptr.r_idx == test_monster_name("Elder vampire")) then
					-- He really likes it if you kill Vampires (but not the adventurer kind :P)
					set_grace(player.grace + 50)
				end

				if
				(m_ptr.r_idx == test_monster_name("Vampire elf")) or 
				(m_ptr.r_idx == test_monster_name("Thuringwethil, the Vampire Messenger")) then
					-- He *loves* it if you kill vampire Elves
					-- He will also thank you extra kindly if you kill Thuringwethil
					set_grace(player.grace + 200)
				end

				if 
				(m_ptr.r_idx == test_monster_name("Dark elf")) or 
				(m_ptr.r_idx == test_monster_name("Dark elven druid")) or 
				(m_ptr.r_idx == test_monster_name("Eol, the Dark Elf")) or 
				(m_ptr.r_idx == test_monster_name("Maeglin, the Traitor of Gondolin")) or 
				(m_ptr.r_idx == test_monster_name("Dark elven mage")) or 
				(m_ptr.r_idx == test_monster_name("Dark elven warrior")) or 
				(m_ptr.r_idx == test_monster_name("Dark elven priest")) or 
				(m_ptr.r_idx == test_monster_name("Dark elven lord")) or 
				(m_ptr.r_idx == test_monster_name("Dark elven warlock")) or 
				(m_ptr.r_idx == test_monster_name("Dark elven sorcerer")) then 
					-- He doesn't like it if you kill normal Elves (means more work for him :P)
					set_grace(player.grace - 20)
				end
				if 
				(m_ptr.r_idx == test_monster_name("Glorfindel of Rivendell")) or 
				(m_ptr.r_idx == test_monster_name("Finrod Felagund")) or 
				(m_ptr.r_idx == test_monster_name("Thranduil, King of the Wood Elves")) or 
				(m_ptr.r_idx == test_monster_name("Aquatic elven warrior")) or 
				(m_ptr.r_idx == test_monster_name("Aquatic elven mage")) or 
				(m_ptr.r_idx == test_monster_name("High-elven ranger")) or 
				(m_ptr.r_idx == test_monster_name("Elven archer")) then
					-- He hates it if you kill coaligned Elves
					set_grace(player.grace - 200)
				end
				if 
				(m_ptr.r_idx == test_monster_name("Child spirit")) or 
				(m_ptr.r_idx == test_monster_name("Young spirit")) or 
				(m_ptr.r_idx == test_monster_name("Mature spirit")) or 
				(m_ptr.r_idx == test_monster_name("Experienced spirit")) or 
				(m_ptr.r_idx == test_monster_name("Wise spirit")) then
					-- He *hates* it if you kill the coaligned Spirits
					set_grace(player.grace - 1000)
				end
			end
		end
	}
}
