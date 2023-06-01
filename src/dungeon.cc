/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "dungeon.hpp"

#include "birth.hpp"
#include "cave.hpp"
#include "cave_type.hpp"
#include "cmd1.hpp"
#include "cmd2.hpp"
#include "cmd3.hpp"
#include "cmd4.hpp"
#include "cmd5.hpp"
#include "cmd6.hpp"
#include "cmd7.hpp"
#include "corrupt.hpp"
#include "dungeon_flag.hpp"
#include "dungeon_info_type.hpp"
#include "feature_flag.hpp"
#include "feature_type.hpp"
#include "files.hpp"
#include "game.hpp"
#include "generate.hpp"
#include "gen_evol.hpp"
#include "gods.hpp"
#include "help.hpp"
#include "hooks.hpp"
#include "init2.hpp"
#include "levels.hpp"
#include "loadsave.hpp"
#include "lua_bind.hpp"
#include "melee1.hpp"
#include "melee2.hpp"
#include "monster2.hpp"
#include "monster3.hpp"
#include "monster_race.hpp"
#include "monster_race_flag.hpp"
#include "monster_type.hpp"
#include "modules.hpp"
#include "notes.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "object_flag.hpp"
#include "object_kind.hpp"
#include "object_type.hpp"
#include "options.hpp"
#include "player_race.hpp"
#include "player_race_flag.hpp"
#include "player_race_mod.hpp"
#include "player_spec.hpp"
#include "player_type.hpp"
#include "powers.hpp"
#include "quest.hpp"
#include "skills.hpp"
#include "spell_type.hpp"
#include "spells1.hpp"
#include "spells2.hpp"
#include "spells5.hpp"
#include "squeltch.hpp"
#include "stats.hpp"
#include "store.hpp"
#include "tables.hpp"
#include "timer_type.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "wild.hpp"
#include "wilderness_map.hpp"
#include "wilderness_type_info.hpp"
#include "wizard2.hpp"
#include "xtra1.hpp"
#include "xtra2.hpp"
#include "z-rand.hpp"
#include "z-util.hpp"

#include <boost/filesystem.hpp>
#include <cassert>

#define TY_CURSE_CHANCE 100
#define DG_CURSE_CHANCE 50
#define AUTO_CURSE_CHANCE 15
#define CHAINSWORD_NOISE 100

/*
 * Can a player be resurrected?
 */
static bool granted_resurrection()
{
	if (praying_to(GOD_ERU))
	{
		if (p_ptr->grace > 100000)
		{
			return magik(70);
		}
	}
	return false;
}

/*
 * If player has inscribed the object with "!!", let him know when it's
 * recharged. -LM-
 */
static void recharged_notice(object_type *o_ptr)
{
	/* No inscription */
	if (o_ptr->inscription.empty())
	{
		return;
	}

	/* Find a '!' */
	auto s = strchr(o_ptr->inscription.c_str(), '!');

	/* Process notification request. */
	while (s)
	{
		/* Find another '!' */
		if (s[1] == '!')
		{
			/* Describe (briefly) */
			char o_name[80];
			object_desc(o_name, o_ptr, false, 0);

			/* Notify the player */
			if (o_ptr->number > 1)
			{
				msg_format("Your %s are recharged.", o_name);
			}
			else
			{
				msg_format("Your %s is recharged.", o_name);
			}

			/* Done. */
			return;
		}

		/* Keep looking for '!'s */
		s = strchr(s + 1, '!');
	}
}



/*
 * Regenerate hit points				-RAK-
 */
static void regenhp(int percent)
{
	s32b new_chp, new_chp_frac;

	int old_chp;


	/* Only if alive */
	if (!(p_ptr->necro_extra & CLASS_UNDEAD))
	{
		/* Save the old hitpoints */
		old_chp = p_ptr->chp;

		/* Extract the new hitpoints */
		new_chp = ((long)p_ptr->mhp) * percent + PY_REGEN_HPBASE;

		/* div 65536 */
		p_ptr->chp += new_chp >> 16;

		/* check for overflow */
		if ((p_ptr->chp < 0) && (old_chp > 0)) p_ptr->chp = MAX_SHORT;

		/* mod 65536 */
		new_chp_frac = (new_chp & 0xFFFF) + p_ptr->chp_frac;

		if (new_chp_frac >= 0x10000L)
		{
			p_ptr->chp_frac = new_chp_frac - 0x10000L;
			p_ptr->chp++;
		}
		else
		{
			p_ptr->chp_frac = new_chp_frac;
		}

		/* Fully healed */
		if (p_ptr->chp >= p_ptr->mhp)
		{
			p_ptr->chp = p_ptr->mhp;
			p_ptr->chp_frac = 0;
		}

		/* Notice changes */
		if (old_chp != p_ptr->chp)
		{
			/* Redraw */
			p_ptr->redraw |= (PR_FRAME);

			/* Window stuff */
			p_ptr->window |= (PW_PLAYER);
		}
	}
}


/*
 * Regenerate mana points				-RAK-
 */
static void regenmana(int percent)
{
	s32b new_mana, new_mana_frac;

	int old_csp;

	/* Incraese regen with int */
	percent += adj_str_blow[p_ptr->stat_ind[A_INT]] * 3;

	old_csp = p_ptr->csp;
	new_mana = ((long)p_ptr->msp) * percent + PY_REGEN_MNBASE;

	/* div 65536 */
	p_ptr->csp += new_mana >> 16;

	/* check for overflow */
	if ((p_ptr->csp < 0) && (old_csp > 0))
	{
		p_ptr->csp = MAX_SHORT;
	}

	/* mod 65536 */
	new_mana_frac = (new_mana & 0xFFFF) + p_ptr->csp_frac;

	if (new_mana_frac >= 0x10000L)
	{
		p_ptr->csp_frac = new_mana_frac - 0x10000L;
		p_ptr->csp++;
	}
	else
	{
		p_ptr->csp_frac = new_mana_frac;
	}

	/* Must set frac to zero even if equal */
	if (p_ptr->csp >= p_ptr->msp)
	{
		p_ptr->csp = p_ptr->msp;
		p_ptr->csp_frac = 0;
	}

	/* Redraw mana */
	if (old_csp != p_ptr->csp)
	{
		/* Redraw */
		p_ptr->redraw |= (PR_FRAME);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);
	}
}






/*
 * Regenerate the monsters (once per 100 game turns)
 *
 * XXX XXX XXX Should probably be done during monster turns.
 */
static void regen_monsters()
{
	auto const &r_info = game->edit_data.r_info;

	auto o_ptr = &p_ptr->inventory[INVEN_CARRY];

	if (o_ptr->k_ptr)
	{
		auto r_ptr = &r_info[o_ptr->pval];

		/* Allow regeneration (if needed) */
		if (o_ptr->pval2 < o_ptr->pval3)
		{
			/* Hack -- Base regeneration */
			int frac = o_ptr->pval3 / 100;

			/* Hack -- Minimal regeneration rate */
			if (!frac) frac = 1;

			/* Hack -- Some monsters regenerate quickly */
			if (r_ptr->flags & RF_REGENERATE) frac *= 2;


			/* Hack -- Regenerate */
			o_ptr->pval2 += frac;

			/* Do not over-regenerate */
			if (o_ptr->pval2 > o_ptr->pval3) o_ptr->pval2 = o_ptr->pval3;

			/* Redraw (later) */
			p_ptr->redraw |= (PR_FRAME);
		}
	}

	/* Regenerate everyone */
	for (int i = 1; i < m_max; i++)
	{
		/* Check the i'th monster */
		monster_type *m_ptr = &m_list[i];

		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Dont regen bleeding/poisonned monsters */
		if (m_ptr->bleeding || m_ptr->poisoned) continue;

		/* Allow regeneration (if needed) */
		if (m_ptr->hp < m_ptr->maxhp)
		{
			/* Hack -- Base regeneration */
			int frac = m_ptr->maxhp / 100;

			/* Hack -- Minimal regeneration rate */
			if (!frac) frac = 1;

			/* Hack -- Some monsters regenerate quickly */
			auto const r_ptr = m_ptr->race();
			if (r_ptr->flags & RF_REGENERATE) frac *= 2;

			/* Hack -- Regenerate */
			m_ptr->hp += frac;

			/* Do not over-regenerate */
			if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

			/* Redraw (later) if needed */
			if (health_who == i) p_ptr->redraw |= (PR_FRAME);
		}
	}
}


/*
 * Does an object decay?
 *
 * Should belong to object1.c, renamed to object_decays() -- pelpel
 */
static bool decays(object_type *o_ptr)
{
	auto const flags = object_flags(o_ptr);
	return bool(flags & TR_DECAY);
}


static int process_lasting_spell(s16b music)
{
	spell_type *spell = spell_at(-music);
	return spell_type_produce_effect_lasting(spell);
}

static void check_music()
{
	int use_mana;

	/* Music sung by player */
	if (!p_ptr->music_extra) return;

	use_mana = process_lasting_spell(p_ptr->music_extra);

	if (p_ptr->csp < use_mana)
	{
		msg_print("You stop your spell.");
		p_ptr->music_extra = 0;
	}
	else
	{
		p_ptr->csp -= use_mana;

		/* Redraw mana */
		p_ptr->redraw |= (PR_FRAME);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);
	}
}


/*
 * Generate the feature effect
 */
static void apply_effect(int y, int x)
{
	auto const &f_info = game->edit_data.f_info;

	cave_type *c_ptr = &cave[y][x];

	auto f_ptr = &f_info[c_ptr->feat];


	if (f_ptr->d_frequency[0] != 0)
	{
		int i;

		for (i = 0; i < 4; i++)
		{
			/* Check the frequency */
			if (f_ptr->d_frequency[i] == 0) continue;

			if (((turn % f_ptr->d_frequency[i]) == 0) &&
			                ((f_ptr->d_side[i] != 0) || (f_ptr->d_dice[i] != 0)))
			{
				int l, dam = 0;
				int d = f_ptr->d_dice[i], s = f_ptr->d_side[i];

				if (d == -1) d = p_ptr->lev;
				if (s == -1) s = p_ptr->lev;

				/* Roll damage */
				for (l = 0; l < d; l++)
				{
					dam += randint(s);
				}

				/* Apply damage */
				project( -100, 0, y, x, dam, f_ptr->d_type[i],
				         PROJECT_KILL | PROJECT_HIDE);

				/* Hack -- notice death */
				if (!alive || death) return;
			}
		}
	}
}



/* XXX XXX XXX */
static bool is_recall = false;


/*
 * Hook for corruptions
 */
static void process_world_corruptions()
{
	if (player_has_corruption(CORRUPT_RANDOM_TELEPORT))
	{
		if (rand_int(300) == 1)
		{
			if (magik(70))
			{
				if (get_check("Teleport?"))
				{
					teleport_player(50);
				}
				else
				{
					disturb();
					msg_print("Your corruption takes over you, you teleport!");
					teleport_player(50);
				}
			}
		}
	}

	if (player_has_corruption(CORRUPT_ANTI_TELEPORT))
	{
		if (p_ptr->corrupt_anti_teleport_stopped)
		{
			int amt = p_ptr->msp + p_ptr->csp;
			amt = amt / 100;
			if (amt < 1) {
				amt = 1;
			}
			increase_mana(-amt);
			if (p_ptr->csp == 0)
			{
				p_ptr->corrupt_anti_teleport_stopped = false;
				msg_print("You stop controlling your corruption.");
				p_ptr->update = p_ptr->update | PU_BONUS;
			}
		}
	}
}


static bool grace_delay_trigger()
{
	p_ptr->grace_delay++;

	if (p_ptr->grace_delay >= 15)
	{
		/* reset */
		p_ptr->grace_delay = 0;
		/* triggered */
		return true;
	}
	else
	{
		/* not triggered */
		return false;
	}
}

/*
 * Hook for gods
 */
static void process_world_gods()
{
	auto const &race_name = rp_ptr->title;
	auto const &subrace_name = rmp_ptr->title;

	if (p_ptr->pgod == GOD_VARDA)
	{
		if (grace_delay_trigger())
		{
			/* Piety increases if in light. */
			if (cave[p_ptr->py][p_ptr->px].info & CAVE_GLOW)
			{
				inc_piety(GOD_ALL, 2);
			}

			if ((race_name == "Orc") ||
			    (race_name == "Troll") ||
			    (race_name == "Dragon") ||
			    (race_name == "Demon"))
			{
				/* Varda hates evil races */
				inc_piety(GOD_ALL, -2);
			} else {
				/* ... and everyone slightly less */
				inc_piety(GOD_ALL, -1);
			}

			/* Prayer uses piety */
			if (p_ptr->praying)
			{
				inc_piety(GOD_ALL, -1);
			}
		}
	}

	if (p_ptr->pgod == GOD_ULMO)
	{
		if (grace_delay_trigger())
		{
			int i;
			/* Ulmo likes the Edain (except Easterlings) */
			if ((race_name == "Human") ||
			    (race_name == "Dunadan") ||
			    (race_name == "Druadan") ||
			    (race_name == "RohanKnight"))
			{
				inc_piety(GOD_ALL, 2);
			}
			else if ((race_name == "Easterling") ||
				 (race_name == "Demon") ||
				 (race_name == "Orc"))
			{
				/* hated races */
				inc_piety(GOD_ALL, -2);
			}
			else
			{
				inc_piety(GOD_ALL, 1);
			}

			if (p_ptr->praying)
			{
				inc_piety(GOD_ALL, -1);
			}

			/* Gain 1 point for each trident in inventory */
			for (i = 0; i < INVEN_TOTAL; i++)
			{
				if ((p_ptr->inventory[i].tval == TV_POLEARM) &&
				    (p_ptr->inventory[i].sval == SV_TRIDENT))
				{
					inc_piety(GOD_ALL, 1);
				}
			}
		}
	}

	if (p_ptr->pgod == GOD_AULE)
	{
		if (grace_delay_trigger())
		{
			int i;

			/* Aule likes Dwarves and Dark Elves (Eol's
			 * influence here) */
			if  (!((race_name == "Dwarf") ||
			       (race_name == "Petty-dwarf") ||
			       (race_name == "Gnome") ||
			       (race_name == "Dark-Elf")))
			{
				inc_piety(GOD_ALL, -1);
			}

			/* Search inventory for axe or hammer - Gain 1
			 * point of grace for each hammer or axe */
			for (i = 0; i < INVEN_TOTAL; i++)
			{
				int tval = p_ptr->inventory[i].tval;
				int sval = p_ptr->inventory[i].sval;

				switch (tval)
				{
				case TV_AXE:
					inc_piety(GOD_ALL, 1);
					break;

				case TV_HAFTED:
					if ((sval == SV_WAR_HAMMER) ||
					    (sval == SV_LUCERN_HAMMER) ||
					    (sval == SV_GREAT_HAMMER))
					{
						inc_piety(GOD_ALL, 1);
					}
					break;
				}
			}

			/* Praying may grant you a free stone skin
			 * once in a while */
			if (p_ptr->praying)
			{
				int chance;
				s32b grace;

				inc_piety(GOD_ALL, -2);
				grace = p_ptr->grace; /* shorthand */

				chance = 1;
				if (grace >= 50000)
				{
					chance = 50000;
				}
				else
				{
					chance = 50000 - grace;
				}

				if (randint(100000) <= 100000 / chance)
				{
					s16b type = 0;

					if (grace >= 10000)
					{
						type = SHIELD_COUNTER;
					}

					set_shield(
						randint(10) + 10 + (grace / 100),
						10 + (grace / 100),
						type,
						2 + (grace / 200),
						3 + (grace / 400));

					msg_print("Aule casts Stone Skin on you.");
				}
			}
		}
	}

	if (p_ptr->pgod == GOD_MANDOS)
	{
		if (grace_delay_trigger())
		{
			/* He loves astral beings  */
			if (subrace_name == "LostSoul")
			{
				inc_piety(GOD_ALL, 1);
			}

			/* He likes High Elves only, though, as races */
			if (!(race_name == "High-Elf"))
			{
				inc_piety(GOD_ALL, -1);
			}

			/* Really hates vampires and demons */
			if ((subrace_name == "Vampire") ||
			    (race_name == "Demon"))
			{
				inc_piety(GOD_ALL, -10);
			}
			else
			{
				inc_piety(GOD_ALL, 2);
			}
			/* he really doesn't like to be disturbed */
			if (p_ptr->praying)
			{
				inc_piety(GOD_ALL, -5);
			}
		}
	}

}

