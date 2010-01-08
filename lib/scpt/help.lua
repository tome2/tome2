-- Ingame contextual help

-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-----------------------Here comes the definition of help-----------------------
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------

ingame_help
{
	["hook"] =      HOOK_MOVE,
	["event"] =     function(y, x) if cave(y, x).feat == FEAT_BETWEEN then return TRUE end end,
	["desc"] =
	{
		"Void Jumpgates can be entered by pressing the > key. They will transport",
		"you to another jumpgate, but beware of the cold damage that might kill you.",
	}
}

ingame_help
{
	["hook"] =      HOOK_MOVE,
	["event"] =     function(y, x) if cave(y, x).feat == FEAT_FOUNTAIN then return TRUE end end,
	["desc"] =
	{
		"Fountains are always magical. You can quaff from them by pressing H.",
		"Beware that unlike potions they cannot be identified.",
	}
}

ingame_help
{
	["hook"] =      HOOK_MOVE,
	["event"] =     function(y, x) if cave(y, x).o_idx ~= 0 then return TRUE end end,
	["desc"] =
	{
		"So you found your first item! Nice, eh? Now when you stumble across",
		"objects, you can pick them up by pressing g, and if you are wondering",
		"what they do, press I (then *, then the letter for the item) to get",
		"some basic information. You may also want to identify them with scrolls,",
		"staves, rods or spells.",
	}
}

ingame_help
{
	["hook"] =      HOOK_MOVE,
	["event"] =     function(y, x) if (cave(y, x).feat >= FEAT_ALTAR_HEAD) and (cave(y, x).feat <= FEAT_ALTAR_TAIL) then return TRUE end end,
	["desc"] =
	{
		"Altars are the way to reach the Valar, powers of the world,",
		"usualy called Gods. You can press O to become a follower.",
		"Beware that once you follow a god, you are not allowed to change.",
		"For an exact description of what gods do and want, read the documentation."
	}
}

-- Beware this one, if Bree is moved from 21, 34 (y, x) on the wilderness map it will break
ingame_help
{
	["hook"] =      HOOK_END_TURN,
	["event"] =     function(y, x)
		if ((player.wilderness_x ~= 34) or (player.wilderness_y ~= 21) and (player.astral == FALSE)) then return TRUE end
	end,
	["desc"] =
	{
		"Ahh wilderness travel... The overview mode will allow you to travel",
		"fast, but that comes to the cost of GREATLY increased food consumption.",
		"So you should bring lots of food and really watch your hunger status.",
		"To enter the overview mode, press < while in the wilderness.",
	}
}

ingame_help
{
	["hook"] =      HOOK_PLAYER_LEVEL,
	["event"] =     function(y, x) if player.lev > 1 then return TRUE end end,
	["desc"] =
	{
		"Ok, so you now gained a level, and you have skill points to spend.",
		"To do so simply press G to learn skills. Reading the documentation",
		"about skills and abilities is also strongly recommended.",
	}
}

ingame_help
{
	["hook"] =      HOOK_MOVE,
	["event"] =     function(y, x) if cave(y, x).feat == FEAT_MORE then return TRUE end end,
	["desc"] =
	{
		"Ah, this is a stair, or a way into something. Press > to enter it.",
		"But be ready to fight what lies within, for it might not be too friendly.",
	}
}

ingame_help
{
	["callback"] =  "monster_chat",
	["desc"] =
	{
		"Somebody is speaking to you it seems. You can talk back with the Y key.",
		"This can lead to quests. You can also give items to 'monsters' with the y key.",
	}
}

ingame_help
{
	["hook"] =      HOOK_END_TURN,
	["event"] =     function(y, x) return TRUE end,
	["desc"] =
	{
		"Welcome to ToME! I am the spirit of knowledge and my task is to help you",
		"to get used to how to play. I have prepared a #vparchment#y for you to #vread#y.",
		"Press r, then space then select it. You can also check the documentation",
		"by pressing ? at (nearly) any time.",
		"The first place you can explore is Barrow-downs. Go to the west of town",
		"and you should see a #v>#y there.",
		"If you miss any of this you can press ctrl+p to see your message log.",
		"Now I must reveal your task here. You are on a quest to investigate",
		"the dreadful tower of Dol Guldur in the Mirkwood forest to see what evil",
		"lurks there, but beware, you are not yet ready.",
		"If you do not want me to bother you any more with tips, press = then go",
		"into the ToME options and deactivate the ingame_help option.",
		"You can see your quest log by pressing ctrl+q. Now go to your destiny!",
	}
}

