/* File: loadsave.c */

/* Purpose: interact with savefiles. This file was made by
   unifying load2.c and save.c from the old codebase. Doing it
   this way makes maintenance easier and lets us share code. */

#include "angband.h"

#include "messages.h"
#include "quark.h"

static void do_byte(byte *, int);
static void do_bool(bool_ *, int);
static void do_u16b(u16b *, int);
static void do_s16b(s16b *, int);
static void do_u32b(u32b *, int);
static void do_s32b(s32b *, int);
static void do_string(char *, int, int);
static void do_lore(int, int);
static void do_monster(monster_type *, int);
static void do_randomizer(int flag);
static void do_spells(int, int);
static void note(cptr);
static void do_fate(int, int);
static void do_item(object_type *, int);
static void do_options(int);
static bool_ do_store(store_type *, int);
static void do_messages(int flag);
static void do_xtra(int, int);
static bool_ do_savefile_aux(int);
static void junkinit(void);
static void morejunk(void);
static bool_ do_inventory(int);
static bool_ do_dungeon(int, bool_);
static void do_grid(int);
static void my_sentinel(char *, u16b, int);

static void do_ver_s16b(s16b *, u32b, s16b, int);

static void skip_ver_byte(u32b, int);

errr rd_savefile(void);

static FILE *fff; 	/* Local savefile ptr */

/*
 * Basic byte-level reading from savefile. This provides a single point
 * of interface to the pseudoencryption that ToME (and Angband)
 * uses. I'm thinking about if it might be faster/better to modify all
 * the do_* functions to directly do this stuff -- it'd make the code
 * somewhat uglier to maintain, but concievably might be much faster. Or
 * is it better maybe to scrap the pseudoencryption entirely and adopt
 * some other means of obfuscation, should it still prove useful in any
 * way? -- Improv
 *
 * What's the point of encryption on savefiles anyway? If I wanted to
 * make a cheater savefile, I'd activate debug mode, and hack the game
 * not to save it. There's no point. -- takkaria
 */

static byte sf_get(void)
{
	byte c;

	/* Get a character, decode the value */
	c = getc(fff) & 0xFF;

	/* Return the value */
	return (c);
}


static void sf_put(byte v)
{
	(void)putc((int)v, fff);
}

/*
 * Do object memory and similar stuff
 */
static void do_xtra(int k_idx, int flag)
{
	byte tmp8u = 0;
	object_kind *k_ptr = &k_info[k_idx];

	if (flag == LS_SAVE)
	{
		if (k_ptr->aware) tmp8u |= 0x01;
		if (k_ptr->tried) tmp8u |= 0x02;
		if (k_ptr->know) tmp8u |= 0x04;
		if (k_ptr->artifact) tmp8u |= 0x80;

		do_byte(&tmp8u, flag);
	}
	if (flag == LS_LOAD)
	{
		do_byte(&tmp8u, flag);
		k_ptr->aware = ((tmp8u & 0x01) ? TRUE : FALSE);
		k_ptr->tried = ((tmp8u & 0x02) ? TRUE : FALSE);
		k_ptr->know = ((tmp8u & 0x04) ? TRUE : FALSE);
		k_ptr->artifact = ((tmp8u & 0x80) ? TRUE : FALSE);
	}
}

/*
 * Load/Save quick start data
 */
void do_quick_start(int flag)
{
	int i;

	do_s16b(&previous_char.sex, flag);
	do_s16b(&previous_char.race, flag);
	do_s16b(&previous_char.rmod, flag);
	do_s16b(&previous_char.pclass, flag);
	do_s16b(&previous_char.spec, flag);
	do_byte(&previous_char.quests, flag);
	do_byte(&previous_char.god, flag);
	do_s32b(&previous_char.grace, flag);
	do_s16b(&previous_char.age, flag);
	do_s16b(&previous_char.wt, flag);
	do_s16b(&previous_char.ht, flag);
	do_s16b(&previous_char.sc, flag);
	do_s32b(&previous_char.au, flag);

	for (i = 0; i < 6; i++) do_s16b(&(previous_char.stat[i]), flag);
	do_s16b(&previous_char.luck, flag);

	do_s16b(&previous_char.chaos_patron, flag);
	do_u32b(&previous_char.weapon, flag);
	do_byte((byte*)&previous_char.quick_ok, flag);

	for (i = 0; i < 4; i++) do_string(previous_char.history[i], 60, flag);
}

/*
 * The special saved subrace
 */
static void do_subrace(int flag)
{
	player_race_mod *sr_ptr = &race_mod_info[SUBRACE_SAVE];
	int i;
	char buf[81];

	if (flag == LS_SAVE)
		strncpy(buf, sr_ptr->title + rmp_name, 80);
	do_string(buf, 80, flag);
	if (flag == LS_LOAD)
		strncpy(sr_ptr->title + rmp_name, buf, 80);

	if (flag == LS_SAVE)
		strncpy(buf, sr_ptr->desc + rmp_text, 80);
	do_string(buf, 80, flag);
	if (flag == LS_LOAD)
		strncpy(sr_ptr->desc + rmp_text, buf, 80);

	do_byte((byte*)&sr_ptr->place, flag);

	for (i = 0; i < 6; i++)
		do_s16b(&sr_ptr->r_adj[i], flag);

	do_byte((byte*)&sr_ptr->luck, flag);
	do_s16b(&sr_ptr->mana, flag);

	do_s16b(&sr_ptr->r_dis, flag);
	do_s16b(&sr_ptr->r_dev, flag);
	do_s16b(&sr_ptr->r_sav, flag);
	do_s16b(&sr_ptr->r_stl, flag);
	do_s16b(&sr_ptr->r_srh, flag);
	do_s16b(&sr_ptr->r_fos, flag);
	do_s16b(&sr_ptr->r_thn, flag);
	do_s16b(&sr_ptr->r_thb, flag);

	do_byte((byte*)&sr_ptr->r_mhp, flag);
	do_s16b(&sr_ptr->r_exp, flag);

	do_byte((byte*)&sr_ptr->b_age, flag);
	do_byte((byte*)&sr_ptr->m_age, flag);

	do_byte((byte*)&sr_ptr->m_b_ht, flag);
	do_byte((byte*)&sr_ptr->m_m_ht, flag);
	do_byte((byte*)&sr_ptr->f_b_ht, flag);
	do_byte((byte*)&sr_ptr->f_m_ht, flag);

	do_byte((byte*)&sr_ptr->m_b_wt, flag);
	do_byte((byte*)&sr_ptr->m_m_wt, flag);
	do_byte((byte*)&sr_ptr->f_b_wt, flag);
	do_byte((byte*)&sr_ptr->f_m_wt, flag);

	do_byte((byte*)&sr_ptr->infra, flag);

	for (i = 0; i < 4; i++)
		do_s16b(&sr_ptr->powers[i], flag);

	for (i = 0; i < BODY_MAX; i++)
		do_byte((byte*)&sr_ptr->body_parts[i], flag);

	do_u32b(&sr_ptr->flags1, flag);
	do_u32b(&sr_ptr->flags2, flag);

	for (i = 0; i < PY_MAX_LEVEL + 1; i++)
	{
		do_u32b(&sr_ptr->oflags1[i], flag);
		do_u32b(&sr_ptr->oflags2[i], flag);
		do_u32b(&sr_ptr->oflags3[i], flag);
		do_u32b(&sr_ptr->oflags4[i], flag);
		do_u32b(&sr_ptr->oflags5[i], flag);
		do_u32b(&sr_ptr->oesp[i], flag);
		do_s16b(&sr_ptr->opval[i], flag);
	}

	do_byte(&sr_ptr->g_attr, flag);
	do_byte((byte*)&sr_ptr->g_char, flag);

	for (i = 0; i < MAX_SKILLS; i++)
	{
		do_byte((byte*)&sr_ptr->skill_basem[i], flag);
		do_u32b(&sr_ptr->skill_base[i], flag);
		do_byte((byte*)&sr_ptr->skill_modm[i], flag);
		do_s16b(&sr_ptr->skill_mod[i], flag);
	}
}

/*
 * Misc. other data
 */