/**
 * Is the light source safe for creatures sensitive to light?
 */
static bool is_light_safe(object_type const *o_ptr)
{
	// Get the flags of the object; we don't bother with sets since we're only
	// interested in "innate" flags on the light source itself.
	object_flags_no_set = true;
	auto flags = object_flags(o_ptr);
	object_flags_no_set = false;

	// We only allow really badly cursed items. This is to provide
	// an "out" in case of severely bad luck, but these lights cannot
	// be used for any non-trivial length of time.
	return (flags | TR_TY_CURSE) && (flags | TR_DG_CURSE);
}


/**
 * Process lasting effects
 */
static void process_lasting_effects()
{
	auto &lasting_effects = game->lasting_effects;
	size_t max_lasting_effects = std::size(game->lasting_effects);

	for (int j = 0; j < cur_hgt - 1; j++)
	{
		for (int i = 0; i < cur_wid - 1; i++)
		{
			if (auto ei = cave[j][i].maybe_effect)
			{
				effect_type *e_ptr = &lasting_effects[*ei];

				if (e_ptr->time)
				{
					/* Apply damage */
					project(0, 0, j, i, e_ptr->dam, e_ptr->type,
						PROJECT_KILL | PROJECT_ITEM | PROJECT_HIDE);
				}
				else
				{
					cave[j][i].maybe_effect = boost::none;
				}

				if ((e_ptr->flags & EFF_WAVE) && !(e_ptr->flags & EFF_LAST))
				{
					if (distance(e_ptr->cy, e_ptr->cx, j, i) < e_ptr->rad - 1)
					{
						cave[j][i].maybe_effect = boost::none;
					}
				}
				else if ((e_ptr->flags & EFF_STORM) && !(e_ptr->flags & EFF_LAST))
				{
					cave[j][i].maybe_effect = boost::none;
				}

				lite_spot(j, i);
			}
		}
	}

	// Reduce & handle effects
	for (std::size_t i = 0; i < max_lasting_effects; i++)
	{
		effect_type &e = lasting_effects[i];

		if (e.time > 0)
		{
			// Reduce duration
			e.time--;

			// Set up the effect in the dungeon.
			if (e.flags & EFF_WAVE)
			{
				// Expand the wave front
				e.rad++;

				// Check direction
				if (e.flags & EFF_DIR8)
				{
					for (int y = e.cy - e.rad, z = 0; y <= e.cy; y++, z++)
					{
						for (int x = e.cx - (e.rad - z); x <= e.cx + (e.rad - z); x++)
						{
							if (!in_bounds(y, x))
							{
								continue;
							}

							if (los(e.cy, e.cx, y, x) &&
									(distance(e.cy, e.cx, y, x) == e.rad))
							{
								cave[y][x].maybe_effect = i;
							}
						}
					}
				}
				else if (e.flags & EFF_DIR2)
				{
					for (int y = e.cy, z = e.rad; y <= e.cy + e.rad; y++, z--)
					{
						for (int x = e.cx - (e.rad - z); x <= e.cx + (e.rad - z); x++)
						{
							if (!in_bounds(y, x))
							{
								continue;
							}

							if (los(e.cy, e.cx, y, x) &&
									(distance(e.cy, e.cx, y, x) == e.rad))
							{
								cave[y][x].maybe_effect = i;
							}
						}
					}
				}
				else if (e.flags & EFF_DIR6)
				{
					for (int x = e.cx, z = e.rad; x <= e.cx + e.rad; x++, z--)
					{
						for (int y = e.cy - (e.rad - z); y <= e.cy + (e.rad - z); y++)
						{
							if (!in_bounds(y, x))
							{
								continue;
							}

							if (los(e.cy, e.cx, y, x) &&
									(distance(e.cy, e.cx, y, x) == e.rad))
							{
								cave[y][x].maybe_effect = i;
							}
						}
					}
				}
				else if (e.flags & EFF_DIR4)
				{
					for (int x = e.cx - e.rad, z = 0; x <= e.cx; x++, z++)
					{
						for (int y = e.cy - (e.rad - z); y <= e.cy + (e.rad - z); y++)
						{
							if (!in_bounds(y, x))
							{
								continue;
							}

							if (los(e.cy, e.cx, y, x) &&
									(distance(e.cy, e.cx, y, x) == e.rad))
							{
								cave[y][x].maybe_effect = i;
							}
						}
					}
				}
				else if (e.flags & EFF_DIR9)
				{
					for (int y = e.cy - e.rad; y <= e.cy; y++)
					{
						for (int x = e.cx; x <= e.cx + e.rad; x++)
						{
							if (!in_bounds(y, x))
							{
								continue;
							}

							if (los(e.cy, e.cx, y, x) &&
									(distance(e.cy, e.cx, y, x) == e.rad))
							{
								cave[y][x].maybe_effect = i;
							}
						}
					}
				}
				else if (e.flags & EFF_DIR1)
				{
					for (int y = e.cy; y <= e.cy + e.rad; y++)
					{
						for (int x = e.cx - e.rad; x <= e.cx; x++)
						{
							if (!in_bounds(y, x))
							{
								continue;
							}

							if (los(e.cy, e.cx, y, x) &&
									(distance(e.cy, e.cx, y, x) == e.rad))
							{
								cave[y][x].maybe_effect = i;
							}
						}
					}
				}
				else if (e.flags & EFF_DIR7)
				{
					for (int y = e.cy - e.rad; y <= e.cy; y++)
					{
						for (int x = e.cx - e.rad; x <= e.cx; x++)
						{
							if (!in_bounds(y, x))
							{
								continue;
							}

							if (los(e.cy, e.cx, y, x) &&
									(distance(e.cy, e.cx, y, x) == e.rad))
							{
								cave[y][x].maybe_effect = i;
							}
						}
					}
				}
				else if (e.flags & EFF_DIR3)
				{
					for (int y = e.cy; y <= e.cy + e.rad; y++)
					{
						for (int x = e.cx; x <= e.cx + e.rad; x++)
						{
							if (!in_bounds(y, x))
							{
								continue;
							}

							if (los(e.cy, e.cx, y, x) &&
									(distance(e.cy, e.cx, y, x) == e.rad))
							{
								cave[y][x].maybe_effect = i;
							}
						}
					}
				}
				else
				{
					for (int y = e.cy - e.rad; y <= e.cy + e.rad; y++)
					{
						for (int x = e.cx - e.rad; x <= e.cx + e.rad; x++)
						{
							if (!in_bounds(y, x))
							{
								continue;
							}

							if (los(e.cy, e.cx, y, x) &&
									(distance(e.cy, e.cx, y, x) == e.rad))
							{
								cave[y][x].maybe_effect = i;
							}
						}
					}
				}
			}
			else if (e.flags & EFF_STORM)
			{
				// Center on player
				e.cy = p_ptr->py;
				e.cx = p_ptr->px;
				// Set up the effect
				for (int y = e.cy - e.rad; y <= e.cy + e.rad; y++)
				{
					for (int x = e.cx - e.rad; x <= e.cx + e.rad; x++)
					{
						if (!in_bounds(y, x))
						{
							continue;
						}

						if (los(e.cy, e.cx, y, x) &&
								(distance(e.cy, e.cx, y, x) <= e.rad))
						{
							cave[y][x].maybe_effect = i;
							lite_spot(y, x);
						}
					}
				}
			}
		}
	}

	apply_effect(p_ptr->py, p_ptr->px);
}


/*
 * Handle certain things once every 10 game turns
 *
 * Note that a single movement in the overhead wilderness mode
 * consumes 132 times as much energy as a normal one...
 */
