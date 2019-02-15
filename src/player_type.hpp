#pragma once

#include "corrupt.hpp"
#include "defines.hpp"
#include "h-basic.hpp"
#include "help_info.hpp"
#include "inventory.hpp"
#include "object_type.hpp"
#include "powers.hpp"
#include "random_spell.hpp"
#include "spellbinder.hpp"

#include <array>
#include <unordered_set>

/*
 * Most of the "player" information goes here.
 *
 * This stucture gives us a large collection of player variables.
 *
 * This structure contains several "blocks" of information.
 *   (1) the "permanent" info
 *   (2) the "variable" info
 *   (3) the "transient" info
 *
 * All of the "permanent" info, and most of the "variable" info,
 * is saved in the savefile.  The "transient" info is recomputed
 * whenever anything important changes.
 */

struct player_type
{
	s32b lives = 0;                                         /* How many times we resurected */

	s16b oldpy = 0;                                         /* Previous player location */
	s16b oldpx = 0;                                         /* Previous player location */

	s16b py = 0;                                            /* Player location */
	s16b px = 0;                                            /* Player location */

	byte prace = 0;                                         /* Race index */
	byte pracem = 0;                                        /* Race Mod index */
	byte pclass = 0;                                        /* Class index */
	byte pspec = 0;                                         /* Class spec index */
	byte mimic_form = 0;                                    /* Actualy transformation */
	s16b mimic_level = 0;                                   /* Level of the mimic effect */

	std::array<object_type, INVEN_TOTAL> inventory { };     /* Player inventory */

	byte hitdie = 0;                                        /* Hit dice (sides) */
	u16b expfact = 0;                                       /* Experience factor */

	byte allow_one_death = 0;                               /* Blood of life */

	s32b au = 0;                                            /* Current Gold */

	s32b max_exp = 0;                                       /* Max experience */
	s32b exp = 0;                                           /* Cur experience */
	u16b exp_frac = 0;                                      /* Cur exp frac (times 2^16) */

	s16b lev = 0;                                           /* Level */

	s16b town_num = 0;                                      /* Current town number */
	s16b inside_quest = 0;                                  /* Inside quest level */

	s32b wilderness_x = 0;                                  /* Coordinates in the wilderness */
	s32b wilderness_y = 0;
	bool wild_mode = false;                                /* true = Small map, FLASE = Big map */
	bool old_wild_mode = false;                            /* true = Small map, FLASE = Big map */

	s16b mhp = 0;                                           /* Max hit pts */
	s16b chp = 0;                                           /* Cur hit pts */
	u16b chp_frac = 0;                                      /* Cur hit frac (times 2^16) */
	s16b hp_mod = 0;                                        /* A modificator (permanent) */

	s16b msp = 0;                                           /* Max mana pts */
	s16b csp = 0;                                           /* Cur mana pts */
	u16b csp_frac = 0;                                      /* Cur mana frac (times 2^16) */

	s16b msane = 0;                                         /* Max sanity */
	s16b csane = 0;                                         /* Cur sanity */
	u16b csane_frac = 0;                                    /* Cur sanity frac */

	s32b grace = 0;                                         /* Your God's appreciation factor. */
	s32b grace_delay = 0;                                   /* Delay factor for granting piety. */
	byte pgod = 0;                                          /* Your God. */
	bool praying = false;                                   /* Praying to your god. */
	s16b melkor_sacrifice = 0;                              /* How much hp has been sacrified for damage */

	s16b max_plv = 0;                                       /* Max Player Level */

	std::array<s16b, 6> stat_max { };                       /* Current "maximal" stat values */
	std::array<s16b, 6> stat_cur { };                       /* Current "natural" stat values */

	s16b luck_cur = 0;                                      /* Current "natural" luck value (range -30 <> 30) */
	s16b luck_max = 0;                                      /* Current "maximal base" luck value (range -30 <> 30) */
	s16b luck_base = 0;                                     /* Current "base" luck value (range -30 <> 30) */

