-- Definition of the corruptions
-- Theme adds the restriction T-Plus has for Maiar: they may only gain the Balrog corruptions.

-- The Balrog corruptions
CORRUPT_BALROG_AURA = add_corruption
{
	["color"]       = TERM_ORANGE,
	["name"]	= "Balrog Aura",
	["get_text"]    = "A corrupted wall of flames surrounds you.",
	["lose_text"]   = "The wall of corrupted flames abandons you.",
	["desc"]	=
	{
			  "  Surrounds you with a fiery aura",
			  "  But it can burn scrolls when you read them"
	},
}

CORRUPT_BALROG_WINGS = add_corruption
{
	["color"]       = TERM_ORANGE,
	["name"]	= "Balrog Wings",
	["get_text"]    = "Wings of shadow grow in your back.",
	["lose_text"]   = "The wings in your back fall apart.",
	["desc"]	=
	{
			  "  Creates ugly, but working, wings allowing you to fly",
			  "  But it reduces charisma by 4 and dexterity by 2"
	},
}

CORRUPT_BALROG_STRENGTH = add_corruption
{
	["color"]       = TERM_ORANGE,
	["name"]	= "Balrog Strength",
	["get_text"]    = "Your muscles get unnatural strength.",
	["lose_text"]   = "Your muscles get weaker again.",
	["desc"]	=
	{
			  "  Provides 3 strength and 1 constitution",
			  "  But it reduces charisma by 1 and dexterity by 3"
	},
}

CORRUPT_BALROG_FORM = add_corruption
{
	["color"]       = TERM_YELLOW,
	["name"]	= "Balrog Form",
	["get_text"]    = "You feel the might of a Balrog inside you.",
	["lose_text"]   = "The presence of the Balrog seems to abandon you.",
	["desc"]	=
	{
			  "  Allows you to turn into a Balrog at will",
			  "  You need Balrog Wings, Balrog Aura and Balrog Strength to activate it"
	},
	["depends"]     =
	{
			[CORRUPT_BALROG_AURA] = TRUE,
			[CORRUPT_BALROG_WINGS] = TRUE,
			[CORRUPT_BALROG_STRENGTH] = TRUE
	},
}


-- The Demon corruptions
CORRUPT_DEMON_SPIRIT = add_corruption
{
	["color"]       = TERM_RED,
	["name"]	= "Demon Spirit",
	["get_text"]    = "Your spirit opens to corrupted thoughts.",
	["lose_text"]   = "Your spirit closes again to the corrupted thoughts.",
	["desc"]	=
	{
			  "  Increases your intelligence by 1",
			  "  But reduce your charisma by 2",
	},
}

CORRUPT_DEMON_HIDE = add_corruption
{
	["color"]       = TERM_RED,
	["name"]	= "Demon Hide",
	["get_text"]    = "Your skin grows into a thick hide.",
	["lose_text"]   = "Your skin returns to a natural state.",
	["desc"]	=
	{
			  "  Increases your armour class by your level",
			  "  Provides immunity to fire at level 40",
			  "  But reduces speed by your level / 7",
	},
}

CORRUPT_DEMON_BREATH = add_corruption
{
	["color"]       = TERM_RED,
	["name"]	= "Demon Breath",
	["get_text"]    = "Your breath becomes mephitic.",
	["lose_text"]   = "Your breath is once again normal.",
	["desc"]	=
	{
			  "  Provides fire breath",
			  "  But gives a small chance to spoil potions when you quaff them",
	},
}

CORRUPT_DEMON_REALM = add_corruption
{
	["color"]       = TERM_L_RED,
	["name"]	= "Demon Realm",
	["get_text"]    = "You feel more attuned to the demon realm.",
	["lose_text"]   = "You lose your attunement to the demon realm.",
	["desc"]	=
	{
			  "  Provides access to the demon school skill and the use of demonic equipment",
			  "  You need Demon Spirit, Demon Hide and Demon Breath to activate it"
	},
	["depends"]     =
	{
			[CORRUPT_DEMON_SPIRIT] = TRUE,
			[CORRUPT_DEMON_HIDE] = TRUE,
			[CORRUPT_DEMON_BREATH] = TRUE
	},
}


-- Teleportation corruptions

-- Random teleportation will ask for confirmation 70% of the time
-- But 30% of the time it will teleport, without asking
CORRUPT_RANDOM_TELEPORT = add_corruption
{
	["color"]       = TERM_GREEN,
	["name"]	= "Random teleportation",
	["get_text"]    = "Space seems to fizzle around you.",
	["lose_text"]   = "Space solidify again around you.",
	["desc"]	=
	{
			  "  Randomly teleports you around",
	},
	-- No oppose field, it will be automatically set when we declare the anti-telep corruption to oppose us
}