static void process_world()
{
	auto const &d_info = game->edit_data.d_info;
	auto const &r_info = game->edit_data.r_info;
	auto const &f_info = game->edit_data.f_info;
	auto &timers = game->timers;
	auto const &dungeon_flags = game->dungeon_flags;

	int x, y;

	int regen_amount;
	bool cave_no_regen = false;
	int upkeep_factor = 0;

	auto d_ptr = &d_info[dungeon_type];

	cave_type *c_ptr;

	object_type *o_ptr;

	/*
	 * Every 10 game turns -- which means this section is invoked once
	 * in a player turn under the normal speed, and 132 times in a move
	 * in the reduced map mode.
	 */
	if (turn % 10) return;

	/*
	 * I don't know if this is the right thing to do because I'm totally
	 * ignorant (yes, I must admit that...) about the scripting part of
	 * the game, but since there have been complaints telling us it
	 * runs terribly slow in the reduced map mode... -- pelpel
	 *
	 * Note to coders: if it is desirable to make this active in the
	 * reduced map mode, remove the if condition surrounding the line
	 * and move the code inside into every 1000 game turns section.
	 */
	if (dun_level || (!p_ptr->wild_mode))
	{
		/* Handle corruptions */
		process_world_corruptions();

		/* Handle gods */
		process_world_gods();

		/* Handle the player song */
		check_music();
	}

	/* Process timers */
	for (auto &&timer: timers)
	{
		timer->count_down();
	}

	/* Check the fate */
	if (options->fate_option && (p_ptr->lev > 10))
	{
		/*
		 * WAS: == 666 against randint(50000).
		 * Since CPU's don't know Judeo-Christian / Cabalistic traditions,
		 * and since comparisons with zero is more efficient in many
		 * architectures...
		 */
		if (rand_int(50000) == 0) gain_fate(0);
	}

	/*** Is the wielded monsters still hypnotised ***/
	o_ptr = &p_ptr->inventory[INVEN_CARRY];

	if (o_ptr->k_ptr)
	{
		auto r_ptr = &r_info[o_ptr->pval];

		if ((randint(1000) < r_ptr->level - ((p_ptr->lev * 2) + get_skill(SKILL_SYMBIOTIC))))
		{
			msg_format("%s breaks free from hypnosis!",
			           symbiote_name(true).c_str());
			carried_make_attack_normal(o_ptr->pval);
		}
	}

	/*** Attempt timed autosave ***/
	if (options->autosave_t && options->autosave_freq)
	{
		if ((turn % (static_cast<s32b>(options->autosave_freq) * 10)) == 0)
		{
			is_autosave = true;
			msg_print("Autosaving the game...");
			do_cmd_save_game();
			is_autosave = false;
		}
	}


	/*** Handle the wilderness/town (sunshine) ***/

	/* While in town/wilderness and not in the overworld mode */
	if (!dun_level && !p_ptr->wild_mode)
	{
		/* Hack -- Daybreak/Nighfall in town */
		if ((turn % ((10L * DAY) / 2)) == 0)
		{
			/* Day breaks */
			if (((turn % (10L * DAY)) == 0))
			{
				/* Message */
				msg_print("The sun has risen.");

				/* Hack -- Scan the town */
				for (y = 0; y < cur_hgt; y++)
				{
					for (x = 0; x < cur_wid; x++)
					{
						/* Get the cave grid */
						c_ptr = &cave[y][x];

						/* Assume lit */
						c_ptr->info |= CAVE_GLOW;

						/* Hack -- Memorize lit grids if allowed */
						if (options->view_perma_grids)
						{
							c_ptr->info |= CAVE_MARK;
						}

						/* Hack -- Notice spot */
						note_spot(y, x);
					}
				}
			}

			/* Night falls */
			else
			{
				/* Message */
				msg_print("The sun has set.");

				/* Hack -- Scan the town */
				for (y = 0; y < cur_hgt; y++)
				{
					for (x = 0; x < cur_wid; x++)
					{
						/* Get the cave grid */
						c_ptr = &cave[y][x];

						/* Darken "boring" features */
						if (cave_plain_floor_grid(c_ptr))
						{
							/* Forget the grid */
							c_ptr->info &= ~(CAVE_GLOW | CAVE_MARK);

							/* Hack -- Notice spot */
							note_spot(y, x);
						}
					}
				}
			}

			/* Update the monsters */
			p_ptr->update |= (PU_MONSTERS);

			/* Redraw map */
			p_ptr->redraw |= (PR_MAP);

			/* Window stuff */
			p_ptr->window |= (PW_OVERHEAD);
		}
	}

	/*** Process the monsters ***/

	/* Check for creature generation. */
	if (!p_ptr->wild_mode &&
	                !p_ptr->inside_quest &&
	                (rand_int(d_info[(dun_level) ? dungeon_type : DUNGEON_WILDERNESS].max_m_alloc_chance) == 0))
	{
		/* Make a new monster */
		if (!(dungeon_flags & DF_NO_NEW_MONSTER))
		{
			alloc_monster(MAX_SIGHT + 5, false);
		}
	}

	/* Hack -- Check for creature regeneration */
	if (!p_ptr->wild_mode && ((turn % 100) == 0)) regen_monsters();


	/*** Damage over Time ***/

	/* Take damage from poison */
	if (p_ptr->poisoned && !p_ptr->invuln)
	{
		/* Take damage */
		take_hit(1, "poison");
	}


	/* Vampires take damage from sunlight */
	if (p_ptr->sensible_lite)
	{
		if ((!dun_level) && (((turn / ((10L * DAY) / 2)) % 2) == 0))
		{
			if (cave[p_ptr->py][p_ptr->px].info & (CAVE_GLOW))
			{
				/* Take damage */
				msg_print("The sun's rays scorch your undead flesh!");
				take_hit(1, "sunlight");
				cave_no_regen = true;
				drop_from_wild();
			}
		}

		if (!is_light_safe(&p_ptr->inventory[INVEN_LITE]))
		{
			object_type * o_ptr = &p_ptr->inventory[INVEN_LITE];
			char o_name [80];
			char ouch [80];

			/* Get an object description */
			object_desc(o_name, o_ptr, false, 0);

			msg_format("The %s scorches your undead flesh!", o_name);

			cave_no_regen = true;

			/* Get an object description */
			object_desc(o_name, o_ptr, true, 0);

			sprintf(ouch, "wielding %s", o_name);
			take_hit(1, ouch);
		}
	}

	/* Drown in deep water unless the player have levitation, water walking
	   water breathing, or magic breathing.*/
	if (!p_ptr->ffall && !p_ptr->magical_breath &&
	                !p_ptr->water_breath &&
	                (cave[p_ptr->py][p_ptr->px].feat == FEAT_DEEP_WATER))
	{
		if (calc_total_weight() > ((weight_limit()) / 2))
		{
			/* Take damage */
			msg_print("You are drowning!");
			take_hit(randint(p_ptr->lev), "drowning");
			cave_no_regen = true;
		}
	}


	/* Spectres -- take damage when moving through walls */

	/*
	 * Added: ANYBODY takes damage if inside through walls
	 * without wraith form -- NOTE: Spectres will never be
	 * reduced below 0 hp by being inside a stone wall; others
	 * WILL BE!
	 */
	if (!cave_floor_bold(p_ptr->py, p_ptr->px))
	{
		int feature = cave[p_ptr->py][p_ptr->px].feat;

		/* Player can walk through or fly over trees */
		if ((p_ptr->has_ability(AB_TREE_WALK) || p_ptr->fly) && (feature == FEAT_TREES))
		{
			/* Do nothing */
		}
		/* Player can climb over mountains */
		else if ((p_ptr->climb) && (f_info[feature].flags & FF_CAN_CLIMB))
		{
			/* Do nothing */
		}
		else if (race_flags_p(PR_SEMI_WRAITH) && (!p_ptr->wraith_form) && (f_info[cave[p_ptr->py][p_ptr->px].feat].flags & FF_CAN_PASS))
		{
			int amt = 1 + ((p_ptr->lev) / 5);

			cave_no_regen = true;
			if (amt > p_ptr->chp - 1) amt = p_ptr->chp - 1;
			take_hit(amt, " walls ...");
		}
	}


	/* Take damage from cuts */
	if ((p_ptr->cut) && !(p_ptr->invuln))
	{
		int i;

		/* Mortal wound or Deep Gash */
		if (p_ptr->cut > 200)
		{
			i = 3;
		}

		/* Severe cut */
		else if (p_ptr->cut > 100)
		{
			i = 2;
		}

		/* Other cuts */
		else
		{
			i = 1;
		}

		/* Take damage */
		take_hit(i, "a fatal wound");
	}


	/*** Check the Food, and Regenerate ***/

	/* Digest normally */
	if (p_ptr->food < PY_FOOD_MAX)
	{
		/* Every 100 game turns */
		if ((turn % 100) == 0)
		{
			int speed_use = p_ptr->pspeed;

			/* Maximum */
			if (speed_use > 199)
			{
				speed_use = 199;
			}

			/* Minimum */
			else if (speed_use < 0)
			{
				speed_use = 0;
			}

			/* Basic digestion rate based on speed */
			int i = extract_energy[speed_use] * 2;

			/* Regeneration takes more food */
			if (p_ptr->regenerate) i += 30;

			/* Regeneration takes more food */
			if (p_ptr->tim_regen) i += p_ptr->tim_regen_pow / 10;

			/* Invisibility consume a lot of food */
			i += p_ptr->invis / 2;

			/* Invulnerability consume a lot of food */
			if (p_ptr->invuln) i += 40;

			/* Wraith Form consume a lot of food */
			if (p_ptr->wraith_form) i += 30;

			/* Get the weapon */
			o_ptr = &p_ptr->inventory[INVEN_WIELD];

			/* Examine the sword */
			auto const flags = object_flags(o_ptr);

			/* Hitpoints multiplier consume a lot of food */
			if (o_ptr->k_ptr && (flags & TR_LIFE))
			{
				i += o_ptr->pval * 5;
			}

			/* Slow digestion takes less food */
			if (p_ptr->slow_digest) i -= 10;

			/* Minimal digestion */
			if (i < 1) i = 1;

			/* Digest some food */
			set_food(p_ptr->food - i);
		}
	}

	/* Digest quickly when gorged */
	else
	{
		/* Digest a lot of food */
		set_food(p_ptr->food - 100);
	}

	/* Starve to death (slowly) */
	if (p_ptr->food < PY_FOOD_STARVE)
	{
		/* Calculate damage */
		int i = (PY_FOOD_STARVE - p_ptr->food) / 10;

		/* Take damage */
		if (!(p_ptr->invuln)) take_hit(i, "starvation");
	}

	/* Default regeneration */
	regen_amount = PY_REGEN_NORMAL;

	/* Getting Weak */
	if (p_ptr->food < PY_FOOD_WEAK)
	{
		/* Lower regeneration */
		if (p_ptr->food < PY_FOOD_STARVE)
		{
			regen_amount = 0;
		}
		else if (p_ptr->food < PY_FOOD_FAINT)
		{
			regen_amount = PY_REGEN_FAINT;
		}
		else
		{
			regen_amount = PY_REGEN_WEAK;
		}

		/* Getting Faint */
		if (p_ptr->food < PY_FOOD_FAINT)
		{
			/* Faint occasionally */
			if (!p_ptr->paralyzed && (rand_int(100) < 10))
			{
				/* Message */
				msg_print("You faint from the lack of food.");
				disturb();

				/* Hack -- faint (bypass free action) */
				set_paralyzed(1 + rand_int(5));
			}
		}
	}

	/* Regeneration ability */
	if (p_ptr->regenerate)
	{
		regen_amount = regen_amount * 2;
	}


	/* Resting boosts regeneration */
	if (resting)
	{
		regen_amount = regen_amount * 2;
	}

	if (total_friends)
	{
		int upkeep_divider = 20;

		if (p_ptr->has_ability(AB_PERFECT_CASTING))
			upkeep_divider = 15;

		if (total_friends > 1 + (p_ptr->lev / (upkeep_divider)))
		{
			upkeep_factor = (total_friend_levels);

			if (upkeep_factor > 100) upkeep_factor = 100;
			else if (upkeep_factor < 10) upkeep_factor = 10;
		}
	}

	/* Regenerate the mana */
	if (p_ptr->csp < p_ptr->msp)
	{
		if (upkeep_factor)
		{
			s16b upkeep_regen = (((100 - upkeep_factor) * regen_amount) / 100);
			regenmana(upkeep_regen);
		}
		else
		{
			regenmana(regen_amount);
		}
	}

	/* Eru piety incraese with time */
	if (((turn % 100) == 0) && (!p_ptr->did_nothing) && (!p_ptr->wild_mode))
	{
		if ((p_ptr->pgod == GOD_ERU) && !p_ptr->praying)
		{
			int inc = wisdom_scale(10);

			/* Increase by wisdom/4 */
			if (!inc) inc = 1;
			inc_piety(GOD_ERU, inc);
		}
	}
	/* Most gods piety decrease with time */
	if (((turn % 300) == 0) && (!p_ptr->did_nothing) && (!p_ptr->wild_mode) && (dun_level))
	{
		if (p_ptr->pgod == GOD_MANWE)
		{
			int dec = 4 - wisdom_scale(3);

			if (p_ptr->praying)
			{
				dec++;
			}

			if (race_flags_p(PR_ELF))
			{
				dec -= wisdom_scale(2);
			}

			dec = std::max(1, dec);

			inc_piety(GOD_MANWE, -dec);
		}

		if (p_ptr->pgod == GOD_MELKOR)
		{
			int dec = 8 - wisdom_scale(6);

			if (p_ptr->praying)
			{
				dec++;
			}

			if (race_flags_p(PR_ELF))
			{
				dec += 5 - wisdom_scale(4);
			}

			dec = std::max(1, dec);

			inc_piety(GOD_MELKOR, -dec);
		}

		if (praying_to(GOD_TULKAS))
		{
			int dec = std::max(1, 4 - wisdom_scale(3));

			inc_piety(GOD_TULKAS, -dec);
		}
	}
	/* Yavanna piety decrease with time */
	if (((turn % 400) == 0) && (!p_ptr->did_nothing) && (!p_ptr->wild_mode) && (dun_level))
	{
		if (p_ptr->pgod == GOD_YAVANNA)
		{
			int dec = 5 - wisdom_scale(3);

			/* Blech what an hideous hack */
			if (!(rp_ptr->title == "Ent"))
			{
				dec -= wisdom_scale(2);
			}

			dec = std::max(1, dec);
			inc_piety(GOD_YAVANNA, -dec);
		}
	}
	p_ptr->did_nothing = false;

	/* Increase regen by tim regen */
	if (p_ptr->tim_regen) regen_amount += p_ptr->tim_regen_pow;

	/* Poisoned or cut yields no healing */
	if (p_ptr->poisoned) regen_amount = 0;
	if (p_ptr->cut) regen_amount = 0;

	/* Special floor -- Pattern, in a wall -- yields no healing */
	if (cave_no_regen) regen_amount = 0;

	/* Being over grass allows Yavanna to regen you */
	if (praying_to(GOD_YAVANNA))
	{
		if (cave[p_ptr->py][p_ptr->px].feat == FEAT_GRASS)
		{
			regen_amount += 200 + wisdom_scale(800);
		}
	}

	/* Regenerate Hit Points if needed */
	if ((p_ptr->chp < p_ptr->mhp) && !cave_no_regen)
	{
		regenhp(regen_amount);
	}


	/*** Timeout Various Things ***/

	/* Handle temporary stat drains */
	for (int i = 0; i < 6; i++)
	{
		if (p_ptr->stat_cnt[i] > 0)
		{
			p_ptr->stat_cnt[i]--;
			if (p_ptr->stat_cnt[i] == 0)
			{
				do_res_stat(i, false);
			}
		}
	}

	/* Hack -- Hallucinating */
	if (p_ptr->image)
	{
		set_image(p_ptr->image - 1);
	}

	/* Holy Aura */
	if (p_ptr->holy)
	{
		set_holy(p_ptr->holy - 1);
	}

	/* Soul absorbtion */
	if (p_ptr->absorb_soul)
	{
		set_absorb_soul(p_ptr->absorb_soul - 1);
	}

	/* Undead loose Death Points */
	if (p_ptr->necro_extra & CLASS_UNDEAD)
	{
		int old_chp = p_ptr->chp;
		int warning = (p_ptr->mhp * options->hitpoint_warn / 10);

		/* Bypass invulnerability and wraithform */
		p_ptr->chp--;

		/* Display the hitpoints */
		p_ptr->redraw |= (PR_FRAME);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);

		/* Dead player */
		if (p_ptr->chp < 0)
		{
			bool old_quick = options->quick_messages;

			/* Hack -- Note death */
			if (!options->last_words)
			{
				msg_print("You die.");
				msg_print(NULL);
			}
			else
			{
				char death_message[80];

				get_rnd_line("death.txt", death_message);
				msg_print(death_message);
			}

			/* Note cause of death; hallucinating characters don't get to know. */
			game->died_from = "being undead too long";
			if (p_ptr->image)
			{
				game->died_from = "(?)";
			}

			/* No longer a winner */
			total_winner = false;

			/* Leaving */
			p_ptr->leaving = true;

			/* Note death */
			death = true;

			options->quick_messages = false;
			if (get_check("Make a last screenshot? "))
			{
				do_cmd_html_dump();
			}
			options->quick_messages = old_quick;

			/* Dead */
			return;
		}

		/* Hitpoint warning */
		if (p_ptr->chp < warning)
		{
			/* Hack -- bell on first notice */
			if (old_chp > warning)
			{
				bell();
			}

			/* Message */
			msg_print("*** LOW DEATHPOINT WARNING! ***");
			msg_print(NULL);
		}
	}

	/* True Strike */
	if (p_ptr->strike)
	{
		set_strike(p_ptr->strike - 1);
	}

	/* Timed project */
	if (p_ptr->tim_project)
	{
		set_project(p_ptr->tim_project - 1, p_ptr->tim_project_gf, p_ptr->tim_project_dam, p_ptr->tim_project_rad, p_ptr->tim_project_flag);
	}

	/* Timed roots */
	if (p_ptr->tim_roots)
	{
		set_roots(p_ptr->tim_roots - 1, p_ptr->tim_roots_ac, p_ptr->tim_roots_dam);
	}

	/* Timed breath */
	if (p_ptr->tim_water_breath)
	{
		set_tim_breath(p_ptr->tim_water_breath - 1, false);
	}
	if (p_ptr->tim_magic_breath)
	{
		set_tim_breath(p_ptr->tim_magic_breath - 1, true);
	}

	/* Timed precognition */
	if (p_ptr->tim_precognition > 0)
	{
		set_tim_precognition(p_ptr->tim_precognition - 1);
	}

	/* Timed regen */
	if (p_ptr->tim_regen)
	{
		set_tim_regen(p_ptr->tim_regen - 1, p_ptr->tim_regen_pow);
	}

	/* Timed Disrupt shield */
	if (p_ptr->disrupt_shield)
	{
		set_disrupt_shield(p_ptr->disrupt_shield - 1);
	}

	/* Timed Parasite */
	if (p_ptr->parasite)
	{
		set_parasite(p_ptr->parasite - 1, p_ptr->parasite_r_idx);
	}

	/* Timed Reflection */
	if (p_ptr->tim_reflect)
	{
		set_tim_reflect(p_ptr->tim_reflect - 1);
	}

	/* Timed Prob Travel */
	if (p_ptr->prob_travel)
	{
		set_prob_travel(p_ptr->prob_travel - 1);
	}

	/* Timed Levitation */
	if (p_ptr->tim_ffall)
	{
		set_tim_ffall(p_ptr->tim_ffall - 1);
	}
	if (p_ptr->tim_fly)
	{
		set_tim_fly(p_ptr->tim_fly - 1);
	}

	/* Thunderstorm */
	if (p_ptr->tim_thunder)
	{
		int dam = damroll(p_ptr->tim_thunder_p1, p_ptr->tim_thunder_p2);
		int i, tries = 600;
		monster_type *m_ptr = NULL;

		while (tries)
		{
			/* Access the monster */
			m_ptr = &m_list[i = rand_range(1, m_max - 1)];

			tries--;

			/* Ignore "dead" monsters */
			if (!m_ptr->r_idx) continue;

			/* Cant see ? cant hit */
			if (!los(p_ptr->py, p_ptr->px, m_ptr->fy, m_ptr->fx)) continue;

			/* Do not hurt friends! */
			if (is_friend(m_ptr) >= 0) continue;
			break;
		}

		if (tries)
		{
			char m_name[80];

			monster_desc(m_name, m_ptr, 0);
			msg_format("Lightning strikes %s.", m_name);
			project(0, 0, m_ptr->fy, m_ptr->fx, dam / 3, GF_ELEC,
			        PROJECT_KILL | PROJECT_ITEM | PROJECT_HIDE);
			project(0, 0, m_ptr->fy, m_ptr->fx, dam / 3, GF_LITE,
			        PROJECT_KILL | PROJECT_ITEM | PROJECT_HIDE);
			project(0, 0, m_ptr->fy, m_ptr->fx, dam / 3, GF_SOUND,
			        PROJECT_KILL | PROJECT_ITEM | PROJECT_HIDE);
		}

		set_tim_thunder(p_ptr->tim_thunder - 1, p_ptr->tim_thunder_p1, p_ptr->tim_thunder_p2);
	}

	/* Poisonned hands */
	if (p_ptr->tim_poison)
	{
		set_poison(p_ptr->tim_poison - 1);
	}

	/* Brightness */
	if (p_ptr->tim_lite)
	{
		set_lite(p_ptr->tim_lite - 1);
	}

	/* Blindness */
	if (p_ptr->blind)
	{
		set_blind(p_ptr->blind - 1);
	}

	/* Timed no_breeds */
	if (no_breeds)
	{
		set_no_breeders(no_breeds - 1);
	}

	/* Timed mimic */
	if (p_ptr->tim_mimic)
	{
		set_mimic(p_ptr->tim_mimic - 1, p_ptr->mimic_form, p_ptr->mimic_level);
	}

	/* Timed special move commands */
	if (p_ptr->immov_cntr)
	{
		p_ptr->immov_cntr--;
	}

	/* Timed invisibility */
	if (p_ptr->tim_invisible)
	{
		set_invis(p_ptr->tim_invisible - 1, p_ptr->tim_inv_pow);
	}

	/* Times see-invisible */
	if (p_ptr->tim_invis)
	{
		set_tim_invis(p_ptr->tim_invis - 1);
	}

	/* Timed esp */
	if (p_ptr->tim_esp)
	{
		set_tim_esp(p_ptr->tim_esp - 1);
	}

	/* Timed infra-vision */
	if (p_ptr->tim_infra)
	{
		set_tim_infra(p_ptr->tim_infra - 1);
	}

	/* Paralysis */
	if (p_ptr->paralyzed)
	{
		dec_paralyzed();
	}

	/* Confusion */
	if (p_ptr->confused)
	{
		set_confused(p_ptr->confused - 1);
	}

	/* Afraid */
	if (p_ptr->afraid)
	{
		set_afraid(p_ptr->afraid - 1);
	}

	/* Fast */
	if (p_ptr->fast)
	{
		set_fast(p_ptr->fast - 1, p_ptr->speed_factor);
	}

	/* Light speed */
	if (p_ptr->lightspeed)
	{
		set_light_speed(p_ptr->lightspeed - 1);
	}

	/* Slow */
	if (p_ptr->slow)
	{
		set_slow(p_ptr->slow - 1);
	}

	/* Protection from evil */
	if (p_ptr->protevil)
	{
		set_protevil(p_ptr->protevil - 1);
	}

	/* Invulnerability */
	if (p_ptr->invuln)
	{
		set_invuln(p_ptr->invuln - 1);
	}

	/* Wraith form */
	if (p_ptr->tim_wraith)
	{
		set_shadow(p_ptr->tim_wraith - 1);
	}

	/* Heroism */
	if (p_ptr->hero)
	{
		set_hero(p_ptr->hero - 1);
	}

	/* Super Heroism */
	if (p_ptr->shero)
	{
		set_shero(p_ptr->shero - 1);
	}

	/* Blessed */
	if (p_ptr->blessed)
	{
		set_blessed(p_ptr->blessed - 1);
	}

	/* Shield */
	if (p_ptr->shield)
	{
		set_shield(p_ptr->shield - 1, p_ptr->shield_power, p_ptr->shield_opt, p_ptr->shield_power_opt, p_ptr->shield_power_opt2);
	}

	/* Oppose Acid */
	if (p_ptr->oppose_acid)
	{
		set_oppose_acid(p_ptr->oppose_acid - 1);
	}

	/* Oppose Lightning */
	if (p_ptr->oppose_elec)
	{
		set_oppose_elec(p_ptr->oppose_elec - 1);
	}

	/* Oppose Fire */
	if (p_ptr->oppose_fire)
	{
		set_oppose_fire(p_ptr->oppose_fire - 1);
	}

	/* Oppose Cold */
	if (p_ptr->oppose_cold)
	{
		set_oppose_cold(p_ptr->oppose_cold - 1);
	}

	/* Oppose Poison */
	if (p_ptr->oppose_pois)
	{
		set_oppose_pois(p_ptr->oppose_pois - 1);
	}

	/* Oppose Chaos & Confusion */
	if (p_ptr->oppose_cc)
	{
		set_oppose_cc(p_ptr->oppose_cc - 1);
	}

	/* Timed mimicry */
	if (get_skill(SKILL_MIMICRY))
	{
		/* Extract the value and the flags */
		u32b value = p_ptr->mimic_extra >> 16;

		u32b att = p_ptr->mimic_extra & 0xFFFF;

		if ((att & CLASS_LEGS) || (att & CLASS_WALL) || (att & CLASS_ARMS))
		{
			value--;

			if (!value)
			{
				if (att & CLASS_LEGS) msg_print("You lose your extra pair of legs.");
				if (att & CLASS_ARMS) msg_print("You lose your extra pair of arms.");
				if (att & CLASS_WALL) msg_print("You lose your affinity for walls.");

				att &= ~(CLASS_ARMS);
				att &= ~(CLASS_LEGS);
				att &= ~(CLASS_WALL);

				disturb_on_state();
			}

			p_ptr->update |= (PU_BODY);
			p_ptr->mimic_extra = att + (value << 16);
		}
	}


	/*** Poison and Stun and Cut ***/

	/* Poison */
	if (p_ptr->poisoned)
	{
		int adjust = (adj_con_fix[p_ptr->stat_ind[A_CON]] + 1);

		/* Apply some healing */
		set_poisoned(p_ptr->poisoned - adjust);
	}

	/* Stun */
	if (p_ptr->stun)
	{
		int adjust = (adj_con_fix[p_ptr->stat_ind[A_CON]] + 1);

		/* Apply some healing */
		set_stun(p_ptr->stun - adjust);
	}

	/* Cut */
	if (p_ptr->cut)
	{
		int adjust = (adj_con_fix[p_ptr->stat_ind[A_CON]] + 1);

		/* Hack -- Truly "mortal" wound */
		if (p_ptr->cut > 1000) adjust = 0;

		/* Apply some healing */
		set_cut(p_ptr->cut - adjust);
	}

	/* Hack - damage done by the dungeon -SC- */
	if ((dun_level != 0) && (d_ptr->d_frequency[0] != 0))
	{
		int i, j, k;

		/* Apply damage to every grid in the dungeon */
		for (i = 0; i < 4; i++)
		{
			/* Check the frequency */
			if (d_ptr->d_frequency[i] == 0) continue;

			if (((turn % d_ptr->d_frequency[i]) == 0) &&
			                ((d_ptr->d_side[i] != 0) || (d_ptr->d_dice[i] != 0)))
			{
				for (j = 0; j < cur_hgt - 1; j++)
				{
					for (k = 0; k < cur_wid - 1; k++)
					{
						int l, dam = 0;

						if (!(dungeon_flags & DF_DAMAGE_FEAT))
						{
							/* If the grid is empty, skip it */
							if ((cave[j][k].o_idxs.empty()) &&
							                ((j != p_ptr->py) && (i != p_ptr->px))) continue;
						}

						/* Let's not hurt poor monsters */
						if (cave[j][k].m_idx) continue;

						/* Roll damage */
						for (l = 0; l < d_ptr->d_dice[i]; l++)
						{
							dam += randint(d_ptr->d_side[i]);
						}

						/* Apply damage */
						project( -100, 0, j, k, dam, d_ptr->d_type[i],
						         PROJECT_KILL | PROJECT_ITEM | PROJECT_HIDE);
					}
				}
			}
		}
	}

	/* handle spell effects */
	if (!p_ptr->wild_mode)
	{
		process_lasting_effects();
	}

	/* Arg cannot breath? */
	if ((dungeon_flags & DF_WATER_BREATH) && (!p_ptr->water_breath))
	{
		cmsg_print(TERM_L_RED, "You cannot breathe water!  You suffocate!");
		take_hit(damroll(3, p_ptr->lev), "suffocating");
	}
	if ((dungeon_flags & DF_NO_BREATH) && (!p_ptr->magical_breath))
	{
		cmsg_print(TERM_L_RED, "There is no air here!  You suffocate!");
		take_hit(damroll(3, p_ptr->lev), "suffocating");
	}

	/*
	 * Every 1500 turns, warn about any Black Breath not gotten from
	 * an equipped object, and stop any resting. -LM-
	 *
	 * It's apparent that someone has halved the frequency... -- pelpel
	 */
	if (((turn % 3000) == 0) && p_ptr->black_breath)
	{
		bool be_silent = false;

		/* check all equipment for the Black Breath flag. */
		for (int i = INVEN_WIELD; i < INVEN_TOTAL; i++)
		{
			o_ptr = &p_ptr->inventory[i];

			/* Skip non-objects */
			if (!o_ptr->k_ptr)
			{
				continue;
			}

			/* Extract the item flags */
			auto const flags = object_flags(o_ptr);

			/* No messages if object has the flag, to avoid annoyance. */
			if (flags & TR_BLACK_BREATH)
			{
				be_silent = true;
			}

		}

		/* If we are allowed to speak, warn and disturb. */
		if (!be_silent)
		{
			cmsg_print(TERM_L_DARK, "The Black Breath saps your soul!");
			disturb();
		}
	}


	/*** Process Light ***/

	/* Check for light being wielded */
	o_ptr = &p_ptr->inventory[INVEN_LITE];

	/* Burn some fuel in the current lite */
	if (o_ptr->tval == TV_LITE)
	{
		/* Extract the item flags */
		auto const flags = object_flags(o_ptr);

		/* Hack -- Use some fuel */
		if ((flags & TR_FUEL_LITE) && (o_ptr->timeout > 0))
		{
			/* Decrease life-span */
			o_ptr->timeout--;

			/* Hack -- notice interesting fuel steps */
			if ((o_ptr->timeout < 100) || ((o_ptr->timeout % 100) == 0))
			{
				/* Window stuff */
				p_ptr->window |= (PW_EQUIP);
			}

			/* Hack -- Special treatment when blind */
			if (p_ptr->blind)
			{
				/* Hack -- save some light for later */
				if (o_ptr->timeout == 0) o_ptr->timeout++;
			}

			/* The light is now out */
			else if (o_ptr->timeout < 1)
			{
				disturb();
				cmsg_print(TERM_YELLOW, "Your light has gone out!");
			}

			/* The light is getting dim */
			else if ((o_ptr->timeout < 100) && (o_ptr->timeout % 10 == 0))
			{
				if (options->disturb_minor)
				{
					disturb();
				}
				cmsg_print(TERM_YELLOW, "Your light is growing faint.");
			}
		}
	}

	/* Calculate torch radius */
	p_ptr->update |= (PU_TORCH);


	/*** Process Inventory ***/

	/*
	 * Handle experience draining.  In Oangband, the effect is worse,
	 * especially for high-level characters.  As per Tolkien, hobbits
	 * are resistant.
	 */
	if (p_ptr->black_breath)
	{
		byte chance = 0;
		int plev = p_ptr->lev;

		if (race_flags_p(PR_RESIST_BLACK_BREATH)) chance = 2;
		else chance = 5;

		if ((rand_int(100) < chance) && (p_ptr->exp > 0))
		{
			p_ptr->exp -= 1 + plev / 5;
			p_ptr->max_exp -= 1 + plev / 5;
			do_dec_stat(rand_int(6), STAT_DEC_NORMAL);
			check_experience();
		}
	}

	/* Drain Mana */
	if (p_ptr->drain_mana && p_ptr->csp)
	{
		p_ptr->csp -= p_ptr->drain_mana;
		if (magik(30)) p_ptr->csp -= p_ptr->drain_mana;

		if (p_ptr->csp < 0)
		{
			p_ptr->csp = 0;
			disturb();
		}

		/* Redraw */
		p_ptr->redraw |= (PR_FRAME);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);
	}

	/* Partial summons drain mana */
	if (p_ptr->maintain_sum)
	{
		u32b oldcsp = p_ptr->csp;
		p_ptr->csp -= p_ptr->maintain_sum / 10000;

		if (p_ptr->csp < 0)
		{
			p_ptr->csp = 0;
			disturb();

			p_ptr->maintain_sum = 0;
		}
		else
		{
			/* Leave behind any fractional sp */
			p_ptr->maintain_sum -= (oldcsp - p_ptr->csp) * 10000;
		}

		/* Redraw */
		p_ptr->redraw |= (PR_FRAME);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);

	}

	/* Drain Hitpoints */
	if (p_ptr->drain_life)
	{
		int drain = p_ptr->drain_life + rand_int(p_ptr->mhp / 100);

		p_ptr->chp -= (drain < p_ptr->chp ? drain : p_ptr->chp);

		if (p_ptr->chp == 0)
		{
			disturb();
		}

		/* Redraw */
		p_ptr->redraw |= (PR_FRAME);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);

	}

	/* Handle experience draining */
	if (p_ptr->exp_drain)
	{
		if ((rand_int(100) < 10) && (p_ptr->exp > 0))
		{
			p_ptr->exp--;
			p_ptr->max_exp--;
			check_experience();
		}
	}

	/* Process equipment */
	bool changed = false;
	for (int i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		/* Get the object */
		o_ptr = &p_ptr->inventory[i];

		auto const flags = object_flags(o_ptr);


		/* TY Curse */
		if ((flags & TR_TY_CURSE) && (rand_int(TY_CURSE_CHANCE) == 0))
		{
			activate_ty_curse();
		}

		/* DG Curse */
		if ((flags & TR_DG_CURSE) && (rand_int(DG_CURSE_CHANCE) == 0))
		{
			activate_dg_curse();

			/* The object recurse itself ! */
			o_ptr->art_flags |= TR_CURSED;
		}

		/* Auto Curse */
		if ((flags & TR_AUTO_CURSE) && (rand_int(AUTO_CURSE_CHANCE) == 0))
		{
			/* The object recurse itself ! */
			o_ptr->art_flags |= TR_CURSED;
		}

		/*
		 * Hack: Uncursed teleporting items (e.g. Dragon Weapons)
		 * can actually be useful!
		 */
		if ((flags & TR_TELEPORT) && (rand_int(100) < 1))
		{
			if ((o_ptr->art_flags & TR_CURSED) && !p_ptr->anti_tele)
			{
				disturb();

				/* Teleport player */
				teleport_player(40);
			}
			else
			{
				if (p_ptr->wild_mode || strchr(o_ptr->inscription.c_str(), '.'))
				{
					/* Suppress teleportation */
				}
				else if (get_check("Teleport? "))
				{
					disturb();
					teleport_player(50);
				}
			}
		}


		/* Skip non-objects */
		if (!o_ptr->k_ptr)
		{
			continue;
		}

		/* Hack: Skip wielded lights that need fuel (already handled above) */
		if ((i == INVEN_LITE) && (o_ptr->tval == TV_LITE) && (flags & TR_FUEL_LITE)) continue;

		/* Recharge activatable objects */
		if (o_ptr->timeout > 0)
		{
			/* Recharge */
			o_ptr->timeout--;

			/* Notice changes */
			if (o_ptr->timeout == 0)
			{
				recharged_notice(o_ptr);
				changed = true;
			}
		}
	}

	/* Notice changes */
	if (changed)
	{
		p_ptr->window |= PW_EQUIP;
	}

	/* Recharge rods */
	changed = false;
	for (int i = 0; i < INVEN_TOTAL; i++)
	{
		o_ptr = &p_ptr->inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_ptr)
		{
			continue;
		}

		/* Examine the rod */
		auto const flags = object_flags(o_ptr);

		/* Temporary items are destroyed */
		if (flags & TR_TEMPORARY)
		{
			o_ptr->timeout--;

			if (o_ptr->timeout <= 0)
			{
				inc_stack_size(i, -99);

				/* Combine and Reorder pack */
				p_ptr->notice |= (PN_COMBINE | PN_REORDER);
			}
		}

		/* Examine all charging rods or stacks of charging rods. */
		if ((o_ptr->tval == TV_ROD_MAIN) && (o_ptr->timeout < o_ptr->pval2))
		{
			/* Increase the rod's mana. */
			o_ptr->timeout += (flags & TR_CHARGING) ? 2 : 1;

			/* Always notice */
			changed = true;

			/* Notice changes, provide message if object is inscribed. */
			if (o_ptr->timeout >= o_ptr->pval2)
			{
				o_ptr->timeout = o_ptr->pval2;
				recharged_notice(o_ptr);
			}
		}

		/* Examine all charging random artifacts */
		if ((flags & TR_ACTIVATE_NO_WIELD) && (o_ptr->timeout > 0))
		{
			/* Charge it */
			o_ptr->timeout--;

			/* Notice changes */
			if (o_ptr->timeout == 0)
			{
				changed = true;
				recharged_notice(o_ptr);
			}
		}

		/* Decay objects in pack */
		if (decays(o_ptr))
		{
			/* Decay it */
			if (o_ptr->pval != 0)
			{
				if (o_ptr->timeout > 0)
				{
					if (dungeon_flags & DF_HOT)
					{
						o_ptr->pval -= 2;
					}
					else if ((dungeon_flags & DF_COLD) && rand_int(2))
					{
						if (magik(50)) o_ptr->pval--;
					}
					else
					{
						o_ptr->pval--;
					}
				}

				if ((o_ptr->timeout > 0) && o_ptr->timeout < o_ptr->weight) o_ptr->timeout--;

				/* Notice changes */
				if (o_ptr->pval <= 0)
				{
					pack_decay(i);
					changed = true;
				}
			}
		}

		/* Hatch eggs */
		if (o_ptr->tval == TV_EGG)
		{
			if (o_ptr->timeout == 0)
			{
				o_ptr->pval--;

				/* Notice changes */
				if (o_ptr->pval <= 0)
				{
					int mx = p_ptr->px;
					int my = p_ptr->py + 1;
					get_pos_player(5, &my, &mx);
					msg_print("Your egg hatches!");
					place_monster_aux(my, mx, o_ptr->pval2, false, false, MSTATUS_PET);

					monster_type *m_ptr = &m_list[cave[my][mx].m_idx];
					auto const r_ptr = m_ptr->race();

					if ((r_ptr->flags & RF_IMPRESED) && can_create_companion())
					{
						msg_format("And you have given the imprint to your %s!", r_ptr->name);
						m_ptr->status = MSTATUS_COMPANION;
					}

					inc_stack_size(i, -1);
					changed = true;
				}
			}
		}
	}

	/* Notice changes */
	if (changed)
	{
		/* Combine pack */
		p_ptr->notice |= (PN_COMBINE);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN);
	}

	/*** Process Objects ***/

	/* Process objects */
	for (int i = 1; i < o_max; i++)
	{
		/* Access object */
		o_ptr = &o_list[i];

		/* Skip dead objects */
		if (!o_ptr->k_ptr)
		{
			continue;
		}

		/* Examine the rod */
		auto const flags = object_flags(o_ptr);

		/* Temporary items are destroyed */
		if (flags & TR_TEMPORARY)
		{
			o_ptr->timeout--;

			if (o_ptr->timeout <= 0)
			{
				floor_item_increase(i, -99);
				floor_item_optimize(i);

				/* Combine and Reorder pack */
				p_ptr->notice |= (PN_COMBINE | PN_REORDER);
			}
		}

		/* Recharge rods on the ground.  No messages. */
		if ((o_ptr->tval == TV_ROD_MAIN) && (o_ptr->timeout < o_ptr->pval2))
		{
			/* Increase the rod's mana. */
			o_ptr->timeout += (flags & TR_CHARGING) ? 2 : 1;

			/* Do not overflow */
			if (o_ptr->timeout >= o_ptr->pval2)
			{
				o_ptr->timeout = o_ptr->pval2;
			}
		}

		/* Decay objects on the ground*/
		if (decays(o_ptr))
		{
			/* Decay it */
			if (o_ptr->pval != 0)
			{
				if (o_ptr->timeout > 0)
				{
					if (dungeon_flags & DF_HOT)
					{
						o_ptr->pval -= 2;
					}
					else if ((dungeon_flags & DF_COLD) && rand_int(2))
					{
						if (magik(50)) o_ptr->pval--;
					}
					else
					{
						o_ptr->pval--;
					}
				}

				if ((o_ptr->timeout > 0) && o_ptr->timeout < o_ptr->weight) o_ptr->timeout--;

				/* Turn it into a skeleton */
				if (o_ptr->pval <= 0)
				{
					floor_decay(i);
				}
			}
		}

		/* Hatch eggs */
		if (o_ptr->tval == TV_EGG)
		{
			int mx, my;
			if (o_ptr->timeout > 0) o_ptr->pval--;

			/* Notice changes */
			if (o_ptr->pval <= 0)
			{
				mx = o_ptr->ix;
				my = o_ptr->iy;
				get_pos_player(5, &my, &mx);
				msg_print("An egg hatches!");
				place_monster_one(my, mx, o_ptr->pval2, 0, false, MSTATUS_ENEMY);
				floor_item_increase(i, -1);
				floor_item_describe(i);
				floor_item_optimize(i);
			}
		}
	}


	/*** Involuntary Movement ***/

	/* Delayed Word-of-Recall */
	if (p_ptr->word_recall)
	{
		/* Can we ? */
		if (process_hooks_new(HOOK_RECALL, NULL, NULL))
		{
			p_ptr->word_recall = 0;
		}

		/* No recall. sorry */
		else if (dungeon_flags & DF_NO_RECALL_OUT)
		{
			cmsg_print(TERM_L_DARK, "You cannot recall from here.");
			p_ptr->word_recall = 0;
		}

		/* Cannot WoR out of death fate levels */
		else if (dungeon_type == DUNGEON_DEATH)
		{
			cmsg_print(TERM_L_DARK, "You are fated to die here.  FIGHT for your life!");
			p_ptr->word_recall = 0;
		}

		/* I think the 'inside_quest' code belongs here -- pelpel */

		/* They cannot use word of recall until reaching surface */
		else if (p_ptr->astral)
		{
			msg_print("As an astral being you can't recall.");
			p_ptr->word_recall = 0;
		}

		/* Normal WoR */
		else
		{
			/*
			 * HACK: Autosave BEFORE resetting the recall counter (rr9)
			 * The player is yanked up/down as soon as
			 * he loads the autosaved game.
			 */
			if (p_ptr->word_recall == 1)
			{
				autosave_checkpoint();
			}

			/* Make SURE that persistent levels are saved
			 * I don't know if this is needed, but I'm getting reports,
			 * so I'm adding this extra save -- Neil
			 */
			save_dungeon();

			/* Count down towards recall */
			p_ptr->word_recall--;

			/* Activate the recall */
			if (p_ptr->word_recall == 0)
			{
				/* Disturbing! */
				disturb();

				/* Determine the level */
				if (p_ptr->inside_quest)
				{
					msg_print("The recall is cancelled by a powerful magic force!");
				}
				else if (dun_level)
				{
					msg_print("You feel yourself yanked upwards!");

					p_ptr->recall_dungeon = dungeon_type;
					dungeon_type = DUNGEON_WILDERNESS;
					dun_level = 0;

					is_recall = true;

					p_ptr->inside_quest = 0;
					p_ptr->leaving = true;
				}
				else
				{
					msg_print("You feel yourself yanked downwards!");

					/* New depth */
					dungeon_type = p_ptr->recall_dungeon;
					dun_level = max_dlv[dungeon_type];
					if (dun_level < 1) dun_level = 1;

					/* Reset player position */
					p_ptr->oldpx = p_ptr->px;
					p_ptr->oldpy = p_ptr->py;

					/* Leaving */
					is_recall = true;

					p_ptr->leaving = true;
					p_ptr->wild_mode = false;
				}
			}
		}
	}
}