	s16b speed_factor = 0;                                  /* Timed -- Fast */
	s16b fast = 0;                                          /* Timed -- Fast */
	s16b lightspeed = 0;                                    /* Timed -- Light Speed */
	s16b slow = 0;                                          /* Timed -- Slow */
	s16b blind = 0;                                         /* Timed -- Blindness */
	s16b paralyzed = 0;                                     /* Timed -- Paralysis */
	s16b confused = 0;                                      /* Timed -- Confusion */
	s16b afraid = 0;                                        /* Timed -- Fear */
	s16b image = 0;                                         /* Timed -- Hallucination */
	s16b poisoned = 0;                                      /* Timed -- Poisoned */
	s16b cut = 0;                                           /* Timed -- Cut */
	s16b stun = 0;                                          /* Timed -- Stun */

	s16b protevil = 0;                                      /* Timed -- Protection from Evil*/
	s16b invuln = 0;                                        /* Timed -- Invulnerable */
	s16b hero = 0;	                                        /* Timed -- Heroism */
	s16b shero = 0;	                                        /* Timed -- Super Heroism */
	s16b shield = 0;                                        /* Timed -- Shield Spell */
	s16b shield_power = 0;                                  /* Timed -- Shield Spell Power */
	s16b shield_opt = 0;                                    /* Timed -- Shield Spell options */
	s16b shield_power_opt = 0;                              /* Timed -- Shield Spell Power */
	s16b shield_power_opt2 = 0;                             /* Timed -- Shield Spell Power */
	s16b blessed = 0;                                       /* Timed -- Blessed */
	s16b tim_invis = 0;                                     /* Timed -- See Invisible */
	s16b tim_infra = 0;                                     /* Timed -- Infra Vision */

	s16b oppose_acid = 0;                                   /* Timed -- oppose acid */
	s16b oppose_elec = 0;                                   /* Timed -- oppose lightning */
	s16b oppose_fire = 0;                                   /* Timed -- oppose heat */
	s16b oppose_cold = 0;                                   /* Timed -- oppose cold */
	s16b oppose_pois = 0;                                   /* Timed -- oppose poison */
	s16b oppose_cc = 0;                                     /* Timed -- oppose chaos & confusion */

	s16b tim_esp = 0;                                       /* Timed ESP */
	s16b tim_wraith = 0;                                    /* Timed wraithform */
	s16b tim_ffall = 0;                                     /* Timed Levitation */
	s16b tim_fly = 0;                                       /* Timed Levitation */
	s16b tim_poison = 0;                                    /* Timed poison hands */
	s16b tim_thunder = 0;                                   /* Timed thunderstorm */
	s16b tim_thunder_p1 = 0;                                /* Timed thunderstorm */
	s16b tim_thunder_p2 = 0;                                /* Timed thunderstorm */

	s16b tim_project = 0;                                   /* Timed project upon melee blow */
	s16b tim_project_dam = 0;
	s16b tim_project_gf = 0;
	s16b tim_project_rad = 0;
	s16b tim_project_flag = 0;

	s16b tim_roots = 0;                                     /* Timed roots */
	s16b tim_roots_ac = 0;
	s16b tim_roots_dam = 0;

	s16b tim_invisible = 0;                                 /* Timed Invisibility */
	s16b tim_inv_pow = 0;                                   /* Power of timed invisibility */
	s16b tim_mimic = 0;                                     /* Timed Mimic */
	s16b tim_lite = 0;                                      /* Timed Lite */
	s16b tim_regen = 0;                                     /* Timed extra regen */
	s16b tim_regen_pow = 0;                                 /* Timed extra regen power */
	s16b holy = 0;                                          /* Holy Aura */
	s16b strike = 0;                                        /* True Strike(+25 hit) */
	s16b tim_reflect = 0;                                   /* Timed Reflection */
	s16b tim_deadly = 0;                                    /* Timed deadly blow */
	s16b prob_travel = 0;                                   /* Timed probability travel */
	s16b disrupt_shield = 0;                                /* Timed disruption shield */
	s16b parasite = 0;                                      /* Timed parasite */
	s16b parasite_r_idx = 0;                                /* Timed parasite monster */
	s16b absorb_soul = 0;                                   /* Timed soul absordtion */
	s16b tim_magic_breath = 0;                              /* Magical breathing -- can breath anywhere */
	s16b tim_water_breath = 0;                              /* Water breathing -- can breath underwater */
	s16b tim_precognition = 0;                              /* Timed precognition */