ingame_help
{
	["no_test"] =   TRUE,
	["callback"] =  "select_context",
	["fct"] =       function(typ, name)
			-- list of files for classes, { filename, anchor }
			local t =
			{
				["race"] =
				{
					["Beorning"] = { "r_beorn.txt", 0 },
					["DeathMold"] = { "r_deathm.txt", 0 },
					["Dark-Elf"] = { "r_drkelf.txt", 0 },
					["Dunadan"] = { "r_dunad.txt", 0 },
					["Dwarf"] = { "r_dwarf.txt", 0 },
					["Elf"] = { "r_elf.txt", 0 },
					["Ent"] = { "r_ent.txt", 0 },
					["Gnome"] = { "r_gnome.txt", 0 },
					["Half-Elf"] = { "r_hafelf.txt", 0 },
					["Half-Ogre"] = { "r_hafogr.txt", 0 },
					["High-Elf"] = { "r_hielf.txt", 0 },
					["Hobbit"] = { "r_hobbit.txt", 0 },
					["Human"] = { "r_human.txt", 0 },
					["Kobold"] = { "r_kobold.txt", 0 },
					["Maia"] = { "r_maia.txt", 0 },
					["Orc"] = { "r_orc.txt", 0 },
					["Petty-Dwarf"] = { "r_pettyd.txt", 0 },
					["RohanKnight"] = { "r_rohank.txt", 0 },
					["Thunderlord"] = { "r_thlord.txt", 0 },
					["Troll"] = { "r_troll.txt", 0 },
					["Wood-Elf"] = { "r_wodelf.txt", 0 },
					["Yeek"] = { "r_yeek.txt", 0 },
				},
				["subrace"] =
				{
					["Barbarian"] = { "rm_barb.txt", 0 },
					["Classical"] = { "rm_class.txt", 0 },
					["Corrupted"] = { "rm_corru.txt", 0 },
					["Hermit"] = { "rm_herm.txt", 0 },
					["LostSoul"] = { "rm_lsoul.txt", 0 },
					["Skeleton"] = { "rm_skel.txt", 0 },
					["Spectre"] = { "rm_spec.txt", 0 },
					["Vampire"] = { "rm_vamp.txt", 0 },
					["Zombie"] = { "rm_zomb.txt", 0 },
				},
				["class"] =
				{
					["Alchemist"] = { "c_alchem.txt", 0 },
					["Archer"] = { "c_archer.txt", 0 },
					["Assassin"] = { "c_assass.txt", 0 },
					["Axemaster"] = { "c_axemas.txt", 0 },
					["Bard"] = { "c_bard.txt", 0 },
					["Dark-Priest"] = { "c_pr_drk.txt", 0 },
					["Demonologist"] = { "c_demono.txt", 0 },
					["Druid"] = { "c_druid.txt", 0 },
					["Geomancer"] = { "c_geoman.txt", 0 },
					["Haftedmaster"] = { "c_hafted.txt", 0 },
					["Loremaster"] = { "c_lorema.txt", 0 },
					["Mage"] = { "c_mage.txt", 0 },
					["Mimic"] = { "c_mimic.txt", 0 },
					["Mindcrafter"] = { "c_mindcr.txt", 0 },
					["Monk"] = { "c_monk.txt", 0 },
					["Necromancer"] = { "c_necro.txt", 0 },
					["Paladin"] = { "c_palad.txt", 0 },
					["Polearmmaster"] = { "c_polear.txt", 0 },
					["Possessor"] = { "c_posses.txt", 0 },
					["Priest"] = { "c_priest.txt", 0 },
					["Priest(Eru)"] = { "c_pr_eru.txt", 0 },
					["Priest(Manwe)"] = { "c_pr_man.txt", 0 },
					["Ranger"] = { "c_ranger.txt", 0 },
					["Rogue"] = { "c_rogue.txt", 0 },
					["Runecrafter"] = { "c_runecr.txt", 0 },
					["Sorceror"] = { "c_sorcer.txt", 0 },
					["Summoner"] = { "c_summon.txt", 0 },
					["Swordmaster"] = { "c_swordm.txt", 0 },
					["Symbiant"] = { "c_symbia.txt", 0 },
					["Thaumaturgist"] = { "c_thaum.txt", 0 },
					["Unbeliever"] = { "c_unbel.txt", 0 },
					["Warper"] = { "c_warper.txt", 0 },
					["Warrior"] = { "c_warrio.txt", 0 },
				},
				["god"] =
				{
					["Eru Iluvatar"] = { "g_eru.txt", 0 },
					["Manwe Sulimo"] = { "g_manwe.txt", 0 },
					["Tulkas"] = { "g_tulkas.txt", 0 },
					["Melkor Bauglir"] = { "g_melkor.txt", 0 },
					["Yavanna Kementari"] = { "g_yavann.txt", 0 },  
				},
				["skill"] =
				{
					["Air"] = { "skills.txt", 27 },
					["Alchemy"] = { "skills.txt", 49 },
					["Antimagic"] = { "skills.txt", 50 },
					["Archery"] = { "skills.txt", 08 },
					["Axe-mastery"] = { "skills.txt", 05 },
					["Backstab"] = { "skills.txt", 18 },
					["Barehand-combat"] = { "skills.txt", 13 },
					["Boomerang-mastery"] = { "skills.txt", 12 },
					["Boulder-throwing"] = { "skills.txt", 58 },
					["Bow-mastery"] = { "skills.txt", 10 },
					["Combat"] = { "skills.txt", 01 },
					["Conveyance"] = { "skills.txt", 30 },
					["Corpse-preservation"] = { "skills.txt", 44 },
					["Critical-hits"] = { "skills.txt", 04 },
					["Crossbow-mastery"] = { "skills.txt", 11 },
					["Demonology"] = { "skills.txt", 52 },
					["Disarming"] = { "skills.txt", 16 },
					["Divination"] = { "skills.txt", 31 },
					["Dodging"] = { "skills.txt", 20 },
					["Druidistic"] = { "skills.txt", 40 },
					["Earth"] = { "skills.txt", 28 },
					["Fire"] = { "skills.txt", 25 },
					["Geomancy"] = { "skills.txt", 60 },
					["Hafted-mastery"] = { "skills.txt", 06 },
					["Magic"] = { "skills.txt", 21 },
					["Magic-Device"] = { "skills.txt", 54 },
					["Mana"] = { "skills.txt", 24 },
					["Meta"] = { "skills.txt", 29 },
					["Mimicry"] = { "skills.txt", 47 },
					["Mind"] = { "skills.txt", 33 },
					["Mindcraft"] = { "skills.txt", 41 },
					["Monster-lore"] = { "skills.txt", 42 },
					["Music"] = { "skills.txt", 59 },
					["Nature"] = { "skills.txt", 34 },
					["Necromancy"] = { "skills.txt", 35 },
					["Polearm-mastery"] = { "skills.txt", 07 },
					["Possession"] = { "skills.txt", 45 },
					["Prayer"] = { "skills.txt", 39 },
					["Runecraft"] = { "skills.txt", 36 },
					["Sling-mastery"] = { "skills.txt", 09 },
					["Sneakiness"] = { "skills.txt", 14 },
					["Spell-power"] = { "skills.txt", 22 },
					["Spirituality"] = { "skills.txt", 38 },
					["Sorcery"] = { "skills.txt", 23 },
					["Stealing"] = { "skills.txt", 19 },
					["Stealth"] = { "skills.txt", 15 },
					["Stunning-blows"] = { "skills.txt", 53 },
					["Summoning"] = { "skills.txt", 43 },
					["Sword-mastery"] = { "skills.txt", 03 },
					["Symbiosis"] = { "skills.txt", 46 },
					["Temporal"] = { "skills.txt", 32 },
					["Thaumaturgy"] = { "skills.txt", 37 },
					["Udun"] = { "skills.txt", 48 },
					["Weaponmastery"] = { "skills.txt", 02 },
					["Water"] = { "skills.txt", 26 },
				},
				["ability"] = 
				{
				    ["Spread blows"] = { "ability.txt", 02 },
				    ["Tree walking"] = { "ability.txt", 03 },
				    ["Perfect casting"] = { "ability.txt", 04 },
				    ["Extra Max Blow(1)"] = { "ability.txt", 05 },
				    ["Extra Max Blow(2)"] = { "ability.txt", 06 },
				    ["Ammo creation"] = { "ability.txt", 07 },
				    ["Touch of death"] = { "ability.txt", 08 },
				    ["Artifact Creation"] = { "ability.txt", 09 },
				    ["Far reaching attack"] = { "ability.txt", 10 },
				    ["Trapping"] = { "ability.txt", 11 },
				    ["Undead Form"] = { "ability.txt", 12 },
				},
			}

			if t[typ][name] then ingame_help_doc(t[typ][name][1], t[typ][name][2])
			else ingame_help_doc("help.hlp", 0)
			end
	end,
}