static char loaded_game_module[80];
static bool_ do_extra(int flag)
{
	int i, j;
	byte tmp8u;
	s16b tmp16s;
	u32b tmp32u;
	u16b tmp16b;
	u32b dummy32u = 0;

	do_string(player_name, 32, flag);

	do_string(died_from, 80, flag);

	for (i = 0; i < 4; i++)
	{
		do_string(history[i], 60, flag);
	}

	/* Handle the special levels info */
	if (flag == LS_SAVE)
	{
		tmp8u = max_d_idx;
		tmp16s = MAX_DUNGEON_DEPTH;
	}
	do_byte(&tmp8u, flag);

	if (flag == LS_LOAD)
	{
		if (tmp8u > max_d_idx)
		{
			note(format("Too many (%d) dungeon types!", tmp8u));
		}
	}

	do_s16b(&tmp16s, flag);

	if (flag == LS_LOAD)
	{
		if (tmp16s > MAX_DUNGEON_DEPTH)
		{
			note(format("Too many (%d) max level by dungeon type!", tmp16s));
		}
	}

	/* Load the special levels history */
	for (i = 0; i < tmp8u; i++)
	{
		for (j = 0; j < tmp16s; j++)
		{
			do_byte((byte*)&special_lvl[j][i], flag);
		}
	}

	do_byte((byte*)&generate_special_feeling, flag);

	/* Load the quick start data */
	do_quick_start(flag);

	/* Load/save the special subrace */
	do_subrace(flag);

	/* Race/Class/Gender/Spells */
	do_s32b(&p_ptr->lives, flag);
	do_byte(&p_ptr->prace, flag);
	do_byte(&p_ptr->pracem, flag);
	do_byte(&p_ptr->pclass, flag);
	do_byte(&p_ptr->pspec, flag);
	do_byte(&p_ptr->psex, flag);
	do_u16b(&tmp16b, flag);
	do_u16b(&tmp16b, flag);
	do_byte(&p_ptr->mimic_form, flag);
	do_s16b(&p_ptr->mimic_level, flag);
	if (flag == LS_SAVE) tmp8u = 0;

	do_byte(&p_ptr->hitdie, flag);
	do_u16b(&p_ptr->expfact, flag);

	do_s16b(&p_ptr->age, flag);
	do_s16b(&p_ptr->ht, flag);
	do_s16b(&p_ptr->wt, flag);

	/* Dump the stats (maximum and current) */
	for (i = 0; i < 6; ++i) do_s16b(&p_ptr->stat_max[i], flag);
	for (i = 0; i < 6; ++i) do_s16b(&p_ptr->stat_cur[i], flag);
	for (i = 0; i < 6; ++i) do_s16b(&p_ptr->stat_cnt[i], flag);
	for (i = 0; i < 6; ++i) do_s16b(&p_ptr->stat_los[i], flag);

	/* Dump the skills */
	do_s16b(&p_ptr->skill_points, flag);
	do_s16b(&p_ptr->skill_last_level, flag);
	do_s16b(&p_ptr->melee_style, flag);
	do_s16b(&p_ptr->use_piercing_shots, flag);

	tmp16s = MAX_SKILLS;
	do_s16b(&tmp16s, flag);

	if ((flag == LS_LOAD) && (tmp16s > MAX_SKILLS))
	{
		quit("Too many skills");
	}

	if (flag == LS_SAVE) old_max_s_idx = max_s_idx;
	do_u16b(&old_max_s_idx, flag);
	for (i = 0; i < tmp16s; ++i)
	{
		if (i < old_max_s_idx)
		{
			do_s32b(&s_info[i].value, flag);
			do_s32b(&s_info[i].mod, flag);
			do_byte((byte*)&s_info[i].dev, flag);
			do_byte((byte*)&s_info[i].hidden, flag);
			do_u32b(&s_info[i].uses, flag);
		}
		else
		{
			do_u32b(&tmp32u, flag);
			do_s16b(&tmp16s, flag);
			do_byte(&tmp8u, flag);
			do_byte(&tmp8u, flag);
			do_u32b(&tmp32u, flag);
		}
	}

	tmp16s = max_ab_idx;
	do_s16b(&tmp16s, flag);

	if ((flag == LS_LOAD) && (tmp16s > max_ab_idx))
	{
		quit("Too many abilities");
	}

	for (i = 0; i < tmp16s; ++i)
	{
		do_byte((byte*)&ab_info[i].acquired, flag);
	}

	do_s16b(&p_ptr->luck_base, flag);
	do_s16b(&p_ptr->luck_max, flag);

	/* Found 24 unused bytes here...
	   Converted it to be the alchemist's
	   known artifact flags.
	   Note that the ego flags and the gained levels
	   record are recorded here too, but we use the
	   _ver_ format to protect save file compatablity.
	   Note that the other alchemist knowledge (item types known)
	   is stored in do_aux, and is a bit flag in a previously
	   unused bit.
	   */
	for (i = 0; i < 6 ; ++i)
		do_u32b(&alchemist_known_artifacts[i], flag);

	for (i = 0; i < 32 ; ++i)
		do_u32b(&alchemist_known_egos[i], flag);

	do_u32b(&alchemist_gained, flag);

	do_s32b(&p_ptr->au, flag);

	do_s32b(&p_ptr->max_exp, flag);
	do_s32b(&p_ptr->exp, flag);
	do_u16b(&p_ptr->exp_frac, flag);
	do_s16b(&p_ptr->lev, flag);

	do_s16b(&p_ptr->town_num, flag); 	/* -KMW- */

	/* Write arena and rewards information -KMW- */
	do_s16b(&p_ptr->arena_number, flag);
	do_s16b(&p_ptr->inside_arena, flag);
	do_s16b(&p_ptr->inside_quest, flag);
	do_byte((byte*)&p_ptr->exit_bldg, flag);


	/* Save/load spellbinder */
	do_byte(&p_ptr->spellbinder_num, flag);
	do_byte(&p_ptr->spellbinder_trigger, flag);
	for (i = 0; i < 4; i++)
		do_u32b(&p_ptr->spellbinder[i], flag);


	do_byte(&tmp8u, flag); 		/* tmp8u should be 0 at this point */

	if (flag == LS_SAVE) tmp8u = MAX_PLOTS;
	do_byte(&tmp8u, flag);

	if ((flag == LS_LOAD) && (tmp8u > MAX_PLOTS))
	{
		quit(format("Too many plots, %d %d", tmp8u, MAX_PLOTS));
	}

	for (i = 0; i < tmp8u; i++)
	{
		do_s16b(&plots[i], flag);
	}

	if (flag == LS_SAVE)
	{
		tmp8u = MAX_RANDOM_QUEST;
	}
	do_byte(&tmp8u, flag);

	if ((flag == LS_LOAD) &&
	                (tmp8u > MAX_RANDOM_QUEST)) quit("Too many random quests");
	for (i = 0; i < tmp8u; i++)
	{
		do_byte(&random_quests[i].type, flag);
		do_s16b(&random_quests[i].r_idx, flag);
		do_byte((byte*)&random_quests[i].done, flag);
	}

	do_s16b(&p_ptr->oldpx, flag);
	do_s16b(&p_ptr->oldpy, flag);

	do_s16b(&p_ptr->mhp, flag);
	do_s16b(&p_ptr->chp, flag);
	do_u16b(&p_ptr->chp_frac, flag);
	do_s16b(&p_ptr->hp_mod, flag);

	do_s16b(&p_ptr->msane, flag);
	do_s16b(&p_ptr->csane, flag);
	do_u16b(&p_ptr->csane_frac, flag);

	do_s16b(&p_ptr->msp, flag);
	do_s16b(&p_ptr->csp, flag);
	do_u16b(&p_ptr->csp_frac, flag);

	/* XXX
	   Here's where tank points were.
	   Those who run the estate of you-know-who is really stupid.
	   I'll never even consider reading her books now. -- neil */
	do_s16b(&tmp16s, flag);
	do_s16b(&tmp16s, flag);
	do_s16b(&tmp16s, flag);
	do_s16b(&tmp16s, flag);

	/* Gods */
	do_s32b(&p_ptr->grace, flag);
	do_s32b(&p_ptr->grace_delay, flag);
	do_byte((byte*)&p_ptr->praying, flag);
	do_s16b(&p_ptr->melkor_sacrifice, flag);
	do_byte(&p_ptr->pgod, flag);

	/* Max Player and Dungeon Levels */
	do_s16b(&p_ptr->max_plv, flag);

	if (flag == LS_SAVE)
		tmp8u = max_d_idx;
	do_byte(&tmp8u, flag);
	for (i = 0; i < tmp8u; i++)
	{
		if (flag == LS_SAVE)
			tmp16s = max_dlv[i];
		do_s16b(&tmp16s, flag);
		if ((flag == LS_LOAD) && (i <= max_d_idx))
			max_dlv[i] = tmp16s;
	}
	/* Repair max player level??? */
	if ((flag == LS_LOAD) && (p_ptr->max_plv < p_ptr->lev))
		p_ptr->max_plv = p_ptr->lev;

	do_byte((byte*)&(p_ptr->help.enabled), flag);
	for (i = 0; i < HELP_MAX; i++)
	{
		do_bool(&(p_ptr->help.activated[i]), flag);
	}

	/* More info */
	tmp16s = 0;
	do_s16b(&p_ptr->sc, flag);
	do_s16b(&p_ptr->blind, flag);
	do_s16b(&p_ptr->paralyzed, flag);
	do_s16b(&p_ptr->confused, flag);
	do_s16b(&p_ptr->food, flag);
	do_s32b(&p_ptr->energy, flag);
	do_s16b(&p_ptr->fast, flag);
	do_s16b(&p_ptr->speed_factor, flag);
	do_s16b(&p_ptr->slow, flag);
	do_s16b(&p_ptr->afraid, flag);
	do_s16b(&p_ptr->cut, flag);
	do_s16b(&p_ptr->stun, flag);
	do_s16b(&p_ptr->poisoned, flag);
	do_s16b(&p_ptr->image, flag);
	do_s16b(&p_ptr->protevil, flag);
	do_s16b(&p_ptr->protundead, flag);
	do_s16b(&p_ptr->invuln, flag);
	do_s16b(&p_ptr->hero, flag);
	do_s16b(&p_ptr->shero, flag);
	do_s16b(&p_ptr->shield, flag);
	do_s16b(&p_ptr->shield_power, flag);
	do_s16b(&p_ptr->shield_power_opt, flag);
	do_s16b(&p_ptr->shield_power_opt2, flag);
	do_s16b(&p_ptr->shield_opt, flag);
	do_s16b(&p_ptr->blessed, flag);
	do_s16b(&p_ptr->control, flag);
	do_byte(&p_ptr->control_dir, flag);
	do_s16b(&p_ptr->tim_precognition, flag);
	do_s16b(&p_ptr->tim_thunder, flag);
	do_s16b(&p_ptr->tim_thunder_p1, flag);
	do_s16b(&p_ptr->tim_thunder_p2, flag);
	do_s16b(&p_ptr->tim_project, flag);
	do_s16b(&p_ptr->tim_project_dam, flag);
	do_s16b(&p_ptr->tim_project_gf, flag);
	do_s16b(&p_ptr->tim_project_rad, flag);
	do_s16b(&p_ptr->tim_project_flag, flag);

	do_s16b(&p_ptr->tim_magic_breath, flag);
	do_s16b(&p_ptr->tim_water_breath, flag);

	do_s16b(&p_ptr->tim_roots, flag);
	do_s16b(&p_ptr->tim_roots_ac, flag);
	do_s16b(&p_ptr->tim_roots_dam, flag);

	do_s16b(&p_ptr->tim_invis, flag);
	do_s16b(&p_ptr->word_recall, flag);
	do_s16b(&p_ptr->recall_dungeon, flag);
	do_s16b(&p_ptr->see_infra, flag);
	do_s16b(&p_ptr->tim_infra, flag);
	do_s16b(&p_ptr->oppose_fire, flag);
	do_s16b(&p_ptr->oppose_cold, flag);
	do_s16b(&p_ptr->oppose_acid, flag);
	do_s16b(&p_ptr->oppose_elec, flag);
	do_s16b(&p_ptr->oppose_pois, flag);
	do_s16b(&p_ptr->oppose_ld, flag);
	do_s16b(&p_ptr->oppose_cc, flag);
	do_s16b(&p_ptr->oppose_ss, flag);
	do_s16b(&p_ptr->oppose_nex, flag);

	do_s16b(&p_ptr->tim_esp, flag);
	do_s16b(&p_ptr->tim_wraith, flag);
	do_s16b(&p_ptr->tim_ffall, flag);
	do_ver_s16b(&p_ptr->tim_fly, SAVEFILE_VERSION, 0, flag);
	do_s16b(&p_ptr->tim_fire_aura, flag);
	do_ver_s16b(&p_ptr->tim_poison, SAVEFILE_VERSION, 0, flag);
	do_s16b(&p_ptr->resist_magic, flag);
	do_s16b(&p_ptr->tim_invisible, flag);
	do_s16b(&p_ptr->tim_inv_pow, flag);
	do_s16b(&p_ptr->tim_mimic, flag);
	do_s16b(&p_ptr->lightspeed, flag);
	do_s16b(&p_ptr->tim_lite, flag);
	do_ver_s16b(&p_ptr->tim_regen, SAVEFILE_VERSION, 0, flag);
	do_ver_s16b(&p_ptr->tim_regen_pow, SAVEFILE_VERSION, 0, flag);
	do_s16b(&p_ptr->holy, flag);
	do_s16b(&p_ptr->walk_water, flag);
	do_s16b(&p_ptr->tim_mental_barrier, flag);
	do_s16b(&p_ptr->immov_cntr, flag);
	do_s16b(&p_ptr->strike, flag);
	do_s16b(&p_ptr->meditation, flag);
	do_s16b(&p_ptr->tim_reflect, flag);
	do_s16b(&p_ptr->tim_res_time, flag);
	do_s16b(&p_ptr->tim_deadly, flag);
	do_s16b(&p_ptr->prob_travel, flag);
	do_s16b(&p_ptr->disrupt_shield, flag);
	do_s16b(&p_ptr->parasite, flag);
	do_s16b(&p_ptr->parasite_r_idx, flag);
	do_s32b(&p_ptr->loan, flag);
	do_s32b(&p_ptr->loan_time, flag);
	do_s16b(&p_ptr->absorb_soul, flag);
	do_s32b(&p_ptr->inertia_controlled_spell, flag);
	do_s16b(&p_ptr->last_rewarded_level, flag);

	do_s16b(&p_ptr->chaos_patron, flag);

	if (flag == LS_SAVE) { tmp16s = CORRUPTIONS_MAX; }
	do_s16b(&tmp16s, flag);
	if (tmp16s > CORRUPTIONS_MAX) {
		quit("Too many corruptions");
	}

	for (i = 0; i < tmp16s; i++)
	{
		if (flag == LS_SAVE)
			tmp8u = p_ptr->corruptions[i];

		do_byte(&tmp8u, flag);

		if (flag == LS_LOAD)
			p_ptr->corruptions[i] = tmp8u;
	}

	do_byte((byte*)&p_ptr->corrupt_anti_teleport_stopped, flag);

	do_byte(&p_ptr->confusing, flag);
	do_byte((byte*)&p_ptr->black_breath, flag);
	do_byte((byte*)&fate_flag, flag);
	do_byte(&p_ptr->searching, flag);
	do_byte(&p_ptr->maximize, flag);
	do_byte(&p_ptr->preserve, flag);
	do_byte(&p_ptr->special, flag);
	do_byte((byte*)&ambush_flag, flag);
	do_byte(&p_ptr->allow_one_death, flag);
	do_s16b(&p_ptr->xtra_spells, flag);

	do_byte(&tmp8u, flag);

	do_s16b(&no_breeds, flag);
	do_s16b(&p_ptr->protgood, flag);

	/* Auxilliary variables */
	do_u32b(&p_ptr->mimic_extra, flag);
	do_u32b(&p_ptr->antimagic_extra, flag);
	do_u32b(&p_ptr->druid_extra, flag);
	do_u32b(&p_ptr->druid_extra2, flag);
	do_u32b(&p_ptr->druid_extra3, flag);
	do_u32b(&p_ptr->music_extra, flag);
	do_u32b(&p_ptr->music_extra2, flag);
	do_u32b(&p_ptr->necro_extra, flag);
	do_u32b(&p_ptr->necro_extra2, flag);

	do_u32b(&p_ptr->race_extra1, flag);
	do_u32b(&p_ptr->race_extra2, flag);
	do_u32b(&p_ptr->race_extra3, flag);
	do_u32b(&p_ptr->race_extra4, flag);
	do_u32b(&p_ptr->race_extra5, flag);
	do_u32b(&p_ptr->race_extra6, flag);
	do_u32b(&p_ptr->race_extra7, flag);

	do_u16b(&p_ptr->body_monster, flag);
	do_byte((byte*)&p_ptr->disembodied, flag);

	/* Are we in astral mode? */
	do_byte((byte*)&p_ptr->astral, flag);

	if (flag == LS_SAVE) tmp16s = POWER_MAX;
	do_s16b(&tmp16s, flag);
	if ((flag == LS_LOAD) && (tmp16s > POWER_MAX))
		note(format("Too many (%u) powers!", tmp16s));
	for (i = 0; i < POWER_MAX; i++)
		do_byte((byte*)&p_ptr->powers_mod[i], flag);

	skip_ver_byte(100, flag);

	/* The tactic */
	do_byte((byte*)&p_ptr->tactic, flag);

	/* The movement */
	do_byte((byte*)&p_ptr->movement, flag);

	/* The comapnions killed */
	do_s16b(&p_ptr->companion_killed, flag);

	/* The fate */
	do_byte((byte*)&p_ptr->no_mortal, flag);

	/* The bounties */
	for (i = 0; i < MAX_BOUNTIES; i++)
	{
		do_s16b(&bounties[i][0], flag);
		do_s16b(&bounties[i][1], flag);
	}
	do_u32b(&total_bounties, flag);
	do_s16b(&spell_num, flag);
	for (i = 0; i < MAX_SPELLS; i++)
		do_spells(i, flag);
	do_s16b(&rune_num, flag);
	for (i = 0; i < MAX_RUNES; i++)
	{
		do_string(rune_spells[i].name, 30, flag);
		do_s16b(&rune_spells[i].type, flag);
		do_s16b(&rune_spells[i].rune2, flag);
		do_s16b(&rune_spells[i].mana, flag);
	}

	/* Load random seeds */
	do_u32b(&dummy32u, flag);    /* Load-compatibility with old savefiles. */
	do_u32b(&seed_flavor, flag); /* For consistent object flavors. */
	do_u32b(&dummy32u, flag);    /* Load-compatibility with old savefiles. */

	/* Special stuff */
	do_u16b(&tmp16b, flag);      /* Dummy */
	do_u16b(&total_winner, flag);
	do_u16b(&has_won, flag);
	do_u16b(&noscore, flag);

	/* Write death */
	if (flag == LS_SAVE) tmp8u = death;
	do_byte(&tmp8u, flag);
	if (flag == LS_LOAD) death = tmp8u;

	/* Incompatible module? */
	if (flag == LS_LOAD)
	{
		s32b ok;

		ok = module_savefile_loadable(loaded_game_module);

		/* Argh bad game module! */
		if (!ok)
		{
			note(format("Bad game module. Savefile was saved with module '%s' but game is '%s'.", loaded_game_module, game_module));
			return (FALSE);
		}
	}

	/* Write feeling */
	if (flag == LS_SAVE) tmp8u = feeling;
	do_byte(&tmp8u, flag);
	if (flag == LS_LOAD) feeling = tmp8u;

	/* Turn of last "feeling" */
	do_s32b(&old_turn, flag);

	/* Current turn */
	do_s32b(&turn, flag);

	return TRUE;
}

