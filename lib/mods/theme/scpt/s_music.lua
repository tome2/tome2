-- handle the music school
-- *ALL* lasting spell must return the mana cost in the lasting function

MUSIC_STOP = add_spell
{
	["name"] =      "Stop singing(I)",
	["school"] =    {SCHOOL_MUSIC},
	["level"] =     1,
	["mana"] =      0,
	["mana_max"] =  0,
	["fail"] =      -400,
	["stat"] =      A_CHR,
	["random"] =    SKILL_MUSIC,
	["pval"] =      1,
	["blind"] =     FALSE,
	["spell"] =     function() return music_stop_singing_spell() end,
	["info"] =      function() return music_stop_singing_info() end,
	["desc"] =      {
			"Stops the current song, if any."
	}
}

--- Drums
MUSIC_HOLD = add_spell
{
	["name"] =      "Holding Pattern(I)",
	["school"] =    {SCHOOL_MUSIC},
	["level"] =     1,
	["mana"] =      1,
	["mana_max"] =  10,
	["fail"] =      20,
	["stat"] =      A_CHR,
	["random"] =    SKILL_MUSIC,
	["pval"] =      1,
	["blind"] =     FALSE,
	["lasting"] =   function() return music_holding_pattern_lasting() end,
	["spell"] =     function() return music_holding_pattern_spell() end,
	["info"] =      function() return music_holding_pattern_info() end,
	["desc"] =      {
			"Slows down all monsters listening the song.",
			"Consumes the amount of mana each turn.",
	}
}

MUSIC_CONF = add_spell
{
	["name"] =      "Illusion Pattern(II)",
	["school"] =    {SCHOOL_MUSIC},
	["level"] =     5,
	["mana"] =      2,
	["mana_max"] =  15,
	["fail"] =      30,
	["stat"] =      A_CHR,
	["random"] =    SKILL_MUSIC,
	["pval"] =      2,
	["blind"] =     FALSE,
	["lasting"] =   function() return music_illusion_pattern_lasting() end,
	["spell"] =     function() return music_illusion_pattern_spell() end,
	["info"] =      function() return music_illusion_pattern_info() end,
	["desc"] =      {
			"Tries to confuse all monsters listening the song.",
			"Consumes the amount of mana each turn.",
	}
}

MUSIC_STUN = add_spell
{
	["name"] =      "Stun Pattern(IV)",
	["school"] =    {SCHOOL_MUSIC},
	["level"] =     10,
	["mana"] =      3,
	["mana_max"] =  25,
	["fail"] =      45,
	["stat"] =      A_CHR,
	["random"] =    SKILL_MUSIC,
	["pval"] =      4,
	["blind"] =     FALSE,
	["lasting"] =   function() return music_stun_pattern_lasting() end,
	["spell"] =     function() return music_stun_pattern_spell() end,
	["info"] =      function() return music_stun_pattern_info() end,
	["desc"] =      {
			"Stuns all monsters listening the song.",
			"Consumes the amount of mana each turn.",
	}
}

--- Harps
MUSIC_LITE = add_spell
{
	["name"] =      "Song of the Sun(I)",
	["school"] =    {SCHOOL_MUSIC},
	["level"] =     1,
	["mana"] =      1,
	["mana_max"] =  1,
	["fail"] =      20,
	["stat"] =      A_CHR,
	["random"] =    SKILL_MUSIC,
	["blind"] =     FALSE,
	["pval"] =      1,
	["lasting"] =   function() return music_song_of_the_sun_lasting() end,
	["spell"] =     function() return music_song_of_the_sun_spell() end,
	["info"] =      function() return music_song_of_the_sun_info() end,
	["desc"] =      {
			"Provides light as long as you sing.",
			"Consumes the amount of mana each turn.",
	}
}

MUSIC_HEAL = add_spell
{
	["name"] =      "Flow of Life(II)",
	["school"] =    {SCHOOL_MUSIC},
	["level"] =     7,
	["mana"] =      5,
	["mana_max"] =  30,
	["fail"] =      35,
	["stat"] =      A_CHR,
	["random"] =    SKILL_MUSIC,
	["pval"] =      2,
	["lasting"] =   function() return music_flow_of_life_lasting() end,
	["spell"] =     function() return music_flow_of_life_spell() end,
	["info"] =      function() return music_flow_of_life_info() end,
	["desc"] =      {
			"Heals you as long as you sing.",
			"Consumes the amount of mana each turn.",
	}
}