ingame_help
{
	["hook"] =      HOOK_IDENTIFY,
	["event"] =     function(i, mode)
				if mode == "full" then
					local obj = get_object(i)
					local f1, f2, f3, f4, f5, esp = object_flags(obj)
					if band(f5, TR5_SPELL_CONTAIN) ~= 0 then return TRUE end
				end
			end,
	["desc"] =
	{
		"Ah, an item that can contain a spell. To use it you must have some levels of",
		"Magic skill and then you get the option to copy a spell when pressing m.",
		"Then just select which spell to copy and to which object. Note that doing so",
		"is permanent; the spell cannot be removed or changed later.",
	}
}

ingame_help
{
	["hook"] =      HOOK_GET,
	["event"] =     function(obj, idx) if obj.tval == TV_BATERIE then return TRUE end end,
	["desc"] =
	{
		"Ah, an essence! Those magical containers stores energies. They are used",
		"with the Alchemy skill to create or modify the powers of items.",
	}
}

ingame_help
{
	["hook"] =      HOOK_GET,
	["event"] =     function(obj, idx) if obj.tval == TV_RUNE1 or obj.tval == TV_RUNE2 then return TRUE end end,
	["desc"] =
	{
		"Ah, a rune! Runes are used with the Runecraft skill to allow you to",
		"create spells on your own.",
	}
}