/* Save the current persistent dungeon -SC- */
void save_dungeon(void)
{
	char tmp[16];
	char name[1024], buf[5];

	/* Save only persistent dungeons */
	if (!get_dungeon_save(buf) || (!dun_level)) return;

	/* Construct filename */
	sprintf(tmp, "%s.%s", player_base, buf);
	path_build(name, 1024, ANGBAND_DIR_SAVE, tmp);

	/* Open the file */
	fff = my_fopen(name, "wb");

	/* Save the dungeon */
	do_dungeon(LS_SAVE, TRUE);

	my_fclose(fff);
}

/*
 * Medium level player saver
 */
static bool_ save_player_aux(char *name)
{
	bool_ ok = FALSE;
	int fd = -1;
	int mode = 0644;

	/* No file yet */
	fff = NULL;

	/* File type is "SAVE" */
	FILE_TYPE(FILE_TYPE_SAVE);

	/* Create the savefile */
	fd = fd_make(name, mode);

	/* File is okay */
	if (fd >= 0)
	{
		/* Close the "fd" */
		(void)fd_close(fd);

		/* Open the savefile */
		fff = my_fopen(name, "wb");

		/* Successful open */
		if (fff)
		{
			/* Write the savefile */
			if (do_savefile_aux(LS_SAVE)) ok = TRUE;

			/* Attempt to close it */
			if (my_fclose(fff)) ok = FALSE;
		}

		/* "broken" savefile */
		if (!ok)
		{
			/* Remove "broken" files */
			(void)fd_kill(name);
		}
	}

	/* Failure */
	if (!ok) return (FALSE);

	/* Successful save */
	character_saved = TRUE;

	/* Success */
	return (TRUE);
}

/*
 * Attempt to save the player in a savefile
 */
bool_ save_player(void)
{
	int result = FALSE;
	char safe[1024];

	/* New savefile */
	strcpy(safe, savefile);
	strcat(safe, ".new");

	/* Remove it */
	fd_kill(safe);

	/* Attempt to save the player */
	if (save_player_aux(safe))
	{
		char temp[1024];

		/* Old savefile */
		strcpy(temp, savefile);
		strcat(temp, ".old");

		/* Remove it */
		fd_kill(temp);

		/* Preserve old savefile */
		fd_move(savefile, temp);

		/* Activate new savefile */
		fd_move(safe, savefile);

		/* Remove preserved savefile */
		fd_kill(temp);

		/* Hack -- Pretend the character was loaded */
		character_loaded = TRUE;

		/* Success */
		result = TRUE;
	}

	save_savefile_names();

	/* Return the result */
	return (result);
}

bool_ file_exist(cptr buf)
{
	int fd;
	bool_ result;

	/* Open savefile */
	fd = fd_open(buf, O_RDONLY);

	/* File exists */
	if (fd >= 0)
	{
		fd_close(fd);
		result = TRUE;
	}
	else
		result = FALSE;

	return result;
}

/*
 * Attempt to Load a "savefile"
 *
 * On multi-user systems, you may only "read" a savefile if you will be
 * allowed to "write" it later, this prevents painful situations in which
 * the player loads a savefile belonging to someone else, and then is not
 * allowed to save his game when he quits.
 *
 * We return "TRUE" if the savefile was usable, and we set the global
 * flag "character_loaded" if a real, living, character was loaded.
 *
 * Note that we always try to load the "current" savefile, even if
 * there is no such file, so we must check for "empty" savefile names.
 */
bool_ load_player(void)
{
	int fd = -1;

	errr err = 0;

	cptr what = "generic";

	/* Paranoia */
	turn = 0;

	/* Paranoia */
	death = FALSE;


	/* Allow empty savefile name */
	if (!savefile[0]) return (TRUE);


	/* XXX XXX XXX Fix this */

	/* Verify the existance of the savefile */
	if (!file_exist(savefile))
	{
		/* Give a message */
		msg_format("Savefile does not exist: %s", savefile);
		msg_print(NULL);

		/* Allow this */
		return (TRUE);
	}

	/* Okay */
	if (!err)
	{
		/* Open the savefile */
		fd = fd_open(savefile, O_RDONLY);

		/* No file */
		if (fd < 0) err = -1;

		/* Message (below) */
		if (err) what = "Cannot open savefile";
	}

	/* Process file */
	if (!err)
	{
		/* Open the file XXX XXX XXX XXX Should use Angband file interface */
		fff = my_fopen(savefile, "rb");
/*		fff = fdopen(fd, "r"); */

		/* Read the first four bytes */
		do_u32b(&vernum, LS_LOAD);
		do_byte(&sf_extra, LS_LOAD);

		/* XXX XXX XXX XXX Should use Angband file interface */
		my_fclose(fff);
		/* fclose(fff) */

		/* Close the file */
		fd_close(fd);
	}

	/* Process file */
	if (!err)
	{

		/* Extract version */
		sf_major = VERSION_MAJOR;
		sf_minor = VERSION_MINOR;
		sf_patch = VERSION_PATCH;
		sf_extra = VERSION_EXTRA;

		/* Clear screen */
		Term_clear();

		/* Attempt to load */
		err = rd_savefile();

		/* Message (below) */
		if (err) what = "Cannot parse savefile";
	}

	/* Paranoia */
	if (!err)
	{
		/* Invalid turn */
		if (!turn) err = -1;

		/* Message (below) */
		if (err) what = "Broken savefile";
	}


	/* Okay */
	if (!err)
	{
		/* Maybe the scripts want to resurrect char */
		if (process_hooks_ret(HOOK_LOAD_END, "d", "(d)", death))
		{
			character_loaded = process_hooks_return[0].num;
			death = process_hooks_return[1].num;
			return TRUE;
		}

		/* Player is dead */
		if (death)
		{
			/* Player is no longer "dead" */
			death = FALSE;

			/* Cheat death (unless the character retired) */
			if (arg_wizard && !total_winner)
			{
				/* A character was loaded */
				character_loaded = TRUE;

				/* Done */
				return (TRUE);
			}

			/* Count lives */
			sf_lives++;

			/* Forget turns */
			turn = old_turn = 0;

			/* Done */
			return (TRUE);
		}

		/* A character was loaded */
		character_loaded = TRUE;

		/* Still alive */
		if (p_ptr->chp >= 0)
		{
			/* Reset cause of death */
			(void)strcpy(died_from, "(alive and well)");
		}

		/* Success */
		return (TRUE);
	}


	/* Message */
	msg_format("Error (%s) reading %d.%d.%d savefile.",
	           what, sf_major, sf_minor, sf_patch);
	msg_print(NULL);

	/* Oops */
	return (FALSE);
}


/*
 * Size-aware read/write routines for the savefile, do all their
 * work through sf_get and sf_put.
 */

static void do_byte(byte *v, int flag)
{
	if (flag == LS_LOAD)
	{
		*v = sf_get();
		return;
	}
	if (flag == LS_SAVE)
	{
		byte val = *v;
		sf_put(val);
		return;
	}
	/* We should never reach here, so bail out */
	printf("FATAL: do_byte passed %d\n", flag);
	exit(0);
}

static void do_bool(bool_ *f, int flag)
{
	byte b = *f;
	do_byte(&b, flag);
	if (flag == LS_LOAD)
	{
		*f = b;
	}
}

static void do_u16b(u16b *v, int flag)
{
	if (flag == LS_LOAD)
	{
		(*v) = sf_get();
		(*v) |= ((u16b)(sf_get()) << 8);
		return;
	}
	if (flag == LS_SAVE)
	{
		u16b val;
		val = *v;
		sf_put((byte)(val & 0xFF));
		sf_put((byte)((val >> 8) & 0xFF));
		return;
	}
	/* Never should reach here, bail out */
	printf("FATAL: do_u16b passed %d\n", flag);
	exit(0);
}