/*
 * Verify use of "wizard" mode
 */
static bool enter_wizard_mode()
{
	/* Ask first time, but not while loading a dead char with the -w option */
	if (!noscore && !(p_ptr->chp < 0))
	{
		/* Mention effects */
		msg_print("Wizard mode is for debugging and experimenting.");
		msg_print("The game will not be scored if you enter wizard mode.");
		msg_print(NULL);

		/* Verify request */
		if (!get_check("Are you sure you want to enter wizard mode? "))
		{
			return false;
		}

		/* Mark savefile */
		noscore |= 0x0002;
	}

	/* Success */
	return true;
}


/*
 * Verify use of "debug" commands
 */
static bool enter_debug_mode()
{
	/* Ask first time */
	if (!noscore && !wizard)
	{
		/* Mention effects */
		msg_print("The debug commands are for debugging and experimenting.");
		msg_print("The game will not be scored if you use debug commands.");
		msg_print(NULL);

		/* Verify request */
		if (!get_check("Are you sure you want to use debug commands? "))
		{
			return false;
		}

		/* Mark savefile */
		noscore |= 0x0008;
	}

	/* Success */
	return true;
}


/*
 * Parse and execute the current command
 * Give "Warning" on illegal commands.
 *
 * XXX XXX XXX Make some "blocks"
 */