	s16b immov_cntr = 0;                                    /* Timed -- Last ``immovable'' command. */

	byte recall_dungeon = 0;                                /* Recall in which dungeon */
	s16b word_recall = 0;                                   /* Word of recall counter */

	s32b energy = 0;                                        /* Current energy */

	s16b food = 0;                                          /* Current nutrition */

	byte confusing = 0;                                     /* Glowing hands */

	bool old_cumber_armor = false;
	bool old_cumber_glove = false;
	bool old_heavy_wield = false;
	bool old_heavy_shoot = false;
	bool old_icky_wield = false;

	s16b old_lite = 0;                                      /* Old radius of lite (if any) */
	s16b old_view = 0;                                      /* Old radius of view (if any) */

	s16b old_food_aux = 0;                                  /* Old value of food */

	bool cumber_armor = false;                              /* Mana draining armor */
	bool cumber_glove = false;                              /* Mana draining gloves */
	bool heavy_wield = false;                               /* Heavy weapon */
	bool heavy_shoot = false;                               /* Heavy shooter */
	bool icky_wield = false;                                /* Icky weapon */

	bool immovable = false;                                 /* Immovable character */

	s16b cur_lite = 0;                                      /* Radius of lite (if any) */

	u32b notice = 0;                                        /* Special Updates (bit flags) */
	u32b update = 0;                                        /* Pending Updates (bit flags) */
	u32b redraw = 0;                                        /* Normal Redraws (bit flags) */
	u32b window = 0;                                        /* Window Redraws (bit flags) */

	std::array<s16b, 6> stat_use { };                       /* Current modified stats */
	std::array<s16b, 6> stat_top { };                       /* Maximal modified stats */

	std::array<s16b, 6> stat_add { };                       /* Modifiers to stat values */
	std::array<s16b, 6> stat_ind { };                       /* Indexes into stat tables */
	std::array<s16b, 6> stat_cnt { };                       /* Counter for temporary drains */
	std::array<s16b, 6> stat_los { };                       /* Amount of temporary drains */

	bool immune_acid = false;                               /* Immunity to acid */
	bool immune_elec = false;                               /* Immunity to lightning */
	bool immune_fire = false;                               /* Immunity to fire */
	bool immune_cold = false;                               /* Immunity to cold */
	bool immune_neth = false;                               /* Immunity to nether */

	bool resist_acid = false;                               /* Resist acid */
	bool resist_elec = false;                               /* Resist lightning */
	bool resist_fire = false;                               /* Resist fire */
	bool resist_cold = false;                               /* Resist cold */
	bool resist_pois = false;                               /* Resist poison */

	bool resist_conf = false;                               /* Resist confusion */
	bool resist_sound = false;                              /* Resist sound */
	bool resist_lite = false;                               /* Resist light */
	bool resist_dark = false;                               /* Resist darkness */
	bool resist_chaos = false;                              /* Resist chaos */
	bool resist_disen = false;                              /* Resist disenchant */
	bool resist_shard = false;                              /* Resist shards */
	bool resist_nexus = false;                              /* Resist nexus */
	bool resist_blind = false;                              /* Resist blindness */
	bool resist_neth = false;                               /* Resist nether */
	bool resist_fear = false;                               /* Resist fear */
	bool resist_continuum = false;                          /* Resist space-time continuum disruption */

