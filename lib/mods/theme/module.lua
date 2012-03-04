add_module
{
	["name"]    = "Theme",
	["version"] = { 1, 2, 0 },
	["author"]  = { "furiosity", "furiosity@gmail.com" },
	["desc"] = {
		"A module that goes back to Tolkien roots, though by no means canonical.",
		"A new wilderness map, new monsters, objects, artifacts, uniques, ego items,",
		"terrain features, gods, races, subraces, and classes. Have fun. :-)",
	},

	["rand_quest"] = TRUE,
	["C_quest"] = TRUE,

	["base_dungeon"] = 4,
	["death_dungeon"] = 28,

	["astral_dungeon"] = 8,
	["astral_wild_x"] = 45,
	["astral_wild_y"] = 19,

	["random_artifact_weapon_chance"] = 30,
	["random_artifact_armor_chance"] = 30,
	["random_artifact_jewelry_chance"] = 30,

	["max_plev"] = 50,
	["max_skill_overage"] = 5,

	["mod_savefiles"]=
	{
		"Theme",
	},
	["layout"] = 
	{ 
	["apex"] = "theme", 
	["core"] = "theme",
	["data"] = "theme", 
	["dngn"] = "theme",
	["edit"] = "theme", 
	["file"] = "theme",
	["help"] = "theme",
	["note"] = "theme",
	["save"] = "theme",
	["scpt"] = "theme",
	["user"] = "theme",
	["pref"] = "theme",
	}, 
}