static void process_command(s16b *command_ptr)
{
	auto const &wf_info = game->edit_data.wf_info;

	assert(command_ptr);
	auto &command_cmd = *command_ptr;

	/* Handle repeating the last command */
	repeat_check(command_ptr);

	/* Parse the command */
	switch (command_cmd)
	{
		/* Ignore */
	case ESCAPE:
	case ' ':
	case 0:
		{
			break;
		}

		/* Ignore return */
	case '\r':
		{
			break;
		}



		/*** Wizard Commands ***/

		/* Toggle Wizard Mode */
	case KTRL('W'):
		{
			if (wizard)
			{
				wizard = false;
				msg_print("Wizard mode off.");
			}
			else if (enter_wizard_mode())
			{
				wizard = true;
				msg_print("Wizard mode on.");
			}

			/* Update monsters */
			p_ptr->update |= (PU_MONSTERS);

			/* Redraw "title" */
			p_ptr->redraw |= (PR_FRAME);

			break;
		}

		/* Special "debug" commands */
	case KTRL('A'):
		{
			/* Enter debug mode */
			if (enter_debug_mode())
			{
				do_cmd_debug();
			}
			break;
		}

	
	/*** Inventory Commands ***/

		/* Wear/wield equipment */
	case 'w':
		{
			if (p_ptr->control) break;
			if (!p_ptr->wild_mode) do_cmd_wield();
			break;
		}

		/* Take off equipment */
	case 't':
		{
			if (p_ptr->control) break;
			if (!p_ptr->wild_mode) do_cmd_takeoff();
			p_ptr->redraw |= (PR_FRAME);
			break;
		}

		/* Drop an item */
	case 'd':
		{
			if (do_control_drop()) break;
			if (!p_ptr->wild_mode) do_cmd_drop();
			break;
		}

		/* Destroy an item */
	case 'k':
		{
			if (p_ptr->control) break;
			do_cmd_destroy();
			break;
		}

		/* Equipment list */
	case 'e':
		{
			if (p_ptr->control) break;
			do_cmd_equip();
			break;
		}

		/* Inventory list */
	case 'i':
		{
			if (do_control_inven()) break;
			do_cmd_inven();
			break;
		}


		/*** Various commands ***/

		/* Identify an object */
	case 'I':
		{
			do_cmd_observe();
			break;
		}

		/* Hack -- toggle windows */
	case KTRL('I'):
		{
			toggle_inven_equip();
			break;
		}


		/*** Standard "Movement" Commands ***/

		/* Alter a grid */
	case '+':
		{
			if (p_ptr->control) break;
			if (!p_ptr->wild_mode) do_cmd_alter();
			break;
		}

		/* Dig a tunnel */
	case 'T':
		{
			if (p_ptr->control) break;
			if (!p_ptr->wild_mode) do_cmd_tunnel();
			break;
		}

		/* Move (usually pick up things) */
	case ';':
		{
			if (do_control_walk()) break;

			do_cmd_walk(options->always_pickup);

			break;
		}

		/* Move (usually do not pick up) */
	case '-':
		{
			if (do_control_walk()) break;

			do_cmd_walk(!options->always_pickup);

			break;
		}


		/*** Running, Resting, Searching, Staying */

		/* Begin Running -- Arg is Max Distance */
	case '.':
		{
			if (p_ptr->control || p_ptr->wild_mode) break;
			do_cmd_run();
			break;
		}

		/* Stay still (usually pick things up) */
	case ',':
		{
			if (do_control_pickup()) break;
			do_cmd_stay(options->always_pickup);
			break;
		}

		/* Stay still (usually do not pick up) */
	case 'g':
		{
			if (p_ptr->control) break;
			do_cmd_stay(!options->always_pickup);
			break;
		}

		/* Rest -- Arg is time */
	case 'R':
		{
			if (p_ptr->control) break;
			do_cmd_rest();
			break;
		}

		/*** Stairs and Doors and Chests and Traps ***/

		/* Enter store */
	case '_':
		{
			if (p_ptr->control) break;
			if (!p_ptr->wild_mode) do_cmd_store();
			break;
		}

		/* Go up staircase */
	case '<':
		{
			/* Cannot move if rooted in place */
			if (p_ptr->tim_roots) break;

			if (p_ptr->control) break;
			/* Normal cases */
			if (p_ptr->wild_mode || dun_level || is_quest(dun_level))
			{
				do_cmd_go_up();
			}
			/* Don't let the player < when he'd just drop right back down */
			else if (p_ptr->food < PY_FOOD_ALERT)
			{
				msg_print("You are too hungry to travel.");
			}
			else if (p_ptr->sensible_lite &&
			                (((turn / ((10L * DAY) / 2)) % 2) == 0))
			{
				/* Burn vampires! burn! */
				msg_print("You can't travel during the day!");
			}
			else if (p_ptr->sensible_lite &&
					!is_light_safe(&p_ptr->inventory[INVEN_LITE]))
			{
				msg_print("Travel with your present light would be unsafe.");
			}
			else if (p_ptr->cut || p_ptr->poisoned)
			{
				/* I actually died this way once -- neil */
				msg_print("You are too injured to travel.");
			}
			else if (ambush_flag)
			{
				msg_print("To flee the ambush you have to reach the edge of the map.");
			}
			/* TODO: make the above stuff use this hook */
			else if (!process_hooks_new(HOOK_FORBID_TRAVEL, NULL, NULL))
			{
				p_ptr->oldpx = p_ptr->px;
				p_ptr->oldpy = p_ptr->py;
				change_wild_mode();

				/* Update the known wilderness */
				reveal_wilderness_around_player(p_ptr->wilderness_y,
				                                p_ptr->wilderness_x,
				                                0, WILDERNESS_SEE_RADIUS);
			}

			break;
		}

		/* Go down staircase */
	case '>':
		{
			/* Cannot move if rooted in place */
			if (p_ptr->tim_roots) break;

			if (p_ptr->control) break;
			/* Normal cases */
			if (!p_ptr->wild_mode)
			{
				do_cmd_go_down();
			}

			/* Special cases */
			else
			{
				auto const &wilderness = game->wilderness;
				auto const &tile = wilderness(p_ptr->px, p_ptr->py);
				if ((wf_info[tile.feat].entrance >= 1000) || (tile.entrance > 1000))
				{
					p_ptr->wilderness_x = p_ptr->px;
					p_ptr->wilderness_y = p_ptr->py;
					p_ptr->wild_mode = !p_ptr->wild_mode;
					do_cmd_go_down();

					if (dun_level == 0)
					{
						p_ptr->wild_mode = !p_ptr->wild_mode;
					}
					else
					{
						p_ptr->wilderness_x = p_ptr->px;
						p_ptr->wilderness_y = p_ptr->py;
						change_wild_mode();
					}
				}
				else
				{
					p_ptr->wilderness_x = p_ptr->px;
					p_ptr->wilderness_y = p_ptr->py;
					change_wild_mode();
				}
			}

			break;
		}

		/* Open a door or chest */
	case 'o':
		{
			if (p_ptr->control) break;
			if (!p_ptr->wild_mode) do_cmd_open();
			break;
		}

		/* Close a door */
	case 'c':
		{
			if (p_ptr->control) break;
			if (!p_ptr->wild_mode) do_cmd_close();
			break;
		}

		/* Give an item */
	case 'y':
		{
			if (p_ptr->control) break;
			if (!p_ptr->wild_mode) do_cmd_give();
			break;
		}

		/* Chat */
	case 'Y':
		{
			if (p_ptr->control) break;
			if (!p_ptr->wild_mode) do_cmd_chat();
			break;
		}

		/* Jam a door with spikes */
	case 'j':
		{
			if (p_ptr->control) break;
			if (!p_ptr->wild_mode) do_cmd_spike();
			break;
		}

		/* Bash a door */
	case 'B':
		{
			if (p_ptr->control) break;
			if (!p_ptr->wild_mode) do_cmd_bash();
			break;
		}

		/*** Magic and Prayers ***/

		/* Interact with skills */
	case 'G':
		{
			if (p_ptr->control) break;
			do_cmd_skill();
			break;
		}

		/* Interact with abilities */
	case 'N':
		{
			if (p_ptr->control) break;
			do_cmd_ability();
			break;
		}

		/* Browse a book */
	case 'b':
		{
			if (p_ptr->control) break;
			do_cmd_browse();
			break;
		}

		/* Cast a spell */
	case 'm':
		{
			if (do_control_magic()) break;

			/* No magic in the overworld map */
			if (p_ptr->wild_mode) break;

			do_cmd_activate_skill();
			squeltch_inventory();
			squeltch_grid();
			break;
		}

		/* Pray a prayer */
	case 'p':
		{
			if (p_ptr->control || p_ptr->wild_mode) break;
			do_cmd_pray();
			squeltch_inventory();
			squeltch_grid();
			break;
		}

		/* Issue commands to pets */
	case 'P':
		{
			if (p_ptr->control) break;
			if (!p_ptr->wild_mode) do_cmd_pet();
			break;
		}

		/* Cut up a corpse */
	case 'h':
		{
			if (p_ptr->control || p_ptr->wild_mode) break;
			do_cmd_cut_corpse();
			squeltch_inventory();
			squeltch_grid();
			break;
		}

		/* Cure some meat */
	case 'K':
		{
			if (p_ptr->control) break;
			do_cmd_cure_meat();
			squeltch_inventory();
			squeltch_grid();
			break;
		}

		/* Steal an item form a monster */
	case 'Z':
		{
			if (p_ptr->control || p_ptr->wild_mode) break;
			do_cmd_steal();
			squeltch_inventory();
			squeltch_grid();
			break;
		}

		/*** Use various objects ***/

		/* Inscribe an object */
	case '{':
		{
			if (p_ptr->control) break;
			do_cmd_inscribe();
			break;
		}

		/* Uninscribe an object */
	case '}':
		{
			if (p_ptr->control) break;
			do_cmd_uninscribe();
			break;
		}

		/* Activate an artifact */
	case 'A':
		{
			if (p_ptr->control) break;
			if (p_ptr->wild_mode) break;

			do_cmd_activate();
			squeltch_inventory();
			squeltch_grid();
			break;
		}

		/* Eat some food */
	case 'E':
		{
			if (p_ptr->control) break;
			do_cmd_eat_food();
			break;
		}

		/* Fuel your lantern/torch */
	case 'F':
		{
			if (p_ptr->control) break;
			do_cmd_refill();
			break;
		}

		/* Fire an item */
	case 'f':
		{
			object_type *j_ptr;

			if (p_ptr->control) break;
			if (p_ptr->wild_mode) break;

			j_ptr = &p_ptr->inventory[INVEN_BOW];

			if (j_ptr->tval == TV_BOOMERANG)
			{
				do_cmd_boomerang();
			}
			else
			{
				do_cmd_fire();
			}

			break;
		}

		/* Throw an item */
	case 'v':
		{
			if (p_ptr->control) break;
			if (p_ptr->wild_mode) break;

			do_cmd_throw();
			break;
		}

		/* Aim a wand */
	case 'a':
		{
			if (p_ptr->control) break;
			if (p_ptr->wild_mode) break;

			do_cmd_aim_wand();
			squeltch_inventory();
			squeltch_grid();
			break;
		}

		/* Zap a rod */
	case 'z':
		{
			if (p_ptr->control) break;
			if (p_ptr->wild_mode) break;

			do_cmd_zap_rod();
			squeltch_inventory();
			squeltch_grid();
			break;
		}

		/* Quaff a potion */
	case 'q':
		{
			if (p_ptr->control) break;
			if (p_ptr->wild_mode) break;

			do_cmd_quaff_potion();
			squeltch_inventory();
			squeltch_grid();
			break;
		}

		/* Drink from a fountain -SC- */
	case 'H':
		{
			cave_type *c_ptr = &cave[p_ptr->py][p_ptr->px];

			if (p_ptr->control) break;
			if ((c_ptr->feat == FEAT_FOUNTAIN) ||
			                (c_ptr->feat == FEAT_EMPTY_FOUNTAIN))
			{
				do_cmd_drink_fountain();
				squeltch_inventory();
				squeltch_grid();
			}
			else
			{
				msg_print("You see no fountain here.");
			}

			break;
		}

		/* Read a scroll */
	case 'r':
		{
			if (p_ptr->control) break;
			if (p_ptr->wild_mode) break;

			do_cmd_read_scroll();
			squeltch_inventory();
			squeltch_grid();
			break;
		}

		/* Use a staff */
	case 'u':
		{
			if (p_ptr->control) break;
			if (p_ptr->wild_mode) break;

			do_cmd_use_staff();
			squeltch_inventory();
			squeltch_grid();
			break;
		}

		/* Use racial power */
	case 'U':
		{
			if (p_ptr->control) break;
			if (p_ptr->wild_mode) break;

			do_cmd_power();
			squeltch_inventory();
			squeltch_grid();
			break;
		}

		/* Sacrifice at an altar */
	case 'O':
		{
			if (p_ptr->control) break;
			if (p_ptr->wild_mode) break;

			if (race_flags_p(PR_NO_GOD))
			{
				msg_print("You cannot worship gods.");
			}
			else
			{
				do_cmd_sacrifice();
			}

			break;
		}

		/*** Looking at Things (nearby or on map) ***/

		/* Full dungeon map */
	case 'M':
		{
			if (!p_ptr->wild_mode) do_cmd_view_map();
			break;
		}

		/* Locate player on map */
	case 'L':
		{
			do_cmd_locate();
			break;
		}

		/* Look around */
	case 'l':
		{
			do_cmd_look();
			break;
		}

		/* Target monster or location */
	case '*':
		{
			if (p_ptr->control) break;
			if (!p_ptr->wild_mode) do_cmd_target();
			break;
		}

		/* Engrave the floor */
	case 'x':
		{
			if (p_ptr->control) break;
			if (p_ptr->wild_mode) break;

			/* No point in engraving if there isn't any mana on this grid. */
			/* DG - actualy there is, it doesnt break macros */
			do_cmd_sense_grid_mana();
			do_cmd_engrave();

			break;
		}

		/*** Help and Such ***/

		/* Help */
	case '?':
		{
			do_cmd_help();
			break;
		}

		/* Identify symbol */
	case '/':
		{
			do_cmd_query_symbol();
			break;
		}

		/* Character description */
	case 'C':
		{
			do_cmd_change_name();
			break;
		}


		/*** System Commands ***/

		/* Single line from a pref file */
	case '"':
		{
			do_cmd_pref();
			break;
		}

		/* Interact with macros */
	case '@':
		{
			do_cmd_macros();
			break;
		}

		/* Interact with visuals */
	case '%':
		{
			do_cmd_visuals();
			break;
		}

		/* Interact with colors */
	case '&':
		{
			do_cmd_colors();
			break;
		}

		/* Interact with options */
	case '=':
		{
			do_cmd_options();
			break;
		}


		/*** Misc Commands ***/

		/* Take notes */
	case ':':
		{
			do_cmd_note();
			break;
		}

		/* Version info */
	case 'V':
		{
			do_cmd_version();
			break;
		}

		/* Repeat level feeling */
	case KTRL('F'):
		{
			if (!p_ptr->wild_mode)
				do_cmd_feeling();
			break;
		}

		/* Show previous message */
	case KTRL('O'):
		{
			do_cmd_message_one();
			break;
		}

		/* Show previous messages */
	case KTRL('P'):
		{
			do_cmd_messages();
			break;
		}

		/* Show quest status -KMW- */
	case KTRL('Q'):
				case CMD_QUEST:
{
			do_cmd_checkquest();
			break;
		}

		/* Redraw the screen */
	case KTRL('R'):
		{
			do_cmd_redraw();
			break;
		}

		/* Hack -- Save and don't quit */
	case KTRL('S'):
		{
			is_autosave = false;
			do_cmd_save_game();
			break;
		}

	case KTRL('T'):
		{
			do_cmd_time();
		}
		break;

		/* Save and quit */
	case KTRL('X'):
		{
			alive = false;

			/* Leaving */
			p_ptr->leaving = true;

			break;
		}

		/* Quit (commit suicide) */
	case 'Q':
		{
			do_cmd_suicide();
			break;
		}

		/* Extended command */
	case '#':
		{
			do_cmd_cli();
			break;
		}

		/* Check artifacts, uniques, objects */
	case '~':
		{
			do_cmd_knowledge();
			break;
		}

		/* Commands only available as extended commands: */

		/* Extended command help. */
	case CMD_CLI_HELP:
		{
			do_cmd_cli_help();
			break;
		}

		/* Game time. */
	case CMD_SHOW_TIME:
		{
			do_cmd_time();
			break;
		}

		/* Check skills. */
	case CMD_SHOW_SKILL:
		{
			do_cmd_skill();
			break;
		}

		/* Check abilities. */
	case CMD_SHOW_ABILITY:
		{
			do_cmd_ability();
			break;
		}

		/* Save a html screenshot. */
	case CMD_DUMP_HTML:
		{
			do_cmd_html_dump();
			break;
		}

		/* Record a macro. */
	case '$':
	case CMD_MACRO:
		{
			do_cmd_macro_recorder();
			break;
		}

		/* Hack -- Unknown command */
	default:
		{
			int insanity = (p_ptr->msane - p_ptr->csane) * 100 / p_ptr->msane;

			/* Would like to have an option disabling this -- pelpel */
			if (rand_int(100) < insanity)
			{
				char error_m[80];

				get_rnd_line("error.txt", error_m);
				msg_print(error_m);
			}
			else
			{
				prt("Type '?' for help.", 0, 0);
			}

			break;
		}
	}
}




