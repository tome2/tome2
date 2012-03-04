-- Spells for Aule school

BOOK_AULE = 63 
 
AULE_FIREBRAND = add_spell 
{ 
	["name"] =	"Firebrand", 
	["school"] =	{SCHOOL_AULE}, 
	["level"] =	1, 
	["mana"] =	10, 
	["mana_max"] =	100, 
	["fail"] =	20, 
	["piety"] =	TRUE, 
	["stat"] =	A_WIS, 
	["random"] =	SKILL_SPIRITUALITY, 
	["spell"] =	function() 
		local type, rad
		local level = get_level(AULE_FIREBRAND)
		type = GF_FIRE
		
		if (get_level(AULE_FIREBRAND) > 30) then
			type = GF_HOLY_FIRE
		end
		
		rad = 0
		if (level >= 15) then 
			rad = 1 
		end
		return set_project(level + randint(20), 
		type, 4 + level, rad, 
		bor(PROJECT_STOP, PROJECT_KILL)) 
	end, 
	["info"] =	function() 
		local level = get_level(AULE_FIREBRAND) 
		return "dur "..(level).."+d20 dam "..(4 + level).."/blow" 
	end, 
	["desc"] =	{ 
		"Imbues your melee weapon with fire to deal more damage", 
		"At level 15 it spreads over a 1 radius zone around your target", 
		"At level 30 it deals holy fire damage" 
	} 
} 
 
AULE_ENCHANT_WEAPON = add_spell 
{
	["name"] =	"Enchant Weapon",
	["school"] =	{SCHOOL_AULE},
	["level"] =	10,
	["mana"] =	100, 
	["mana_max"] =	200, 
	["fail"] =	20, 
	["piety"] =	TRUE, 
	["stat"] =	A_WIS, 
	["random"] =	SKILL_SPIRITUALITY, 
	["spell"] =	function() 
		local level = get_level(AULE_ENCHANT_WEAPON) 
		local num_h, num_d, num_p 
		
		local ret, item, obj
		
		num_h = 1 + randint(level/12) 
		num_d = 0 
		num_p = 0 
		if (level >= 5) then 
			num_d = 1 + randint(level/12) 
		end 
		if (level >= 45) then 
			num_p = 1
		end 
		--return enchant_spell(num_h, num_d, 0, num_p) 
		
		ret, item = get_item("Which object do you want to enchant?",
					"You have no objects to enchant.",
					bor(USE_INVEN),
					function (obj)
						if obj.name1 > 0 then return FALSE end
						if (obj.tval == TV_MSTAFF) then
							return TRUE
						elseif (obj.tval == TV_BOW) then
							return TRUE
						elseif (obj.tval == TV_HAFTED) then
							return TRUE
						elseif (obj.tval == TV_POLEARM) then
							return TRUE
						elseif (obj.tval == TV_SWORD) then
							return TRUE
						elseif (obj.tval == TV_AXE) then
							return TRUE
						end
						return FALSE
					     end
		)
		if ret == FALSE then return FALSE end
		
		obj = get_object(item)
		
		obj.to_h = obj.to_h + num_h
		obj.to_d = obj.to_d + num_h
		obj.pval = obj.pval + num_p
		
		return TRUE

	end, 
	["info"] =	function() 
		return "tries "..(1 + get_level(AULE_ENCHANT_WEAPON)/12) 
	end, 
	["desc"] =	{
		"Tries to enchant a weapon to-hit", 
		"At level 5 it also enchants to-dam", 
		"At level 45 it enhances the special powers of magical weapons", 
		"The might of the enchantment increases with the level" 
	} 
} 
 
AULE_ENCHANT_ARMOUR = add_spell { 
	["name"] =	"Enchant Armour", 
	["school"] =	{SCHOOL_AULE}, 
	["level"] =	15, 
	["mana"] =	100, 
	["mana_max"] =	200, 
	["fail"] =	20, 
	["piety"] =	TRUE, 
	["stat"] =	A_WIS, 
	["random"] =	SKILL_SPIRITUALITY, 
	["spell"] =	function() 
		local level = get_level(AULE_ENCHANT_ARMOUR) 
		local num_h, num_d, num_a, num_p 
 		local ret, item, obj

		num_a = 1 + randint(level/10) 
		num_h = 0 
		num_d = 0 
		num_p = 0 
		if (level >= 20) then 
			num_h = 1 
			num_d = 1 
		end 
		if (level >= 40) then 
			num_p = 1 
		end 
		--return enchant_spell(num_h, num_d, num_a, num_p)
		
		ret, item = get_item("Which object do you want to enchant?",
					"You have no objects to enchant.",
					bor(USE_INVEN),
					function (obj)
						if obj.name1 > 0 then return FALSE end
						if (obj.tval == TV_BOOTS) then
							return TRUE
						elseif (obj.tval == TV_GLOVES) then
							return TRUE
						elseif (obj.tval == TV_HELM) then
							return TRUE
						elseif (obj.tval == TV_CROWN) then
							return TRUE
						elseif (obj.tval == TV_SHIELD) then
							return TRUE
						elseif (obj.tval == TV_CLOAK) then
							return TRUE
						elseif (obj.tval == TV_SOFT_ARMOR) then
							return TRUE
						elseif (obj.tval == TV_HARD_ARMOR) then
							return TRUE
						elseif (obj.tval == TV_DRAG_ARMOR) then
							return TRUE
						end
						return FALSE
					     end
		)
		if ret == FALSE then return FALSE end
		
		obj = get_object(item)
		
		obj.to_h = obj.to_h + num_h
		obj.to_d = obj.to_d + num_h
		obj.pval = obj.pval + num_p
		obj.to_a = obj.to_a + num_h
		
		return TRUE
		
	end, 
	["info"] =	function() 
		return "tries "..(1 + get_level(AULE_ENCHANT_ARMOUR)/10) 
	end, 
	["desc"] =	{
		"Tries to enchant a piece of armour", 
		"At level 20 it also enchants to-hit and to-dam", 
		"At level 40 it enhances the special powers of magical armour", 
		"The might of the enchantment increases with the level" 
	} 
} 
 
AULE_CHILD = add_spell
{ 
	["name"] =	"Child of Aule", 
	["school"] =	{SCHOOL_AULE}, 
	["level"] =	20, 
	["mana"] =	200, 
	["mana_max"] =	500, 
	["fail"] =	40, 
	["piety"] =	TRUE, 
	["stat"] =	A_WIS, 
	["random"] =	SKILL_SPIRITUALITY, 
	["spell"] =	function() 
		local y, x, m_idx

		y, x = find_position(player.py, player.px) 
		m_idx = place_monster_one(y, x, test_monster_name("Dwarven warrior"), 0, FALSE, MSTATUS_FRIEND) 
 
		if m_idx ~= 0 then 
			monster_set_level(m_idx, 20 + get_level(AULE_CHILD, 70, 0)) 
			return TRUE 
		end 
	end, 
	["info"] =	function() 
		return "level "..(20 + get_level(AULE_CHILD, 70)) 
	end, 
	["desc"] =	{ 
		"Summons a levelled Dwarven warrior to help you battle the forces", 
		"of Morgoth" 
	} 
} 