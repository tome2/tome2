-- Various 'U' powers

-- Invisibility power, for the mouse mimic shape
POWER_INVISIBILITY = add_power
{
	["name"] =      "invisibility",
	["desc"] =      "You are able melt into the shadows to become invisible.",
	["desc_get"] =  "You suddenly become able to melt into the shadows.",
	["desc_lose"] = "You lose your shadow-melting ability.",
	["level"] =     30,
	["cost"] =      10,
	["stat"] =      A_DEX,
	["fail"] =      20,
	["power"] =     function()
			set_invis(20 + randint(30), 30)
	end,
}

-- Web power, for the spider mimic shape
POWER_WEB = add_power
{
	["name"] =      "web",
	["desc"] =      "You are able throw a thick and very resistant spider web.",
	["desc_get"] =  "You suddenly become able to weave webs.",
	["desc_lose"] = "You lose your web-weaving capability.",
	["level"] =     25,
	["cost"] =      30,
	["stat"] =      A_DEX,
	["fail"] =      20,
	["power"] =     function()
			-- Warning, beware of f_info changes .. I hate to do that ..
			grow_things(16, 1 + (player.lev / 10))
	end,
}

-- Activating/stopping space-continuum
-- When stopped it will induce constant mana loss
player.corrupt_anti_teleport_stopped = FALSE
add_loadsave("player.corrupt_anti_teleport_stopped", FALSE)
POWER_COR_SPACE_TIME = add_power
{
	["name"] =      "control space/time continuum",
	["desc"] =      "You are able to control the space/time continuum.",
	["desc_get"] =  "You become able to control the space/time continuum.",
	["desc_lose"] = "You are no more able to control the space/time continuum.",
	["level"] =     1,
	["cost"] =      10,
	["stat"] =      A_WIS,
	["fail"] =      10,
	["power"] =     function()
		if player.corrupt_anti_teleport_stopped == TRUE then
			player.corrupt_anti_teleport_stopped = FALSE
			msg_print("You stop controlling your corruption.")
			player.update = bor(player.update, PU_BONUS)
		else
			player.corrupt_anti_teleport_stopped = TRUE
			msg_print("You start controlling your corruption, teleportation works once more.")
			player.update = bor(player.update, PU_BONUS)
		end
	end,
}