-- Anti-teleportation corruption, can be stopped with this power
CORRUPT_ANTI_TELEPORT = add_corruption
{
	["color"]       = TERM_GREEN,
	["name"]	= "Anti-teleportation",
	["get_text"]    = "Space continuum freezes around you.",
	["lose_text"]   = "Space continuum can once more be altered around you.",
	["desc"]	=
	{
			  "  Prevents all teleportations, be it of you or monsters",
	},
	["oppose"]     	=
	{
			[CORRUPT_RANDOM_TELEPORT] = TRUE
	},
}


-- Troll blood
CORRUPT_TROLL_BLOOD = add_corruption
{
	["color"]       = TERM_GREEN,
	["name"]	= "Troll Blood",
	["get_text"]    = "Your blood thickens, you sense corruption in it.",
	["lose_text"]   = "Your blood returns to a normal state.",
	["desc"]	=
	{
			  "  Troll blood flows in your veins, granting increased regeneration",
			  "  It also enables you to feel the presence of other troll beings",
			  "  But it will make your presence more noticeable and aggravating",
	},
}

-- The vampire corruption set
CORRUPT_VAMPIRE_TEETH = add_corruption
{
	["group"]       = "Vampire",
	["removable"]   = FALSE,
	["color"]       = TERM_L_DARK,
	["name"]	= "Vampiric Teeth",
	["get_text"]    = "You grow vampiric teeth!",
	["lose_text"]   = "BUG! this should not happen",
	["desc"]	=
	{
			  "  Your teeth allow you to drain blood to feed yourself",
			  "  However your stomach now only accepts blood.",
	},
	["allow"]       = function()
		if test_race_flags(1, PR1_NO_SUBRACE_CHANGE) == FALSE then return not nil else return nil end
	end,
	["gain"]	= function() player_gain_vampire_teeth() end,
}
CORRUPT_VAMPIRE_STRENGTH = add_corruption
{
	["group"]       = "Vampire",
	["removable"]   = FALSE,
	["color"]       = TERM_L_DARK,
	["name"]	= "Vampiric Strength",
	["get_text"]    = "Your body seems more dead than alive.",
	["lose_text"]   = "BUG! this should not happen",
	["desc"]	=
	{
			  "  Your body seems somewhat dead",
			  "  In this near undead state it has improved strength, constitution and intelligence",
			  "  But reduced dexterity, wisdom and charisma.",
	},
	["depends"]     =
	{
			[CORRUPT_VAMPIRE_TEETH] = TRUE,
	},
	["gain"]	= function() player_gain_vampire_strength() end,
}
CORRUPT_VAMPIRE_VAMPIRE = add_corruption
{
	["group"]       = "Vampire",
	["removable"]   = FALSE,
	["color"]       = TERM_L_DARK,
	["name"]	= "Vampire",
	["get_text"]    = "You die to be reborn in a Vampire form.",
	["lose_text"]   = "BUG! this should not happen",
	["desc"]	=
	{
			  "  You are a Vampire. As such you resist cold, poison, darkness and nether.",
			  "  Your life is sustained, but you cannot stand the light of the sun."
	},
	["depends"]     =
	{
			[CORRUPT_VAMPIRE_STRENGTH] = TRUE,
	},
	["gain"]	= function() player_gain_vampire() end,
}

-- The old activable corruptions / mutations

MUT1_SPIT_ACID = add_corruption
{
	["color"] = TERM_RED,
	["name"]  = "Ancalagon's Breath",
	["get_text"]    = "You gain the ability to spit acid.",
	["lose_text"]   = "You lose the ability to spit acid.",
	["desc"]        = 
	{
			  "  Fires an acid ball.",
			  "  Damage=level Radius 1+(level/30)",
			  "  Level=9, Cost=9, Stat=DEX, Difficulty=15",
	},
}

MUT1_BR_FIRE = add_corruption
{
	["color"] = TERM_RED,
	["name"]  = "Smaug's Breath",
	["get_text"]    = "You gain the ability to breathe fire.",
	["lose_text"]   = "You lose the ability to breathe fire.",
	["desc"]        = 
	{
			  "  Fires a fire ball.",
			  "  Damage=2*level Radius 1+(level/20)",
			  "  Level=20, Cost=10, Stat=CON, Difficulty=18",
	},
}

