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
			}

			if t[typ][name] then ingame_help_doc(t[typ][name][1], t[typ][name][2])
			else ingame_help_doc("help.hlp", 0)
			end
	end,
}
