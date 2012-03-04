-- This file holds various things that govern monster behaviour with respect to the player

-- Enables player to push past any monster who is >= MSTATUS_NEUTRAL.
-- Written by BauMog for the Intets Hevn module; permission granted to use in the Theme module

-- Adapted from defines.h
function cave_floor_bold(y, x)
	local c_ptr = cave(y, x);
	if(cave_is(c_ptr, FF1_FLOOR) == TRUE) and (c_ptr.feat ~= FEAT_MON_TRAP) then
		return TRUE
	else
		return FALSE
	end
end

-- Adapted from cmd1.c
function __hook_push_past(y, x)
	local c_ptr = cave(y, x);
	
	if(c_ptr.m_idx > 0) then
		m_ptr = monster(c_ptr.m_idx);
		if(m_ptr.status >= MSTATUS_NEUTRAL) then
			if(cave_floor_bold(y, x) == TRUE) or (m_ptr.flags2 == RF2_PASS_WALL) then
				msg_print(format("You push past %s.", monster_desc(m_ptr, 0)));
				m_ptr.fy = player.py;
				m_ptr.fx = player.px;
				cave(player.py, player.px).m_idx = c_ptr.m_idx;
				c_ptr.m_idx = 0;
			else
				msg_print(format("%s is in your way!", monster_desc(m_ptr, 0)));
				energy_use = 0;
			end
		end
	end
	
end

add_hook_script(HOOK_MOVE, "__hook_push_past", "__hook_push_past");

-- Monster vs. Player Race alignment script
-- From T-Plus by Ingeborg S. Norden

monst_al = {} 

function monst_al_add(status, mrs, prs) 
for i,v in mrs do 
-- added end 
if not monst_al[v] then monst_al[v] = {} end 
for j, w in prs do 
monst_al[v][w] = status 
end 
end 
end 

function monst_al_get(mr,pr) 
 if monst_al[mr] then return monst_al[mr][pr] 
 else return end 
end 

-- Maia aggravation for evil beings (provided that no demonic corruptions are present)
-- Based on parts of angel.lua from T-Plus by Ingeborg S. Norden

-- cast dispel evil with 0 damage every 10 turns 

TIMER_AGGRAVATE_EVIL = new_timer 
{ 
   ["enabled"] = FALSE, 
   ["delay"] = 10, 
   ["callback"] = function() 
      dispel_evil(0) 
   end, 
} 