MUT1_HYPN_GAZE = add_corruption
{
	["color"] = TERM_RED,
	["name"]  = "Glaurung's Gaze",
	["get_text"]    = "Your eyes look mesmerizing...",
	["lose_text"]   = "Your eyes look uninteresting.",
	["desc"]        = 
	{
			  "  Tries to make a monster your pet.",
			  "  Power=level",
			  "  Level=12, Cost=12, Stat=CHR, Difficulty=18",
	},
}

MUT1_TELEKINES = add_corruption
{
	["color"] = TERM_RED,
	["name"]  = "Saruman's Power",
	["get_text"]    = "You gain the ability to move objects telekinetically.",
	["lose_text"]   = "You lose the ability to move objects telekinetically.",
	["desc"]        = 
	{
			  "  Move an object in line of sight to you.",
			  "  Max weight equal to (level) pounds",
			  "  Level=9, Cost=9, Stat=WIS, Difficulty=14",
	},
}

MUT1_VTELEPORT = add_corruption
{
	["color"] = TERM_RED,
	["name"]  = "Teleport",
	["get_text"]    = "You gain the power of teleportation at will.",
	["lose_text"]   = "You lose the power of teleportation at will.",
	["desc"]        = 
	{
			  "  Teleports the player at will.",
			  "  Distance 10+4*level squares",
			  "  Level=7, Cost=7, Stat=WIS, Difficulty=15",
	},
}

MUT1_MIND_BLST = add_corruption
{
	["color"] = TERM_RED,
	["name"]  = "Glaurung's Spell",
	["get_text"]    = "You gain the power of Mind Blast.",
	["lose_text"]   = "You lose the power of Mind Blast.",
	["desc"]        = 
	{
			  "  Fires a mind blasting bolt (psi damage).",
			  "  Psi Damage (3+(level-1)/5)d3",
			  "  Level=5, Cost=3, Stat=WIS, Difficulty=15",
	},
}

MUT1_VAMPIRISM = add_corruption
{
	["color"] = TERM_RED,
	["name"]  = "Vampiric Drain",
	["get_text"]    = "You become vampiric.",
	["lose_text"]   = "You are no longer vampiric.",
	["desc"]        = 
	{
			  "  You can drain life from a foe like a vampire.",
			  "  Drains (level+1d(level))*(level/10) hitpoints,",
			  "  heals you and satiates you. Doesn't work on all monsters",
			  "  Level=4, Cost=5, Stat=CON, Difficulty=9",
	},
}

MUT1_SMELL_MET = add_corruption
{
	["color"] = TERM_RED,
	["name"]  = "Carcharoth's Nose",
	["get_text"]    = "You smell a metallic odour.",
	["lose_text"]   = "You no longer smell a metallic odour.",
	["desc"]        = 
	{
			  "  You can detect nearby precious metal (treasure).",
			  "  Radius 25",
			  "  Level=3, Cost=2, Stat=INT, Difficulty=12",
	},
}

MUT1_SMELL_MON = add_corruption
{
	["color"] = TERM_RED,
	["name"]  = "Huan's Nose",
	["get_text"]    = "You smell filthy monsters.",
	["lose_text"]   = "You no longer smell filthy monsters.",
	["desc"]        = 
	{
			  "  You can detect nearby monsters.",
			  "  Radius 25",
			  "  Level=5, Cost=4, Stat=INT, Difficulty=15",
	},
}

MUT1_BLINK = add_corruption
{
	["color"] = TERM_RED,
	["name"]  = "Blink",
	["get_text"]    = "You gain the power of minor teleportation.",
	["lose_text"]   = "You lose the power of minor teleportation.",
	["desc"]        = 
	{
			  "  You can teleport yourself short distances (10 squares).",
			  "  Level=3, Cost=3, Stat=WIS, Difficulty=12",
	},
}

MUT1_EAT_ROCK = add_corruption
{
	["color"] = TERM_RED,
	["name"]  = "Eat Rock",
	["get_text"]    = "The walls look delicious.",
	["lose_text"]   = "The walls look unappetizing.",
	["desc"]        = 
	{
			  "  You can consume solid rock with food benefit,",
			  "  leaving an empty space behind.",
			  "  Level=8, Cost=12, Stat=CON, Difficulty=18",
	},
}

