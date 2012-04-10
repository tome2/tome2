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
	}
}
