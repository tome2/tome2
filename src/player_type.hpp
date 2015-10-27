#pragma once

#include "corrupt.hpp"
#include "h-basic.h"
#include "help_info.hpp"
#include "inventory.hpp"
#include "object_type.hpp"
#include "powers.hpp"

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
	s32b lives;             /* How many times we resurected */

	s16b oldpy;		/* Previous player location -KMW- */
	s16b oldpx;		/* Previous player location -KMW- */

	s16b py;		/* Player location */
	s16b px;		/* Player location */

	byte psex;              /* Sex index */
	byte prace;             /* Race index */
	byte pracem;            /* Race Mod index */
	byte pclass;		/* Class index */
	byte pspec;		/* Class spec index */
	byte mimic_form;        /* Actualy transformation */
	s16b mimic_level;       /* Level of the mimic effect */
	byte oops;              /* Unused */

	object_type inventory[INVEN_TOTAL];     /* Player inventory */

	byte hitdie;		/* Hit dice (sides) */
	u16b expfact;           /* Experience factor */

	byte preserve;		/* Preserve artifacts */
	byte special;           /* Special levels */
	byte allow_one_death;   /* Blood of life */

	s16b age;			/* Characters age */
	s16b ht;			/* Height */
	s16b wt;			/* Weight */
	s16b sc;			/* Social Class */


	s32b au;			/* Current Gold */

	s32b max_exp;		/* Max experience */
	s32b exp;			/* Cur experience */
	u16b exp_frac;		/* Cur exp frac (times 2^16) */

	s16b lev;			/* Level */

	s16b town_num;			/* Current town number */
	s16b inside_quest;		/* Inside quest level */

	s32b wilderness_x;              /* Coordinates in the wilderness */
	s32b wilderness_y;
	bool_ wild_mode;                 /* TRUE = Small map, FLASE = Big map */
	bool_ old_wild_mode;             /* TRUE = Small map, FLASE = Big map */

	s16b mhp;			/* Max hit pts */
	s16b chp;			/* Cur hit pts */
	u16b chp_frac;                  /* Cur hit frac (times 2^16) */
	s16b hp_mod;                    /* A modificator(permanent) */

	s16b msp;                       /* Max mana pts */
	s16b csp;                       /* Cur mana pts */
	u16b csp_frac;          /* Cur mana frac (times 2^16) */

	s16b msane;                   /* Max sanity */
	s16b csane;                   /* Cur sanity */
	u16b csane_frac;              /* Cur sanity frac */

	s32b grace;                     /* Your God's appreciation factor. */
	s32b grace_delay;               /* Delay factor for granting piety. */
	byte pgod;                      /* Your God. */
	bool_ praying;                   /* Praying to your god. */
	s16b melkor_sacrifice;          /* How much hp has been sacrified for damage */

	s16b max_plv;                   /* Max Player Level */

	s16b stat_max[6];               /* Current "maximal" stat values */
	s16b stat_cur[6];               /* Current "natural" stat values */

	s16b luck_cur;                  /* Current "natural" luck value (range -30 <> 30) */
	s16b luck_max;                  /* Current "maximal base" luck value (range -30 <> 30) */
	s16b luck_base;                 /* Current "base" luck value (range -30 <> 30) */

	s16b speed_factor;		/* Timed -- Fast */
	s16b fast;			/* Timed -- Fast */
	s16b lightspeed;                /* Timed -- Light Speed */
	s16b slow;			/* Timed -- Slow */
	s16b blind;			/* Timed -- Blindness */
	s16b paralyzed;		/* Timed -- Paralysis */
	s16b confused;		/* Timed -- Confusion */
	s16b afraid;		/* Timed -- Fear */
	s16b image;			/* Timed -- Hallucination */
	s16b poisoned;		/* Timed -- Poisoned */
	s16b cut;			/* Timed -- Cut */
	s16b stun;			/* Timed -- Stun */

	s16b protevil;          /* Timed -- Protection from Evil*/
	s16b protgood;          /* Timed -- Protection from Good*/
	s16b protundead;        /* Timed -- Protection from Undead*/
	s16b invuln;		/* Timed -- Invulnerable */
	s16b hero;			/* Timed -- Heroism */
	s16b shero;			/* Timed -- Super Heroism */
	s16b shield;		/* Timed -- Shield Spell */
	s16b shield_power;      /* Timed -- Shield Spell Power */
	s16b shield_opt;        /* Timed -- Shield Spell options */
	s16b shield_power_opt;  /* Timed -- Shield Spell Power */
	s16b shield_power_opt2; /* Timed -- Shield Spell Power */
	s16b blessed;		/* Timed -- Blessed */
	s16b tim_invis;		/* Timed -- See Invisible */
	s16b tim_infra;		/* Timed -- Infra Vision */

	s16b oppose_acid;	/* Timed -- oppose acid */
	s16b oppose_elec;	/* Timed -- oppose lightning */
	s16b oppose_fire;	/* Timed -- oppose heat */
	s16b oppose_cold;	/* Timed -- oppose cold */
	s16b oppose_pois;	/* Timed -- oppose poison */
	s16b oppose_ld;         /* Timed -- oppose light & dark */
	s16b oppose_cc;         /* Timed -- oppose chaos & confusion */
	s16b oppose_ss;         /* Timed -- oppose sound & shards */
	s16b oppose_nex;        /* Timed -- oppose nexus */

	s16b tim_esp;       	/* Timed ESP */
	s16b tim_wraith;    	/* Timed wraithform */
	s16b tim_ffall;     	/* Timed Levitation */
	s16b tim_fly;       	/* Timed Levitation */
	s16b tim_poison;    	/* Timed poison hands */
	s16b tim_thunder;   	/* Timed thunderstorm */
	s16b tim_thunder_p1;	/* Timed thunderstorm */
	s16b tim_thunder_p2;	/* Timed thunderstorm */

	s16b tim_project;       /* Timed project upon melee blow */
	s16b tim_project_dam;
	s16b tim_project_gf;
	s16b tim_project_rad;
	s16b tim_project_flag;

	s16b tim_roots;         /* Timed roots */
	s16b tim_roots_ac;
	s16b tim_roots_dam;

	s16b tim_invisible; /* Timed Invisibility */
	s16b tim_inv_pow;   /* Power of timed invisibility */
	s16b tim_mimic;     /* Timed Mimic */
	s16b tim_lite;      /* Timed Lite */
	s16b tim_regen;     /* Timed extra regen */
	s16b tim_regen_pow; /* Timed extra regen power */
	s16b holy;          /* Holy Aura */
	s16b strike;        /* True Strike(+25 hit) */
	s16b tim_reflect;   /* Timed Reflection */
	s16b tim_deadly;    /* Timed deadly blow */
	s16b prob_travel;   /* Timed probability travel */
	s16b disrupt_shield;/* Timed disruption shield */
	s16b parasite;      /* Timed parasite */
	s16b parasite_r_idx;/* Timed parasite monster */
	s16b absorb_soul;   /* Timed soul absordtion */
	s16b tim_magic_breath;      /* Magical breathing -- can breath anywhere */
	s16b tim_water_breath;      /* Water breathing -- can breath underwater */
	s16b tim_precognition;      /* Timed precognition */

	s16b immov_cntr;    /* Timed -- Last ``immovable'' command. */

	s16b recall_dungeon;    /* Recall in which dungeon */
	s16b word_recall;	/* Word of recall counter */

	s32b energy;            /* Current energy */

	s16b food;			/* Current nutrition */

	byte confusing;		/* Glowing hands */
	byte searching;		/* Currently searching */

	bool_ old_cumber_armor;
	bool_ old_cumber_glove;
	bool_ old_heavy_wield;
	bool_ old_heavy_shoot;
	bool_ old_icky_wield;

	s16b old_lite;		/* Old radius of lite (if any) */
	s16b old_view;		/* Old radius of view (if any) */

	s16b old_food_aux;	/* Old value of food */


	bool_ cumber_armor;	/* Mana draining armor */
	bool_ cumber_glove;	/* Mana draining gloves */
	bool_ heavy_wield;	/* Heavy weapon */
	bool_ heavy_shoot;	/* Heavy shooter */
	bool_ icky_wield;	/* Icky weapon */
	bool_ immovable;         /* Immovable character */

	s16b cur_lite;		/* Radius of lite (if any) */


	u32b notice;		/* Special Updates (bit flags) */
	u32b update;		/* Pending Updates (bit flags) */
	u32b redraw;		/* Normal Redraws (bit flags) */
	u32b window;		/* Window Redraws (bit flags) */

	s16b stat_use[6];	/* Current modified stats */
	s16b stat_top[6];	/* Maximal modified stats */

	s16b stat_add[6];	/* Modifiers to stat values */
	s16b stat_ind[6];	/* Indexes into stat tables */
	s16b stat_cnt[6];	/* Counter for temporary drains */
	s16b stat_los[6];	/* Amount of temporary drains */

	bool_ immune_acid;	/* Immunity to acid */
	bool_ immune_elec;	/* Immunity to lightning */
	bool_ immune_fire;	/* Immunity to fire */
	bool_ immune_cold;	/* Immunity to cold */
	bool_ immune_neth;       /* Immunity to nether */

	bool_ resist_acid;	/* Resist acid */
	bool_ resist_elec;	/* Resist lightning */
	bool_ resist_fire;	/* Resist fire */
	bool_ resist_cold;	/* Resist cold */
	bool_ resist_pois;	/* Resist poison */

	bool_ resist_conf;	/* Resist confusion */
	bool_ resist_sound;	/* Resist sound */
	bool_ resist_lite;	/* Resist light */
	bool_ resist_dark;	/* Resist darkness */
	bool_ resist_chaos;	/* Resist chaos */
	bool_ resist_disen;	/* Resist disenchant */
	bool_ resist_shard;	/* Resist shards */
	bool_ resist_nexus;	/* Resist nexus */
	bool_ resist_blind;	/* Resist blindness */
	bool_ resist_neth;	/* Resist nether */
	bool_ resist_fear;	/* Resist fear */
	bool_ resist_continuum;  /* Resist space-time continuum disruption */

	bool_ sensible_fire;     /* Fire does more damage on the player */
	bool_ sensible_lite;     /* Lite does more damage on the player and blinds her/him */

	bool_ reflect;       /* Reflect 'bolt' attacks */
	bool_ sh_fire;       /* Fiery 'immolation' effect */
	bool_ sh_elec;       /* Electric 'immolation' effect */
	bool_ wraith_form;   /* wraithform */

	bool_ anti_magic;    /* Anti-magic */
	bool_ anti_tele;     /* Prevent teleportation */

	bool_ sustain_str;	/* Keep strength */
	bool_ sustain_int;	/* Keep intelligence */
	bool_ sustain_wis;	/* Keep wisdom */
	bool_ sustain_dex;	/* Keep dexterity */
	bool_ sustain_con;	/* Keep constitution */
	bool_ sustain_chr;	/* Keep charisma */

	bool_ aggravate;		/* Aggravate monsters */
	bool_ teleport;		/* Random teleporting */

	bool_ exp_drain;		/* Experience draining */
	byte drain_mana;        /* mana draining */
	byte drain_life;        /* hp draining */

	bool_ magical_breath;    /* Magical breathing -- can breath anywhere */
	bool_ water_breath;      /* Water breathing -- can breath underwater */
	bool_ climb;             /* Can climb mountains */
	bool_ fly;               /* Can fly over some features */
	bool_ ffall;             /* No damage falling */
	bool_ lite;              /* Permanent light */
	bool_ free_act;		/* Never paralyzed */
	bool_ see_inv;		/* Can see invisible */
	bool_ regenerate;	/* Regenerate hit pts */
	bool_ hold_life;		/* Resist life draining */
	u32b telepathy;         /* Telepathy */
	bool_ slow_digest;	/* Slower digestion */
	bool_ bless_blade;	/* Blessed blade */
	byte xtra_might;        /* Extra might bow */
	bool_ impact;		/* Earthquake blows */
	bool_ auto_id;           /* Auto id items */

	s16b invis;             /* Invisibility */

	s16b dis_to_h;		/* Known bonus to hit */
	s16b dis_to_d;		/* Known bonus to dam */
	s16b dis_to_a;		/* Known bonus to ac */

	s16b dis_ac;		/* Known base ac */

	s16b to_l;                      /* Bonus to life */
	s16b to_m;                      /* Bonus to mana */
	s16b to_s;                      /* Bonus to spell */
	s16b to_h;			/* Bonus to hit */
	s16b to_d;			/* Bonus to dam */
	s16b to_h_melee;                /* Bonus to hit for melee */
	s16b to_d_melee;                /* Bonus to dam for melee */
	s16b to_h_ranged;               /* Bonus to hit for ranged */
	s16b to_d_ranged;               /* Bonus to dam for ranged */
	s16b to_a;			/* Bonus to ac */

	s16b ac;			/* Base ac */

	byte antimagic;         /* Power of the anti magic field */
	byte antimagic_dis;     /* Radius of the anti magic field */

	s16b see_infra;		/* Infravision range */

	s16b skill_dis;		/* Skill: Disarming */
	s16b skill_dev;		/* Skill: Magic Devices */
	s16b skill_sav;		/* Skill: Saving throw */
	s16b skill_stl;		/* Skill: Stealth factor */
	s16b skill_srh;		/* Skill: Searching ability */
	s16b skill_fos;		/* Skill: Searching frequency */
	s16b skill_thn;		/* Skill: To hit (normal) */
	s16b skill_thb;		/* Skill: To hit (shooting) */
	s16b skill_tht;		/* Skill: To hit (throwing) */
	s16b skill_dig;		/* Skill: Digging */

	s16b num_blow;		/* Number of blows */
	s16b num_fire;		/* Number of shots */
	s16b xtra_crit;         /* % of increased crits */

	byte throw_mult;	/* Multiplier for throw damage */

	byte tval_ammo;		/* Correct ammo tval */

	s16b pspeed;		/* Current speed */

	u32b mimic_extra;       /* Mimicry powers use that */
	u32b antimagic_extra;   /* Antimagic powers */
	u32b music_extra;       /* Music songs */
	u32b necro_extra;       /* Necro powers */
	u32b necro_extra2;       /* Necro powers */

	s16b dodge_chance;      /* Dodging chance */

	u32b maintain_sum;      /* Do we have partial summons */

	byte spellbinder_num;   /* Number of spells bound */
	u32b spellbinder[4];    /* Spell bounds */
	byte spellbinder_trigger;       /* Spellbinder trigger condition */

	cptr mimic_name;

	char tactic;                  /* from 128-4 extremely coward to */
				      /* 128+4 berserker */
	char movement;                /* base movement way */

	s16b companion_killed;  /* Number of companion death */

	bool_ no_mortal;         /* Fated to never die by the hand of a mortal being */

	bool_ black_breath;      /* The Tolkien's Black Breath */

	bool_ precognition;      /* Like the cheat mode */

	/*** Extra flags -- used for lua and easying stuff ***/
	u32b xtra_f1;
	u32b xtra_f2;
	u32b xtra_f3;
	u32b xtra_f4;
	u32b xtra_f5;
	u32b xtra_esp;

	/* Corruptions */
	bool_ corruptions[CORRUPTIONS_MAX];
	bool_ corrupt_anti_teleport_stopped;

	/*** Pet commands ***/
	byte pet_follow_distance; /* Length of the imaginary "leash" for pets */
	byte pet_open_doors;      /* flag - allow pets to open doors */
	byte pet_pickup_items;    /* flag - allow pets to pickup items */

	s16b control;                   /* Controlled monster */
	byte control_dir;               /* Controlled monster */

	/*** Body changing variables ***/
	u16b body_monster;        /* In which body is the player */
	bool_ disembodied;         /* Is the player in a body ? */
	byte body_parts[INVEN_TOTAL - INVEN_WIELD]; /* Which body parts does he have ? */

	/* Astral */
	bool_ astral;              /* We started at the bottom ? */

	/* Powers */
	bool_ powers[POWER_MAX];     /* Actual powers */
	bool_ powers_mod[POWER_MAX]; /* Intrinsinc powers */

	/* Skills */
	s16b skill_points;
	s16b skill_last_level;  /* Prevents gaining skills by losing level and regaining them */
	s16b melee_style;       /* How are  */
	s16b use_piercing_shots; /* for archery */

	/* Dripping Tread spell timer */
	s16b dripping_tread;

	/* Help */
	help_info help;

	/* Inertia control */
	s32b inertia_controlled_spell;

	/* For automatic stat-gain */
	s16b last_rewarded_level;

	/*** Temporary fields ***/

	bool_ did_nothing;               /* True if the last action wasnt a real action */
	bool_ leaving;                   /* True if player is leaving */
};