	bool sensible_fire = false;                             /* Fire does more damage on the player */
	bool sensible_lite = false;                             /* Lite does more damage on the player and blinds her/him */

	bool reflect = false;                                   /* Reflect 'bolt' attacks */

	bool sh_fire = false;                                   /* Fiery 'immolation' effect */
	bool sh_elec = false;                                   /* Electric 'immolation' effect */

	bool wraith_form = false;                               /* wraithform */

	bool anti_magic = false;                                /* Anti-magic */
	bool anti_tele = false;                                 /* Prevent teleportation */

	bool sustain_str = false;                               /* Keep strength */
	bool sustain_int = false;                               /* Keep intelligence */
	bool sustain_wis = false;                               /* Keep wisdom */
	bool sustain_dex = false;                               /* Keep dexterity */
	bool sustain_con = false;                               /* Keep constitution */
	bool sustain_chr = false;                               /* Keep charisma */

	bool aggravate = false;                                 /* Aggravate monsters */
	bool teleport = false;                                  /* Random teleporting */

	bool exp_drain = false;                                 /* Experience draining */
	byte drain_mana = false;                                /* mana draining */
	byte drain_life = false;                                /* hp draining */

	bool magical_breath = false;                            /* Magical breathing -- can breath anywhere */
	bool water_breath = false;                              /* Water breathing -- can breath underwater */

	bool climb = false;                                     /* Can climb mountains */
	bool fly = false;                                       /* Can fly over some features */
	bool ffall = false;                                     /* No damage falling */

	bool lite = false;                                      /* Permanent light */
	bool free_act = false;                                  /* Never paralyzed */
	bool see_inv = false;                                   /* Can see invisible */
	bool regenerate = false;                                /* Regenerate hit pts */
	bool hold_life = false;                                 /* Resist life draining */
	bool slow_digest = false;                               /* Slower digestion */
	bool bless_blade = false;                               /* Blessed blade */
	byte xtra_might = 0;                                    /* Extra might bow */
	bool impact = false;                                    /* Earthquake blows */

	s16b invis = 0;                                         /* Invisibility */

	s16b dis_to_h = 0;                                      /* Known bonus to hit */
	s16b dis_to_d = 0;                                      /* Known bonus to dam */
	s16b dis_to_a = 0;                                      /* Known bonus to ac */

	s16b dis_ac = 0;                                        /* Known base ac */

	s16b to_l = 0;                                          /* Bonus to life */
	s16b to_m = 0;                                          /* Bonus to mana */
	s16b to_s = 0;                                          /* Bonus to spell */
	s16b to_h = 0;                                          /* Bonus to hit */
	s16b to_d = 0;                                          /* Bonus to dam */
	s16b to_h_melee = 0;                                    /* Bonus to hit for melee */
	s16b to_d_melee = 0;                                    /* Bonus to dam for melee */
	s16b to_h_ranged = 0;                                   /* Bonus to hit for ranged */
	s16b to_d_ranged = 0;                                   /* Bonus to dam for ranged */
	s16b to_a = 0;                                          /* Bonus to ac */

	s16b ac = 0;                                            /* Base ac */

	byte antimagic = 0;                                     /* Power of the anti magic field */
	byte antimagic_dis = 0;                                 /* Radius of the anti magic field */

	s16b see_infra;                                         /* Infravision range */

	s16b skill_dev = 0;                                     /* Skill: Magic Devices */
	s16b skill_sav = 0;                                     /* Skill: Saving throw */
	s16b skill_stl = 0;                                     /* Skill: Stealth factor */
	s16b skill_thn = 0;                                     /* Skill: To hit (normal) */
	s16b skill_thb = 0;                                     /* Skill: To hit (shooting) */
	s16b skill_tht = 0;                                     /* Skill: To hit (throwing) */
	s16b skill_dig = 0;                                     /* Skill: Digging */

