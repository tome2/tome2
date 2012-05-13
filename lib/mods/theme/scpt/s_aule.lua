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
	["spell"] =	function() return aule_firebrand_spell() end,
	["info"] =	function() return aule_firebrand_info() end,
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
	["spell"] =	function() return aule_enchant_weapon_spell() end,
	["info"] =	function() return aule_enchant_weapon_info() end,
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
	["spell"] =	function() return aule_enchant_armour_spell() end,
	["info"] =	function() return aule_enchant_armour_info() end,
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
	["spell"] =	function() return aule_child_spell() end,
	["info"] =	function() return aule_child_info() end,
	["desc"] =	{ 
		"Summons a levelled Dwarven warrior to help you battle the forces", 
		"of Morgoth" 
	} 
}