static void do_s16b(s16b *ip, int flag)
{
	do_u16b((u16b *)ip, flag);
}

static void do_u32b(u32b *ip, int flag)
{
	if (flag == LS_LOAD)
	{
		(*ip) = sf_get();
		(*ip) |= ((u32b)(sf_get()) << 8);
		(*ip) |= ((u32b)(sf_get()) << 16);
		(*ip) |= ((u32b)(sf_get()) << 24);
		return;
	}
	if (flag == LS_SAVE)
	{
		u32b val = *ip;
		sf_put((byte)(val & 0xFF));
		sf_put((byte)((val >> 8) & 0xFF));
		sf_put((byte)((val >> 16) & 0xFF));
		sf_put((byte)((val >> 24) & 0xFF));
		return;
	}
	/* Bad news if you're here, because you're going down */
	printf("FATAL: do_u32b passed %d\n", flag);
	exit(0);
}

static void do_s32b(s32b *ip, int flag)
{
	do_u32b((u32b *)ip, flag);
}

static void do_string(char *str, int max, int flag)
/* Max is ignored for writing */
{
	if (flag == LS_LOAD)
	{
		int i;

		/* Read the string */
		for (i = 0; TRUE; i++)
		{
			byte tmp8u;

			/* Read a byte */
			do_byte(&tmp8u, LS_LOAD);

			/* Collect string while legal */
			if (i < max) str[i] = tmp8u;

			/* End of string */
			if (!tmp8u) break;
		}
		/* Terminate */
		str[max - 1] = '\0';
		return;
	}
	if (flag == LS_SAVE)
	{
		while (*str)
		{
			do_byte((byte*)str, flag);
			str++;
		}
		do_byte((byte*)str, flag); 		/* Output a terminator */
		return;
	}
	printf("FATAL: do_string passed flag %d\n", flag);
	exit(0);
}

static void skip_ver_byte(u32b version, int flag)
/* Reads and discards a byte if the savefile is as old as/older than version */
{
	if ((flag == LS_LOAD) && (vernum <= version))
	{
		byte forget;
		do_byte(&forget, flag);
	}
	return;
}

static void do_ver_s16b(s16b *v, u32b version, s16b defval, int flag)
{
	if ((flag == LS_LOAD) && (vernum < version))
	{
		*v = defval;
		return;
	}
	do_s16b(v, flag);
}

/*
 * Show information on the screen, one line at a time.
 *
 * Avoid the top two lines, to avoid interference with "msg_print()".
 */
static void note(cptr msg)
{
	static int y = 2;

	/* Draw the message */
	prt(msg, y, 0);

	/* Advance one line (wrap if needed) */
	if (++y >= 24) y = 2;

	/* Flush it */
	Term_fresh();
}


/*
 * Determine if an item can be wielded/worn (e.g. helmet, sword, bow, arrow)
 */
static bool_ wearable_p(object_type *o_ptr)
{
	/* Valid "tval" codes */
	switch (o_ptr->tval)
	{
	case TV_WAND:
	case TV_STAFF:
	case TV_ROD:
	case TV_ROD_MAIN:
	case TV_SHOT:
	case TV_ARROW:
	case TV_BOLT:
	case TV_BOOMERANG:
	case TV_BOW:
	case TV_DIGGING:
	case TV_HAFTED:
	case TV_POLEARM:
	case TV_MSTAFF:
	case TV_SWORD:
	case TV_AXE:
	case TV_BOOTS:
	case TV_GLOVES:
	case TV_HELM:
	case TV_CROWN:
	case TV_SHIELD:
	case TV_CLOAK:
	case TV_SOFT_ARMOR:
	case TV_HARD_ARMOR:
	case TV_DRAG_ARMOR:
	case TV_SCROLL:
	case TV_LITE:
	case TV_POTION:
	case TV_POTION2:
	case TV_AMULET:
	case TV_RING:
	case TV_HYPNOS:
	case TV_INSTRUMENT:
	case TV_DAEMON_BOOK:
	case TV_TRAPKIT:
	case TV_TOOL:
		{
			return (TRUE);
		}
	}

	/* Nope */
	return (FALSE);
}


/*
 * rd/wr an object
 *
 * FIXME! This code probably has a lot of cruft from the old Z/V codebase.
 *
 */
static void do_item(object_type *o_ptr, int flag)
{
	byte old_dd;
	byte old_ds;

	u32b f1, f2, f3, f4, f5, esp;

	object_kind *k_ptr;

	/* Kind */
	do_s16b(&o_ptr->k_idx, flag);

	/* Location */
	do_byte(&o_ptr->iy, flag);
	do_byte(&o_ptr->ix, flag);

	/* Type/Subtype */
	do_byte(&o_ptr->tval, flag);
	do_byte(&o_ptr->sval, flag);

	/* Special pval */
	do_s32b(&o_ptr->pval, flag);

	/* Special pval */
	do_s16b(&o_ptr->pval2, flag);

	/* Special pval */
	do_s32b(&o_ptr->pval3, flag);

	do_byte(&o_ptr->discount, flag);
	do_byte(&o_ptr->number, flag);
	do_s32b(&o_ptr->weight, flag);

	do_byte(&o_ptr->name1, flag);
	do_s16b(&o_ptr->name2, flag);
	do_s16b(&o_ptr->name2b, flag);
	do_s16b(&o_ptr->timeout, flag);

	do_s16b(&o_ptr->to_h, flag);
	do_s16b(&o_ptr->to_d, flag);
	do_s16b(&o_ptr->to_a, flag);

	do_s16b(&o_ptr->ac, flag);

	/* We do special processing of this flag when reading */
	if (flag == LS_LOAD)
	{
		do_byte(&old_dd, LS_LOAD);
		do_byte(&old_ds, LS_LOAD);
	}
	if (flag == LS_SAVE)
	{
		do_byte(&o_ptr->dd, LS_SAVE);
		do_byte(&o_ptr->ds, LS_SAVE);
	}

	do_byte(&o_ptr->ident, flag);

	do_byte(&o_ptr->marked, flag);

	/* flags */
	do_u32b(&o_ptr->art_flags1, flag);
	do_u32b(&o_ptr->art_flags2, flag);
	do_u32b(&o_ptr->art_flags3, flag);
	do_u32b(&o_ptr->art_flags4, flag);
	do_u32b(&o_ptr->art_flags5, flag);
	do_u32b(&o_ptr->art_esp, flag);

	/* obvious flags */
	do_u32b(&o_ptr->art_oflags1, flag);
	do_u32b(&o_ptr->art_oflags2, flag);
	do_u32b(&o_ptr->art_oflags3, flag);
	do_u32b(&o_ptr->art_oflags4, flag);
	do_u32b(&o_ptr->art_oflags5, flag);
	do_u32b(&o_ptr->art_oesp, flag);

	/* Monster holding object */
	do_s16b(&o_ptr->held_m_idx, flag);

	/* Special powers */
	do_byte(&o_ptr->xtra1, flag);
	do_s16b(&o_ptr->xtra2, flag);

	do_byte(&o_ptr->elevel, flag);
	do_s32b(&o_ptr->exp, flag);

	/* Read the pseudo-id */
	do_byte(&o_ptr->sense, flag);

	/* Read the found info */
	do_byte(&o_ptr->found, flag);
	do_s16b(&o_ptr->found_aux1, flag);
	do_s16b(&o_ptr->found_aux2, flag);
	do_s16b(&o_ptr->found_aux3, flag);
	do_s16b(&o_ptr->found_aux4, flag);

	if (flag == LS_LOAD)
	{
		char buf[128];
		/* Inscription */
		do_string(buf, 128, LS_LOAD);
		/* Save the inscription */
		if (buf[0]) o_ptr->note = quark_add(buf);

		do_string(buf, 128, LS_LOAD);
		if (buf[0]) o_ptr->art_name = quark_add(buf);
	}
	if (flag == LS_SAVE)
	{
		/* Save the inscription (if any) */
		if (o_ptr->note)
		{
			do_string((char *)quark_str(o_ptr->note), 0, LS_SAVE);
		}
		else
		{
			do_string("", 0, LS_SAVE);
		}
		if (o_ptr->art_name)
		{
			do_string((char *)quark_str(o_ptr->art_name), 0, LS_SAVE);
		}
		else
		{
			do_string("", 0, LS_SAVE);
		}
	}

	if (flag == LS_SAVE) return ; 	/* Stick any more shared code before this. The rest
										   of this function is reserved for LS_LOAD's
										   cleanup functions */
	/*********** END OF LS_SAVE ***************/

	/* Obtain the "kind" template */
	k_ptr = &k_info[o_ptr->k_idx];

	/* Obtain tval/sval from k_info */
	o_ptr->tval = k_ptr->tval;
	if (o_ptr->tval != TV_RANDART) o_ptr->sval = k_ptr->sval;


	/* Repair non "wearable" items */
	if (!wearable_p(o_ptr))
	{
		/* Acquire correct fields */
		o_ptr->to_h = k_ptr->to_h;
		o_ptr->to_d = k_ptr->to_d;
		o_ptr->to_a = k_ptr->to_a;

		/* Acquire correct fields */
		o_ptr->ac = k_ptr->ac;
		o_ptr->dd = k_ptr->dd;
		o_ptr->ds = k_ptr->ds;

		/* All done */
		return;
	}


	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* Paranoia */
	if (o_ptr->name1)
	{
		artifact_type *a_ptr;

		/* Obtain the artifact info */
		a_ptr = &a_info[o_ptr->name1];

		/* Verify that artifact */
		if (!a_ptr->name) o_ptr->name1 = 0;
	}

	/* Paranoia */
	if (o_ptr->name2)
	{
		ego_item_type *e_ptr;

		/* Obtain the ego-item info */
		e_ptr = &e_info[o_ptr->name2];

		/* Verify that ego-item */
		if (!e_ptr->name) o_ptr->name2 = 0;
	}


	/* Acquire standard fields */
	o_ptr->ac = k_ptr->ac;
	o_ptr->dd = k_ptr->dd;
	o_ptr->ds = k_ptr->ds;

	/* Artifacts */
	if (o_ptr->name1)
	{
		artifact_type *a_ptr;

		/* Obtain the artifact info */
		a_ptr = &a_info[o_ptr->name1];

		/* Acquire new artifact fields */
		o_ptr->ac = a_ptr->ac;
		o_ptr->dd = a_ptr->dd;
		o_ptr->ds = a_ptr->ds;

		/* Acquire new artifact weight */
		o_ptr->weight = a_ptr->weight;
	}

	/* Ego items */
	if (o_ptr->name2)
	{
		o_ptr->dd = old_dd;
		o_ptr->ds = old_ds;
	}

	if (o_ptr->art_name)		/* A random artifact */
	{
		o_ptr->dd = old_dd;
		o_ptr->ds = old_ds;
	}
}




/*
 * Read a monster
 */