/*
 * Process the player
 *
 * Notice the annoying code to handle "pack overflow", which
 * must come first just in case somebody manages to corrupt
 * the savefiles by clever use of menu commands or something.
 */
static void process_player()
{
	auto const &f_info = game->edit_data.f_info;
	auto const &dungeon_flags = game->dungeon_flags;

	int i, j;

	int speed_use;


	/*** Apply energy ***/

	/* Obtain current speed */
	speed_use = p_ptr->pspeed;

	/* Maximum value */
	if (speed_use > 199)
	{
		speed_use = 199;
	}

	/* Minimum value */
	else if (speed_use < 0)
	{
		speed_use = 0;
	}

	/* Give the player some energy */
	p_ptr->energy += extract_energy[speed_use];

	/* No turn yet */
	if (p_ptr->energy < 100) return;


	/*** Check for interupts ***/

	/* Complete resting */
	if (resting < 0)
	{
		/* Basic resting */
		if (resting == -1)
		{
			/* Stop resting */
			if ((p_ptr->chp == p_ptr->mhp) && (p_ptr->csp >= p_ptr->msp))
			{
				disturb();
			}
		}

		/* Complete resting */
		else if (resting == -2)
		{
			bool stop = true;
			object_type *o_ptr;

			/* Get the carried monster */
			o_ptr = &p_ptr->inventory[INVEN_CARRY];

			/* Stop resting */
			if ((!p_ptr->drain_life) && (p_ptr->chp != p_ptr->mhp)) stop = false;
			if ((!p_ptr->drain_mana) && (p_ptr->csp != p_ptr->msp)) stop = false;
			if (o_ptr->pval2 < o_ptr->pval3) stop = false;
			if (p_ptr->blind || p_ptr->confused) stop = false;
			if (p_ptr->poisoned || p_ptr->afraid) stop = false;
			if (p_ptr->stun || p_ptr->cut) stop = false;
			if (p_ptr->slow || p_ptr->paralyzed) stop = false;
			if (p_ptr->image || p_ptr->word_recall) stop = false;
			if (p_ptr->immov_cntr != 0) stop = false;

			for (i = 0; i < 6; i++)
			{
				if (p_ptr->stat_cnt[i] > 0)
				{
					stop = false;
				}
			}

			if (stop)
			{
				disturb();
			}
			p_ptr->redraw |= (PR_FRAME);
		}
	}

	/* Handle "abort" */
	if (!options->avoid_abort)
	{
		/* Check for "player abort" (semi-efficiently for resting) */
		if (running || command_rep || (resting && !(resting & 0x0F)))
		{
			/* Check for a key */
			if (inkey_scan())
			{
				/* Flush input */
				flush();

				/* Disturb */
				disturb();

				/* Hack -- Show a Message */
				msg_print("Cancelled.");
			}
		}
	}


	/*** Handle actual user input ***/

	/* Repeat until out of energy */
	while (p_ptr->energy >= 100)
	{
		/* Notice stuff (if needed) */
		if (p_ptr->notice) notice_stuff();

		/* Update stuff (if needed) */
		if (p_ptr->update) update_stuff();

		/* Redraw stuff (if needed) */
		if (p_ptr->redraw) redraw_stuff();

		/* Redraw stuff (if needed) */
		if (p_ptr->window) window_stuff();

		/* Hack -- mark current wilderness location as known */
		if (!p_ptr->wild_mode && dun_level == 0)
		{
			auto &wilderness = game->wilderness;
			wilderness(p_ptr->wilderness_x, p_ptr->wilderness_y).known = true;
		}


		/* Place the cursor on the player */
		move_cursor_relative(p_ptr->py, p_ptr->px);

		/* Refresh (optional) */
		if (options->fresh_before) Term_fresh();

		/* Hack -- Pack Overflow */
		if (p_ptr->inventory[INVEN_PACK].k_ptr)
		{
			int item = INVEN_PACK;

			/* Access the slot to be dropped */
			auto o_ptr = &p_ptr->inventory[item];

			/* Disturbing */
			disturb();

			/* Warning */
			msg_print("Your pack overflows!");

			/* Describe */
			char o_name[80];
			object_desc(o_name, o_ptr, true, 3);

			/* Message */
			msg_format("You drop %s (%c).", o_name, index_to_label(item));

			/* Drop it (carefully) near the player */
			drop_near(o_ptr, 0, p_ptr->py, p_ptr->px);

			/* Modify, Describe, Optimize */
			inc_stack_size(item, -255);

			/* Notice stuff (if needed) */
			if (p_ptr->notice) notice_stuff();

			/* Update stuff (if needed) */
			if (p_ptr->update) update_stuff();

			/* Redraw stuff (if needed) */
			if (p_ptr->redraw) redraw_stuff();

			/* Redraw stuff (if needed) */
			if (p_ptr->window) window_stuff();
		}


		/* Assume free turn */
		energy_use = 0;


		/* Paralyzed or Knocked Out */
		if ((p_ptr->paralyzed) || (p_ptr->stun >= 100))
		{
			/* Take a turn */
			energy_use = 100;
		}

		/* Resting */
		else if (resting)
		{
			/* Timed rest */
			if (resting > 0)
			{
				/* Reduce rest count */
				resting--;

				/* Redraw the state */
				p_ptr->redraw |= (PR_FRAME);
			}

			p_ptr->did_nothing = true;

			/* Take a turn */
			energy_use = 100;
		}

		/* Running */
		else if (running)
		{
			/* Take a step */
			run_step(0);

			/*
			 * Commented out because it doesn't make any sense
			 * to require a player holding down direction keys
			 * instead of using running commands when s/he follows
			 * Eru and do the opposite for the other deities -- pelpel
			 */
			/* p_ptr->did_nothing = true; */
		}

		/* Repeated command */
		else if (command_rep)
		{
			/* Count this execution */
			command_rep--;

			/* Redraw the state */
			p_ptr->redraw |= (PR_FRAME);

			/* Redraw stuff */
			redraw_stuff();

			/* Hack -- Assume messages were seen */
			msg_flag = false;

			/* Clear the top line */
			prt("", 0, 0);

			/* Process the command */
			process_command(&command_cmd);

			p_ptr->did_nothing = true;
		}

		/* Normal command */
		else
		{
			/* Place the cursor on the player */
			move_cursor_relative(p_ptr->py, p_ptr->px);

			/* Get a command (normal) */
			request_command(false);

			/* Process the command */
			process_command(&command_cmd);
		}


		/*** Clean up ***/

		/* Significant */
		if (energy_use)
		{
			/* Use some energy */
			p_ptr->energy -= energy_use;


			/* Hack -- constant hallucination */
			if (p_ptr->image) p_ptr->redraw |= (PR_MAP);


			/* Shimmer monsters if needed */
			if (!options->avoid_other && shimmer_monsters)
			{
				/* Clear the flag */
				shimmer_monsters = false;

				/* Shimmer multi-hued monsters */
				for (i = 1; i < m_max; i++)
				{
					/* Access monster */
					monster_type *m_ptr = &m_list[i];

					/* Skip dead monsters */
					if (!m_ptr->r_idx) continue;

					/* Access the monster race */
					auto const r_ptr = m_ptr->race();

					/* Skip non-multi-hued monsters */
					if (!(r_ptr->flags & RF_ATTR_MULTI)) continue;

					/* Reset the flag */
					shimmer_monsters = true;

					/* Redraw regardless */
					lite_spot(m_ptr->fy, m_ptr->fx);
				}
			}

			/* Shimmer objects if needed and requested */
			if (!options->avoid_other && !options->avoid_shimmer && shimmer_objects)
			{
				/* Clear the flag */
				shimmer_objects = false;

				/* Shimmer multi-hued objects */
				for (i = 1; i < o_max; i++)
				{
					/* Acquire object -- for speed only base items are allowed to shimmer */
					object_type *o_ptr = &o_list[i];

					/* Skip dead or carried objects */
					if ((!o_ptr->k_ptr) || (!o_ptr->ix)) continue;

					/* Skip non-multi-hued monsters */
					if (!(o_ptr->k_ptr->flags & TR_ATTR_MULTI)) continue;

					/* Reset the flag */
					shimmer_objects = true;

					/* Redraw regardless */
					lite_spot(o_ptr->iy, o_ptr->ix);
				}
			}

			/*
			 * Shimmer features if needed and requested
			 *
			 * Note: this can be unbearably slow when a player chooses
			 * to use a REALLY big screen in levels filled with shallow
			 * water.  I believe this also hurts a lot on multiuser systems.
			 * However fast modern processors are, I/O cannot be made that
			 * fast, and that's why shimmering has been limited to small
			 * number of monsters -- pelpel
			 */
			if (!options->avoid_other && !options->avoid_shimmer &&
			                !resting && !running)
			{
				for (j = panel_row_min; j <= panel_row_max; j++)
				{
					for (i = panel_col_min; i <= panel_col_max; i++)
					{
						cave_type *c_ptr = &cave[j][i];
						auto f_ptr = c_ptr->mimic
							? &f_info[c_ptr->mimic]
							: &f_info[f_info[c_ptr->feat].mimic];

						/* Skip normal features */
						if (!(f_ptr->flags & FF_ATTR_MULTI))
						{
							continue;
						}

						/* Redraw a shimmering spot */
						lite_spot(j, i);
					}
				}
			}


			/* Handle monster detection */
			if (repair_monsters)
			{
				/* Reset the flag */
				repair_monsters = false;

				/* Rotate detection flags */
				for (i = 1; i < m_max; i++)
				{
					monster_type *m_ptr;

					/* Access monster */
					m_ptr = &m_list[i];

					/* Skip dead monsters */
					if (!m_ptr->r_idx) continue;

					/* Nice monsters get mean */
					if (m_ptr->mflag & (MFLAG_NICE))
					{
						/* Nice monsters get mean */
						m_ptr->mflag &= ~(MFLAG_NICE);
					}

					/* Handle memorized monsters */
					if (m_ptr->mflag & (MFLAG_MARK))
					{
						/* Maintain detection */
						if (m_ptr->mflag & (MFLAG_SHOW))
						{
							/* Forget flag */
							m_ptr->mflag &= ~(MFLAG_SHOW);

							/* Still need repairs */
							repair_monsters = true;
						}

						/* Remove detection */
						else
						{
							/* Forget flag */
							m_ptr->mflag &= ~(MFLAG_MARK);

							/* Assume invisible */
							m_ptr->ml = false;

							/* Update the monster */
							update_mon(i, false);

							/* Redraw regardless */
							lite_spot(m_ptr->fy, m_ptr->fx);
						}
					}
				}
			}

			/*
			 * Moved from dungeon() -- It'll get called whenever player
			 * spends energy, so that maze isn't incredibly easy for
			 * Sorcerors and alike any longer -- pelpel
			 *
			 * Forget everything when requested hehe I'm *NASTY*
			 */
			if (dun_level && (dungeon_flags & DF_FORGET))
			{
				wiz_dark();
			}
		}


		/* Hack -- notice death */
		if (!alive || death) break;

		/* Handle "leaving" */
		if (p_ptr->leaving) break;
	}
}