ingame_help
{
	["hook"] =      HOOK_GET,
	["event"] =     function(obj, idx) if obj.tval == TV_ROD_MAIN then return TRUE end end,
	["desc"] =
	{
		"This is a rod. You will need to attach a rod tip to it before you",
		"can use it. This main part of the rod may give the rod bonuses",
		"like quicker charging time, or a larger capacity for charges.",
	}
}

ingame_help
{
	["hook"] =      HOOK_GET,
	["event"] =     function(obj, idx) if obj.tval == TV_ROD then return TRUE end end,
	["desc"] =
	{
		"You've found a rod-tip! You will need to attach it to a rod base",
		"before you can use it. Once it has been attatched (use the 'z' key)",
		"you cannot unattach it! The rod tip will determine the effect of",
		"the rod. To use your rod, 'z'ap it once it has been assembled.",
	}
}

ingame_help
{
	["hook"] =      HOOK_GET,
	["event"] =     function(obj, idx) if obj.tval == TV_TRAPKIT then return TRUE end end,
	["desc"] =
	{
		"Ooooh, a trapping kit. If you have ability in the trapping skill,",
		"you can lay this trap (via the 'm' key) to harm unsuspecting foes.",
		"You'll generally need either some ammo or magic device depending",
		"on the exact type of trap kit.",
	}
}

ingame_help
{
	["hook"] =      HOOK_RECALC_SKILLS,
	["event"] =     function() if game.started and (get_melee_skills() > 1) then return TRUE end end,
	["desc"] =
	{
		"Ah, you now possess more than one melee type. To switch between them press m",
		"and select the switch melee type option.",
	}
}

ingame_help
{
	["hook"] =      HOOK_GET,
	["event"] =     function(obj, idx) if obj.tval == TV_WAND or obj.tval == TV_STAFF then return TRUE end end,
	["desc"] =
	{
		"You've found a magical device, either a staff or a wand. Each staff",
		"contains a spell, often from one of the primary magic schools. There",
		"is a lot of information you can find about this object if you identify",
		"it and 'I'nspect it. Check the help file on Magic for more about these.",
	}
}

ingame_help
{
	["hook"] =      HOOK_PLAYER_LEVEL,
	["event"] =     function(y, x) if player.lev >= 20 then return TRUE end end,
	["desc"] =
	{
		"I see you are now at least level 20. Nice! If you want to gloat about your",
		"character you could press 'C' then 'f' to make a character dump and post it to",
		"http://angband.oook.cz/ where it will end up in the ladder.",
	}
}