static void do_monster(monster_type *m_ptr, int flag)
{
	int i;
	bool_ tmp;

	/* Read the monster race */
	do_s16b(&m_ptr->r_idx, flag);

	do_u16b(&m_ptr->ego, flag);

	/* Read the other information */
	do_byte(&m_ptr->fy, flag);
	do_byte(&m_ptr->fx, flag);

	do_s32b(&m_ptr->hp, flag);
	do_s32b(&m_ptr->maxhp, flag);

	do_s16b(&m_ptr->csleep, flag);
	do_byte(&m_ptr->mspeed, flag);
	do_byte(&m_ptr->energy, flag);
	do_byte(&m_ptr->stunned, flag);
	do_byte(&m_ptr->confused, flag);
	do_byte(&m_ptr->monfear, flag);
	do_u32b(&m_ptr->smart, flag);
	do_s16b(&m_ptr->status, flag);
	do_s16b(&m_ptr->possessor, flag);
	do_byte(&m_ptr->speed, flag);
	do_byte(&m_ptr->level, flag);
	do_s16b(&m_ptr->ac, flag);
	do_u32b(&m_ptr->exp, flag);
	do_s16b(&m_ptr->target, flag);

	do_s16b(&m_ptr->bleeding, flag);
	do_s16b(&m_ptr->poisoned, flag);

	do_s32b(&m_ptr->mflag, flag);

	if (flag == LS_LOAD) m_ptr->mflag &= PERM_MFLAG_MASK;

	/* Attacks */
	for (i = 0; i < 4; i++)
	{
		do_byte(&m_ptr->blow[i].method, flag);
		do_byte(&m_ptr->blow[i].effect, flag);
		do_byte(&m_ptr->blow[i].d_dice, flag);
		do_byte(&m_ptr->blow[i].d_side, flag);
	}

	/* Mind */
	tmp = (m_ptr->mind) ? TRUE : FALSE;
	do_byte((byte*)&tmp, flag);
	if (tmp)
	{
		if (flag == LS_LOAD)
		{
			MAKE(m_ptr->mind, monster_mind);
		}
	}

	/* Special race */
	tmp = (m_ptr->sr_ptr) ? TRUE : FALSE;
	do_byte((byte*)&tmp, flag);
	if (tmp)
	{
		if (flag == LS_LOAD)
		{
			MAKE(m_ptr->sr_ptr, monster_race);
		}
		do_u32b(&m_ptr->sr_ptr->name, flag);
		do_u32b(&m_ptr->sr_ptr->text, flag);

		do_u16b(&m_ptr->sr_ptr->hdice, flag);
		do_u16b(&m_ptr->sr_ptr->hside, flag);

		do_s16b(&m_ptr->sr_ptr->ac, flag);

		do_s16b(&m_ptr->sr_ptr->sleep, flag);
		do_byte(&m_ptr->sr_ptr->aaf, flag);
		do_byte(&m_ptr->sr_ptr->speed, flag);

		do_s32b(&m_ptr->sr_ptr->mexp, flag);

		do_s32b(&m_ptr->sr_ptr->weight, flag);

		do_byte(&m_ptr->sr_ptr->freq_inate, flag);
		do_byte(&m_ptr->sr_ptr->freq_spell, flag);

		do_u32b(&m_ptr->sr_ptr->flags1, flag);
		do_u32b(&m_ptr->sr_ptr->flags2, flag);
		do_u32b(&m_ptr->sr_ptr->flags3, flag);
		do_u32b(&m_ptr->sr_ptr->flags4, flag);
		do_u32b(&m_ptr->sr_ptr->flags5, flag);
		do_u32b(&m_ptr->sr_ptr->flags6, flag);
		do_u32b(&m_ptr->sr_ptr->flags7, flag);
		do_u32b(&m_ptr->sr_ptr->flags8, flag);
		do_u32b(&m_ptr->sr_ptr->flags9, flag);

		/* Attacks */
		for (i = 0; i < 4; i++)
		{
			do_byte(&m_ptr->sr_ptr->blow[i].method, flag);
			do_byte(&m_ptr->sr_ptr->blow[i].effect, flag);
			do_byte(&m_ptr->sr_ptr->blow[i].d_dice, flag);
			do_byte(&m_ptr->sr_ptr->blow[i].d_side, flag);
		}

		for (i = 0; i < BODY_MAX; i++)
			do_byte(&m_ptr->sr_ptr->body_parts[i], flag);

		do_byte(&m_ptr->sr_ptr->level, flag);
		do_byte(&m_ptr->sr_ptr->rarity, flag);

		do_byte((byte*)&m_ptr->sr_ptr->d_char, flag);
		do_byte(&m_ptr->sr_ptr->d_attr, flag);

		do_byte((byte*)&m_ptr->sr_ptr->x_char, flag);
		do_byte(&m_ptr->sr_ptr->x_attr, flag);

		do_s16b(&m_ptr->sr_ptr->max_num, flag);
		do_byte(&m_ptr->sr_ptr->cur_num, flag);
	}
}





/*
 * Handle monster lore
 */
static void do_lore(int r_idx, int flag)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Count sights/deaths/kills */
	do_s16b(&r_ptr->r_sights, flag);
	do_s16b(&r_ptr->r_deaths, flag);
	do_s16b(&r_ptr->r_pkills, flag);
	do_s16b(&r_ptr->r_tkills, flag);

	/* Count wakes and ignores */
	do_byte(&r_ptr->r_wake, flag);
	do_byte(&r_ptr->r_ignore, flag);

	/* Extra stuff */
	do_byte(&r_ptr->r_xtra1, flag);
	do_byte(&r_ptr->r_xtra2, flag);

	/* Count drops */
	do_byte(&r_ptr->r_drop_gold, flag);
	do_byte(&r_ptr->r_drop_item, flag);

	/* Count spells */
	do_byte(&r_ptr->r_cast_inate, flag);
	do_byte(&r_ptr->r_cast_spell, flag);

	/* Count blows of each type */
	do_byte(&r_ptr->r_blows[0], flag);
	do_byte(&r_ptr->r_blows[1], flag);
	do_byte(&r_ptr->r_blows[2], flag);
	do_byte(&r_ptr->r_blows[3], flag);

	/* Memorize flags */
	do_u32b(&r_ptr->r_flags1, flag); 	/* Just to remind you */
	do_u32b(&r_ptr->r_flags2, flag); 	/* flag is unrelated to */
	do_u32b(&r_ptr->r_flags3, flag); 	/* the other argument */
	do_u32b(&r_ptr->r_flags4, flag);
	do_u32b(&r_ptr->r_flags5, flag);
	do_u32b(&r_ptr->r_flags6, flag);
	do_u32b(&r_ptr->r_flags7, flag);
	do_u32b(&r_ptr->r_flags8, flag);
	do_u32b(&r_ptr->r_flags9, flag);

	/* Read the "Racial" monster tmp16b per level */
	do_s16b(&r_ptr->max_num, flag);

	do_byte((byte*)&r_ptr->on_saved, flag);

	if (flag == LS_LOAD)
	{
		/* Lore flag repair? */
		r_ptr->r_flags1 &= r_ptr->flags1;
		r_ptr->r_flags2 &= r_ptr->flags2;
		r_ptr->r_flags3 &= r_ptr->flags3;
		r_ptr->r_flags4 &= r_ptr->flags4;
		r_ptr->r_flags5 &= r_ptr->flags5;
		r_ptr->r_flags6 &= r_ptr->flags6;
	}
}




/*
 * Read a store
 */
static bool_ do_store(store_type *str, int flag)
/* FIXME! Why does this return anything when
   it always returns the same thing? */
{
	int j;

	byte num;

	byte store_inven_max = STORE_INVEN_MAX;

	/* Some basic info */
	do_s32b(&str->store_open, flag);
	do_s16b(&str->insult_cur, flag);
	do_u16b(&str->owner, flag);
	if (flag == LS_SAVE) num = str->stock_num;

	/* Could be cleaner, done this way for benefit of the for loop later on */
	do_byte(&num, flag);

	do_s16b(&str->good_buy, flag);
	do_s16b(&str->bad_buy, flag);

	/* Last visit */
	do_s32b(&str->last_visit, flag);

	/* Items */
	for (j = 0; j < num; j++)
	{
		if (flag == LS_LOAD)
			/* Can't this be cleaner? */
		{
			object_type forge;
			/* Wipe the object */
			object_wipe(&forge);
			/* Read the item */
			do_item(&forge, LS_LOAD);
			/* Acquire valid items */
			if ((str->stock_num < store_inven_max) && (str->stock_num < str->stock_size))
			{
				int k = str->stock_num++;

				/* Acquire the item */
				object_copy(&str->stock[k], &forge);
			}
		}
		if (flag == LS_SAVE) do_item(&str->stock[j], flag);
	}

	/* Success */
	return (TRUE);
}

/*
 * RNG state
 */
static void do_randomizer(int flag)
{
	int i;

	u16b tmp16u = 0;

	/* Tmp */
	do_u16b(&tmp16u, flag);

	/* Place */
	do_u16b(&Rand_place, flag);

	/* State */
	for (i = 0; i < RAND_DEG; i++)
	{
		do_u32b(&Rand_state[i], flag);
	}

	/* Accept */
	if (flag == LS_LOAD)
	{
		Rand_quick = FALSE;
	}
}

/*
 * Handle options
 *
 * Normal options are stored as a set of 256 bit flags,
 * plus a set of 256 bit masks to indicate which bit flags were defined
 * at the time the savefile was created.  This will allow new options
 * to be added, and old options to be removed, at any time, without
 * hurting old savefiles.
 *
 * The window options are stored in the same way, but note that each
 * window gets 32 options, and their order is fixed by certain defines.
 */
static void do_options(int flag)
{
	int i, n;

	u32b oflag[8];
	u32b mask[8];

	/*** Special info */

	/* Read "delay_factor" */
	do_byte(&delay_factor, flag);

	/* Read "hitpoint_warn" */
	do_byte(&hitpoint_warn, flag);

	/*** Cheating options ***/
	if (flag == LS_LOAD)		/* There *MUST* be some nice way to unify this! */
	{
		u16b c;
		do_u16b(&c, LS_LOAD);
		if (c & 0x0002) wizard = TRUE;
		cheat_peek = (c & 0x0100) ? TRUE : FALSE;
		cheat_hear = (c & 0x0200) ? TRUE : FALSE;
		cheat_room = (c & 0x0400) ? TRUE : FALSE;
		cheat_xtra = (c & 0x0800) ? TRUE : FALSE;
		cheat_know = (c & 0x1000) ? TRUE : FALSE;
		cheat_live = (c & 0x2000) ? TRUE : FALSE;
	}
	if (flag == LS_SAVE)
	{
		u16b c = 0;
		if (wizard) c |= 0x0002;
		if (cheat_peek) c |= 0x0100;
		if (cheat_hear) c |= 0x0200;
		if (cheat_room) c |= 0x0400;
		if (cheat_xtra) c |= 0x0800;
		if (cheat_know) c |= 0x1000;
		if (cheat_live) c |= 0x2000;
		do_u16b(&c, LS_SAVE);
	}

	do_byte((byte*)&autosave_l, flag);
	do_byte((byte*)&autosave_t, flag);
	do_s16b(&autosave_freq, flag);

	if (flag == LS_LOAD)
	{
		/* Read the option flags */
		for (n = 0; n < 8; n++) do_u32b(&oflag[n], flag);

		/* Read the option masks */
		for (n = 0; n < 8; n++) do_u32b(&mask[n], flag);

		/* Analyze the options */
		for (n = 0; n < 8; n++)
		{
			/* Analyze the options */
			for (i = 0; i < 32; i++)
			{
				/* Process valid flags */
				if (mask[n] & (1L << i))
				{
					/* Process valid flags */
					if (option_mask[n] & (1L << i))
					{
						/* Set */
						if (oflag[n] & (1L << i))
						{
							/* Set */
							option_flag[n] |= (1L << i);
						}

						/* Clear */
						else
						{
							/* Clear */
							option_flag[n] &= ~(1L << i);
						}
					}
				}
			}
		}


		/*** Window Options ***/

		/* Read the window flags */
		for (n = 0; n < 8; n++) do_u32b(&oflag[n], flag);

		/* Read the window masks */
		for (n = 0; n < 8; n++) do_u32b(&mask[n], flag);

		/* Analyze the options */
		for (n = 0; n < 8; n++)
		{
			/* Analyze the options */
			for (i = 0; i < 32; i++)
			{
				/* Process valid flags */
				if (mask[n] & (1L << i))
				{
					/* Process valid flags */
					if (window_mask[n] & (1L << i))
					{
						/* Set */
						if (oflag[n] & (1L << i))
						{
							/* Set */
							window_flag[n] |= (1L << i);
						}

						/* Clear */
						else
						{
							/* Clear */
							window_flag[n] &= ~(1L << i);
						}
					}
				}
			}
		}
	}
	if (flag == LS_SAVE)
	{
		/* Analyze the options */
		for (i = 0; option_info[i].o_desc; i++)
		{
			int os = option_info[i].o_page;
			int ob = option_info[i].o_bit;

			/* Process real entries */
			if (option_info[i].o_var)
			{
				/* Set */
				if (*option_info[i].o_var)
				{
					/* Set */
					option_flag[os] |= (1L << ob);
				}

				/* Clear */
				else
				{
					/* Clear */
					option_flag[os] &= ~(1L << ob);
				}
			}
		}


		/*** Normal options ***/

		/* Dump the flags */
		for (i = 0; i < 8; i++) do_u32b(&option_flag[i], flag);

		/* Dump the masks */
		for (i = 0; i < 8; i++) do_u32b(&option_mask[i], flag);

		/*** Window options ***/

		/* Dump the flags */
		for (i = 0; i < 8; i++) do_u32b(&window_flag[i], flag);

		/* Dump the masks */
		for (i = 0; i < 8; i++) do_u32b(&window_mask[i], flag);
	}
}


