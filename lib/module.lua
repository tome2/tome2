add_module
{
	["name"]    = "ToME",
	["version"] = { 2, 3, 5 },
	["author"]  = { "DarkGod", "darkgod@t-o-m-e.net" },
	["desc"] = {
		"The Tales of Middle-earth, the standard and official game.",
		"You are set on a quest to investigate the old tower of Dol Guldur.",
		"But who knows what will happen...",
	},

	["rand_quest"] = TRUE,
	["C_quest"] = TRUE,

	["base_dungeon"] = 4,
	["death_dungeon"] = 28,

	["astral_dungeon"] = 8,
	["astral_wild_x"] = 45,
	["astral_wild_y"] = 19,

	["random_artifact_weapon_chance"] = 30,
	["random_artifact_armor_chance"] = 20,
	["random_artifact_jewelry_chance"] = 20,

	["max_plev"] = 50,
	["max_skill_overage"] = 4,
	["skill_per_level"] = function()
		return 6
	end,

	["mod_savefiles"]=
	{
		"ToME",
	},
}