MUT1_SWAP_POS = add_corruption
{
	["color"] = TERM_RED,
	["name"]  = "Swap Position",
	["get_text"]    = "You feel like walking a mile in someone else's shoes.",
	["lose_text"]   = "You feel like staying in your own shoes.",
	["desc"]        = 
	{
			  "  You can switch locations with another being,",
			  "  unless it resists teleportation.",
			  "  Level=15, Cost=12, Stat=DEX, Difficulty=16",
	},
}

MUT1_SHRIEK = add_corruption
{
	["color"] = TERM_RED,
	["name"]  = "Shriek",
	["get_text"]    = "Your vocal cords get much tougher.",
	["lose_text"]   = "Your vocal cords get much weaker.",
	["desc"]        = 
	{
			  "  Fires a sound ball and aggravates monsters.",
			  "  Damage=level*4, Radius=8, centered on player",
			  "  Level=4, Cost=4, Stat=CON, Difficulty=6",
	},
}

MUT1_ILLUMINE = add_corruption
{
	["color"] = TERM_RED,
	["name"]  = "Illuminate",
	["get_text"]    = "You can light up rooms with your presence.",
	["lose_text"]   = "You can no longer light up rooms with your presence.",
	["desc"]        = 
	{
			  "  You can emit bright light that illuminates an area.",
			  "  Damage=2d(level/2) Radius=(level/10)+1",
			  "  Level=3, Cost=2, Stat=INT, Difficulty=10",
	},
}

MUT1_DET_CURSE = add_corruption
{
	["color"] = TERM_RED,
	["name"]  = "Detect Curses",
	["get_text"]    = "You can feel evil magics.",
	["lose_text"]   = "You can no longer feel evil magics.",
	["desc"]        = 
	{
			  "  You can feel the danger of evil magic.",
			  "  It detects cursed items in the inventory",
			  "  Level=7, Cost=14, Stat=WIS, Difficulty=14",
	},
}

MUT1_BERSERK = add_corruption
{
	["color"] = TERM_RED,
	["name"]  = "Berserk",
	["get_text"]    = "You feel a controlled rage.",
	["lose_text"]   = "You no longer feel a controlled rage.",
	["desc"]        = 
	{
			  "  You can drive yourself into a berserk frenzy.",
			  "  It grants super-heroism. Duration=10+1d(level)",
			  "  Level=8, Cost=8, Stat=STR, Difficulty=14",
	},
}


MUT1_MIDAS_TCH = add_corruption
{
	["color"] = TERM_RED,
	["name"]  = "Midas touch",
	["get_text"]    = "You gain the Midas touch.",
	["lose_text"]   = "You lose the Midas touch.",
	["desc"]        = 
	{
			  "  You can turn ordinary items to gold.",
			  "  Turns a non-artifact object into 1/3 its value in gold",
			  "  Level=10, Cost=5, Stat=INT, Difficulty=12",
	},
}

MUT1_GROW_MOLD = add_corruption
{
	["color"] = TERM_RED,
	["name"]  = "Grow Mold",
	["get_text"]    = "You feel a sudden affinity for mold.",
	["lose_text"]   = "You feel a sudden dislike for mold.",
	["desc"]        = 
	{
			  "  You can cause mold to grow near you.",
			  "  Summons up to 8 molds around the player",
			  "  Level=1, Cost=6, Stat=CON, Difficulty=14",
	},
}

MUT1_RESIST = add_corruption
{
	["color"] = TERM_RED,
	["name"]  = "Resist Elements",
	["get_text"]    = "You feel like you can protect yourself.",
	["lose_text"]   = "You feel like you might be vulnerable.",
	["desc"]        = 
	{
			  "  You can harden yourself to the ravages of the elements.",
			  "  Level-dependent chance of gaining resistances to the four ",
			  "  elements and poison. Duration=20 + d20",
			  "  Level=10, Cost=12, Stat=CON, Difficulty=12",
	},
}

MUT1_EARTHQUAKE = add_corruption
{
	["color"] = TERM_RED,
	["name"]  = "Earthquake",
	["get_text"]    = "You gain the ability to wreck the dungeon.",
	["lose_text"]   = "You lose the ability to wreck the dungeon.",
	["desc"]        = 
	{
			  "  You can bring down the dungeon around your ears.",
			  "  Radius=10, center on the player",
			  "  Level=12, Cost=12, Stat=STR, Difficulty=16",
	},
}
--[[
CORRUPT_ = add_corruption
{
	["color"]       = TERM_GREEN,
	["name"]	= "",
	["get_text"]    = "",
	["lose_text"]   = "",
	["desc"]	=
	{
			  "  ",
	},
	["hooks"]       =
	{
	},
}
]]