/* Load/Save the random spells info */
static void do_spells(int i, int flag)
{
	random_spell *s_ptr = &random_spells[i];
	do_string(s_ptr->name, 30, flag);
	do_string(s_ptr->desc, 30, flag);
	do_s16b(&s_ptr->mana, flag);
	do_s16b(&s_ptr->fail, flag);
	do_u32b(&s_ptr->proj_flags, flag);
	do_byte(&s_ptr->GF, flag);
	do_byte(&s_ptr->radius, flag);
	do_byte(&s_ptr->dam_sides, flag);
	do_byte(&s_ptr->dam_dice, flag);
	do_byte(&s_ptr->level, flag);
	do_byte((byte*)&s_ptr->untried, flag);
}


/*
 * Handle player inventory
 *
 * FIXME! This function probably could be unified better
 * Note that the inventory is "re-sorted" later by "dungeon()".
 */
static bool_ do_inventory(int flag)
{
	if (flag == LS_LOAD)
	{
		int slot = 0;

		object_type forge;
		object_type *q_ptr;

		/* No items */
		inven_cnt = 0;
		equip_cnt = 0;

		/* Read until done */
		while (1)
		{
			u16b n;

			/* Get the next item index */
			do_u16b(&n, LS_LOAD);

			/* Nope, we reached the end */
			if (n == 0xFFFF) break;

			/* Get local object */
			q_ptr = &forge;

			/* Wipe the object */
			object_wipe(q_ptr);

			/* Read the item */
			do_item(q_ptr, LS_LOAD);

			/* Hack -- verify item */
			if (!q_ptr->k_idx) return (FALSE);

			/* Wield equipment */
			if (n >= INVEN_WIELD)
			{
				/* Copy object */
				object_copy(&p_ptr->inventory[n], q_ptr);

				/* Take care of item sets */
				if (q_ptr->name1)
				{
					wield_set(q_ptr->name1, a_info[q_ptr->name1].set, TRUE);
				}

				/* One more item */
				equip_cnt++;
			}

			/* Warning -- backpack is full */
			else if (inven_cnt == INVEN_PACK)
			{
				/* Oops */
				note("Too many items in the inventory!");

				/* Fail */
				return (FALSE);
			}

			/* Carry inventory */
			else
			{
				/* Get a slot */
				n = slot++;

				/* Copy object */
				object_copy(&p_ptr->inventory[n], q_ptr);

				/* One more item */
				inven_cnt++;
			}
		}
	}
	if (flag == LS_SAVE)
	{
		u16b i;
		u16b sent = 0xFFFF;
		for (i = 0; i < INVEN_TOTAL; i++)
		{
			object_type *o_ptr = &p_ptr->inventory[i];
			if (!o_ptr->k_idx) continue;
			do_u16b(&i, flag);
			do_item(o_ptr, flag);
		}
		do_u16b(&sent, LS_SAVE); 	/* Sentinel */
	}
	/* Success */
	return (TRUE);
}



/*
 * Read the saved messages
 */
static void do_messages(int flag)   /* FIXME! We should be able to unify this better */
{
	int i;
	char buf[128];
	byte color;

	s16b num;

	if (flag == LS_SAVE) num = message_num();

	/* Total */
	do_s16b(&num, flag);

	/* Read the messages */
	if (flag == LS_LOAD)
	{
		byte tmp8u = 0;
		for (i = 0; i < num; i++)
		{
			/* Read the message */
			do_string(buf, 128, LS_LOAD);
			do_byte(&color, flag);
			do_byte(&tmp8u, flag);

			/* Save the message */
			message_add(buf, color);
		}
	}
	if (flag == LS_SAVE)
	{
		byte holder;
		byte zero = 0;
		for (i = num - 1; i >= 0; i--)
		{
			do_string((char *)message_str((s16b)i), 0, LS_SAVE);
			holder = message_color((s16b)i);
			do_byte(&holder, flag);
			do_byte(&zero, flag);
		}
	}
}

/*
 * Handle dungeon
 *
 * The monsters/objects must be loaded in the same order
 * that they were stored, since the actual indexes matter.
 */
static bool_ do_dungeon(int flag, bool_ no_companions)
{
	int i;

	cave_type *c_ptr;

	/* Read specific */
	u16b tmp16b = 0;

	my_sentinel("Before do_dungeon", 324, flag);

	/* Header info */
	do_s16b(&dun_level, flag);
	do_byte(&dungeon_type, flag);
	do_s16b(&num_repro, flag);
	do_s16b(&p_ptr->py, flag);
	do_s16b(&p_ptr->px, flag);
	do_s16b(&cur_hgt, flag);
	do_s16b(&cur_wid, flag);
	do_s16b(&max_panel_rows, flag);
	do_s16b(&max_panel_cols, flag);

	do_u32b(&dungeon_flags1, flag);
	do_u32b(&dungeon_flags2, flag);

	/* Last teleportation */
	do_s16b(&last_teleportation_y, flag);
	do_s16b(&last_teleportation_y, flag);

	/* Spell effects */
	tmp16b = MAX_EFFECTS;
	do_u16b(&tmp16b, flag);

	if ((flag == LS_LOAD) && (tmp16b > MAX_EFFECTS))
	{
		quit("Too many spell effects");
	}

	for (i = 0; i < tmp16b; ++i)
	{
		do_s16b(&effects[i].type, flag);
		do_s16b(&effects[i].dam, flag);
		do_s16b(&effects[i].time, flag);
		do_u32b(&effects[i].flags, flag);
		do_s16b(&effects[i].cx, flag);
		do_s16b(&effects[i].cy, flag);
		do_s16b(&effects[i].rad, flag);
	}

	/* TO prevent bugs with evolving dungeons */
	for (i = 0; i < 100; i++)
	{
		do_s16b(&floor_type[i], flag);
		do_s16b(&fill_type[i], flag);
	}

	if ((flag == LS_LOAD) && (!dun_level && !p_ptr->inside_quest))
	{
		int xstart = 0;
		int ystart = 0;
		/* Init the wilderness */
		process_dungeon_file("w_info.txt", &ystart, &xstart, cur_hgt, cur_wid,
		                     TRUE, FALSE);

		/* Init the town */
		xstart = 0;
		ystart = 0;
		init_flags = 0;
		process_dungeon_file("t_info.txt", &ystart, &xstart, cur_hgt, cur_wid,
		                     TRUE, FALSE);
	}

	do_grid(flag);

	/*** Objects ***/

	if (flag == LS_SAVE) compact_objects(0);
	if (flag == LS_SAVE) compact_monsters(0);
	if (flag == LS_SAVE)
	{
		tmp16b = o_max;
	
		if (no_companions)
		{
			for (i = 1; i < o_max; i++)
			{
				object_type *o_ptr = &o_list[i];

				if (o_ptr->held_m_idx && (m_list[o_ptr->held_m_idx].status == MSTATUS_COMPANION)) tmp16b--;
			}
		}

		/* Item count */
		do_u16b(&tmp16b, flag);

		tmp16b = o_max;
	}
	else
		/* Read item count */
		do_u16b(&tmp16b, flag);

	/* Verify maximum */
	if ((flag == LS_LOAD) && (tmp16b > max_o_idx))
	{
		note(format("Too many (%d) object entries!", tmp16b));
		return (FALSE);
	}

	/* Dungeon items */
	for (i = 1; i < tmp16b; i++)
	{
		int o_idx;

		object_type *o_ptr;

		if (flag == LS_SAVE)
		{
			o_ptr = &o_list[i];
			/* Don't save objects held by companions when no_companions is set */
			if (no_companions && o_ptr->held_m_idx && (m_list[o_ptr->held_m_idx].status == MSTATUS_COMPANION)) continue;

			do_item(o_ptr, LS_SAVE);
			continue; 			/* Saving is easy */
		}
		/* Until the end of the loop, this is all LS_LOAD */

		/* Get a new record */
		o_idx = o_pop();

		/* Oops */
		if (i != o_idx)
		{
			note(format("Object allocation error (%d <> %d)", i, o_idx));
			return (FALSE);
		}


		/* Acquire place */
		o_ptr = &o_list[o_idx];

		/* Read the item */
		do_item(o_ptr, LS_LOAD);

		/* Monster */
		if (o_ptr->held_m_idx)
		{
			monster_type *m_ptr;

			/* Monster */
			m_ptr = &m_list[o_ptr->held_m_idx];

			/* Build a stack */
			o_ptr->next_o_idx = m_ptr->hold_o_idx;

			/* Place the object */
			m_ptr->hold_o_idx = o_idx;
		}

		/* Dungeon */
		else
		{
			/* Access the item location */
			c_ptr = &cave[o_ptr->iy][o_ptr->ix];

			/* Build a stack */
			o_ptr->next_o_idx = c_ptr->o_idx;

			/* Place the object */
			c_ptr->o_idx = o_idx;
		}
	}

	/*** Monsters ***/

	if (flag == LS_SAVE)
	{
		tmp16b = m_max;

		if (no_companions)
		{
			for (i = 1; i < m_max; i++)
			{
				monster_type *m_ptr = &m_list[i];

				if (m_ptr->status == MSTATUS_COMPANION) tmp16b--;
			}
		}

		/* Write the monster count */
		do_u16b(&tmp16b, flag);

		tmp16b = m_max;
	}
	else
		/* Read the monster count */
		do_u16b(&tmp16b, flag);

	/* Validate */
	if ((flag == LS_LOAD) && (tmp16b > max_m_idx))
	{
		note(format("Too many (%d) monster entries!", tmp16b));
		return (FALSE);
	}

	/* Read the monsters */
	for (i = 1; i < tmp16b; i++)
	{
		int m_idx;
		monster_type *m_ptr;
		monster_race *r_ptr;

		if (flag == LS_SAVE)
		{
			m_ptr = &m_list[i];

			/* Don't save companions when no_companions is set */
			if (no_companions && m_ptr->status == MSTATUS_COMPANION) continue;

			do_monster(m_ptr, LS_SAVE);
			continue; 			/* Easy to save a monster */
		}
		/* From here on, it's all LS_LOAD */
		/* Get a new record */
		m_idx = m_pop();

		/* Oops */
		if (i != m_idx)
		{
			note(format("Monster allocation error (%d <> %d)", i, m_idx));
			return (FALSE);
		}

		/* Acquire monster */
		m_ptr = &m_list[m_idx];

		/* Read the monster */
		do_monster(m_ptr, LS_LOAD);

		/* Access grid */
		c_ptr = &cave[m_ptr->fy][m_ptr->fx];

		/* Mark the location */
		c_ptr->m_idx = m_idx;

		/* Controlled ? */
		if (m_ptr->mflag & MFLAG_CONTROL)
			p_ptr->control = m_idx;

		/* Access race */
		r_ptr = &r_info[m_ptr->r_idx];

		/* Count XXX XXX XXX */
		r_ptr->cur_num++;
	}

	/* Read the kept monsters */

	tmp16b = (flag == LS_SAVE && !no_companions) ? max_m_idx : 0;

	/* Read the monster count */
	do_u16b(&tmp16b, flag);

	/* Hack -- verify */
	if ((flag == LS_LOAD) && (tmp16b > max_m_idx))
	{
		note(format("Too many (%d) monster entries!", tmp16b));
		return (FALSE);
	}
	for (i = 1; i < tmp16b; i++)
	{
		monster_type *m_ptr;

		/* Acquire monster */
		m_ptr = &km_list[i];

		/* Read the monster */
		do_monster(m_ptr, flag);
	}

	/*** Success ***/

	/* The dungeon is ready */
	if (flag == LS_LOAD) character_dungeon = TRUE;

	/* Success */
	return (TRUE);
}

