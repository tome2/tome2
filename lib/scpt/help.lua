-- Ingame contextual help

-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-----------------------Here comes the definition of help-----------------------
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------

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
	["no_test"] =   TRUE,
	["callback"] =  "select_context",
	["fct"] =       function(typ, name)
			-- list of files for classes, { filename, anchor }
			local t =
			{
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