MUSIC_HERO = add_spell
{
	["name"] =      "Heroic Ballad(II)",
	["school"] =    {SCHOOL_MUSIC},
	["level"] =     10,
	["mana"] =      4,
	["mana_max"] =  14,
	["fail"] =      45,
	["stat"] =      A_CHR,
	["random"] =    SKILL_MUSIC,
	["pval"] =      2,
	["lasting"] =   function() return music_heroic_ballad_lasting() end,
	["spell"] =     function() return music_heroic_ballad_spell() end,
	["info"] =      function() return music_heroic_ballad_info() end,
	["desc"] =      {
			"Increases melee accuracy",
			"At level 10 it increases it even more and reduces armour a bit",
			"At level 20 it increases it again",
			"At level 25 it grants protection against chaos and confusion",
			"Consumes the amount of mana each turn.",
	}
}

MUSIC_TIME = add_spell
{
	["name"] =      "Hobbit Melodies(III)",
	["school"] =    {SCHOOL_MUSIC},
	["level"] =     20,
	["mana"] =      10,
	["mana_max"] =  30,
	["fail"] =      70,
	["stat"] =      A_CHR,
	["random"] =    SKILL_MUSIC,
	["pval"] =      3,
	["lasting"] =   function() return music_hobbit_melodies_lasting() end,
	["spell"] =     function() return music_hobbit_melodies_spell() end,
	["info"] =      function() return music_hobbit_melodies_info() end,
	["desc"] =      {
			"Greatly increases your reflexes allowing you to block more melee blows.",
			"At level 15 it also makes you faster.",
			"Consumes the amount of mana each turn.",
	}
}

MUSIC_MIND = add_spell
{
	["name"] =      "Clairaudience(IV)",
	["school"] =    {SCHOOL_MUSIC},
	["level"] =     25,
	["mana"] =      15,
	["mana_max"] =  30,
	["fail"] =      75,
	["stat"] =      A_CHR,
	["random"] =    SKILL_MUSIC,
	["pval"] =      4,
	["lasting"] =   function() return music_clairaudience_lasting() end,
	["spell"] =     function() return music_clairaudience_spell() end,
	["info"] =      function() return music_clairaudience_info() end,
	["desc"] =      {
			"Allows you to sense monster minds as long as you sing.",
			"At level 10 it identifies all objects in a radius on the floor,",
			"as well as probing monsters in that radius.",
			"Consumes the amount of mana each turn.",
	}
}

--- Horns

MUSIC_BLOW = add_spell
{
	["name"] =      "Blow(I)",
	["school"] =    {SCHOOL_MUSIC},
	["level"] =     4,
	["mana"] =      3,
	["mana_max"] =  30,
	["fail"] =      20,
	["stat"] =      A_CHR,
	["random"] =    SKILL_MUSIC,
	["pval"] =      1,
	["spell"] =     function() return music_blow_spell() end,
	["info"] =      function() return music_blow_info() end,
	["desc"] =      {
			"Produces a powerful, blowing, sound all around you.",
	}
}

MUSIC_WIND = add_spell
{
	["name"] =      "Gush of Wind(II)",
	["school"] =    {SCHOOL_MUSIC},
	["level"] =     14,
	["mana"] =      15,
	["mana_max"] =  45,
	["fail"] =      30,
	["stat"] =      A_CHR,
	["random"] =    SKILL_MUSIC,
	["pval"] =      2,
	["spell"] =     function() return music_gush_of_wind_spell() end,
	["info"] =      function() return music_gush_of_wind_info() end,
	["desc"] =      {
			"Produces a outgoing gush of wind that sends monsters away.",
	}
}

MUSIC_YLMIR = add_spell
{
	["name"] =      "Horns of Ylmir(III)",
	["school"] =    {SCHOOL_MUSIC},
	["level"] =     20,
	["mana"] =      25,
	["mana_max"] =  30,
	["fail"] =      20,
	["stat"] =      A_CHR,
	["random"] =    SKILL_MUSIC,
	["pval"] =      3,
	["spell"] =     function() return music_horns_of_ylmir_spell() end,
	["info"] =      function() return music_horns_of_ylmir_info() end,
	["desc"] =      {
			"Produces an earth shaking sound.",
	}
}

MUSIC_AMBARKANTA = add_spell
{
	["name"] =      "Ambarkanta(IV)",
	["school"] =    {SCHOOL_MUSIC},
	["level"] =     25,
	["mana"] =      70,
	["mana_max"] =  70,
	["fail"] =      60,
	["stat"] =      A_CHR,
	["random"] =    SKILL_MUSIC,
	["pval"] =      4,
	["spell"] =     function() return music_ambarkanta_spell() end,
	["info"] =      function() return music_ambarkanta_info() end,
	["desc"] =      {
			"Produces a reality shaking sound that transports you to a nearly",
			"identical reality.",
	}
}
