-- This file contains all the new gods

GOD_AULE = add_god
{
	["name"] = "Aule the Smith",
	["desc"] =
	{
		"Aule is a smith, and the creator of the Dwarves."
	},
	["hooks"] =
	{
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