/* Returns TRUE if we successfully load the dungeon */
bool_ load_dungeon(char *ext)
{
	char tmp[16];
	char name[1024];
	byte old_dungeon_type = dungeon_type;
	s16b old_dun = dun_level;

	/* Construct name */
	sprintf(tmp, "%s.%s", player_base, ext);
	path_build(name, 1024, ANGBAND_DIR_SAVE, tmp);

	/* Open the file */
	fff = my_fopen(name, "rb");

	if (fff == NULL)
	{
		dun_level = old_dun;
		dungeon_type = old_dungeon_type;

		my_fclose(fff);
		return (FALSE);
	}

	/* Read the dungeon */
	if (!do_dungeon(LS_LOAD, FALSE))
	{
		dun_level = old_dun;
		dungeon_type = old_dungeon_type;

		my_fclose(fff);
		return (FALSE);
	}

	dun_level = old_dun;
	dungeon_type = old_dungeon_type;

	/* Done */
	my_fclose(fff);
	return (TRUE);
}

void do_blocks(int flag)
/* Handle blocked-allocation stuff for quests and lua stuff
   This depends on a dyn_tosave array of s32b's. Adjust as needed
   if other data structures are desirable. This also is not hooked
   in yet. Ideally, plug it near the end of the savefile.
 */
{
	s16b numblks, n_iter = 0; 	/* How many blocks do we have? */
	do_s16b(&numblks, flag);
	while (n_iter < numblks)
	{
		/*	do_s32b(dyn_tosave[n_iter], flag); */
		n_iter++;
	}
	my_sentinel("In blocked-allocation area", 37, flag);
}

void do_fate(int i, int flag)
{
	if ((flag == LS_LOAD) && (i >= MAX_FATES)) i = MAX_FATES - 1;

	do_byte(&fates[i].fate, flag);
	do_byte(&fates[i].level, flag);
	do_byte(&fates[i].serious, flag);
	do_s16b(&fates[i].o_idx, flag);
	do_s16b(&fates[i].e_idx, flag);
	do_s16b(&fates[i].a_idx, flag);
	do_s16b(&fates[i].v_idx, flag);
	do_s16b(&fates[i].r_idx, flag);
	do_s16b(&fates[i].count, flag);
	do_s16b(&fates[i].time, flag);
	do_byte((byte*)&fates[i].know, flag);
}

/*
 * Load/save timers.
 */
static void do_timers(int flag)
{
	timer_type *t_ptr;

	for (t_ptr = gl_timers; t_ptr != NULL; t_ptr = t_ptr->next)
	{
		do_bool(&t_ptr->enabled, flag);
		do_s32b(&t_ptr->delay, flag);
		do_s32b(&t_ptr->countdown, flag);
	}
}

/*
 * Actually read the savefile
 */
static bool_ do_savefile_aux(int flag)
{
	int i, j;

	byte tmp8u;
	u16b tmp16u;

	bool_ *reals;
	u16b real_max = 0;

	/* Mention the savefile version */
	if (flag == LS_LOAD)
	{
		if (vernum < 100)
		{
			note(format("Savefile version %lu too old! ", vernum));
			return FALSE;
		}
		else
		{
			note(format("Loading version %lu savefile... ", vernum));
		}
	}
	if (flag == LS_SAVE)
	{
		sf_when = time((time_t *) 0); 	/* Note when file was saved */
		sf_xtra = 0L; 			/* What the hell is this? */
		sf_saves++; 				/* Increment the saves ctr */
	}

	/* Handle version bytes. FIXME! DG wants me to change this all around */
	if (flag == LS_LOAD)
	{
		u32b mt32b;
		byte mtbyte;

		/* Discard all this, we've already read it */
		do_u32b(&mt32b, flag);
		do_byte(&mtbyte, flag);
	}
	if (flag == LS_SAVE)
	{
		u32b saver;
		saver = SAVEFILE_VERSION;
		do_u32b(&saver, flag);
		tmp8u = (byte)rand_int(256);
		do_byte(&tmp8u, flag); 	/* 'encryption' */
	}

	/* Operating system info? Not really. This is just set to 0L */
	do_u32b(&sf_xtra, flag);

	/* Time of last save */
	do_u32b(&sf_when, flag);

	/* Number of past lives */
	do_u16b(&sf_lives, flag);

	/* Number of times saved */
	do_u16b(&sf_saves, flag);

	/* Game module */
	if (flag == LS_SAVE)
		strcpy(loaded_game_module, game_module);
	do_string(loaded_game_module, 80, flag);

	/* Timers */
	do_timers(flag);

	/* Read RNG state */
	do_randomizer(flag);

	/* Automatizer state */
	do_byte((byte*)&automatizer_enabled, flag);

	/* Then the options */
	do_options(flag);

	/* Then the "messages" */
	do_messages(flag);

	/* Monster Memory */
	if (flag == LS_SAVE) tmp16u = max_r_idx;
	do_u16b(&tmp16u, flag);

	/* Incompatible save files */
	if ((flag == LS_LOAD) && (tmp16u > max_r_idx))
	{
		note(format("Too many (%u) monster races!", tmp16u));
		return (FALSE);
	}

	/* Read the available records */
	for (i = 0; i < tmp16u; i++)
	{
		/* Read the lore */
		do_lore(i, flag);
	}

	/* Object Memory */
	if (flag == LS_SAVE) tmp16u = max_k_idx;
	do_u16b(&tmp16u, flag);

	/* Incompatible save files */
	if ((flag == LS_LOAD) && (tmp16u > max_k_idx))
	{
		note(format("Too many (%u) object kinds!", tmp16u));
		return (FALSE);
	}

	/* Read the object memory */
	for (i = 0; i < tmp16u; i++) do_xtra(i, flag);
	if (flag == LS_LOAD) junkinit();

	{
		u16b max_towns_ldsv;
		u16b max_quests_ldsv;
		if (flag == LS_SAVE) max_towns_ldsv = max_towns;
		/* Number of towns */
		do_u16b(&max_towns_ldsv, flag);
		/* Incompatible save files */
		if ((flag == LS_LOAD) && (max_towns_ldsv > max_towns))
		{
			note(format("Too many (%u) towns!", max_towns_ldsv));
			return (FALSE);
		}
		/* Min of random towns */
		if (flag == LS_SAVE) max_towns_ldsv = TOWN_RANDOM;
		do_u16b(&max_towns_ldsv, flag);
		/* Incompatible save files */
		if ((flag == LS_LOAD) && (max_towns_ldsv != TOWN_RANDOM))
		{
			note(format("Different random towns base (%u)!", max_towns_ldsv));
			return (FALSE);
		}

		for (i = 0; i < max_towns; i++)
		{
			do_byte((byte*)&town_info[i].destroyed, flag);

			if (i >= TOWN_RANDOM)
			{
				do_u32b(&town_info[i].seed, flag);
				do_byte(&town_info[i].numstores, flag);
				do_byte(&town_info[i].flags, flag);

				/* If the town is realy used create a sock */
				if ((town_info[i].flags & (TOWN_REAL)) && (flag == LS_LOAD))
				{
					create_stores_stock(i);
				}
			}
		}

		/* Number of dungeon */
		if (flag == LS_SAVE) max_towns_ldsv = max_d_idx;
		do_u16b(&max_towns_ldsv, flag);

		/* Incompatible save files */
		if ((flag == LS_LOAD) && (max_towns_ldsv > max_d_idx))
		{
			note(format("Too many dungeon types (%u)!", max_towns_ldsv));
			return (FALSE);
		}

		/* Number of towns per dungeon */
		if (flag == LS_SAVE) max_quests_ldsv = TOWN_DUNGEON;
		do_u16b(&max_quests_ldsv, flag);
		/* Incompatible save files */
		if ((flag == LS_LOAD) && (max_quests_ldsv > TOWN_DUNGEON))
		{
			note(format("Too many town per dungeons (%u)!", max_quests_ldsv));
			return (FALSE);
		}

		for (i = 0; i < max_towns_ldsv; i++)
		{
			for (j = 0; j < max_quests_ldsv; j++)
			{
				do_s16b(&(d_info[i].t_idx[j]), flag);
				do_s16b(&(d_info[i].t_level[j]), flag);
			}
			do_s16b(&(d_info[i].t_num), flag);
		}

		/* Sanity check number of quests */
		if (flag == LS_SAVE) max_quests_ldsv = MAX_Q_IDX;
		do_u16b(&max_quests_ldsv, flag);

		/* Incompatible save files */
		if ((flag == LS_LOAD) && (max_quests_ldsv != MAX_Q_IDX))
		{
			note(format("Invalid number of quests (%u)!", max_quests_ldsv));
			return (FALSE);
		}

		for (i = 0; i < MAX_Q_IDX; i++)
		{
			do_s16b(&quest[i].status, flag);
			for (j = 0; j < sizeof(quest[i].data)/sizeof(quest[i].data[0]); j++)
			{
				do_s32b(&(quest[i].data[j]), flag);
			}

			/* Init the hooks */
			if (flag == LS_LOAD)
			{
				quest[i].init(i);
			}
		}

		/* Position in the wilderness */
		do_s32b(&p_ptr->wilderness_x, flag);
		do_s32b(&p_ptr->wilderness_y, flag);
		do_byte((byte*)&p_ptr->wild_mode, flag);
		do_byte((byte*)&p_ptr->old_wild_mode, flag);

		{
			s32b wild_x_size, wild_y_size;
			if (flag == LS_SAVE)
			{
				wild_x_size = max_wild_x;
				wild_y_size = max_wild_y;
			}
			/* Size of the wilderness */
			do_s32b(&wild_x_size, flag);
			do_s32b(&wild_y_size, flag);
			/* Incompatible save files */
			if ((flag == LS_LOAD) &&
			                ((wild_x_size > max_wild_x) || (wild_y_size > max_wild_y)))
			{
				note(format("Wilderness is too big (%u/%u)!",
				            wild_x_size, wild_y_size));
				return (FALSE);
			}
			/* Wilderness seeds */
			for (i = 0; i < wild_x_size; i++)
			{
				for (j = 0; j < wild_y_size; j++)
				{
					do_u32b(&wild_map[j][i].seed, flag);
					do_u16b(&wild_map[j][i].entrance, flag);
					do_byte((byte*)&wild_map[j][i].known, flag);
				}
			}
		}
	}

	/* Load the random artifacts. */
	if (flag == LS_SAVE) tmp16u = MAX_RANDARTS;
	do_u16b(&tmp16u, flag);
	if ((flag == LS_LOAD) && (tmp16u > MAX_RANDARTS))
	{
		note(format("Too many (%u) random artifacts!", tmp16u));
		return (FALSE);
	}
	for (i = 0; i < tmp16u; i++)
	{
		random_artifact *ra_ptr = &random_artifacts[i];

		do_string(ra_ptr->name_full, 80, flag);
		do_string(ra_ptr->name_short, 80, flag);
		do_byte(&ra_ptr->level, flag);
		do_byte(&ra_ptr->attr, flag);
		do_u32b(&ra_ptr->cost, flag);
		do_byte(&ra_ptr->activation, flag);
		do_byte(&ra_ptr->generated, flag);
	}

	/* Load the Artifacts */
	if (flag == LS_SAVE) tmp16u = max_a_idx;
	do_u16b(&tmp16u, flag);
	/* Incompatible save files */
	if ((flag == LS_LOAD) && (tmp16u > max_a_idx))
	{
		note(format("Too many (%u) artifacts!", tmp16u));
		return (FALSE);
	}

	/* Read the artifact flags */
	for (i = 0; i < tmp16u; i++)
	{
		do_byte(&(&a_info[i])->cur_num, flag);
	}

	/* Fates */
	if (flag == LS_SAVE) tmp16u = MAX_FATES;
	do_u16b(&tmp16u, flag);

	/* Incompatible save files */
	if ((flag == LS_LOAD) && (tmp16u > MAX_FATES))
	{
		note(format("Too many (%u) fates!", tmp16u));
		return (FALSE);
	}

	/* Read the fate flags */
	for (i = 0; i < tmp16u; i++)
	{
		do_fate(i, flag);
	}

	/* Load the Traps */
	if (flag == LS_SAVE) tmp16u = max_t_idx;
	do_u16b(&tmp16u, flag);

	/* Incompatible save files */
	if ((flag == LS_LOAD) && (tmp16u > max_t_idx))
	{
		note(format("Too many (%u) traps!", tmp16u));
		return (FALSE);
	}

	/* fate flags */
	for (i = 0; i < tmp16u; i++)
	{
		do_byte((byte*)&t_info[i].ident, flag);
	}

	/* inscription knowledge */
	if (flag == LS_SAVE) tmp16u = MAX_INSCRIPTIONS;
	do_u16b(&tmp16u, flag);

	/* Incompatible save files */
	if ((flag == LS_LOAD) && (tmp16u > MAX_INSCRIPTIONS))
	{
		note(format("Too many (%u) inscriptions!", tmp16u));
		return (FALSE);
	}

	/* Read the inscription flag */
	for (i = 0; i < tmp16u; i++)
		do_byte((byte*)&inscription_info[i].know, flag);


	/* Read the extra stuff */
	if (!do_extra(flag))
		return FALSE;


	/* player_hp array */
	if (flag == LS_SAVE) tmp16u = PY_MAX_LEVEL;
	do_u16b(&tmp16u, flag);
	/* Incompatible save files */
	if ((flag == LS_LOAD) && (tmp16u > PY_MAX_LEVEL))
	{
		note(format("Too many (%u) hitpoint entries!", tmp16u));
		return (FALSE);
	}

	/* Read the player_hp array */
	for (i = 0; i < tmp16u; i++)
	{
		do_s16b(&player_hp[i], flag);
	}

	if (flag == LS_LOAD) morejunk();

	/* Read the pet command settings */
	do_byte(&p_ptr->pet_follow_distance, flag);
	do_byte(&p_ptr->pet_open_doors, flag);
	do_byte(&p_ptr->pet_pickup_items, flag);

	/* Dripping Tread */
	do_s16b(&p_ptr->dripping_tread, flag);

	/* Read the inventory */
	if (!do_inventory(flag) && (flag == LS_LOAD))	/* do NOT reverse this ordering */
	{
		note("Unable to read inventory");
		return (FALSE);
	}

	/* Note that this forbids max_towns from shrinking, but that is fine */
	C_MAKE(reals, max_towns, bool_);

	/* Find the real towns */
	if (flag == LS_SAVE)
	{
		for (i = 1; i < max_towns; i++)
		{
			if (!(town_info[i].flags & (TOWN_REAL))) continue;
			reals[real_max++] = i;
		}
	}
	do_u16b(&real_max, flag);
	for (i = 0; i < real_max; i++)
	{
		do_byte((byte*)&reals[i], flag);
	}

	/* Read the stores */
	if (flag == LS_SAVE) tmp16u = max_st_idx;
	do_u16b(&tmp16u, flag);

	/* Ok now read the real towns */
	for (i = 0; i < real_max; i++)
	{
		int z = reals[i];

		/* Ultra paranoia */
		if (!town_info[z].stocked) create_stores_stock(z);

		for (j = 0; j < tmp16u; j++)
			do_store(&town_info[z].store[j], flag);
	}

	C_FREE(reals, max_towns, bool_);

	/* I'm not dead yet... */
	if (!death)
	{
		/* Dead players have no dungeon */
		if (flag == LS_LOAD) note("Restoring Dungeon...");
		if ((flag == LS_LOAD) && (!do_dungeon(LS_LOAD, FALSE)))
		{
			note("Error reading dungeon data");
			return (FALSE);
		}
		if (flag == LS_SAVE) do_dungeon(LS_SAVE, FALSE);
		my_sentinel("Before ghost data", 435, flag);
		my_sentinel("After ghost data", 320, flag);
	}

	{
		byte foo = 0;
		if (flag == LS_SAVE)
		{
			/*
			 * Safety Padding. It's there
			 * for a good reason. Trust me on
			 * this. Keep this at the *END*
			 * of the file, and do *NOT* try to
			 * read it. Insert any new stuff before
			 * this position.
			 */
			do_byte(&foo, LS_SAVE);
		}
	}

	/* Success */
	return (TRUE);
}