	s16b num_blow = 0;                                      /* Number of blows */
	s16b num_fire = 0;                                      /* Number of shots */
	s16b xtra_crit = 0;                                     /* % of increased crits */

	byte throw_mult = 0;                                    /* Multiplier for throw damage */

	byte tval_ammo = 0;                                     /* Correct ammo tval */

	s16b pspeed = 0;                                        /* Current speed */

	u32b mimic_extra = 0;                                   /* Mimicry powers use that */
	u32b antimagic_extra = 0;                               /* Antimagic powers */
	u32b music_extra = 0;                                   /* Music songs */
	u32b necro_extra = 0;                                   /* Necro powers */
	u32b necro_extra2 = 0;                                  /* Necro powers */

	s16b dodge_chance = 0;                                  /* Dodging chance */

	u32b maintain_sum = 0;                                  /* Do we have partial summons */

	struct spellbinder spellbinder;

	const char *mimic_name = nullptr;

	char tactic = '\0';                                     /* from 128-4 "extremely coward" to, 128+4 "berserker" */
	char movement = '\0';                                   /* base movement way */

	s16b companion_killed = 0;                              /* Number of companion death */

	bool no_mortal = false;                                 /* Fated to never die by the hand of a mortal being */

	bool black_breath = false;                              /* The Tolkien's Black Breath */

	bool precognition = false;                              /* Like the cheat mode */

	/*** Extra flags ***/
	object_flag_set xtra_flags;

	/** Computed flags based on all worn items, etc. */
	object_flag_set computed_flags;

	/* Corruptions */
	std::array<bool, CORRUPTIONS_MAX> corruptions { };
	bool corrupt_anti_teleport_stopped = false;

	/*** Pet commands ***/
	byte pet_follow_distance = 0;                           /* Length of the imaginary "leash" for pets */
	byte pet_open_doors = 0;                                /* flag - allow pets to open doors */
	byte pet_pickup_items = 0;                              /* flag - allow pets to pickup items */

	s16b control = 0;                                       /* Controlled monster */
	byte control_dir;                                       /* Controlled monster */

	/*** Body changing variables ***/
	u16b body_monster = 0;                                  /* In which body is the player */
	bool disembodied = false;                               /* Is the player in a body ? */
	std::array<byte, INVEN_TOTAL-INVEN_WIELD> body_parts =  /* Which body parts does he have ? */
		{ };

	/* Astral */
	bool astral = false;                                    /* We started at the bottom ? */

	/* Powers; keys of Game::powers */
	std::unordered_set<int> powers;                 /* Actual powers */
	std::unordered_set<int> powers_mod;             /* Intrinsinc powers */

	/* Acquired abilities; indexes into ab_info[] */
	std::vector<u16b> abilities;

	/* Known inscriptions; true if known, false otherwise. */
	std::array<bool, MAX_INSCRIPTIONS> inscriptions;

	/* Skills */
	s16b skill_points = 0;
	s16b skill_last_level = 0;                              /* Prevents gaining skills by losing level and regaining them */
	s16b melee_style = 0;                                   /* How are  */
	s16b use_piercing_shots = 0;                            /* for archery */

	/* Dripping Tread spell timer */
	s16b dripping_tread = 0;

	/* Help */
	help_info help;

	/* Inertia control */
	s32b inertia_controlled_spell = 0;

	/* For automatic stat-gain */
	s16b last_rewarded_level = 0;

	/*** Temporary fields ***/

	bool did_nothing = false;                               /* True if the last action wasnt a real action */
	bool leaving = false;                                   /* True if player is leaving */

	/**
	 * Random spells.
	 */
	std::vector<random_spell> random_spells;

	/**
	 * Does the player have the given ability?
	 */
	bool has_ability(u16b ability_idx) const;

	/**
	 * Gain the given ability.
	 */
	void gain_ability(u16b ability_idx);

	/**
	 * Lose the given ability.
	 */
	void lose_ability(u16b ability_idx);

};