add_hooks{ 
[HOOK_GAME_START] = function() 

  	if ((get_race_name() == "Maia") and 
	(player.corruption(CORRUPT_BALROG_AURA) ~= TRUE) and 
	(player.corruption(CORRUPT_BALROG_WINGS) ~= TRUE) and 
	(player.corruption(CORRUPT_BALROG_STRENGTH) ~= TRUE) and 
	(player.corruption(CORRUPT_BALROG_FORM) ~= TRUE)) then 
	-- "Proper" Maiar aggravate evil beings
    	TIMER_AGGRAVATE_EVIL.enabled = TRUE
	-- Good beings (except swans, GWoPs, Wyrm Spirits, and some joke uniques) are coaligned with Maiar 

	monst_al_add(MSTATUS_FRIEND, {25, 29, 45, 97, 109, 147, 225, 335, 346, 443, 581, 629, 699, 853, 984, 1007, 1017}, {21})

	-- Non-evil humanoids are neutral to Humans, Dunedain, Druedain, Rohirrim
	elseif ((get_race_name() == "Human") or 
	(get_race_name() == "Dunadan") or 
	(get_race_name() == "Druadan") or 
	(get_race_name() == "RohanKnight")) then
	monst_al_add(MSTATUS_NEUTRAL, {43, 45, 46, 83, 93, 97, 109, 110, 142, 147, 216, 225, 293, 345, 346, 693, 699, 937, 988, 997, 998, 1000},{0, 8, 12, 16})

	-- Non-evil sentient (and non-animal) creatures are neutral to Hobbits, Elves, Wood-Elves
	elseif ((get_race_name() == "Hobbit") or 
	(get_race_name() == "Elf") or 
	(get_race_name() == "Wood-Elf")) then
	monst_al_add(MSTATUS_NEUTRAL, {43, 45, 46, 83, 93, 97, 109, 110, 142, 147, 216, 225, 293, 345, 346, 693, 699, 937, 988, 997, 998, 1000, 74, 103, 882, 1017},{2, 3, 20})

	-- Gnome monsters are neutral to Gnomes
	elseif get_race_name() == "Gnome" then
	monst_al_add(MSTATUS_NEUTRAL, {103, 281, 680, 984, 1001, 1003, 1007, 1011, 1014, 1016},{4})

	-- Dwarven monsters are neutral to Petty-dwarves and Dwarves
	elseif ((get_race_name() == "Dwarf") or
	(get_race_name() == "Petty-Dwarf")) then
	monst_al_add(MSTATUS_NEUTRAL, {111, 112, 179, 180, 181, 182},{5, 13})

	-- If an Orc character worships Melkor, lower-level Orcs are neutral (not Uruk-hai, however)
	elseif ((get_race_name() == "Orc") and 
      (player.pgod == GOD_MELKOR)) then
	monst_al_add(MSTATUS_FRIEND, {87, 118, 126, 149, 244, 251, 264},{6})

	-- If a Troll character worships Melkor, Trolls are neutral (not Eldraks, Ettins, and War trolls, though)
	elseif ((get_race_name() == "Troll") and 
      (player.pgod == GOD_MELKOR)) then
	monst_al_add(MSTATUS_NEUTRAL, {297, 401, 403, 424, 454, 491, 496, 509, 538},{7})

	-- Ogres are neutral to Half-Ogres
	elseif get_race_name() == "Half-Ogre" then
	monst_al_add(MSTATUS_NEUTRAL, {262, 285, 415, 430, 479, 745, 918},{10})

	-- Bears are neutral to Beornings, except werebears.
	elseif get_race_name() == "Beorning" then
	monst_al_add(MSTATUS_NEUTRAL, {160, 173, 191, 854, 855, 867, 873},{11})

	-- Dark elven monsters are coaligned with Dark Elves
	elseif get_race_name() == "Dark-Elf" then
	monst_al_add(MSTATUS_FRIEND, {122, 178, 183, 226, 348, 375, 400, 657},{14})

	-- Plants are coaligned with Ents
	elseif get_race_name() == "Ent" then
	monst_al_add(MSTATUS_FRIEND, {248, 266, 317, 329, 396},{15})

	-- And since the above is largely useless except out in the wild...
	-- If an Ent worships Yavanna, lower-level animals are coaligned
	-- should make the early game a bit easier for Ents.
	elseif ((get_race_name() == "Ent") and
	(player.pgod == GOD_YAVANNA)) then
	monst_al_add(MSTATUS_FRIEND, {21, 23, 24, 25, 26, 27, 28, 29, 30, 31, 33, 35, 36, 37, 38, 39, 41, 49, 50, 52, 56, 57, 58, 59, 60, 61, 62, 69, 70, 75, 77, 78, 79, 86, 88, 89, 90, 95, 96, 105, 106, 114, 119, 120, 121, 123, 127, 134, 141, 143, 151, 154, 155, 156, 160, 161, 168, 171, 173, 174, 175, 176, 187, 191, 196, 197, 198, 210, 211, 213, 230, 236, 250, 259},{15})

	-- All non-evil non-neutral birds are coaligned with Eagles
	elseif get_race_name() == "Eagle" then
	monst_al_add(MSTATUS_FRIEND, {61, 141, 151, 279},{17})

	-- Hatchling dragons are coaligned with Dragons
	elseif get_race_name() == "Dragon" then
	monst_al_add(MSTATUS_FRIEND, {163, 164, 165, 166, 167, 204, 218, 219, 911},{18})

	-- Yeeks are neutral to Yeeks
	elseif get_race_name() == "Yeek" then
	monst_al_add(MSTATUS_NEUTRAL, {580, 583, 594, 653, 655, 659, 661},{19})

	-- Oathbreakers are coaligned if player is wielding Anduril
	-- It's dirty, but it works, and it doesn't bother checking demons and the races who can't wield weapons.
	elseif get_object(INVEN_WIELD).name1 == 83 then
	monst_al_add(MSTATUS_FRIEND, {731},{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 19, 20, 21, 22, 23})
	end
end,

[HOOK_LEVEL_END_GEN] = function() 

for i=0,m_max-1 do
	local monst = monster(i)
	local s = monst_al_get(monst.r_idx, player.prace)
	if s then monst.status = s end
end	

end, 

[HOOK_NEW_MONSTER] = function() 

for i=0,m_max-1 do
	local monst = monster(i)
	local s = monst_al_get(monst.r_idx, player.prace)
	if s then monst.status = s end
end	

end, 

}