/*
 * Actually read the savefile
 */
errr rd_savefile(void)
{
	errr err = 0;

	/* The savefile is a binary file */
	fff = my_fopen(savefile, "rb");
	
	/* Paranoia */
	if (!fff) return ( -1);

	/* Call the sub-function */
	err = !do_savefile_aux(LS_LOAD);

	/* Check for errors */
	if (ferror(fff)) err = -1;

	/* Close the file */
	my_fclose(fff);

	/* Result */
	return (err);
}

/*
 * Note that this function may not be needed at all.
 * It was taken out of load_player_aux(). Do we need it?
 */
static void junkinit(void)
{
	int i, j;
	p_ptr->arena_number = 0;
	p_ptr->inside_arena = 0;
	p_ptr->inside_quest = 0;
	p_ptr->exit_bldg = TRUE;
	p_ptr->exit_bldg = TRUE;
	p_ptr->town_num = 1;
	p_ptr->wilderness_x = 4;
	p_ptr->wilderness_y = 4;
	for (i = 0; i < max_wild_x; i++)
	{
		for (j = 0; j < max_wild_y; j++)
		{
			wild_map[j][i].seed = rand_int(0x10000000);
		}
	}
}

static void morejunk(void)
{
	sp_ptr = &sex_info[p_ptr->psex]; 	/* Sex */
	rp_ptr = &race_info[p_ptr->prace]; 	/* Raceclass */
	rmp_ptr = &race_mod_info[p_ptr->pracem];
	cp_ptr = &class_info[p_ptr->pclass];
	spp_ptr = &class_info[p_ptr->pclass].spec[p_ptr->pspec];
}

static void do_grid(int flag)
/* Does the grid, RLE, blahblah. RLE sucks. I hate it. */
{
	int i = 0, y = 0, x = 0;
	byte count = 0;
	byte tmp8u = 0;
	s16b tmp16s = 0;
	cave_type *c_ptr;
	byte prev_char = 0;
	s16b prev_s16b = 0;
	int ymax = cur_hgt, xmax = cur_wid;

	int part; 	/* Which section of the grid we're on */

	for (part = 0; part < 9; part++)	/* There are 8 fields to the grid, each stored
												   in a seperate RLE data structure */
	{
		if (flag == LS_SAVE)
		{
			count = 0;
			prev_s16b = 0;
			prev_char = 0; 		/* Clear, prepare for RLE */
			for (y = 0; y < cur_hgt; y++)
			{
				for (x = 0; x < cur_wid; x++)
				{
					c_ptr = &cave[y][x];
					switch (part)
					{
					case 0:
						tmp16s = c_ptr->info;
						break;

					case 1:
						tmp8u = c_ptr->feat;
						break;

					case 2:
						tmp8u = c_ptr->mimic;
						break;

					case 3:
						tmp16s = c_ptr->special;
						break;

					case 4:
						tmp16s = c_ptr->special2;
						break;

					case 5:
						tmp16s = c_ptr->t_idx;
						break;

					case 6:
						tmp16s = c_ptr->inscription;
						break;

					case 7:
						tmp8u = c_ptr->mana;
						break;

					case 8:
						tmp16s = c_ptr->effect;
						break;
					}
					/* Flush a full run */
					if ((((part != 1) && (part != 2) && (part != 7)) &&
					                (tmp16s != prev_s16b)) || (((part == 1) || (part == 2)
					                                            || (part == 7)) &&
					                                           (tmp8u != prev_char)) ||
					                (count == MAX_UCHAR))
					{
						do_byte(&count, LS_SAVE);
						switch (part)
						{
						case 0:
						case 3:
						case 4:
						case 5:
						case 6:
						case 8:
							do_s16b(&prev_s16b, LS_SAVE);
							prev_s16b = tmp16s;
							break;

						case 1:
						case 2:
						case 7:
							do_byte(&prev_char, LS_SAVE);
							prev_char = tmp8u;
							break;
						}
						count = 1; 	/* Reset RLE */
					}
					else
						count++; 	/* Otherwise, keep going */
				}
			}
			/* Fallen off the end of the world, flush anything left */
			if (count)
			{
				do_byte(&count, LS_SAVE);
				switch (part)
				{
				case 0:
				case 3:
				case 4:
				case 5:
				case 6:
				case 8:
					do_s16b(&prev_s16b, LS_SAVE);
					break;

				case 1:
				case 2:
				case 7:
					do_byte(&prev_char, LS_SAVE);
					break;
				}
			}
		}
		if (flag == LS_LOAD)
		{
			x = 0;
			for (y = 0; y < ymax; )
			{
				do_byte(&count, LS_LOAD);
				switch (part)
				{
				case 0:
				case 3:
				case 4:
				case 5:
				case 6:
				case 8:
					do_s16b(&tmp16s, LS_LOAD);
					break;

				case 1:
				case 2:
				case 7:
					do_byte(&tmp8u, LS_LOAD);
					break;
				}
				for (i = count; i > 0; i--)	/* RLE */
				{
					c_ptr = &cave[y][x];
					switch (part)
					{
					case 0:
						c_ptr->info = tmp16s;
						break;

					case 1:
						c_ptr->feat = tmp8u;
						break;

					case 2:
						c_ptr->mimic = tmp8u;
						break;

					case 3:
						c_ptr->special = tmp16s;
						break;

					case 4:
						c_ptr->special2 = tmp16s;
						break;

					case 5:
						c_ptr->t_idx = tmp16s;
						break;

					case 6:
						c_ptr->inscription = tmp16s;
						break;

					case 7:
						c_ptr->mana = tmp8u;
						break;

					case 8:
						c_ptr->effect = tmp16s;
						break;
					}
					if (++x >= xmax)
					{
						/* Wrap */
						x = 0;
						if ((++y) >= ymax) break;
					}
				}
			}
		}
	}
}

static void my_sentinel(char *place, u16b value, int flag)
/* This function lets us know exactly where a savefile is
   broken by reading/writing conveniently a sentinel at this
   spot */
{
	if (flag == LS_SAVE)
	{
		do_u16b(&value, flag);
		return;
	}
	if (flag == LS_LOAD)
	{
		u16b found;
		do_u16b(&found, flag);
		if (found == value)		/* All is good */
			return;
		/* All is bad */
		note(format("Savefile broken %s", place));
		return;
	}
	note(format("Impossible has occurred")); 	/* Programmer error */
	exit(0);
}