/*
 * Interact with the current dungeon level.
 *
 * This function will not exit until the level is completed,
 * the user dies, or the game is terminated.
 */
static void dungeon()
{
	auto const &d_info = game->edit_data.d_info;
	auto const &dungeon_flags = game->dungeon_flags;

	/* Reset various flags */
	hack_mind = false;

	/* Not leaving */
	p_ptr->leaving = false;

	/* Reset the "command" vars */
	command_cmd = 0;
	command_new = 0;
	command_rep = 0;
	command_arg = 0;
	command_dir = 0;

	/* Make sure partial summoning counter is initialized. */
	p_ptr->maintain_sum = 0;

	/* Cancel the target */
	target_who = 0;

	/* Cancel the health bar */
	health_track(0);


	/* Check visual effects */
	shimmer_monsters = true;
	shimmer_objects = true;
	repair_monsters = true;


	/* Disturb */
	disturb();

	/* Track maximum player level */
	if (p_ptr->max_plv < p_ptr->lev)
	{
		p_ptr->max_plv = p_ptr->lev;
	}

	/* Track maximum dungeon level (if not in quest -KMW-) */
	if ((max_dlv[dungeon_type] < dun_level) && !p_ptr->inside_quest)
	{
		max_dlv[dungeon_type] = dun_level;
	}

	/* No stairs down from Quest */
	if (is_quest(dun_level) && !p_ptr->astral)
	{
		create_down_stair = false;
		create_down_shaft = false;
	}

	/* Paranoia -- no stairs from town or wilderness */
	if (!dun_level) create_down_stair = create_up_stair = false;
	if (!dun_level) create_down_shaft = create_up_shaft = false;

	/* Option -- no connected stairs */
	if (!options->dungeon_stair) create_down_stair = create_up_stair = false;
	if (!options->dungeon_stair) create_down_shaft = create_up_shaft = false;

	/* no connecting stairs on special levels */
	if (!(dungeon_flags & DF_NO_STAIR)) create_down_stair = create_up_stair = false;
	if (!(dungeon_flags & DF_NO_STAIR)) create_down_shaft = create_up_shaft = false;

	/* Make a stairway. */
	if ((create_up_stair || create_down_stair ||
	                create_up_shaft || create_down_shaft) &&
	                !get_fbranch())
	{
		/* Place a stairway */
		if (cave_valid_bold(p_ptr->py, p_ptr->px))
		{
			/* XXX XXX XXX */
			delete_object(p_ptr->py, p_ptr->px);

			/* Make stairs */
			if (create_down_stair)
			{
				cave_set_feat(p_ptr->py, p_ptr->px, (dungeon_flags & DF_FLAT) ? FEAT_WAY_MORE : FEAT_MORE);
			}
			else if (create_down_shaft)
			{
				cave_set_feat(p_ptr->py, p_ptr->px, (dungeon_flags & DF_FLAT) ? FEAT_WAY_MORE : FEAT_SHAFT_DOWN);
			}
			else if (create_up_shaft)
			{
				cave_set_feat(p_ptr->py, p_ptr->px, (dungeon_flags & DF_FLAT) ? FEAT_WAY_LESS : FEAT_SHAFT_UP);
			}
			else
			{
				cave_set_feat(p_ptr->py, p_ptr->px, (dungeon_flags & DF_FLAT) ? FEAT_WAY_LESS : FEAT_LESS);
			}
		}

		/* Cancel the stair request */
		create_down_stair = create_up_stair = false;
		create_down_shaft = create_up_shaft = false;
	}

	/* Hack - Assume invalid panel */
	panel_row_min = cur_hgt;
	panel_row_max = 0;
	panel_col_min = cur_wid;
	panel_col_max = 0;

	/* Center the panel */
	verify_panel();

	/* Flush messages */
	msg_print(NULL);


	/* Enter "xtra" mode */
	character_xtra = true;

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

	/* Window stuff */
	p_ptr->window |= (PW_MONSTER);

	/* Redraw dungeon */
	p_ptr->redraw |= (PR_WIPE | PR_FRAME);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);

	/* Update stuff */
	p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS | PU_POWERS | PU_SANITY | PU_BODY);

	/* Calculate torch radius */
	p_ptr->update |= (PU_TORCH);

	/* Update stuff */
	update_stuff();

	/* Redraw stuff */
	redraw_stuff();

	/* Redraw stuff */
	window_stuff();

	/* Update stuff */
	p_ptr->update |= (PU_VIEW | PU_FLOW | PU_DISTANCE | PU_MON_LITE);

	/* Update stuff */
	update_stuff();

	/* Redraw stuff */
	redraw_stuff();

	/* Leave "xtra" mode */
	character_xtra = false;

	/* Update stuff */
	p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS | PU_POWERS | PU_BODY);

	/* Combine / Reorder the pack */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Notice stuff */
	notice_stuff();

	/* Update stuff */
	update_stuff();

	/* Redraw stuff */
	redraw_stuff();

	/* Window stuff */
	window_stuff();

	/* Refresh */
	Term_fresh();


	/* Announce (or repeat) the feeling */
	if (dun_level) do_cmd_feeling();


	/* Hack -- notice death or departure */
	if (!alive || death) return;

	/*** Process this dungeon level ***/

	/* Reset the monster generation level */
	monster_level = dun_level;

	/* Reset the object generation level */
	object_level = dun_level;

	hack_mind = true;

	/* Mega Hack, if needed wipe all stairs */
	if (dungeon_type == DUNGEON_DEATH)
	{
		int i, j;

		for (i = 0; i < cur_wid; i++)
		{
			for (j = 0; j < cur_hgt; j++)
			{
				cave_type *c_ptr = &cave[j][i];

				switch (c_ptr->feat)
				{
				case FEAT_MORE:
				case FEAT_LESS:
				case FEAT_SHAFT_UP:
				case FEAT_SHAFT_DOWN:
					{
						cave_set_feat(j, i, FEAT_FLOOR);
						break;
					}
				}
			}
		}

		/* Reset the monster generation level */
		monster_level = 127;

		/* Reset the object generation level */
		object_level = 0;
	}

	/* Main loop */
	while (true)
	{
		/* Hack -- Compact the monster list occasionally */
		if (m_cnt + 32 > max_m_idx) compact_monsters(64);

		/* Hack -- Compress the monster list occasionally */
		if (m_cnt + 32 < m_max) compact_monsters(0);


		/* Hack -- Compact the object list occasionally */
		if (o_cnt + 32 > max_o_idx) compact_objects(64);

		/* Hack -- Compress the object list occasionally */
		if (o_cnt + 32 < o_max) compact_objects(0);



		/* Process the player */
		process_player();

		/* Notice stuff */
		if (p_ptr->notice) notice_stuff();

		/* Update stuff */
		if (p_ptr->update) update_stuff();

		/* Redraw stuff */
		if (p_ptr->redraw) redraw_stuff();

		/* Redraw stuff */
		if (p_ptr->window) window_stuff();

		/* Hack -- Hilite the player */
		move_cursor_relative(p_ptr->py, p_ptr->px);

		/* Optional fresh */
		if (options->fresh_after) Term_fresh();

		/* Hack -- Notice death or departure */
		if (!alive || death) break;


		total_friends = 0;
		total_friend_levels = 0;

		/* Process all of the monsters */
		process_monsters();

		/* Notice stuff */
		if (p_ptr->notice) notice_stuff();

		/* Update stuff */
		if (p_ptr->update) update_stuff();

		/* Redraw stuff */
		if (p_ptr->redraw) redraw_stuff();

		/* Redraw stuff */
		if (p_ptr->window) window_stuff();

		/* Hack -- Hilite the player */
		move_cursor_relative(p_ptr->py, p_ptr->px);

		/* Optional fresh */
		if (options->fresh_after) Term_fresh();

		/* Hack -- Notice death or departure */
		if (!alive || death) break;


		/* Process the world */
		process_world();

		/* Process the appropriate hooks */
		process_hooks_new(HOOK_END_TURN, NULL, NULL);

		/* Make it pulsate and live !!!! */
		if ((dungeon_flags & DF_EVOLVE) && dun_level)
		{
			if (!(turn % 10))
			{
				evolve_level(true);
			}
		}

		/* Notice stuff */
		if (p_ptr->notice) notice_stuff();

		/* Update stuff */
		if (p_ptr->update) update_stuff();

		/* Redraw stuff */
		if (p_ptr->redraw) redraw_stuff();

		/* Window stuff */
		if (p_ptr->window) window_stuff();

		/* Hack -- Hilite the player */
		move_cursor_relative(p_ptr->py, p_ptr->px);

		/* Optional fresh */
		if (options->fresh_after) Term_fresh();

		/* Hack -- Notice death or departure */
		if (!alive || death) break;

		/* Handle "leaving" */
		if (p_ptr->leaving) break;

		/* Count game turns */
		turn++;
	}

	/* Did we leave a dungeon ? */
	if ((dun_level < d_info[dungeon_type].mindepth) && !is_recall)
	{
		dun_level = 0;

		if (d_info[dungeon_type].ix > -1)
		{
			p_ptr->wilderness_x = d_info[dungeon_type].ix;
			p_ptr->wilderness_y = d_info[dungeon_type].iy;
		}

		dungeon_type = DUNGEON_WILDERNESS;
	}

	if (dun_level > d_info[dungeon_type].maxdepth)
	{
		dun_level = 0;

		if (d_info[dungeon_type].ox > -1)
		{
			p_ptr->wilderness_x = d_info[dungeon_type].ox;
			p_ptr->wilderness_y = d_info[dungeon_type].oy;
		}

		dungeon_type = DUNGEON_WILDERNESS;
	}

	is_recall = false;
}




/*
 * Load some "user pref files"
 */
static void load_all_pref_files()
{
	std::string const &player_name = game->player_name;

	/* Process the "race" pref file */
	process_pref_file(name_file_pref(rp_ptr->title));

	/* Process the "class" pref file */
	process_pref_file(name_file_pref(spp_ptr->title));

	/* Process the "character" pref file */
	process_pref_file(name_file_pref(player_name));

	/* Load automatizer settings. Character-specific automatizer
	 * file gets priority over the "template" file. We do not try
	 * to merge the two files since that would require tracking
	 * the providence of rules and such to avoid the same
	 * duplication problems as caused when saving macros/keymaps. */
	boost::filesystem::path userDirectory(ANGBAND_DIR_USER);
	if (automatizer_load(userDirectory / (player_name + ".atm")))
	{
		// Done
	}
	else if (automatizer_load(userDirectory / "automat.atm"))
	{
		// Done
	}
}

/*
 * Actually play a game
 */
void play_game(program_args const &args)
{
	auto const &d_info = game->edit_data.d_info;

	int i, tmp_dun;

	bool cheat_death = false;

	/* Initialize player */
	p_ptr = new player_type();

	/* Hack -- Character is "icky" */
	character_icky = true;


	/* Make sure main term is active */
	Term_activate(angband_term[0]);

	/* Initialise the resize hooks for all the terminals */
	term_set_resize_hook(angband_term[0], resize_map);
	for (i = 1; i < ANGBAND_TERM_MAX; i++)
	{
		if (angband_term[i])
		{
			term_set_resize_hook(angband_term[i], resize_window);
		}
	}


	/* Hack -- turn off the cursor */
	Term_hide_cursor();

	/* Character list */
	bool new_game = false;
	if (!no_begin_screen)
	{
		new_game = begin_screen();
	}
	no_begin_screen = false;

	/* Attempt to load */
	if (!load_player(args))
	{
		/* Oops */
		quit("broken savefile");
	}

	/* Nothing loaded */
	if (!character_loaded)
	{
		/* Make new player */
		new_game = true;

		/* The dungeon is not ready */
		character_dungeon = false;
	}

	/* Process old character */
	if (!new_game)
	{
		/* Process the player name */
		set_player_base(game->player_name);
	}

	/* Force "complex" RNG */
	set_complex_rng();

	/* Roll new character */
	if (new_game)
	{
		/* Show intro */
		modules[game_module_idx].intro();

		/* The dungeon is not ready */
		character_dungeon = false;

		/* Set the seed for flavors */
		seed_flavor() = seed_t::system();

		/* Roll up a new character */
		player_birth();

		/* Start in town, or not */
		if (p_ptr->astral) dun_level = 98;
		else dun_level = 0;
		p_ptr->inside_quest = 0;

		/* Hack -- enter the world */
		/* Mega-hack Vampires and Spectres start at midnight */
		if (race_flags_p(PR_UNDEAD))
		{
			turn = (10L * DAY / 2) + 1;
		}
		else
		{
			turn = 1;
		}
	}

	/* Flash a message */
	prt("Please wait...", 0, 0);

	/* Flush the message */
	Term_fresh();

	/* Be sure to not bother the player */
	calc_powers_silent = true;

	/* Hack -- Enter wizard mode */
	if (args.wizard && enter_wizard_mode())
	{
		wizard = true;
	}

	/* Flavor the objects */
	flavor_init();

	/* Reset the visual mappings */
	reset_visuals();

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

	/* Window stuff */
	p_ptr->window |= (PW_MONSTER);

	/* Window stuff */
	window_stuff();

	/* load user file */
	process_pref_file(name_file_pref("user"));

	/* Load the "pref" files */
	load_all_pref_files();

	/* Set or clear "rogue_like_commands" if requested */
	if (auto it = args.force_key_set)
	{
		assert((*it == 'r') || (*it == 'o'));
		options->rogue_like_commands = (*it == 'r');
	}

	/* Initialize vault info */
	if (init_v_info()) quit("Cannot initialize vaults");

	/* Initialize hooks */
	init_hooks_quests();
	init_hooks_help();
	init_hooks_module();

	/* React to changes */
	Term_xtra_react();

	/* Mega hack, prevent lots of bugs */
	if ((p_ptr->px == 0) || (p_ptr->py == 0))
	{
		p_ptr->px = 1;
		p_ptr->py = 1;
	};

	/* Hack - if note file exists, load it */
	if (!new_game)
	{
		add_note_type(NOTE_ENTER_DUNGEON);
	}

	/* Generate a dungeon level if needed */
	if (!character_dungeon) generate_cave();

	/* Ok tell the scripts that the game is about to start */
	process_hooks_new(HOOK_GAME_START, NULL, NULL);

	/* Character is now "complete" */
	character_generated = true;


	/* Hack -- Character is no longer "icky" */
	character_icky = false;


	/* Start game */
	alive = true;

	/* Hack -- Enforce "delayed death" */
	if (p_ptr->chp < 0) death = true;

	/* Process */
	while (true)
	{
		/* Save the level */
		old_dun_level = dun_level;
		p_ptr->old_wild_mode = p_ptr->wild_mode;

		/* We reached surface ? good, lets go down again !! */
		if (p_ptr->astral && !dun_level)
		{
			p_ptr->astral = false;
			cmsg_print(TERM_L_GREEN,
			           "Well done ! You reached the town ! "
			           "You can now go down again.");
		}

		/* Update monster list window */
		p_ptr->window |= (PW_M_LIST);

		/* Process the level */
		dungeon();

		/* Save the current level if in a persistent level */
		tmp_dun = dun_level;
		dun_level = old_dun_level;
		save_dungeon();
		dun_level = tmp_dun;

		/* A death fate affects level generation */
		for (i = 0; i < MAX_FATES; i++)
		{
			/* Ignore empty slots */
			if (!fates[i].fate) continue;

			/* Ignore non-applicable fates */
			if (fates[i].level != dun_level) continue;

			/* Non-serious fate fails to fire 50% of time */
			if (!fates[i].serious && (rand_int(2) == 0)) continue;

			/* Analyse fate */
			switch (fates[i].fate)
			{
				/* You are doomed */
			case FATE_DIE:
				{
					cmsg_print(TERM_L_DARK, "You were fated to die here.  DIE!");

					/* You shall perish there */
					dungeon_type = DUNGEON_DEATH;
					dun_level = d_info[dungeon_type].mindepth; /* was 1 */

					fates[i].fate = FATE_NONE;
					break;
				}
			}
		}

		/* Notice stuff */
		if (p_ptr->notice) notice_stuff();

		/* Update stuff */
		if (p_ptr->update) update_stuff();

		/* Redraw stuff */
		if (p_ptr->redraw) redraw_stuff();

		/* Window stuff */
		if (p_ptr->window) window_stuff();

		/* Cancel the target */
		target_who = 0;

		/* Cancel the health bar */
		health_track(0);


		/* Forget the lite */
		forget_mon_lite();

		/* Forget the view */
		forget_view();

		/* Handle "quit and save" */
		if (!alive && !death) break;


		/* Erase the old cave */
		wipe_o_list();


		/* XXX XXX XXX */
		msg_print(NULL);

		/* Accidental Death */
		if (alive && death)
		{
			cheat_death = false;

			/* Can we die ? please let us die ! */
			if (process_hooks_new(HOOK_DIE, NULL, NULL))
			{
				cheat_death = true;
			}

			/* Deus ex machina */
			else if (granted_resurrection())
			{
				cheat_death = true;
				p_ptr->grace = -200000;
				cmsg_format(TERM_L_GREEN,
				            "The power of %s raises you back from the grave!",
				            deity_info[p_ptr->pgod].name);
				msg_print(NULL);
			}

			/* Blood of life */
			else if (p_ptr->allow_one_death > 0)
			{
				cheat_death = true;

				/* Lose one extra life */
				p_ptr->allow_one_death--;

				cmsg_print(TERM_L_GREEN,
				           "You have been saved by the Blood of Life!");
				msg_print(NULL);
			}

			/* Cheat death option */
			else if ((wizard || options->cheat_live) && !get_check("Die? "))
			{
				cheat_death = true;

				/* Mark savefile */
				noscore |= 0x0001;
				msg_print("You invoke wizard mode and cheat death.");
				msg_print(NULL);
			}

			if (cheat_death)
			{
				/* Restore the winner status */
				total_winner = has_won;

				/* One more life spent */
				p_ptr->lives++;

				/* Restore hit points */
				p_ptr->chp = p_ptr->mhp;
				p_ptr->chp_frac = 0;

				/* Heal sanity */
				p_ptr->csane = p_ptr->msane;
				p_ptr->csane_frac = 0;

				/* Restore spell points */
				p_ptr->csp = p_ptr->msp;
				p_ptr->csp_frac = 0;

				/* Hack -- Healing */
				set_blind(0);
				set_confused(0);
				set_poisoned(0);
				set_afraid(0);
				set_paralyzed(0);
				set_image(0);
				set_stun(0);
				set_cut(0);

				/* accounting for a new ailment. -LM- */
				p_ptr->black_breath = false;

				/* Hack -- don't go to undead form */
				p_ptr->necro_extra &= ~CLASS_UNDEAD;

				/* Hack -- Prevent starvation */
				set_food(PY_FOOD_MAX - 1);

				/* Hack -- cancel recall */
				if (p_ptr->word_recall)
				{
					/* Message */
					msg_print("A tension leaves the air around you...");
					msg_print(NULL);

					/* Hack -- Prevent recall */
					p_ptr->word_recall = 0;
				}

				/* Note cause of death */
				game->died_from = "Cheating death";

				/* Do not die */
				death = false;

				/* New depth -KMW- */
				/* dun_level = 0; */
				leaving_quest = 0;
				p_ptr->inside_quest = 0;

				/* Leaving */
				p_ptr->leaving = true;
			}
		}

		/* Handle "death" */
		if (death)
		{
			break;
		}

		/* Mega hack */
		if (dun_level) p_ptr->wild_mode = false;

		/* Make a new level */
		process_hooks_new(HOOK_NEW_LEVEL, NULL, NULL);
		generate_cave();
	}

	/* Close stuff */
	close_game();

	/* Quit */
	quit(NULL);
}

