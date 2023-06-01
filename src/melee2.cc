/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

/*
* This file has several additions to it by Keldon Jones (keldon@umr.edu)
* to improve the general quality of the AI (version 0.1.1).
*/

#include "melee2.hpp"

#include "cave.hpp"
#include "cave_type.hpp"
#include "cmd1.hpp"
#include "config.hpp"
#include "dungeon_flag.hpp"
#include "feature_flag.hpp"
#include "feature_type.hpp"
#include "files.hpp"
#include "game.hpp"
#include "hook_mon_speak_in.hpp"
#include "hook_monster_ai_in.hpp"
#include "hook_monster_ai_out.hpp"
#include "hooks.hpp"
#include "melee1.hpp"
#include "messages.hpp"
#include "monster2.hpp"
#include "monster3.hpp"
#include "monster_race.hpp"
#include "monster_race_flag.hpp"
#include "monster_spell.hpp"
#include "monster_spell_flag.hpp"
#include "monster_type.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "object_flag.hpp"
#include "options.hpp"
#include "player_type.hpp"
#include "skills.hpp"
#include "spells1.hpp"
#include "spells2.hpp"
#include "stats.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "xtra2.hpp"
#include "z-form.hpp"
#include "z-rand.hpp"
#include "z-term.hpp"
#include "z-util.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <cassert>

using boost::algorithm::equals;

#define SPEAK_CHANCE 8
#define GRINDNOISE 20

#define FOLLOW_DISTANCE 6


/*
 * Based on mon_take_hit... all monster attacks on
 * other monsters should use
 */
bool mon_take_hit_mon(int s_idx, int m_idx, int dam, const char *note)
{
	monster_type *m_ptr = &m_list[m_idx], *s_ptr = &m_list[s_idx];

	/* Output */
	auto cmonster_msg = [m_ptr](std::string const &suffix) {
		auto &messages = game->messages;
		// Build monster name
		char m_name[80];
		monster_desc(m_name, m_ptr, 0);
		capitalize(m_name);
		// Add suffix
		auto msg = std::string(m_name);
		msg += suffix;
		// Display
		if (options->disturb_other)
		{
			cmsg_print(TERM_L_RED, msg);
		}
		else
		{
			messages.add(msg, TERM_L_RED);
			p_ptr->window |= PW_MESSAGE;
		}
	};

	/* Redraw (later) if needed */
	if (health_who == m_idx) p_ptr->redraw |= (PR_FRAME);

	/* Some monsters are immune to death */
	auto const r_ptr = m_ptr->race();
	if (r_ptr->flags & RF_NO_DEATH)
	{
		return false;
	}

	/* Wake it up */
	m_ptr->csleep = 0;

	/* Hurt it */
	m_ptr->hp -= dam;

	/* It is dead now... or is it? */
	if (m_ptr->hp < 0)
	{
		if (((r_ptr->flags & RF_UNIQUE) && (m_ptr->status <= MSTATUS_NEUTRAL_P)) ||
		                (m_ptr->mflag & MFLAG_QUEST))
		{
			m_ptr->hp = 1;
		}
		else
		{
			s32b dive = s_ptr->level;

			if (!dive)
			{
				dive = 1;
			}

			/* Death by Missile/Spell attack */
			if (note)
			{
				cmonster_msg(note);
			}
			/* Death by Physical attack -- living monster */
			else if (!m_ptr->ml)
			{
				/* Do nothing */
			}
			/* Death by Physical attack -- non-living monster */
			else if ((r_ptr->flags & RF_DEMON) ||
			                (r_ptr->flags & RF_UNDEAD) ||
			                (r_ptr->flags & RF_STUPID) ||
			                (r_ptr->flags & RF_NONLIVING) ||
			                (strchr("Evg", r_ptr->d_char)))
			{
				cmonster_msg(" is destroyed.");
			}
			else
			{
				cmonster_msg(" is killed.");
			}

			dive = r_ptr->mexp * m_ptr->level / dive;
			if (!dive) dive = 1;

			/* Monster gains some xp */
			monster_gain_exp(s_idx, dive);

			/* Monster lore skill allows gaining xp from pets */
			if (get_skill(SKILL_LORE) && (s_ptr->status >= MSTATUS_PET))
			{
				/* Maximum player level */
				s32b div = p_ptr->max_plv;

				/* Give some experience for the kill */
				s32b new_exp = ((long)r_ptr->mexp * m_ptr->level) / div;

				/* Handle fractional experience */
				s32b new_exp_frac = ((((long)r_ptr->mexp * m_ptr->level) % div)
				                * 0x10000L / div) + p_ptr->exp_frac;

				/* Keep track of experience */
				if (new_exp_frac >= 0x10000L)
				{
					new_exp++;
					p_ptr->exp_frac = new_exp_frac - 0x10000;
				}
				else
				{
					p_ptr->exp_frac = new_exp_frac;
				}

				/*
				 * Factor the xp by the skill level
				 * Note that a score of 50 in the skill makes the gain be 120% of the exp
				 */
				new_exp = new_exp * get_skill_scale(SKILL_LORE, 120) / 100;

				/* Gain experience */
				gain_exp(new_exp);
			}

			/* When an Unique dies, it stays dead */
			if (r_ptr->flags & RF_UNIQUE)
			{
				r_ptr->max_num = 0;
			}

			/* Generate treasure */
			monster_death(m_idx);

			/* Delete the monster */
			delete_monster_idx(m_idx);

			/* Monster is dead */
			return true;
		}

	}

	/* Apply fear */
	mon_handle_fear(m_ptr, dam, nullptr);

	/* Not dead yet */
	return false;
}


void mon_handle_fear(monster_type *m_ptr, int dam, bool *fear)
{
	assert(m_ptr != NULL);

	/* Mega-Hack -- Pain cancels fear */
	if (m_ptr->monfear && (dam > 0))
	{
		int tmp = randint(dam);

		/* Cure a little fear */
		if (tmp < m_ptr->monfear)
		{
			/* Reduce fear */
			m_ptr->monfear -= tmp;
		}

		/* Cure all the fear */
		else
		{
			/* Cure fear */
			m_ptr->monfear = 0;

			/* No more fear */
			if (fear != nullptr)
			{
				(*fear) = false;
			}
		}
	}

	/* Sometimes a monster gets scared by damage */
	auto const r_ptr = m_ptr->race();
	if (!m_ptr->monfear && !(r_ptr->flags & RF_NO_FEAR))
	{
		int percentage;

		/* Percentage of fully healthy */
		percentage = (100L * m_ptr->hp) / m_ptr->maxhp;

		/*
		 * Run (sometimes) if at 10% or less of max hit points,
		 * or (usually) when hit for half its current hit points
		 */
		if (((percentage <= 10) && (rand_int(10) < percentage)) ||
		                ((dam >= m_ptr->hp) && (rand_int(100) < 80)))
		{
			/* Hack -- note fear */
			if (fear != nullptr)
			{
				(*fear) = true;
			}

			/* XXX XXX XXX Hack -- Add some timed fear */
			m_ptr->monfear = (randint(10) +
			                  (((dam >= m_ptr->hp) && (percentage > 7)) ?
			                   20 : ((11 - percentage) * 5)));
		}
	}
}


/*
* And now for Intelligent monster attacks (including spells).
*
* Original idea and code by "DRS" (David Reeves Sward).
* Major modifications by "BEN" (Ben Harrison).
*
* Give monsters more intelligent attack/spell selection based on
* observations of previous attacks on the player, and/or by allowing
* the monster to "cheat" and know the player status.
*
* Maintain an idea of the player status, and use that information
* to occasionally eliminate "ineffective" spell attacks.  We could
* also eliminate ineffective normal attacks, but there is no reason
* for the monster to do this, since he gains no benefit.
* Note that MINDLESS monsters are not allowed to use this code.
* And non-INTELLIGENT monsters only use it partially effectively.
*
* Actually learn what the player resists, and use that information
* to remove attacks or spells before using them.  This will require
* much less space, if I am not mistaken.  Thus, each monster gets a
* set of 32 bit flags, "smart", build from the various "SM_*" flags.
*
* This has the added advantage that attacks and spells are related.
* The "smart_learn" option means that the monster "learns" the flags
* that should be set.
*/



/*
* Internal probability routine
*/
static bool int_outof(std::shared_ptr<monster_race> r_ptr, int prob)
{
	/* Non-Smart monsters are half as "smart" */
	if (!(r_ptr->flags & RF_SMART)) prob = prob / 2;

	/* Roll the dice */
	return (rand_int(100) < prob);
}



/*
 * Remove the "bad" spells from a spell list
 */
static void remove_bad_spells(int m_idx, monster_spell_flag_set *spells_p)
{
	monster_type *m_ptr = &m_list[m_idx];
	u32b smart = 0L;

	// Shorthand
	auto spells(*spells_p);

	/* Too stupid to know anything? */
	auto const r_ptr = m_ptr->race();
	if (r_ptr->flags & RF_STUPID)
	{
		return;
	}

	/* Must be cheating or learning */
	if (!options->smart_learn)
	{
		return;
	}

	/* Hack -- Occasionally forget player status */
	if (m_ptr->smart && magik(1)) m_ptr->smart = 0L;

	/* Use the memorized flags */
	smart = m_ptr->smart;

	/* Nothing known */
	if (!smart) return;


	if (smart & (SM_IMM_ACID))
	{
		if (int_outof(r_ptr, 100)) spells &= ~SF_BR_ACID;
		if (int_outof(r_ptr, 100)) spells &= ~SF_BA_ACID;
		if (int_outof(r_ptr, 100)) spells &= ~SF_BO_ACID;
	}
	else if ((smart & (SM_OPP_ACID)) && (smart & (SM_RES_ACID)))
	{
		if (int_outof(r_ptr, 80)) spells &= ~SF_BR_ACID;
		if (int_outof(r_ptr, 80)) spells &= ~SF_BA_ACID;
		if (int_outof(r_ptr, 80)) spells &= ~SF_BO_ACID;
	}
	else if ((smart & (SM_OPP_ACID)) || (smart & (SM_RES_ACID)))
	{
		if (int_outof(r_ptr, 30)) spells &= ~SF_BR_ACID;
		if (int_outof(r_ptr, 30)) spells &= ~SF_BA_ACID;
		if (int_outof(r_ptr, 30)) spells &= ~SF_BO_ACID;
	}


	if (smart & (SM_IMM_ELEC))
	{
		if (int_outof(r_ptr, 100)) spells &= ~SF_BR_ELEC;
		if (int_outof(r_ptr, 100)) spells &= ~SF_BA_ELEC;
		if (int_outof(r_ptr, 100)) spells &= ~SF_BO_ELEC;
	}
	else if ((smart & (SM_OPP_ELEC)) && (smart & (SM_RES_ELEC)))
	{
		if (int_outof(r_ptr, 80)) spells &= ~SF_BR_ELEC;
		if (int_outof(r_ptr, 80)) spells &= ~SF_BA_ELEC;
		if (int_outof(r_ptr, 80)) spells &= ~SF_BO_ELEC;
	}
	else if ((smart & (SM_OPP_ELEC)) || (smart & (SM_RES_ELEC)))
	{
		if (int_outof(r_ptr, 30)) spells &= ~SF_BR_ELEC;
		if (int_outof(r_ptr, 30)) spells &= ~SF_BA_ELEC;
		if (int_outof(r_ptr, 30)) spells &= ~SF_BO_ELEC;
	}


	if (smart & (SM_IMM_FIRE))
	{
		if (int_outof(r_ptr, 100)) spells &= ~SF_BR_FIRE;
		if (int_outof(r_ptr, 100)) spells &= ~SF_BA_FIRE;
		if (int_outof(r_ptr, 100)) spells &= ~SF_BO_FIRE;
	}
	else if ((smart & (SM_OPP_FIRE)) && (smart & (SM_RES_FIRE)))
	{
		if (int_outof(r_ptr, 80)) spells &= ~SF_BR_FIRE;
		if (int_outof(r_ptr, 80)) spells &= ~SF_BA_FIRE;
		if (int_outof(r_ptr, 80)) spells &= ~SF_BO_FIRE;
	}
	else if ((smart & (SM_OPP_FIRE)) || (smart & (SM_RES_FIRE)))
	{
		if (int_outof(r_ptr, 30)) spells &= ~SF_BR_FIRE;
		if (int_outof(r_ptr, 30)) spells &= ~SF_BA_FIRE;
		if (int_outof(r_ptr, 30)) spells &= ~SF_BO_FIRE;
	}


	if (smart & (SM_IMM_COLD))
	{
		if (int_outof(r_ptr, 100)) spells &= ~SF_BR_COLD;
		if (int_outof(r_ptr, 100)) spells &= ~SF_BA_COLD;
		if (int_outof(r_ptr, 100)) spells &= ~SF_BO_COLD;
		if (int_outof(r_ptr, 100)) spells &= ~SF_BO_ICEE;
	}
	else if ((smart & (SM_OPP_COLD)) && (smart & (SM_RES_COLD)))
	{
		if (int_outof(r_ptr, 80)) spells &= ~SF_BR_COLD;
		if (int_outof(r_ptr, 80)) spells &= ~SF_BA_COLD;
		if (int_outof(r_ptr, 80)) spells &= ~SF_BO_COLD;
		if (int_outof(r_ptr, 80)) spells &= ~SF_BO_ICEE;
	}
	else if ((smart & (SM_OPP_COLD)) || (smart & (SM_RES_COLD)))
	{
		if (int_outof(r_ptr, 30)) spells &= ~SF_BR_COLD;
		if (int_outof(r_ptr, 30)) spells &= ~SF_BA_COLD;
		if (int_outof(r_ptr, 30)) spells &= ~SF_BO_COLD;
		if (int_outof(r_ptr, 30)) spells &= ~SF_BO_ICEE;
	}


	if ((smart & (SM_OPP_POIS)) && (smart & (SM_RES_POIS)))
	{
		if (int_outof(r_ptr, 80)) spells &= ~SF_BR_POIS;
		if (int_outof(r_ptr, 80)) spells &= ~SF_BA_POIS;
		if (int_outof(r_ptr, 40)) spells &= ~SF_BA_NUKE;
		if (int_outof(r_ptr, 40)) spells &= ~SF_BR_NUKE;
	}
	else if ((smart & (SM_OPP_POIS)) || (smart & (SM_RES_POIS)))
	{
		if (int_outof(r_ptr, 30)) spells &= ~SF_BR_POIS;
		if (int_outof(r_ptr, 30)) spells &= ~SF_BA_POIS;
	}


	if (smart & (SM_RES_NETH))
	{
		if (int_outof(r_ptr, 50)) spells &= ~SF_BR_NETH;
		if (int_outof(r_ptr, 50)) spells &= ~SF_BA_NETH;
		if (int_outof(r_ptr, 50)) spells &= ~SF_BO_NETH;
	}

	if (smart & (SM_RES_LITE))
	{
		if (int_outof(r_ptr, 50)) spells &= ~SF_BR_LITE;
	}

	if (smart & (SM_RES_DARK))
	{
		if (int_outof(r_ptr, 50)) spells &= ~SF_BR_DARK;
		if (int_outof(r_ptr, 50)) spells &= ~SF_BA_DARK;
	}

	if (smart & (SM_RES_FEAR))
	{
		if (int_outof(r_ptr, 100)) spells &= ~SF_SCARE;
	}

	if (smart & (SM_RES_CONF))
	{
		if (int_outof(r_ptr, 100)) spells &= ~SF_CONF;
		if (int_outof(r_ptr, 50)) spells &= ~SF_BR_CONF;
	}

	if (smart & (SM_RES_CHAOS))
	{
		if (int_outof(r_ptr, 100)) spells &= ~SF_CONF;
		if (int_outof(r_ptr, 50)) spells &= ~SF_BR_CONF;
		if (int_outof(r_ptr, 50)) spells &= ~SF_BR_CHAO;
		if (int_outof(r_ptr, 50)) spells &= ~SF_BA_CHAO;
	}

	if (smart & (SM_RES_DISEN))
	{
		if (int_outof(r_ptr, 100)) spells &= ~SF_BR_DISE;
	}

	if (smart & (SM_RES_BLIND))
	{
		if (int_outof(r_ptr, 100)) spells &= ~SF_BLIND;
	}

	if (smart & (SM_RES_NEXUS))
	{
		if (int_outof(r_ptr, 50)) spells &= ~SF_BR_NEXU;
		if (int_outof(r_ptr, 50)) spells &= ~SF_TELE_LEVEL;
	}

	if (smart & (SM_RES_SOUND))
	{
		if (int_outof(r_ptr, 50)) spells &= ~SF_BR_SOUN;
	}

	if (smart & (SM_RES_SHARD))
	{
		if (int_outof(r_ptr, 50)) spells &= ~SF_BR_SHAR;
		if (int_outof(r_ptr, 20)) spells &= ~SF_ROCKET;
	}

	if (smart & (SM_IMM_REFLECT))
	{
		if (int_outof(r_ptr, 100)) spells &= ~SF_BO_COLD;
		if (int_outof(r_ptr, 100)) spells &= ~SF_BO_FIRE;
		if (int_outof(r_ptr, 100)) spells &= ~SF_BO_ACID;
		if (int_outof(r_ptr, 100)) spells &= ~SF_BO_ELEC;
		if (int_outof(r_ptr, 100)) spells &= ~SF_BO_POIS;
		if (int_outof(r_ptr, 100)) spells &= ~SF_BO_NETH;
		if (int_outof(r_ptr, 100)) spells &= ~SF_BO_WATE;
		if (int_outof(r_ptr, 100)) spells &= ~SF_BO_MANA;
		if (int_outof(r_ptr, 100)) spells &= ~SF_BO_PLAS;
		if (int_outof(r_ptr, 100)) spells &= ~SF_BO_ICEE;
		if (int_outof(r_ptr, 100)) spells &= ~SF_MISSILE;
		if (int_outof(r_ptr, 100)) spells &= ~(SF_ARROW_1);
		if (int_outof(r_ptr, 100)) spells &= ~(SF_ARROW_2);
		if (int_outof(r_ptr, 100)) spells &= ~(SF_ARROW_3);
		if (int_outof(r_ptr, 100)) spells &= ~(SF_ARROW_4);
	}

	if (smart & (SM_IMM_FREE))
	{
		if (int_outof(r_ptr, 100)) spells &= ~SF_HOLD;
		if (int_outof(r_ptr, 100)) spells &= ~SF_SLOW;
	}

	if (smart & (SM_IMM_MANA))
	{
		if (int_outof(r_ptr, 100)) spells &= ~SF_DRAIN_MANA;
	}

	/* XXX XXX XXX No spells left? */
	/* if (!f4 && !f5 && !f6) ... */

	*spells_p = spells;
}


/*
 * Determine if there is a space near the player in which
 * a summoned creature can appear
 */
static bool summon_possible(int y1, int x1)
{
	int y, x;

	/* Start at the player's location, and check 2 grids in each dir */
	for (y = y1 - 2; y <= y1 + 2; y++)
	{
		for (x = x1 - 2; x <= x1 + 2; x++)
		{
			/* Ignore illegal locations */
			if (!in_bounds(y, x)) continue;

			/* Only check a circular area */
			if (distance(y1, x1, y, x) > 2) continue;

			/* Hack: no summon on glyph of warding */
			if (cave[y][x].feat == FEAT_GLYPH) continue;
			if (cave[y][x].feat == FEAT_MINOR_GLYPH) continue;

			/* Nor on the between */
			if (cave[y][x].feat == FEAT_BETWEEN) return false;

			/* Require empty floor grid in line of sight */
			if (cave_empty_bold(y, x) && los(y1, x1, y, x)) return true;
		}
	}

	return false;
}



/*
 * Determine if a bolt spell will hit the player.
 *
 * This is exactly like "projectable", but it will return false if a monster
 * is in the way.
 */
static bool clean_shot(int y1, int x1, int y2, int x2)
{
	int dist, y, x;

	/* Start at the initial location */
	y = y1, x = x1;

	/* See "project()" and "projectable()" */
	for (dist = 0; dist <= MAX_RANGE; dist++)
	{
		/* Never pass through walls */
		if (dist && (!cave_sight_bold(y, x) || !cave_floor_bold(y, x))) break;

		/* Never pass through monsters */
		if (dist && cave[y][x].m_idx > 0)
		{
			if (is_friend(&m_list[cave[y][x].m_idx]) < 0) break;
		}

		/* Check for arrival at "final target" */
		if ((x == x2) && (y == y2)) return true;

		/* Calculate the new location */
		mmove2(&y, &x, y1, x1, y2, x2);
	}

	/* Assume obstruction */
	return false;
}


/*
 * Cast a bolt at the player
 * Stop if we hit a monster
 * Affect monsters and the player
 */
static void bolt(int m_idx, int typ, int dam_hp)
{
	int flg = PROJECT_STOP | PROJECT_KILL;

	/* Target the player with a bolt attack */
	project(m_idx, 0, p_ptr->py, p_ptr->px, dam_hp, typ, flg);
}


/*
 * Calculate the mask for "bolt" spells
 */
static monster_spell_flag_set compute_bolt_mask()
{
	monster_spell_flag_set flags;
	for (auto const &monster_spell: monster_spells())
	{
		if (monster_spell->is_bolt)
		{
			flags |= monster_spell->flag_set;
		}
	}
	return flags;
}


/*
 * Calculate mask for summoning spells
 */
static monster_spell_flag_set compute_summoning_mask()
{
	monster_spell_flag_set flags;
	for (auto const &monster_spell: monster_spells())
	{
		if (monster_spell->is_summon)
		{
			flags |= monster_spell->flag_set;
		}
	}
	return flags;
}


/*
 * Calculate mask for spells requiring SMART flag
 */
static monster_spell_flag_set compute_smart_mask()
{
	monster_spell_flag_set flags;
	for (auto const &monster_spell: monster_spells())
	{
		if (monster_spell->is_smart)
		{
			flags |= monster_spell->flag_set;
		}
	}
	return flags;
}


/*
 * Calculate mask for spells requiring SMART flag
 */
static monster_spell_flag_set compute_innate_mask()
{
	monster_spell_flag_set flags;
	for (auto const &monster_spell: monster_spells())
	{
		if (monster_spell->is_innate)
		{
			flags |= monster_spell->flag_set;
		}
	}
	return flags;
}


/*
 * Have a monster choose a spell from a list of "useful" spells.
 *
 * Note that this list does NOT include spells that will just hit
 * other monsters, and the list is restricted when the monster is
 * "desperate".  Should that be the job of this function instead?
 *
 * Stupid monsters will just pick a spell randomly.  Smart monsters
 * will choose more "intelligently".
 *
 * Use the helper functions above to put spells into categories.
 *
 * This function may well be an efficiency bottleneck.
 */
static monster_spell const *choose_attack_spell(int m_idx, std::vector<monster_spell const *> const &spells)
{
	monster_type *m_ptr = &m_list[m_idx];

	/* Stupid monsters choose randomly */
	auto const r_ptr = m_ptr->race();
	if (r_ptr->flags & RF_STUPID)
	{
		/* Pick at random */
		return spells[rand_int(spells.size())];
	}

	/* Spells by category */
	std::vector<monster_spell const *> escape; escape.reserve(spells.size());
	std::vector<monster_spell const *> attack; attack.reserve(spells.size());
	std::vector<monster_spell const *> summon; summon.reserve(spells.size());
	std::vector<monster_spell const *> tactic; tactic.reserve(spells.size());
	std::vector<monster_spell const *> annoy ;  annoy.reserve(spells.size());
	std::vector<monster_spell const *> haste ;  haste.reserve(spells.size());
	std::vector<monster_spell const *> heal  ;   heal.reserve(spells.size());

	/* Categorize spells */
	for (std::size_t i = 0; i < spells.size(); i++)
	{
		/* Escape spell? */
		if (spells[i]->is_escape)
		{
			escape.push_back(spells[i]);
		}

		/* Attack spell? */
		if (spells[i]->is_damage)
		{
			attack.push_back(spells[i]);
		}

		/* Summon spell? */
		if (spells[i]->is_summon)
		{
			summon.push_back(spells[i]);
		}

		/* Tactical spell? */
		if (spells[i]->is_tactic)
		{
			tactic.push_back(spells[i]);
		}

		/* Annoyance spell? */
		if (spells[i]->is_annoy)
		{
			annoy.push_back(spells[i]);
		}

		/* Haste spell? */
		if (spells[i]->is_haste)
		{
			haste.push_back(spells[i]);
		}

		/* Heal spell? */
		if (spells[i]->is_heal)
		{
			heal.push_back(spells[i]);
		}
	}

	/*** Try to pick an appropriate spell type ***/

	/* Hurt badly or afraid, attempt to flee */
	if ((m_ptr->hp < m_ptr->maxhp / 3) || m_ptr->monfear)
	{
		if (!escape.empty()) return escape[rand_int(escape.size())];
	}

	/* Still hurt badly, couldn't flee, attempt to heal */
	if (m_ptr->hp < m_ptr->maxhp / 3)
	{
		if (!heal.empty()) return heal[rand_int(heal.size())];
	}

	/* Player is close and we have attack spells, blink away */
	if ((distance(p_ptr->py, p_ptr->px, m_ptr->fy, m_ptr->fx) < 4) && !attack.empty() && (rand_int(100) < 75))
	{
		if (!tactic.empty()) return tactic[rand_int(tactic.size())];
	}

	/* We're hurt (not badly), try to heal */
	if ((m_ptr->hp < m_ptr->maxhp * 3 / 4) && (rand_int(100) < 75))
	{
		if (!heal.empty()) return heal[rand_int(heal.size())];
	}

	/* Summon if possible (sometimes) */
	if (!summon.empty() && (rand_int(100) < 50))
	{
		return summon[rand_int(summon.size())];
	}

	/* Attack spell (most of the time) */
	if (!attack.empty() && (rand_int(100) < 85))
	{
		return attack[rand_int(attack.size())];
	}

	/* Try another tactical spell (sometimes) */
	if (!tactic.empty() && (rand_int(100) < 50))
	{
		return tactic[rand_int(tactic.size())];
	}

	/* Haste self if we aren't already somewhat hasted (rarely) */
	if (!haste.empty() && (rand_int(100) < (20 + m_ptr->speed - m_ptr->mspeed)))
	{
		return haste[rand_int(haste.size())];
	}

	/* Annoy player (most of the time) */
	if (!annoy.empty() && (rand_int(100) < 85))
	{
		return annoy[rand_int(annoy.size())];
	}

	/* Choose no spell */
	return nullptr;
}


/*
 * Cast a breath (or ball) attack at the player
 * Pass over any monsters that may be in the way
 * Affect grids, objects, monsters, and the player
 */
static void breath(int m_idx, int typ, int dam_hp, int rad)
{
	int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

	monster_type *m_ptr = &m_list[m_idx];
	auto const r_ptr = m_ptr->race();

	/* Determine the radius of the blast */
	if (rad < 1) rad = (r_ptr->flags & RF_POWERFUL) ? 3 : 2;

	/* Target the player with a ball attack */
	project(m_idx, rad, p_ptr->py, p_ptr->px, dam_hp, typ, flg);
}


/*
 * Monster casts a breath (or ball) attack at another monster.
 * Pass over any monsters that may be in the way
 * Affect grids, objects, monsters, and the player
 */
static void monst_breath_monst(int m_idx, int y, int x, int typ, int dam_hp, int rad)
{
	int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

	monster_type *m_ptr = &m_list[m_idx];
	auto const r_ptr = m_ptr->race();

	/* Determine the radius of the blast */
	if (rad < 1) rad = (r_ptr->flags & RF_POWERFUL) ? 3 : 2;

	project(m_idx, rad, y, x, dam_hp, typ, flg);
}


/*
 * Monster casts a bolt at another monster
 * Stop if we hit a monster
 * Affect monsters and the player
 */
static void monst_bolt_monst(int m_idx, int y, int x, int typ, int dam_hp)
{
	int flg = PROJECT_STOP | PROJECT_KILL;

	project(m_idx, 0, y, x, dam_hp, typ, flg);
}


static void monster_msg(const char *fmt, ...)
{
	va_list vp;

	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args, save the length */
	vstrnfmt(buf, 1024, fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Print */
	monster_msg_simple(buf);
}

void monster_msg_simple(const char *s)
{
	auto &messages = game->messages;

	/* Display */
	if (options->disturb_other)
	{
		msg_print(s);
	}
	else
	{
		messages.add(s, TERM_WHITE);
		p_ptr->window |= PW_MESSAGE;
	}
}

/**
 * Extract list of spell indexes from a flag set.
 */
static std::vector<monster_spell const *> extract_spells(monster_spell_flag_set const &spell_flag_set)
{
	auto result = std::vector<monster_spell const *>();
	result.reserve(spell_flag_set.nbits);

	for (auto const &monster_spell: monster_spells())
	{
		if (bool(spell_flag_set & monster_spell->flag_set))
		{
			result.push_back(monster_spell);
		}
	}

	return result;
}

/*
 * Monster tries to 'cast a spell' (or breath, etc)
 * at another monster.
 */
int monst_spell_monst_spell = -1;

static bool monst_spell_monst(int m_idx)
{
	static const monster_spell_flag_set SF_INT_MASK = compute_smart_mask();

	auto const &dungeon_flags = game->dungeon_flags;

	monster_type *m_ptr = &m_list[m_idx];    /* Attacker */
	bool wake_up = false;

	/* Cannot cast spells when confused */
	if (m_ptr->confused)
	{
		return false;
	}

	// Shorthand and/or optimization
	auto const blind = p_ptr->blind;
	auto const seen = (!blind && m_ptr->ml);
	auto const friendly = (is_friend(m_ptr) > 0);

	/* Hack -- Extract the spell probability */
	const auto r_ptr = m_ptr->race();
	const int chance = (r_ptr->freq_inate + r_ptr->freq_spell) / 2;

	/* Not allowed to cast spells */
	if ((!chance) && (monst_spell_monst_spell == -1))
	{
		return false;
	}

	if ((rand_int(100) >= chance) && (monst_spell_monst_spell == -1))
	{
		return false;
	}

	/* Make sure monster actually has a target */
	if (m_ptr->target <= 0)
	{
		return false;
	}

	int t_idx = m_ptr->target;

	monster_type *t_ptr = &m_list[t_idx];
	auto const tr_ptr = t_ptr->race();

	/* Hack -- no fighting >100 squares from player */
	if (t_ptr->cdis > MAX_RANGE)
	{
		return false;
	}

	/* Monster must be projectable */
	if (!projectable(m_ptr->fy, m_ptr->fx, t_ptr->fy, t_ptr->fx))
	{
		return false;
	}

	/* OK -- we-ve got a target */
	int const y = t_ptr->fy;
	int const x = t_ptr->fx;

	/* Extract the monster level */
	const int rlev = ((m_ptr->level >= 1) ? m_ptr->level : 1);

	/* Which spells are allowed? */
	monster_spell_flag_set allowed_spells = r_ptr->spells;

	/* Hack -- allow "desperate" spells */
	if ((r_ptr->flags & RF_SMART) &&
			(m_ptr->hp < m_ptr->maxhp / 10) &&
			(rand_int(100) < 50))
	{
		/* Require intelligent spells */
		allowed_spells &= SF_INT_MASK;

		/* No spells left? */
		if ((!allowed_spells) && (monst_spell_monst_spell == -1))
		{
			return false;
		}
	}

	/* Extract spells */
	auto spell = extract_spells(allowed_spells);

	/* No spells left? */
	if (spell.empty())
	{
		return false;
	}

	/* Stop if player is dead or gone */
	if (!alive || death)
	{
		return false;
	}

	/* Handle "leaving" */
	if (p_ptr->leaving)
	{
		return false;
	}

	/* Get the monster name (or "it") */
	char m_name[80];
	monster_desc(m_name, m_ptr, 0x00);

	/* Get the monster possessive ("his"/"her"/"its") */
	char m_poss[80];
	monster_desc(m_poss, m_ptr, 0x22);

	/* Get the target's name (or "it") */
	char t_name[80];
	monster_desc(t_name, t_ptr, 0x00);

	/* Hack -- Get the "died from" name */
	char ddesc[80];
	monster_desc(ddesc, m_ptr, 0x88);

	/* Choose a spell to cast */
	auto thrown_spell = spell[rand_int(spell.size())];

	/* Force a spell ? */
	if (monst_spell_monst_spell > -1)
	{
		thrown_spell = spell[monst_spell_monst_spell];
		monst_spell_monst_spell = -1;
	}

	auto const see_m = seen;
	auto const see_t = !blind && t_ptr->ml;
	auto const see_either = see_m || see_t;
	auto const see_both = see_m && see_t;

	/* Do a breath */
	auto do_breath = [&](char const *element, int gf, s32b max, int divisor) -> void {
		// Interrupt
		disturb_on_other();
		// Message
		if (!see_either)
		{
			monster_msg("You hear breathing noise.");
		}
		else if (blind)
		{
			monster_msg("%^s breathes.", m_name);
		}
		else
		{
			monster_msg("%^s breathes %s at %s.", m_name, element, t_name);
		}
		// Breathe
		monst_breath_monst(m_idx, y, x, gf, std::min(max, m_ptr->hp / divisor), 0);
	};

	/* Messages for summoning */
	struct summon_messages {
		char const *singular;
		char const *plural;
	};

	/* Default message for summoning when player is blinded */
	auto blind_msg_default = summon_messages {
		"You hear something appear nearby.",
		"You hear many things appear nearby."
	};

	/* Do a summoning spell */
	auto do_summon = [&](char const *action, int n, int friendly_type, int hostile_type, summon_messages const &blind_msg) -> void {
		// Interrupt
		disturb_on_other();

		// Message
		if (blind || !see_m)
		{
			monster_msg("%^s mumbles.", m_name);
		}
		else
		{
			monster_msg("%^s magically %s", m_name, action);
		}

		// Do the actual summoning
		int count = 0;
		for (int k = 0; k < n; k++)
		{
			if (friendly)
			{
				count += summon_specific_friendly(m_ptr->fy, m_ptr->fx, rlev, friendly_type, true);
			}
			else if (!friendly)
			{
				count += summon_specific(m_ptr->fy, m_ptr->fx, rlev, hostile_type);
			}
		}
		// Message for blinded characters
		if (blind)
		{
			if (count == 1)
			{
				monster_msg(blind_msg.singular);
			}
			else if (count > 1)
			{
				monster_msg(blind_msg.plural);
			}
		}
	};

	/* There's no summoning friendly uniques or Nazgul */
	auto spell_idx = thrown_spell->spell_idx;

	if (friendly)
	{
		if ((thrown_spell->spell_idx == SF_S_UNIQUE_IDX) &&
			(thrown_spell->spell_idx == SF_S_WRAITH_IDX))
		{
			// Summon high undead instead
			spell_idx = SF_S_HI_UNDEAD_IDX;
		}
	}

	/* Spell effect */
	switch (spell_idx)
	{
	case SF_SHRIEK_IDX:
		{
			disturb_on_other();
			if (!see_m) monster_msg("You hear a shriek.");
			else monster_msg("%^s shrieks at %s.", m_name, t_name);
			wake_up = true;
			break;
		}

	case SF_MULTIPLY_IDX:
		{
			break;
		}

	case SF_ROCKET_IDX:
		{
			disturb_on_other();
			if (!see_either) monster_msg("You hear an explosion!");
			else if (blind) monster_msg("%^s shoots something.", m_name);
			else monster_msg("%^s fires a rocket at %s.", m_name, t_name);
			monst_breath_monst(m_idx, y, x, GF_ROCKET,
					   ((m_ptr->hp / 4) > 800 ? 800 : (m_ptr->hp / 4)), 2);
			break;
		}

	case SF_ARROW_1_IDX:
		{
			disturb_on_other();
			if (!see_either) monster_msg("You hear a strange noise.");
			else if (blind) monster_msg("%^s makes a strange noise.", m_name);
			else monster_msg("%^s fires an arrow at %s.", m_name, t_name);
			monst_bolt_monst(m_idx, y, x, GF_ARROW, damroll(1, 6));
			break;
		}

	case SF_ARROW_2_IDX:
		{
			disturb_on_other();
			if (!see_either) monster_msg("You hear a strange noise.");
			else if (blind) monster_msg("%^s makes a strange noise.", m_name);
			else monster_msg("%^s fires an arrow at %s.", m_name, t_name);
			monst_bolt_monst(m_idx, y, x, GF_ARROW, damroll(3, 6));
			break;
		}

	case SF_ARROW_3_IDX:
		{
			disturb_on_other();

			if (!see_either) monster_msg("You hear a strange noise.");
			else if (blind) monster_msg("%^s makes a strange noise.", m_name);
			else monster_msg("%^s fires a missile at %s.", m_name, t_name);
			monst_bolt_monst(m_idx, y, x, GF_ARROW, damroll(5, 6));
			break;
		}

	case SF_ARROW_4_IDX:
		{
			if (!see_either) monster_msg("You hear a strange noise.");
			else disturb_on_other();
			if (blind) monster_msg("%^s makes a strange noise.", m_name);
			else monster_msg("%^s fires a missile at %s.", m_name, t_name);
			monst_bolt_monst(m_idx, y, x, GF_ARROW, damroll(7, 6));
			break;
		}

	case SF_BR_ACID_IDX:
		{
			do_breath("acid", GF_ACID, 1600, 3);
			break;
		}

	case SF_BR_ELEC_IDX:
		{
			do_breath("lightning", GF_ELEC, 1600, 3);
			break;
		}

	case SF_BR_FIRE_IDX:
		{
			do_breath("fire", GF_FIRE, 1600, 3);
			break;
		}

	case SF_BR_COLD_IDX:
		{
			do_breath("frost", GF_COLD, 1600, 3);
			break;
		}

	case SF_BR_POIS_IDX:
		{
			do_breath("gas", GF_POIS, 800, 3);
			break;
		}

	case SF_BR_NETH_IDX:
		{
			do_breath("nether", GF_NETHER, 550, 6);
			break;
		}

	case SF_BR_LITE_IDX:
		{
			do_breath("light", GF_LITE, 400, 6);
			break;
		}

	case SF_BR_DARK_IDX:
		{
			do_breath("darkness", GF_DARK, 400, 6);
			break;
		}

	case SF_BR_CONF_IDX:
		{
			do_breath("confusion", GF_CONFUSION, 400, 6);
			break;
		}

	case SF_BR_SOUN_IDX:
		{
			do_breath("sound", GF_SOUND, 400, 6);
			break;
		}

	case SF_BR_CHAO_IDX:
		{
			do_breath("chaos", GF_CHAOS, 600, 6);
			break;
		}

	case SF_BR_DISE_IDX:
		{
			do_breath("disenchantment", GF_DISENCHANT, 500, 6);
			break;
		}

	case SF_BR_NEXU_IDX:
		{
			do_breath("nexus", GF_NEXUS, 250, 3);
			break;
		}

	case SF_BR_TIME_IDX:
		{
			do_breath("time", GF_TIME, 150, 3);
			break;
		}

	case SF_BR_INER_IDX:
		{
			do_breath("inertia", GF_INERTIA, 200, 6);
			break;
		}

	case SF_BR_GRAV_IDX:
		{
			do_breath("gravity", GF_GRAVITY, 200, 3);
			break;
		}

	case SF_BR_SHAR_IDX:
		{
			do_breath("shards", GF_SHARDS, 400, 6);
			break;
		}

	case SF_BR_PLAS_IDX:
		{
			do_breath("plasma", GF_PLASMA, 150, 6);
			break;
		}

	case SF_BR_WALL_IDX:
		{
			do_breath("force", GF_FORCE, 200, 6);
			break;
		}

	case SF_BR_MANA_IDX:
		{
			do_breath("magical energy", GF_MANA, 250, 3);
			break;
		}

	case SF_BA_NUKE_IDX:
		{
			disturb_on_other();
			if (!see_either) monster_msg("You hear someone mumble.");
			else if (blind) monster_msg("%^s mumbles.", m_name);
			else monster_msg("%^s casts a ball of radiation at %s.", m_name, t_name);
			monst_breath_monst(m_idx, y, x, GF_NUKE,
					   (rlev + damroll(10, 6)), 2);
			break;
		}

	case SF_BR_NUKE_IDX:
		{
			do_breath("toxic waste", GF_NUKE, 800, 3);
			break;
		}

	case SF_BA_CHAO_IDX:
		{
			disturb_on_other();
			if (!see_either) monster_msg("You hear someone mumble frighteningly.");
			else if (blind) monster_msg("%^s mumbles frighteningly.", m_name);
			else monster_msg("%^s invokes a raw Chaos upon %s.", m_name, t_name);
			monst_breath_monst(m_idx, y, x, GF_CHAOS,
					   (rlev * 2) + damroll(10, 10), 4);
			break;
		}

	case SF_BR_DISI_IDX:
		{
			do_breath("disintegration", GF_DISINTEGRATE, 300, 3);
			break;
		}

	case SF_BA_ACID_IDX:
		{
			disturb_on_other();
			if (!see_either) monster_msg ("You hear someone mumble.");
			else if (blind) monster_msg("%^s mumbles.", m_name);
			else monster_msg("%^s casts an acid ball at %s.", m_name, t_name);
			monst_breath_monst(m_idx, y, x, GF_ACID, randint(rlev * 3) + 15, 2);
			break;
		}

	case SF_BA_ELEC_IDX:
		{
			disturb_on_other();
			if (!see_either) monster_msg ("You hear someone mumble.");
			else
				if (blind) monster_msg("%^s mumbles.", m_name);
				else monster_msg("%^s casts a lightning ball at %s.", m_name, t_name);
			monst_breath_monst(m_idx, y, x, GF_ELEC, randint(rlev * 3 / 2) + 8, 2);
			break;
		}

	case SF_BA_FIRE_IDX:
		{
			disturb_on_other();
			if (!see_either) monster_msg ("You hear someone mumble.");
			else
				if (blind) monster_msg("%^s mumbles.", m_name);
				else monster_msg("%^s casts a fire ball at %s.", m_name, t_name);
			monst_breath_monst(m_idx, y, x, GF_FIRE, randint(rlev * 7 / 2) + 10, 2);
			break;
		}

	case SF_BA_COLD_IDX:
		{
			disturb_on_other();
			if (!see_either) monster_msg ("You hear someone mumble.");
			else
				if (blind) monster_msg("%^s mumbles.", m_name);
				else monster_msg("%^s casts a frost ball at %s.", m_name, t_name);
			monst_breath_monst(m_idx, y, x, GF_COLD, randint(rlev * 3 / 2) + 10, 2);
			break;
		}

	case SF_BA_POIS_IDX:
		{
			disturb_on_other();
			if (!see_either) monster_msg ("You hear someone mumble.");
			else
				if (blind) monster_msg("%^s mumbles.", m_name);
				else monster_msg("%^s casts a stinking cloud at %s.", m_name, t_name);
			monst_breath_monst(m_idx, y, x, GF_POIS, damroll(12, 2), 2);
			break;
		}

	case SF_BA_NETH_IDX:
		{
			disturb_on_other();
			if (!see_either) monster_msg ("You hear someone mumble.");
			else
				if (blind) monster_msg("%^s mumbles.", m_name);
				else monster_msg("%^s casts a nether ball at %s.", m_name, t_name);
			monst_breath_monst(m_idx, y, x, GF_NETHER, (50 + damroll(10, 10) + rlev), 2);
			break;
		}

	case SF_BA_WATE_IDX:
		{
			disturb_on_other();
			if (!see_either) monster_msg ("You hear someone mumble.");
			else
				if (blind) monster_msg("%^s mumbles.", m_name);
				else monster_msg("%^s gestures fluidly at %s.", m_name, t_name);
			monster_msg("%^s is engulfed in a whirlpool.", t_name);
			monst_breath_monst(m_idx, y, x, GF_WATER, randint(rlev * 5 / 2) + 50, 4);
			break;
		}

	case SF_BA_MANA_IDX:
		{
			disturb_on_other();
			if (!see_either) monster_msg ("You hear someone mumble powerfully.");
			else
				if (blind) monster_msg("%^s mumbles powerfully.", m_name);
				else monster_msg("%^s invokes a mana storm upon %s.", m_name, t_name);
			monst_breath_monst(m_idx, y, x, GF_MANA, (rlev * 5) + damroll(10, 10), 4);
			break;
		}

	case SF_BA_DARK_IDX:
		{
			disturb_on_other();
			if (!see_either) monster_msg ("You hear someone mumble powerfully.");
			else
				if (blind) monster_msg("%^s mumbles powerfully.", m_name);
				else monster_msg("%^s invokes a darkness storm upon %s.", m_name, t_name);
			monst_breath_monst(m_idx, y, x, GF_DARK, (rlev * 5) + damroll(10, 10), 4);
			break;
		}

	case SF_DRAIN_MANA_IDX:
		{
			/* Attack power */
			int r1 = (randint(rlev) / 2) + 1;

			if (see_m)
			{
				/* Basic message */
				monster_msg("%^s draws psychic energy from %s.", m_name, t_name);
			}

			/* Heal the monster */
			if (m_ptr->hp < m_ptr->maxhp)
			{
				if (!tr_ptr->spells)
				{
					if (see_both)
						monster_msg("%^s is unaffected!", t_name);
				}
				else
				{
					/* Heal */
					m_ptr->hp += (6 * r1);
					if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

					/* Redraw (later) if needed */
					if (health_who == m_idx) p_ptr->redraw |= (PR_FRAME);

					/* Special message */
					if (seen)
					{
						monster_msg("%^s appears healthier.", m_name);
					}
				}
			}

			wake_up = true;
			break;
		}

	case SF_MIND_BLAST_IDX:
		{
			disturb_on_other();

			if (!seen)
			{
				/* */
			}
			else
			{
				monster_msg("%^s gazes intently at %s.", m_name, t_name);
			}

			/* Attempt a saving throw */
			if ((tr_ptr->flags & RF_UNIQUE) ||
					(tr_ptr->flags & RF_NO_CONF) ||
					(t_ptr->level > randint((rlev - 10) < 1 ? 1 : (rlev - 10)) + 10))
			{
				/* No obvious effect */
				if (see_t)
				{
					monster_msg("%^s is unaffected!", t_name);
				}
			}
			else
			{
				monster_msg("%^s is blasted by psionic energy.", t_name);
				t_ptr->confused += rand_int(4) + 4;

				mon_take_hit_mon(m_idx, t_idx, damroll(8, 8), " collapses, a mindless husk.");
			}

			wake_up = true;
			break;
		}

	case SF_BRAIN_SMASH_IDX:
		{
			disturb_on_other();
			if (!seen)
			{
				/* */
			}
			else
			{
				monster_msg("%^s gazes intently at %s.", m_name, t_name);
			}

			/* Attempt a saving throw */
			if ((tr_ptr->flags & RF_UNIQUE) ||
					(tr_ptr->flags & RF_NO_CONF) ||
					(t_ptr->level > randint((rlev - 10) < 1 ? 1 : (rlev - 10)) + 10))
			{
				/* No obvious effect */
				if (see_t)
				{
					monster_msg("%^s is unaffected!", t_name);
				}
			}
			else
			{
				if (see_t)
				{
					monster_msg("%^s is blasted by psionic energy.", t_name);
				}
				t_ptr->confused += rand_int(4) + 4;
				t_ptr->mspeed -= rand_int(4) + 4;
				t_ptr->stunned += rand_int(4) + 4;
				mon_take_hit_mon(m_idx, t_idx, damroll(12, 15), " collapses, a mindless husk.");
			}
			wake_up = true;
			break;
		}

	case SF_CAUSE_1_IDX:
		{
			disturb_on_other();
			if (blind || !see_m) monster_msg("%^s mumbles.", m_name);
			else monster_msg("%^s points at %s and curses.", m_name, t_name);
			if (t_ptr->level > randint((rlev - 10) < 1 ? 1 : (rlev - 10)) + 10)
			{

				if (see_t) monster_msg("%^s resists!", t_name);
			}
			else
			{
				mon_take_hit_mon(m_idx, t_idx, damroll(3, 8), " is destroyed.");
			}
			wake_up = true;
			break;
		}

	case SF_CAUSE_2_IDX:
		{
			disturb_on_other();
			if (blind || !see_m) monster_msg("%^s mumbles.", m_name);
			else monster_msg("%^s points at %s and curses horribly.", m_name, t_name);
			if (t_ptr->level > randint((rlev - 10) < 1 ? 1 : (rlev - 10)) + 10)
			{
				if (see_t) monster_msg("%^s resists!", t_name);
			}
			else
			{
				mon_take_hit_mon(m_idx, t_idx, damroll(8, 8), " is destroyed.");
			}
			wake_up = true;
			break;
		}

	case SF_CAUSE_3_IDX:
		{
			disturb_on_other();
			if (blind || !see_m) monster_msg("%^s mumbles.", m_name);
			else monster_msg("%^s points at %s, incanting terribly!", m_name, t_name);
			if (t_ptr->level > randint((rlev - 10) < 1 ? 1 : (rlev - 10)) + 10)
			{
				if (see_t) monster_msg("%^s resists!", t_name);
			}
			else
			{
				mon_take_hit_mon(m_idx, t_idx, damroll(10, 15), " is destroyed.");
			}
			wake_up = true;
			break;
		}

	case SF_CAUSE_4_IDX:
		{
			disturb_on_other();
			if (blind || !see_m) monster_msg("%^s mumbles.", m_name);
			else monster_msg("%^s points at %s, screaming the word 'DIE!'", m_name, t_name);
			if (t_ptr->level > randint((rlev - 10) < 1 ? 1 : (rlev - 10)) + 10)
			{
				if (see_t) monster_msg("%^s resists!", t_name);
			}
			else
			{
				mon_take_hit_mon(m_idx, t_idx, damroll(15, 15), " is destroyed.");
			}
			wake_up = true;
			break;
		}

	case SF_BO_ACID_IDX:
		{
			disturb_on_other();
			if (blind || !see_m) monster_msg("%^s mumbles.", m_name);
			else monster_msg("%^s casts an acid bolt at %s.", m_name, t_name);
			monst_bolt_monst(m_idx, y, x, GF_ACID,
					 damroll(7, 8) + (rlev / 3));
			break;
		}

	case SF_BO_ELEC_IDX:
		{
			disturb_on_other();
			if (blind || !see_m) monster_msg("%^s mumbles.", m_name);
			else monster_msg("%^s casts a lightning bolt at %s.", m_name, t_name);
			monst_bolt_monst(m_idx, y, x, GF_ELEC,
					 damroll(4, 8) + (rlev / 3));
			break;
		}

	case SF_BO_FIRE_IDX:
		{
			disturb_on_other();
			if (blind || !see_m) monster_msg("%^s mumbles.", m_name);
			else monster_msg("%^s casts a fire bolt at %s.", m_name, t_name);
			monst_bolt_monst(m_idx, y, x, GF_FIRE,
					 damroll(9, 8) + (rlev / 3));
			break;
		}

	case SF_BO_COLD_IDX:
		{
			disturb_on_other();
			if (blind || !see_m) monster_msg("%^s mumbles.", m_name);
			else monster_msg("%^s casts a frost bolt at %s.", m_name, t_name);
			monst_bolt_monst(m_idx, y, x, GF_COLD,
					 damroll(6, 8) + (rlev / 3));
			break;
		}

	case SF_BO_POIS_IDX:
		{
			/* XXX XXX XXX */
			break;
		}

	case SF_BO_NETH_IDX:
		{
			disturb_on_other();
			if (blind || !see_m) monster_msg("%^s mumbles.", m_name);
			else monster_msg("%^s casts a nether bolt at %s.", m_name, t_name);
			monst_bolt_monst(m_idx, y, x, GF_NETHER,
					 30 + damroll(5, 5) + (rlev * 3) / 2);
			break;
		}

	case SF_BO_WATE_IDX:
		{
			disturb_on_other();
			if (blind || !see_m) monster_msg("%^s mumbles.", m_name);
			else monster_msg("%^s casts a water bolt at %s.", m_name, t_name);
			monst_bolt_monst(m_idx, y, x, GF_WATER,
					 damroll(10, 10) + (rlev));
			break;
		}

	case SF_BO_MANA_IDX:
		{
			disturb_on_other();
			if (blind || !see_m) monster_msg("%^s mumbles.", m_name);
			else monster_msg("%^s casts a mana bolt at %s.", m_name, t_name);
			monst_bolt_monst(m_idx, y, x, GF_MANA,
					 randint(rlev * 7 / 2) + 50);
			break;
		}

	case SF_BO_PLAS_IDX:
		{
			disturb_on_other();
			if (blind || !see_m) monster_msg("%^s mumbles.", m_name);
			else monster_msg("%^s casts a plasma bolt at %s.", m_name, t_name);
			monst_bolt_monst(m_idx, y, x, GF_PLASMA,
					 10 + damroll(8, 7) + (rlev));
			break;
		}

	case SF_BO_ICEE_IDX:
		{
			disturb_on_other();
			if (blind || !see_m) monster_msg("%^s mumbles.", m_name);
			else monster_msg("%^s casts an ice bolt at %s.", m_name, t_name);
			monst_bolt_monst(m_idx, y, x, GF_ICE,
					 damroll(6, 6) + (rlev));
			break;
		}

	case SF_MISSILE_IDX:
		{
			disturb_on_other();
			if (blind || !see_m) monster_msg("%^s mumbles.", m_name);
			else monster_msg("%^s casts a magic missile at %s.", m_name, t_name);
			monst_bolt_monst(m_idx, y, x, GF_MISSILE,
					 damroll(2, 6) + (rlev / 3));
			break;
		}

	case SF_SCARE_IDX:
		{
			disturb_on_other();
			if (blind || !see_m) monster_msg("%^s mumbles, and you hear scary noises.", m_name);
			else monster_msg("%^s casts a fearful illusion at %s.", m_name, t_name);
			if (tr_ptr->flags & RF_NO_FEAR)
			{
				if (see_t) monster_msg("%^s refuses to be frightened.", t_name);
			}
			else if (t_ptr->level > randint((rlev - 10) < 1 ? 1 : (rlev - 10)) + 10)
			{
				if (see_t) monster_msg("%^s refuses to be frightened.", t_name);
			}
			else
			{
				if (!(t_ptr->monfear) && see_t) monster_msg("%^s flees in terror!", t_name);
				t_ptr->monfear += rand_int(4) + 4;
			}
			wake_up = true;
			break;
		}

	case SF_BLIND_IDX:
		{
			disturb_on_other();
			if (blind || !see_m) monster_msg("%^s mumbles.", m_name);
			else monster_msg("%^s casts a spell, burning %s%s eyes.", m_name, t_name,
						 (equals(t_name, "it") ? "s" : "'s"));
			if (tr_ptr->flags & RF_NO_CONF)  /* Simulate blindness with confusion */
			{
				if (see_t) monster_msg("%^s is unaffected.", t_name);
			}
			else if (t_ptr->level > randint((rlev - 10) < 1 ? 1 : (rlev - 10)) + 10)
			{
				if (see_t) monster_msg("%^s is unaffected.", t_name);
			}
			else
			{
				if (see_t) monster_msg("%^s is blinded!", t_name);
				t_ptr->confused += 12 + (byte)rand_int(4);
			}
			wake_up = true;
			break;

		}

	case SF_CONF_IDX:
		{
			disturb_on_other();
			if (blind || !see_m) monster_msg("%^s mumbles, and you hear puzzling noises.", m_name);
			else monster_msg("%^s creates a mesmerising illusion in front of %s.", m_name, t_name);
			if (tr_ptr->flags & RF_NO_CONF)
			{
				if (see_t) monster_msg("%^s disbelieves the feeble spell.", t_name);
			}
			else if (t_ptr->level > randint((rlev - 10) < 1 ? 1 : (rlev - 10)) + 10)
			{
				if (see_t) monster_msg("%^s disbelieves the feeble spell.", t_name);
			}
			else
			{
				if (see_t) monster_msg("%^s seems confused.", t_name);
				t_ptr->confused += 12 + (byte)rand_int(4);
			}
			wake_up = true;
			break;
		}

	case SF_SLOW_IDX:
		{
			disturb_on_other();
			if (!blind && see_either) monster_msg("%^s drains power from %s%s muscles.", m_name, t_name,
								      (equals(t_name, "it") ? "s" : "'s"));
			if (tr_ptr->flags & RF_UNIQUE)
			{
				if (see_t) monster_msg("%^s is unaffected.", t_name);
			}
			else if (t_ptr->level > randint((rlev - 10) < 1 ? 1 : (rlev - 10)) + 10)
			{
				if (see_t) monster_msg("%^s is unaffected.", t_name);
			}
			else
			{
				t_ptr->mspeed -= 10;
				if (see_t) monster_msg("%^s starts moving slower.", t_name);
			}
			wake_up = true;
			break;
		}

	case SF_HOLD_IDX:
		{
			disturb_on_other();
			if (!blind && see_m) monster_msg("%^s stares intently at %s.", m_name, t_name);
			if ((tr_ptr->flags & RF_UNIQUE) ||
					(tr_ptr->flags & RF_NO_STUN))
			{
				if (see_t) monster_msg("%^s is unaffected.", t_name);
			}
			else if (t_ptr->level > randint((rlev - 10) < 1 ? 1 : (rlev - 10)) + 10)
			{
				if (see_t) monster_msg("%^s is unaffected.", t_name);
			}
			else
			{
				t_ptr->stunned += randint(4) + 4;
				if (see_t) monster_msg("%^s is paralyzed!", t_name);
			}
			wake_up = true;
			break;
		}

	case SF_HASTE_IDX:
		{
			disturb_on_other();
			if (blind || !see_m)
			{
				monster_msg("%^s mumbles.", m_name);
			}
			else
			{
				monster_msg("%^s concentrates on %s body.", m_name, m_poss);
			}

			/* Allow quick speed increases to base+10 */
			if (m_ptr->mspeed < m_ptr->speed + 10)
			{
				if (see_m) monster_msg("%^s starts moving faster.", m_name);
				m_ptr->mspeed += 10;
			}

			/* Allow small speed increases to base+20 */
			else if (m_ptr->mspeed < m_ptr->speed + 20)
			{
				if (see_m) monster_msg("%^s starts moving faster.", m_name);
				m_ptr->mspeed += 2;
			}

			break;
		}

	case SF_HAND_DOOM_IDX:
		{
			disturb_on_other();
			if (!see_m) monster_msg("You hear someone invoke the Hand of Doom!");
			else if (!blind) monster_msg("%^s invokes the Hand of Doom on %s.", m_name, t_name);
			else
				monster_msg ("You hear someone invoke the Hand of Doom!");
			if (tr_ptr->flags & RF_UNIQUE)
			{
				if (!blind && see_t) monster_msg("^%s is unaffected!", t_name);
			}
			else
			{
				if (((m_ptr->level) + randint(20)) >
						((t_ptr->level) + 10 + randint(20)))
				{
					t_ptr->hp = t_ptr->hp
						    - (((s32b) ((65 + randint(25)) * (t_ptr->hp))) / 100);
					if (t_ptr->hp < 1) t_ptr->hp = 1;
				}
				else
				{
					if (see_t) monster_msg("%^s resists!", t_name);
				}
			}

			wake_up = true;
			break;
		}

	case SF_HEAL_IDX:
		{
			disturb_on_other();

			/* Message */
			if (blind || !see_m)
			{
				monster_msg("%^s mumbles.", m_name);
			}
			else
			{
				monster_msg("%^s concentrates on %s wounds.", m_name, m_poss);
			}

			/* Heal some */
			m_ptr->hp += (rlev * 6);

			/* Fully healed */
			if (m_ptr->hp >= m_ptr->maxhp)
			{
				/* Fully healed */
				m_ptr->hp = m_ptr->maxhp;

				/* Message */
				if (seen)
				{
					monster_msg("%^s looks completely healed!", m_name);
				}
				else
				{
					monster_msg("%^s sounds completely healed!", m_name);
				}
			}

			/* Partially healed */
			else
			{
				/* Message */
				if (seen)
				{
					monster_msg("%^s looks healthier.", m_name);
				}
				else
				{
					monster_msg("%^s sounds healthier.", m_name);
				}
			}

			/* Redraw (later) if needed */
			if (health_who == m_idx) p_ptr->redraw |= (PR_FRAME);

			/* Cancel fear */
			if (m_ptr->monfear)
			{
				/* Cancel fear */
				m_ptr->monfear = 0;

				/* Message */
				if (see_m) monster_msg("%^s recovers %s courage.", m_name, m_poss);
			}

			break;
		}

	case SF_BLINK_IDX:
		{
			disturb_on_other();
			if (see_m) monster_msg("%^s blinks away.", m_name);
			teleport_away(m_idx, 10);
			break;
		}

	case SF_TPORT_IDX:
		{
			if (dungeon_flags & DF_NO_TELEPORT)
			{
				break;  /* No teleport on special levels */
			}
			else
			{
				disturb_on_other();
				if (see_m) monster_msg("%^s teleports away.", m_name);
				teleport_away(m_idx, MAX_SIGHT * 2 + 5);
				break;
			}
		}

	case SF_TELE_TO_IDX:
		{
			/* Not implemented */
			break;
		}

	case SF_TELE_AWAY_IDX:
		{
			if (dungeon_flags & DF_NO_TELEPORT)
			{
				break;
			}

			bool resists_tele = false;
			disturb_on_other();
			monster_msg("%^s teleports %s away.", m_name, t_name);

			if (tr_ptr->flags & RF_RES_TELE)
			{
				if (tr_ptr->flags & RF_UNIQUE)
				{
					if (see_t)
					{
						monster_msg("%^s is unaffected!", t_name);
					}
					resists_tele = true;
				}
				else if (t_ptr->level > randint(100))
				{
					if (see_t)
					{
						monster_msg("%^s resists!", t_name);
					}
					resists_tele = true;
				}
			}

			if (!resists_tele)
			{
				teleport_away(t_idx, MAX_SIGHT * 2 + 5);
			}

			break;
		}

	case SF_TELE_LEVEL_IDX:
		{
			/* Not implemented */
			break;
		}

	case SF_DARKNESS_IDX:
		{
			disturb_on_other();
			if (blind) monster_msg("%^s mumbles.", m_name);
			else monster_msg("%^s gestures in shadow.", m_name);
			if (seen)
				monster_msg("%^s is surrounded by darkness.", t_name);
			project(m_idx, 3, y, x, 0, GF_DARK_WEAK, PROJECT_GRID | PROJECT_KILL);
			/* Lite up the room */
			unlite_room(y, x);
			break;
		}

	case SF_FORGET_IDX:
		{
			/* Not implemented */
			break;
		}

	case SF_S_ANIMAL_IDX:
		{
			do_summon("summons an animal!", 1, SUMMON_ANIMAL, SUMMON_ANIMAL, blind_msg_default);
			break;
		}

	case SF_S_ANIMALS_IDX:
		{
			do_summon("summons some animals!", 4, SUMMON_ANIMAL, SUMMON_ANIMAL, blind_msg_default);
			break;
		}

	case SF_S_BUG_IDX:
		{
			do_summon("codes some software bugs.", 6, SUMMON_BUG, SUMMON_BUG, blind_msg_default);
			break;
		}

	case SF_S_RNG_IDX:
		{
			do_summon("codes some RNGs.", 6, SUMMON_RNG, SUMMON_RNG, blind_msg_default);
			break;
		}

	case SF_S_THUNDERLORD_IDX:
		{
			do_summon("summons a Thunderlord!", 1, SUMMON_THUNDERLORD, SUMMON_THUNDERLORD, blind_msg_default);
			break;
		}

	case SF_S_KIN_IDX:
		{
			// Describe the summons
			char action[256];
			sprintf(action,
				"summons %s %s.",
				m_poss,
				(r_ptr->flags & RF_UNIQUE ? "minions" : "kin"));
			// Force the right type of "kin"
			summon_kin_type = r_ptr->d_char;
			// Summon
			do_summon(action, 6, SUMMON_KIN, SUMMON_KIN, blind_msg_default);
			break;
		}

	case SF_S_HI_DEMON_IDX:
		{
			do_summon("summons greater demons!", 8, SUMMON_HI_DEMON, SUMMON_HI_DEMON, blind_msg_default);
			break;
		}

	case SF_S_MONSTER_IDX:
		{
			do_summon("summons help!", 1, SUMMON_NO_UNIQUES, 0, blind_msg_default);
			break;
		}

	case SF_S_MONSTERS_IDX:
		{
			do_summon("summons monsters!", 8, SUMMON_NO_UNIQUES, 0, blind_msg_default);
			break;
		}

	case SF_S_ANT_IDX:
		{
			do_summon("summons ants.", 6, SUMMON_ANT, SUMMON_ANT, blind_msg_default);
			break;
		}

	case SF_S_SPIDER_IDX:
		{
			do_summon("summons spiders.", 6, SUMMON_SPIDER, SUMMON_SPIDER, blind_msg_default);
			break;
		}

	case SF_S_HOUND_IDX:
		{
			do_summon("summons hounds.", 6, SUMMON_HOUND, SUMMON_HOUND, blind_msg_default);
			break;
		}

	case SF_S_HYDRA_IDX:
		{
			do_summon("summons hydras.", 6, SUMMON_HYDRA, SUMMON_HYDRA, blind_msg_default);
			break;
		}

	case SF_S_ANGEL_IDX:
		{
			do_summon("summons an angel!", 1, SUMMON_ANGEL, SUMMON_ANGEL, blind_msg_default);
			break;
		}

	case SF_S_DEMON_IDX:
		{
			do_summon("summons a demon!", 1, SUMMON_DEMON, SUMMON_DEMON, blind_msg_default);
			break;
		}

	case SF_S_UNDEAD_IDX:
		{
			do_summon("summons an undead adversary!", 1, SUMMON_UNDEAD, SUMMON_UNDEAD, blind_msg_default);
			break;
		}

	case SF_S_DRAGON_IDX:
		{
			do_summon("summons a dragon!", 1, SUMMON_DRAGON, SUMMON_DRAGON, blind_msg_default);
			break;
		}

	case SF_S_HI_UNDEAD_IDX:
		{
			summon_messages blind_msg {
				"You hear a creepy thing appear nearby.",
				"You hear many creepy things appear nearby."
			};
			do_summon("summons greater undead!", 8, SUMMON_HI_UNDEAD_NO_UNIQUES, SUMMON_HI_UNDEAD, blind_msg);
			break;
		}

	case SF_S_HI_DRAGON_IDX:
		{
			summon_messages blind_msg {
				"You hear many a powerful thing appear nearby.",
				"You hear many powerful things appear nearby."
			};
			do_summon("summons ancient dragons!", 8, SUMMON_HI_DRAGON_NO_UNIQUES, SUMMON_HI_DRAGON, blind_msg);
			break;
		}

	case SF_S_WRAITH_IDX:
		{
			// No summoning Nazgul; see the remapping code above the switch.
			assert(!friendly);
			// Summon
			summon_messages blind_msg {
				"You hear an immortal being appear nearby.",
				"You hear immortal beings appear nearby."
			};
			do_summon("summons a wraith!", 8, 0 /* not used */, SUMMON_WRAITH, blind_msg);
			break;
		}

	case SF_S_UNIQUE_IDX:
		{
			// No summoning uniques; see the remapping code above the switch.
			assert(!friendly);
			// Interrupt
			disturb_on_other();
			// Message
			if (blind || !see_m)
			{
				monster_msg("%^s mumbles.", m_name);
			}
			else
			{
				monster_msg("%^s magically summons special opponents!", m_name);
			}
			// Summon
			int count = 0;
			for (int k = 0; k < 8; k++)
			{
				count += summon_specific(m_ptr->fy, m_ptr->fx, rlev, SUMMON_UNIQUE);
			}
			for (int k = 0; k < 8; k++)
			{
				count += summon_specific(m_ptr->fy, m_ptr->fx, rlev, SUMMON_HI_UNDEAD);
			}
			// Message
			if (blind)
			{
				if (count == 1)
				{
					monster_msg("You hear a powerful thing appear nearby.");
				}
				else if (count > 1)
				{
					monster_msg("You hear many powerful things appear nearby.");
				}
			}
			break;
		}
	}

	if (wake_up)
	{
		t_ptr->csleep = 0;
	}

	/* A spell was cast */
	return true;
}


void curse_equipment(int chance, int heavy_chance)
{
	bool changed = false;
	object_type * o_ptr =
		&p_ptr->inventory[rand_range(INVEN_WIELD, INVEN_TOTAL - 1)];

	if (randint(100) > chance)
	{
		return;
	}

	if (!o_ptr->k_ptr)
	{
		return;
	}

	auto const flags = object_flags(o_ptr);

	/* Extra, biased saving throw for blessed items */
	if ((flags & TR_BLESSED) && (randint(888) > chance))
	{
		char o_name[256];
		object_desc(o_name, o_ptr, false, 0);
		msg_format("Your %s resist%s cursing!", o_name,
		           ((o_ptr->number > 1) ? "" : "s"));
		/* Hmmm -- can we wear multiple items? If not, this is unnecessary */
		return;
	}

	if ((randint(100) <= heavy_chance) &&
	    (o_ptr->name1 || o_ptr->name2 || (!o_ptr->artifact_name.empty())))
	{
		if (!(flags & TR_HEAVY_CURSE))
			changed = true;
		o_ptr->art_flags |= TR_HEAVY_CURSE;
		o_ptr->art_flags |= TR_CURSED;
	}
	else
	{
		if (!(o_ptr->art_flags & TR_CURSED))
			changed = true;
		o_ptr->art_flags |= TR_CURSED;
	}

	if (changed)
	{
		msg_print("There is a malignant black aura surrounding you...");
		if (o_ptr->inscription == "uncursed")
		{
			o_ptr->inscription.clear();
		}
	}
}


void curse_equipment_dg(int chance, int heavy_chance)
{
	bool changed = false;

	object_type * o_ptr =
		&p_ptr->inventory[rand_range(INVEN_WIELD, INVEN_TOTAL - 1)];

	if (randint(100) > chance)
	{
		return;
	}

	if (!o_ptr->k_ptr)
	{
		return;
	}

	auto const flags = object_flags(o_ptr);

	/* Extra, biased saving throw for blessed items */
	if ((flags & TR_BLESSED) && (randint(888) > chance))
	{
		char o_name[256];
		object_desc(o_name, o_ptr, false, 0);
		msg_format("Your %s resist%s cursing!", o_name,
		           ((o_ptr->number > 1) ? "" : "s"));
		return;
	}

	if ((randint(100) <= heavy_chance) &&
	    (o_ptr->name1 || o_ptr->name2 || (!o_ptr->artifact_name.empty())))
	{
		if (!(flags & TR_HEAVY_CURSE))
			changed = true;
		o_ptr->art_flags |= TR_HEAVY_CURSE;
		o_ptr->art_flags |= TR_CURSED;
		o_ptr->art_flags |= TR_DG_CURSE;
	}
	else
	{
		if (!(flags & TR_CURSED))
			changed = true;
		o_ptr->art_flags |= TR_CURSED;
		o_ptr->art_flags |= TR_DG_CURSE;
	}

	if (changed)
	{
		msg_print("There is a malignant black aura surrounding you...");
		if (o_ptr->inscription == "uncursed")
		{
			o_ptr->inscription.clear();
		}
	}
}


/*
 * Creatures can cast spells, shoot missiles, and breathe.
 *
 * Returns "true" if a spell (or whatever) was (successfully) cast.
 *
 * XXX XXX XXX This function could use some work, but remember to
 * keep it as optimized as possible, while retaining generic code.
 *
 * Verify the various "blind-ness" checks in the code.
 *
 * XXX XXX XXX Note that several effects should really not be "seen"
 * if the player is blind.  See also "effects.c" for other "mistakes".
 *
 * Perhaps monsters should breathe at locations *near* the player,
 * since this would allow them to inflict "partial" damage.
 *
 * Perhaps smart monsters should decline to use "bolt" spells if
 * there is a monster in the way, unless they wish to kill it.
 *
 * It will not be possible to "correctly" handle the case in which a
 * monster attempts to attack a location which is thought to contain
 * the player, but which in fact is nowhere near the player, since this
 * might induce all sorts of messages about the attack itself, and about
 * the effects of the attack, which the player might or might not be in
 * a position to observe.  Thus, for simplicity, it is probably best to
 * only allow "faulty" attacks by a monster if one of the important grids
 * (probably the initial or final grid) is in fact in view of the player.
 * It may be necessary to actually prevent spell attacks except when the
 * monster actually has line of sight to the player.  Note that a monster
 * could be left in a bizarre situation after the player ducked behind a
 * pillar and then teleported away, for example.
 *
 * Note that this function attempts to optimize the use of spells for the
 * cases in which the monster has no spells, or has spells but cannot use
 * them, or has spells but they will have no "useful" effect.  Note that
 * this function has been an efficiency bottleneck in the past.
 *
 * Note the special "MFLAG_NICE" flag, which prevents a monster from using
 * any spell attacks until the player has had a single chance to move.
 */
static bool make_attack_spell(int m_idx)
{
	static const auto SF_BOLT_MASK = compute_bolt_mask();
	static const auto SF_SUMMON_MASK = compute_summoning_mask();
	static const auto SF_INT_MASK = compute_smart_mask();
	static const auto SF_INNATE_MASK = compute_innate_mask();

	int chance, rlev, failrate;
	char m_name[80];
	bool no_inate = false;

	/* Extract the blind-ness */
	bool blind = (p_ptr->blind ? true : false);

	/* Get a pointer to the monster */
	monster_type *m_ptr = &m_list[m_idx];

	/* Extract the "see-able-ness" */
	bool seen = (!blind && m_ptr->ml);

	/* Target location */
	if (m_ptr->target != 0)
	{
		return false;
	}
	int y = p_ptr->py;
	int x = p_ptr->px;

	/* Cannot cast spells when confused */
	if (m_ptr->confused) return false;

	/* Cannot cast spells when nice */
	if (m_ptr->mflag & (MFLAG_NICE)) return false;
	if (is_friend(m_ptr) >= 0) return false;

	/* Cannot attack the player if mortal and player fated to never die by the ... */
	auto const r_ptr = m_ptr->race();
	if ((r_ptr->flags & RF_MORTAL) && (p_ptr->no_mortal)) return false;

	/* Hack -- Extract the spell probability */
	chance = (r_ptr->freq_inate + r_ptr->freq_spell) / 2;

	/* Not allowed to cast spells */
	if (!chance) return false;

	/* Only do spells occasionally */
	if (rand_int(100) >= chance) return false;

	/* Sometimes forbid inate attacks (breaths) */
	if (rand_int(100) >= (chance * 2)) no_inate = true;

	/* Require projectable player */
	{
		/* Check range */
		if (m_ptr->cdis > MAX_RANGE) return false;

		/* Check path */
		if (!projectable(m_ptr->fy, m_ptr->fx, y, x)) return false;
	}

	/* Extract the monster level */
	rlev = ((m_ptr->level >= 1) ? m_ptr->level : 1);

	/* Extract the racial spell flags */
	monster_spell_flag_set allowed_spells = r_ptr->spells;

	/* Forbid inate attacks sometimes */
	if (no_inate)
	{
		allowed_spells &= ~SF_INNATE_MASK;
	}

	/* Hack -- allow "desperate" spells */
	if ((r_ptr->flags & RF_SMART) &&
	                (m_ptr->hp < m_ptr->maxhp / 10) &&
	                (rand_int(100) < 50))
	{
		/* Require intelligent spells */
		allowed_spells &= SF_INT_MASK;

		/* No spells left? */
		if (!allowed_spells) return false;
	}

	/* Remove the "ineffective" spells */
	remove_bad_spells(m_idx, &allowed_spells);

	/* No spells left */
	if (!allowed_spells) return false;

	/* Check for a clean bolt shot */
	if ((allowed_spells & SF_BOLT_MASK) &&
	    !(r_ptr->flags & RF_STUPID) &&
	    !clean_shot(m_ptr->fy, m_ptr->fx, y, x))
	{
		/* Remove spells that will only hurt friends */
		allowed_spells &= ~SF_BOLT_MASK;
	}

	/* Check for a possible summon */
	if ((allowed_spells & SF_SUMMON_MASK) &&
	    !(r_ptr->flags & RF_STUPID) &&
	    !(summon_possible(y, x)))
	{
		/* Remove summoning spells */
		allowed_spells &= ~SF_SUMMON_MASK;
	}

	/* No spells left */
	if (!allowed_spells) return false;

	/* Extract the "inate" spells */
	auto spell = extract_spells(allowed_spells);

	/* No spells left */
	if (spell.empty()) return false;

	/* Stop if player is dead or gone */
	if (!alive || death) return false;

	/* Stop if player is leaving */
	if (p_ptr->leaving) return false;

	/* Get the monster name (or "it") */
	monster_desc(m_name, m_ptr, 0x00);

	/* Choose a spell to cast */
	auto thrown_spell = choose_attack_spell(m_idx, spell);

	/* Abort if no spell was chosen */
	if (!thrown_spell) return false;

	/* Calculate spell failure rate */
	failrate = 25 - (rlev + 3) / 4;

	/* Hack -- Stupid monsters will never fail (for jellies and such) */
	if (r_ptr->flags & RF_STUPID) failrate = 0;

	/* Check for spell failure (inate attacks never fail) */
	if ((!thrown_spell->is_innate) && (rand_int(100) < failrate))
	{
		/* Message */
		msg_format("%^s tries to cast a spell, but fails.", m_name);

		return true;
	}

	/* Can the player disrupt its puny attempts? */
	if ((p_ptr->antimagic_dis >= m_ptr->cdis) && magik(p_ptr->antimagic) && thrown_spell->is_magic)
	{
		char m_poss[80];

		/* Get monster's possessive noun form ("the Illusionist's") */
		monster_desc(m_poss, m_ptr, 0x06);

		msg_format("Your anti-magic field disrupts %s spell.", m_poss);
	}
	else
	{
		char m_poss[80];
		char ddesc[80];

		/* Get the monster possessive ("his"/"her"/"its") */
		monster_desc(m_poss, m_ptr, 0x22);

		/* Hack -- Get the "died from" name */
		monster_desc(ddesc, m_ptr, 0x88);

		/* Do a breath */
		auto do_breath = [&](char const *element, int gf, s32b max, int divisor, int smart_learn) -> void {
			// Interrupt
			disturb();
			// Message
			if (blind)
			{
				msg_format("%^s breathes.", m_name);
			}
			else
			{
				msg_format("%^s breathes %s.", m_name, element);
			}
			// Breathe
			breath(m_idx, gf, std::min(m_ptr->hp / divisor, max), 0);
			// Update "smart" monster knowledge
			update_smart_learn(m_idx, smart_learn);
		};

		/* Messages for summoning */
		struct summon_messages {
			char const *singular;
			char const *plural;
		};

		/* Default message for summoning when player is blinded */
		summon_messages blind_msg_default {
			"You hear something appear nearby.",
			"You hear many things appear nearby."
		};

		/* Do a summoning spell */
		auto do_summon = [&](char const *action, int n, int type, summon_messages const &blind_msg) -> void {
			// Interrupt
			disturb();
			// Message
			if (blind)
			{
				msg_format("%^s mumbles.", m_name);
			}
			else
			{
				msg_format("%^s magically %s", m_name, action);
			}
			// Do the actual summoning
			int count = 0;
			for (int k = 0; k < n; k++)
			{
				count += summon_specific(m_ptr->fy, m_ptr->fx, rlev, type);
			}
			// Message for blinded characters
			if (blind)
			{
				if (count == 1)
				{
					msg_print(blind_msg.singular);
				}
				else if (count > 1)
				{
					msg_print(blind_msg.plural);
				}
			}
		};

		/* Cast the spell. */
		switch (thrown_spell->spell_idx)
		{
		case SF_SHRIEK_IDX:
			{
				disturb();
				msg_format("%^s makes a high pitched shriek.", m_name);
				aggravate_monsters(m_idx);
				break;
			}

		case SF_MULTIPLY_IDX:
			{
				break;
			}

		case SF_ROCKET_IDX:
			{
				disturb();
				if (blind) msg_format("%^s shoots something.", m_name);
				else msg_format("%^s fires a rocket.", m_name);
				breath(m_idx, GF_ROCKET,
				       ((m_ptr->hp / 4) > 800 ? 800 : (m_ptr->hp / 4)), 2);
				update_smart_learn(m_idx, DRS_SHARD);
				break;
			}

		case SF_ARROW_1_IDX:
			{
				disturb();
				if (blind) msg_format("%^s makes a strange noise.", m_name);
				else msg_format("%^s fires an arrow.", m_name);
				bolt(m_idx, GF_ARROW, damroll(1, 6));
				update_smart_learn(m_idx, DRS_REFLECT);
				break;
			}

		case SF_ARROW_2_IDX:
			{
				disturb();
				if (blind) msg_format("%^s makes a strange noise.", m_name);
				else msg_format("%^s fires an arrow!", m_name);
				bolt(m_idx, GF_ARROW, damroll(3, 6));
				update_smart_learn(m_idx, DRS_REFLECT);
				break;
			}

		case SF_ARROW_3_IDX:
			{
				disturb();
				if (blind) msg_format("%^s makes a strange noise.", m_name);
				else msg_format("%^s fires a missile.", m_name);
				bolt(m_idx, GF_ARROW, damroll(5, 6));
				update_smart_learn(m_idx, DRS_REFLECT);
				break;
			}

		case SF_ARROW_4_IDX:
			{
				disturb();
				if (blind) msg_format("%^s makes a strange noise.", m_name);
				else msg_format("%^s fires a missile!", m_name);
				bolt(m_idx, GF_ARROW, damroll(7, 6));
				update_smart_learn(m_idx, DRS_REFLECT);
				break;
			}

		case SF_BR_ACID_IDX:
			{
			        do_breath("acid", GF_ACID, 1600, 3, DRS_ACID);
				break;
			}

		case SF_BR_ELEC_IDX:
			{
			        do_breath("lightning", GF_ELEC, 1600, 3, DRS_ELEC);
				break;
			}

		case SF_BR_FIRE_IDX:
			{
			        do_breath("fire", GF_FIRE, 1600, 3, DRS_FIRE);
				break;
			}

		case SF_BR_COLD_IDX:
			{
			        do_breath("frost", GF_COLD, 1600, 3, DRS_COLD);
				break;
			}

		case SF_BR_POIS_IDX:
			{
			        do_breath("gas", GF_POIS, 800, 3, DRS_POIS);
				break;
			}

		case SF_BR_NETH_IDX:
			{
			        do_breath("nether", GF_NETHER, 550, 6, DRS_NETH);
				break;
			}

		case SF_BR_LITE_IDX:
			{
			        do_breath("light", GF_LITE, 400, 6, DRS_LITE);
				break;
			}

		case SF_BR_DARK_IDX:
			{
			        do_breath("darkness", GF_DARK, 400, 6, DRS_DARK);
				break;
			}

		case SF_BR_CONF_IDX:
			{
			        do_breath("confusion", GF_CONFUSION, 400, 6, DRS_CONF);
				break;
			}

		case SF_BR_SOUN_IDX:
			{
			        do_breath("sound", GF_SOUND, 400, 6, DRS_SOUND);
				break;
			}

		case SF_BR_CHAO_IDX:
			{
			        do_breath("chaos", GF_CHAOS, 600, 6, DRS_CHAOS);
				break;
			}

		case SF_BR_DISE_IDX:
			{
			        do_breath("disenchantment", GF_DISENCHANT, 500, 6, DRS_DISEN);
				break;
			}

		case SF_BR_NEXU_IDX:
			{
			        do_breath("nexus", GF_NEXUS, 250, 3, DRS_NEXUS);
				break;
			}

		case SF_BR_TIME_IDX:
			{
			        do_breath("time", GF_TIME, 150, 3, DRS_NONE);
				break;
			}

		case SF_BR_INER_IDX:
			{
			        do_breath("inertia", GF_INERTIA, 200, 6, DRS_NONE);
				break;
			}

		case SF_BR_GRAV_IDX:
			{
			        do_breath("gravity", GF_GRAVITY, 200, 3, DRS_NONE);
				break;
			}

		case SF_BR_SHAR_IDX:
			{
			        do_breath("shards", GF_SHARDS, 400, 6, DRS_SHARD);
				break;
			}

		case SF_BR_PLAS_IDX:
			{
			        do_breath("plasma", GF_PLASMA, 150, 6, DRS_NONE);
				break;
			}

		case SF_BR_WALL_IDX:
			{
			        do_breath("force", GF_FORCE, 200, 6, DRS_NONE);
				break;
			}

		case SF_BR_MANA_IDX:
			{
			        do_breath("magical energy", GF_MANA, 250, 3, DRS_NONE);
				break;
			}

		case SF_BA_NUKE_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a ball of radiation.", m_name);
				breath(m_idx, GF_NUKE, (rlev + damroll(10, 6)), 2);
				update_smart_learn(m_idx, DRS_POIS);
				break;
			}

		case SF_BR_NUKE_IDX:
			{
			        do_breath("toxic waste", GF_NUKE, 800, 3, DRS_POIS);
				break;
			}

		case SF_BA_CHAO_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles frighteningly.", m_name);
				else msg_format("%^s invokes a raw chaos.", m_name);
				breath(m_idx, GF_CHAOS, (rlev * 2) + damroll(10, 10), 4);
				update_smart_learn(m_idx, DRS_CHAOS);
				break;
			}

		case SF_BR_DISI_IDX:
			{
			        do_breath("disintegration", GF_DISINTEGRATE, 300, 3, DRS_NONE);
				break;
			}

		case SF_BA_ACID_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts an acid ball.", m_name);
				breath(m_idx, GF_ACID,
				       randint(rlev * 3) + 15, 2);
				update_smart_learn(m_idx, DRS_ACID);
				break;
			}

		case SF_BA_ELEC_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a lightning ball.", m_name);
				breath(m_idx, GF_ELEC,
				       randint(rlev * 3 / 2) + 8, 2);
				update_smart_learn(m_idx, DRS_ELEC);
				break;
			}

		case SF_BA_FIRE_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a fire ball.", m_name);
				breath(m_idx, GF_FIRE,
				       randint(rlev * 7 / 2) + 10, 2);
				update_smart_learn(m_idx, DRS_FIRE);
				break;
			}

		case SF_BA_COLD_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a frost ball.", m_name);
				breath(m_idx, GF_COLD,
				       randint(rlev * 3 / 2) + 10, 2);
				update_smart_learn(m_idx, DRS_COLD);
				break;
			}

		case SF_BA_POIS_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a stinking cloud.", m_name);
				breath(m_idx, GF_POIS,
				       damroll(12, 2), 2);
				update_smart_learn(m_idx, DRS_POIS);
				break;
			}

		case SF_BA_NETH_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a nether ball.", m_name);
				breath(m_idx, GF_NETHER,
				       (50 + damroll(10, 10) + rlev), 2);
				update_smart_learn(m_idx, DRS_NETH);
				break;
			}

		case SF_BA_WATE_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s gestures fluidly.", m_name);
				msg_print("You are engulfed in a whirlpool.");
				breath(m_idx, GF_WATER,
				       randint(rlev * 5 / 2) + 50, 4);
				break;
			}

		case SF_BA_MANA_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles powerfully.", m_name);
				else msg_format("%^s invokes a mana storm.", m_name);
				breath(m_idx, GF_MANA,
				       (rlev * 5) + damroll(10, 10), 4);
				break;
			}

		case SF_BA_DARK_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles powerfully.", m_name);
				else msg_format("%^s invokes a darkness storm.", m_name);
				breath(m_idx, GF_DARK,
				       (rlev * 5) + damroll(10, 10), 4);
				update_smart_learn(m_idx, DRS_DARK);
				break;
			}

		case SF_DRAIN_MANA_IDX:
			{
				if (p_ptr->csp)
				{
					int r1;

					/* Disturb if legal */
					disturb();

					/* Basic message */
					msg_format("%^s draws psychic energy from you!", m_name);

					/* Attack power */
					r1 = (randint(rlev) / 2) + 1;

					/* Full drain */
					if (r1 >= p_ptr->csp)
					{
						r1 = p_ptr->csp;
						p_ptr->csp = 0;
						p_ptr->csp_frac = 0;
					}

					/* Partial drain */
					else
					{
						p_ptr->csp -= r1;
					}

					/* Redraw mana */
					p_ptr->redraw |= (PR_FRAME);

					/* Window stuff */
					p_ptr->window |= (PW_PLAYER);

					/* Heal the monster */
					if (m_ptr->hp < m_ptr->maxhp)
					{
						/* Heal */
						m_ptr->hp += (6 * r1);
						if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

						/* Redraw (later) if needed */
						if (health_who == m_idx) p_ptr->redraw |= (PR_FRAME);

						/* Special message */
						if (seen)
						{
							msg_format("%^s appears healthier.", m_name);
						}
					}
				}
				update_smart_learn(m_idx, DRS_MANA);
				break;
			}

		case SF_MIND_BLAST_IDX:
			{
				disturb();
				if (!seen)
				{
					msg_print("You feel something focusing on your mind.");
				}
				else
				{
					msg_format("%^s gazes deep into your eyes.", m_name);
				}

				if (rand_int(100) < p_ptr->skill_sav)
				{
					msg_print("You resist the effects!");
				}
				else
				{
					msg_print("Your mind is blasted by psionic energy.");

					if (!p_ptr->resist_conf)
					{
						set_confused(p_ptr->confused + rand_int(4) + 4);
					}

					if ((!p_ptr->resist_chaos) && (randint(3) == 1))
					{
						set_image(p_ptr->image + rand_int(250) + 150);
					}

					take_sanity_hit(damroll(8, 8), ddesc);
				}
				break;
			}

		case SF_BRAIN_SMASH_IDX:
			{
				disturb();
				if (!seen)
				{
					msg_print("You feel something focusing on your mind.");
				}
				else
				{
					msg_format("%^s looks deep into your eyes.", m_name);
				}

				if (rand_int(100) < p_ptr->skill_sav)
				{
					msg_print("You resist the effects!");
				}
				else
				{
					msg_print("Your mind is blasted by psionic energy.");
					take_sanity_hit(damroll(12, 15), ddesc);
					if (!p_ptr->resist_blind)
					{
						set_blind(p_ptr->blind + 8 + rand_int(8));
					}
					if (!p_ptr->resist_conf)
					{
						set_confused(p_ptr->confused + rand_int(4) + 4);
					}
					if (!p_ptr->free_act)
					{
						set_paralyzed(rand_int(4) + 4);
					}
					set_slow(p_ptr->slow + rand_int(4) + 4);

					while (rand_int(100) > p_ptr->skill_sav)
						do_dec_stat(A_INT, STAT_DEC_NORMAL);
					while (rand_int(100) > p_ptr->skill_sav)
						do_dec_stat(A_WIS, STAT_DEC_NORMAL);

					if (!p_ptr->resist_chaos)
					{
						set_image(p_ptr->image + rand_int(250) + 150);
					}
				}
				break;
			}

		case SF_CAUSE_1_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s points at you and curses.", m_name);
				if (rand_int(100) < p_ptr->skill_sav)
				{
					msg_print("You resist the effects!");
				}
				else
				{
					curse_equipment(33, 0);
					take_hit(damroll(3, 8), ddesc);
				}
				break;
			}

		case SF_CAUSE_2_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s points at you and curses horribly.", m_name);
				if (rand_int(100) < p_ptr->skill_sav)
				{
					msg_print("You resist the effects!");
				}
				else
				{
					curse_equipment(50, 5);
					take_hit(damroll(8, 8), ddesc);
				}
				break;
			}

		case SF_CAUSE_3_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles loudly.", m_name);
				else msg_format("%^s points at you, incanting terribly!", m_name);
				if (rand_int(100) < p_ptr->skill_sav)
				{
					msg_print("You resist the effects!");
				}
				else
				{
					curse_equipment(80, 15);
					take_hit(damroll(10, 15), ddesc);
				}
				break;
			}

		case SF_CAUSE_4_IDX:
			{
				disturb();
				if (blind) msg_format("%^s screams the word 'DIE!'", m_name);
				else msg_format("%^s points at you, screaming the word DIE!", m_name);
				if (rand_int(100) < p_ptr->skill_sav)
				{
					msg_print("You resist the effects!");
				}
				else
				{
					take_hit(damroll(15, 15), ddesc);
					set_cut(p_ptr->cut + damroll(10, 10));
				}
				break;
			}

		case SF_BO_ACID_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a acid bolt.", m_name);
				bolt(m_idx, GF_ACID, damroll(7, 8) + (rlev / 3));
				update_smart_learn(m_idx, DRS_ACID);
				update_smart_learn(m_idx, DRS_REFLECT);
				break;
			}

		case SF_BO_ELEC_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a lightning bolt.", m_name);
				bolt(m_idx, GF_ELEC, damroll(4, 8) + (rlev / 3));
				update_smart_learn(m_idx, DRS_ELEC);
				update_smart_learn(m_idx, DRS_REFLECT);
				break;
			}

		case SF_BO_FIRE_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a fire bolt.", m_name);
				bolt(m_idx, GF_FIRE, damroll(9, 8) + (rlev / 3));
				update_smart_learn(m_idx, DRS_FIRE);
				update_smart_learn(m_idx, DRS_REFLECT);
				break;
			}

		case SF_BO_COLD_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a frost bolt.", m_name);
				bolt(m_idx, GF_COLD, damroll(6, 8) + (rlev / 3));
				update_smart_learn(m_idx, DRS_COLD);
				update_smart_learn(m_idx, DRS_REFLECT);
				break;
			}

		case SF_BO_POIS_IDX:
			{
				/* XXX XXX XXX */
				break;
			}

		case SF_BO_NETH_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a nether bolt.", m_name);
				bolt(m_idx, GF_NETHER, 30 + damroll(5, 5) + (rlev * 3) / 2);
				update_smart_learn(m_idx, DRS_NETH);
				update_smart_learn(m_idx, DRS_REFLECT);
				break;
			}

		case SF_BO_WATE_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a water bolt.", m_name);
				bolt(m_idx, GF_WATER, damroll(10, 10) + (rlev));
				update_smart_learn(m_idx, DRS_REFLECT);
				break;
			}

		case SF_BO_MANA_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a mana bolt.", m_name);
				bolt(m_idx, GF_MANA, randint(rlev * 7 / 2) + 50);
				update_smart_learn(m_idx, DRS_REFLECT);
				break;
			}

		case SF_BO_PLAS_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a plasma bolt.", m_name);
				bolt(m_idx, GF_PLASMA, 10 + damroll(8, 7) + (rlev));
				update_smart_learn(m_idx, DRS_REFLECT);
				break;
			}

		case SF_BO_ICEE_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts an ice bolt.", m_name);
				bolt(m_idx, GF_ICE, damroll(6, 6) + (rlev));
				update_smart_learn(m_idx, DRS_COLD);
				update_smart_learn(m_idx, DRS_REFLECT);
				break;
			}

		case SF_MISSILE_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a magic missile.", m_name);
				bolt(m_idx, GF_MISSILE, damroll(2, 6) + (rlev / 3));
				update_smart_learn(m_idx, DRS_REFLECT);
				break;
			}

		case SF_SCARE_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles, and you hear scary noises.", m_name);
				else msg_format("%^s casts a fearful illusion.", m_name);
				if (p_ptr->resist_fear)
				{
					msg_print("You refuse to be frightened.");
				}
				else if (rand_int(100) < p_ptr->skill_sav)
				{
					msg_print("You refuse to be frightened.");
				}
				else
				{
					set_afraid(p_ptr->afraid + rand_int(4) + 4);
				}
				update_smart_learn(m_idx, DRS_FEAR);
				break;
			}

		case SF_BLIND_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s casts a spell, burning your eyes!", m_name);
				if (p_ptr->resist_blind)
				{
					msg_print("You are unaffected!");
				}
				else if (rand_int(100) < p_ptr->skill_sav)
				{
					msg_print("You resist the effects!");
				}
				else
				{
					set_blind(12 + rand_int(4));
				}
				update_smart_learn(m_idx, DRS_BLIND);
				break;
			}

		case SF_CONF_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles, and you hear puzzling noises.", m_name);
				else msg_format("%^s creates a mesmerizing illusion.", m_name);
				if (p_ptr->resist_conf)
				{
					msg_print("You disbelieve the feeble spell.");
				}
				else if (rand_int(100) < p_ptr->skill_sav)
				{
					msg_print("You disbelieve the feeble spell.");
				}
				else
				{
					set_confused(p_ptr->confused + rand_int(4) + 4);
				}
				update_smart_learn(m_idx, DRS_CONF);
				break;
			}

		case SF_SLOW_IDX:
			{
				disturb();
				msg_format("%^s drains power from your muscles!", m_name);
				if (p_ptr->free_act)
				{
					msg_print("You are unaffected!");
				}
				else if (rand_int(100) < p_ptr->skill_sav)
				{
					msg_print("You resist the effects!");
				}
				else
				{
					set_slow(p_ptr->slow + rand_int(4) + 4);
				}
				update_smart_learn(m_idx, DRS_FREE);
				break;
			}

		case SF_HOLD_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s stares deep into your eyes!", m_name);
				if (p_ptr->free_act)
				{
					msg_print("You are unaffected!");
				}
				else if (rand_int(100) < p_ptr->skill_sav)
				{
					msg_format("You resist the effects!");
				}
				else
				{
					set_paralyzed(rand_int(4) + 4);
				}
				update_smart_learn(m_idx, DRS_FREE);
				break;
			}

		case SF_HASTE_IDX:
			{
				disturb();
				if (blind)
				{
					msg_format("%^s mumbles.", m_name);
				}
				else
				{
					msg_format("%^s concentrates on %s body.", m_name, m_poss);
				}

				/* Allow quick speed increases to base+10 */
				if (m_ptr->mspeed < m_ptr->speed + 10)
				{
					msg_format("%^s starts moving faster.", m_name);
					m_ptr->mspeed += 10;
				}

				/* Allow small speed increases to base+20 */
				else if (m_ptr->mspeed < m_ptr->speed + 20)
				{
					msg_format("%^s starts moving faster.", m_name);
					m_ptr->mspeed += 2;
				}

				break;
			}

		case SF_HAND_DOOM_IDX:
			{
				disturb();
				msg_format("%^s invokes the Hand of Doom!", m_name);
				if (rand_int(100) < p_ptr->skill_sav)
				{
					msg_format("You resist the effects!");
				}
				else
				{
					int dummy = (((s32b) ((65 + randint(25)) * (p_ptr->chp))) / 100);
					msg_print("Your feel your life fade away!");
					take_hit(dummy, m_name);
					curse_equipment(100, 20);

					if (p_ptr->chp < 1) p_ptr->chp = 1;
				}
				break;
			}

		case SF_HEAL_IDX:
			{
				disturb();

				/* Message */
				if (blind)
				{
					msg_format("%^s mumbles.", m_name);
				}
				else
				{
					msg_format("%^s concentrates on %s wounds.", m_name, m_poss);
				}

				/* Heal some */
				m_ptr->hp += (rlev * 6);

				/* Fully healed */
				if (m_ptr->hp >= m_ptr->maxhp)
				{
					/* Fully healed */
					m_ptr->hp = m_ptr->maxhp;

					/* Message */
					if (seen)
					{
						msg_format("%^s looks completely healed!", m_name);
					}
					else
					{
						msg_format("%^s sounds completely healed!", m_name);
					}
				}

				/* Partially healed */
				else
				{
					/* Message */
					if (seen)
					{
						msg_format("%^s looks healthier.", m_name);
					}
					else
					{
						msg_format("%^s sounds healthier.", m_name);
					}
				}

				/* Redraw (later) if needed */
				if (health_who == m_idx) p_ptr->redraw |= (PR_FRAME);

				/* Cancel fear */
				if (m_ptr->monfear)
				{
					/* Cancel fear */
					m_ptr->monfear = 0;

					/* Message */
					msg_format("%^s recovers %s courage.", m_name, m_poss);
				}
				break;
			}

		case SF_BLINK_IDX:
			{
				disturb();
				msg_format("%^s blinks away.", m_name);
				teleport_away(m_idx, 10);
				break;
			}

		case SF_TPORT_IDX:
			{
				disturb();
				msg_format("%^s teleports away.", m_name);
				teleport_away(m_idx, MAX_SIGHT * 2 + 5);
				break;
			}

		case SF_TELE_TO_IDX:
			{
				disturb();
				msg_format("%^s commands you to return.", m_name);
				teleport_player_to(m_ptr->fy, m_ptr->fx);
				break;
			}

		case SF_TELE_AWAY_IDX:
			{
				disturb();
				msg_format("%^s teleports you away.", m_name);
				teleport_player(100);
				break;
			}

		case SF_TELE_LEVEL_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles strangely.", m_name);
				else msg_format("%^s gestures at your feet.", m_name);
				if (p_ptr->resist_nexus)
				{
					msg_print("You are unaffected!");
				}
				else if (rand_int(100) < p_ptr->skill_sav)
				{
					msg_print("You resist the effects!");
				}
				else
				{
					teleport_player_level();
				}
				update_smart_learn(m_idx, DRS_NEXUS);
				break;
			}

		case SF_DARKNESS_IDX:
			{
				disturb();
				if (blind) msg_format("%^s mumbles.", m_name);
				else msg_format("%^s gestures in shadow.", m_name);
				unlite_area(0, 3);
				break;
			}

		case SF_FORGET_IDX:
			{
				disturb();
				msg_format("%^s tries to blank your mind.", m_name);

				if (rand_int(100) < p_ptr->skill_sav)
				{
					msg_print("You resist the effects!");
				}
				else if (lose_all_info())
				{
					msg_print("Your memories fade away.");
				}
				break;
			}

		case SF_S_ANIMAL_IDX:
		        {
			        do_summon("summons an animal!", 1, SUMMON_ANIMAL, blind_msg_default);
				break;
		        }

		case SF_S_ANIMALS_IDX:
		        {
			        do_summon("summons some animals!", 4, SUMMON_ANIMAL, blind_msg_default);
				break;
		        }

		case SF_S_BUG_IDX:
			{
			        do_summon("codes some software bugs.", 6, SUMMON_BUG, blind_msg_default);
				break;
			}

		case SF_S_RNG_IDX:
			{
			        do_summon("codes some RNGs.", 6, SUMMON_RNG, blind_msg_default);
				break;
			}

		case SF_S_THUNDERLORD_IDX:
			{
			        do_summon("summons a Thunderlord!", 1, SUMMON_THUNDERLORD, blind_msg_default);
				break;
			}

		case SF_S_KIN_IDX:
			{
			        // Describe the summons
			        char action[256];
				sprintf(action,
				        "summons %s %s.",
				        m_poss,
				        (r_ptr->flags & RF_UNIQUE) ? "minions" : "kin");
				// Force the correct type of "kin"
				summon_kin_type = r_ptr->d_char;
				// Summon
				do_summon(action, 6, SUMMON_KIN, blind_msg_default);
				break;
			}

		case SF_S_HI_DEMON_IDX:
			{
			        do_summon("summons greater demons!", 8, SUMMON_HI_DEMON, blind_msg_default);
				break;
			}

		case SF_S_MONSTER_IDX:
			{
			        do_summon("summons help!", 1, 0, blind_msg_default);
				break;
			}

		case SF_S_MONSTERS_IDX:
			{
			        do_summon("summons monsters!", 8, 0, blind_msg_default);
				break;
			}

		case SF_S_ANT_IDX:
			{
			        do_summon("summons ants.", 6, SUMMON_ANT, blind_msg_default);
				break;
			}

		case SF_S_SPIDER_IDX:
			{
			        do_summon("summons spiders.", 6, SUMMON_SPIDER, blind_msg_default);
				break;
			}

		case SF_S_HOUND_IDX:
			{
			        do_summon("summons hounds.", 6, SUMMON_HOUND, blind_msg_default);
				break;
			}

		case SF_S_HYDRA_IDX:
			{
			        do_summon("summons hydras.", 6, SUMMON_HYDRA, blind_msg_default);
				break;
			}

		case SF_S_ANGEL_IDX:
			{
			        do_summon("summons an angel!", 1, SUMMON_ANGEL, blind_msg_default);
				break;
			}

		case SF_S_DEMON_IDX:
			{
			        do_summon("summons a demon!", 1, SUMMON_DEMON, blind_msg_default);
				break;
			}

		case SF_S_UNDEAD_IDX:
			{
			        do_summon("summons an undead adversary!", 1, SUMMON_UNDEAD, blind_msg_default);
				break;
			}

		case SF_S_DRAGON_IDX:
			{
			        do_summon("summons a dragon!", 1, SUMMON_DRAGON, blind_msg_default);
				break;
			}

		case SF_S_HI_UNDEAD_IDX:
			{
			        summon_messages blind_msg {
					"You hear a creepy thing appear nearby.",
					"You hear many creepy things appear nearby."
				};
				do_summon("summons greater undead!", 8, SUMMON_HI_UNDEAD, blind_msg);
				break;
			}

		case SF_S_HI_DRAGON_IDX:
			{
			        summon_messages blind_msg {
					"You hear a powerful thing appear nearby.",
					"You hear many powerful things appear nearby."
				};
				do_summon("summons ancient dragons!", 8, SUMMON_HI_DRAGON, blind_msg);
				break;
			}

		case SF_S_WRAITH_IDX:
			{
			        summon_messages blind_msg {
					"You hear an immortal being appear nearby.",
					"You hear immortal beings appear nearby."
				};
				do_summon("summons Wraiths!", 8, SUMMON_WRAITH, blind_msg);
				break;
			}

		case SF_S_UNIQUE_IDX:
			{
			        // Interrupt
			        disturb();
				// Message
				if (blind)
				{
					msg_format("%^s mumbles.", m_name);
				}
				else
				{
					msg_format("%^s magically summons special opponents!", m_name);
				}
				// Summon
				int count = 0;
				for (int k = 0; k < 8; k++)
				{
					count += summon_specific(m_ptr->fy, m_ptr->fx, rlev, SUMMON_UNIQUE);
				}
				for (int k = 0; k < 8; k++)
				{
					count += summon_specific(m_ptr->fy, m_ptr->fx, rlev, SUMMON_HI_UNDEAD);
				}
				// Message
				if (blind)
				{
					if (count == 1)
					{
						msg_print("You hear a powerful thing appear nearby.");
					}
					else if (count > 1)
					{
						msg_print("You hear many powerful things appear nearby.");
					}
				}
				break;
			}
		}
	}

	/* A spell was cast */
	return true;
}


/*
 * Returns whether a given monster will try to run from the player.
 *
 * Monsters will attempt to avoid very powerful players.  See below.
 *
 * Because this function is called so often, little details are important
 * for efficiency.  Like not using "mod" or "div" when possible.  And
 * attempting to check the conditions in an optimal order.  Note that
 * "(x << 2) == (x * 4)" if "x" has enough bits to hold the result.
 *
 * Note that this function is responsible for about one to five percent
 * of the processor use in normal conditions...
 */
static int mon_will_run(int m_idx)
{
	monster_type *m_ptr = &m_list[m_idx];
	u16b p_lev, m_lev;
	u16b p_chp, p_mhp;
	u16b m_chp, m_mhp;
	u32b p_val, m_val;

	/* Keep monsters from running too far away */
	if (m_ptr->cdis > MAX_SIGHT + 5) return false;

	/* Friends don't run away */
	if (is_friend(m_ptr) >= 0) return false;

	/* All "afraid" monsters will run away */
	if (m_ptr->monfear) return true;

	/* Nearby monsters will not become terrified */
	if (m_ptr->cdis <= 5) return false;

	/* Examine player power (level) */
	p_lev = p_ptr->lev;

	/* Examine monster power (level plus morale) */
	m_lev = m_ptr->level + (m_idx & 0x08) + 25;

	/* Optimize extreme cases below */
	if (m_lev > p_lev + 4) return false;
	if (m_lev + 4 <= p_lev) return true;

	/* Examine player health */
	p_chp = p_ptr->chp;
	p_mhp = p_ptr->mhp;

	/* Examine monster health */
	m_chp = m_ptr->hp;
	m_mhp = m_ptr->maxhp;

	/* Prepare to optimize the calculation */
	p_val = (p_lev * p_mhp) + (p_chp << 2);  /* div p_mhp */
	m_val = (m_lev * m_mhp) + (m_chp << 2);  /* div m_mhp */

	/* Strong players scare strong monsters */
	if (p_val * m_mhp > m_val * p_mhp) return true;

	/* Assume no terror */
	return false;
}




/*
* Choose the "best" direction for "flowing"
*
* Note that ghosts and rock-eaters are never allowed to "flow",
* since they should move directly towards the player.
*
* Prefer "non-diagonal" directions, but twiddle them a little
* to angle slightly towards the player's actual location.
*
* Allow very perceptive monsters to track old "spoor" left by
* previous locations occupied by the player.  This will tend
* to have monsters end up either near the player or on a grid
* recently occupied by the player (and left via "teleport").
*
* Note that if "smell" is turned on, all monsters get vicious.
*
* Also note that teleporting away from a location will cause
* the monsters who were chasing you to converge on that location
* as long as you are still near enough to "annoy" them without
* being close enough to chase directly.  I have no idea what will
* happen if you combine "smell" with low "aaf" values.
*/

/*
* Provide a location to flee to, but give the player a wide berth.
*
* A monster may wish to flee to a location that is behind the player,
* but instead of heading directly for it, the monster should "swerve"
* around the player so that he has a smaller chance of getting hit.
*/
static bool get_fear_moves_aux(int m_idx, int *yp, int *xp)
{
	/* Monster flowing disabled */
	if (!options->flow_by_sound)
	{
		return false;
	}

	/* Monster location */
	monster_type *m_ptr = &m_list[m_idx];
	const int fy = m_ptr->fy;;
	const int fx = m_ptr->fx;

	/* Desired destination */
	int y1 = fy - (*yp);
	int x1 = fx - (*xp);

	/* The player is not currently near the monster grid */
	if (cave[fy][fx].when < cave[p_ptr->py][p_ptr->px].when)
	{
		/* No reason to attempt flowing */
		return false;
	}

	/* Monster is too far away to use flow information */
	auto const r_ptr = m_ptr->race();
	if (cave[fy][fx].cost > MONSTER_FLOW_DEPTH) return false;
	if (cave[fy][fx].cost > r_ptr->aaf) return false;

	/* Loop state */
	int when = 0;
	int gy = 0;
	int gx = 0;
	int score = -1;

	/* Check nearby grids, diagonals first */
	for (int i = 7; i >= 0; i--)
	{
		int dis, s;

		/* Get the location */
		const int y = fy + ddy_ddd[i];
		const int x = fx + ddx_ddd[i];

		/* Ignore illegal locations */
		if (cave[y][x].when == 0) continue;

		/* Ignore ancient locations */
		if (cave[y][x].when < when) continue;

		/* Calculate distance of this grid from our destination */
		dis = distance(y, x, y1, x1);

		/* Score this grid */
		s = 5000 / (dis + 3) - 500 / (cave[y][x].cost + 1);

		/* No negative scores */
		if (s < 0) s = 0;

		/* Ignore lower scores */
		if (s < score) continue;

		/* Save the score and time */
		when = cave[y][x].when;
		score = s;

		/* Save the location */
		gy = y;
		gx = x;
	}

	/* No legal move (?) */
	if (!when) return false;

	/* Find deltas */
	(*yp) = fy - gy;
	(*xp) = fx - gx;

	/* Success */
	return true;
}


/*
* Choose a "safe" location near a monster for it to run toward.
*
* A location is "safe" if it can be reached quickly and the player
* is not able to fire into it (it isn't a "clean shot").  So, this will
* cause monsters to "duck" behind walls.  Hopefully, monsters will also
* try to run towards corridor openings if they are in a room.
*
* This function may take lots of CPU time if lots of monsters are
* fleeing.
*
* Return true if a safe location is available.
*/
static bool find_safety(int m_idx, int *yp, int *xp)
{
	monster_type *m_ptr = &m_list[m_idx];

	int fy = m_ptr->fy;
	int fx = m_ptr->fx;

	int y, x, d, dis;
	int gy = 0, gx = 0, gdis = 0;

	/* Start with adjacent locations, spread further */
	for (d = 1; d < 10; d++)
	{
		/* Check nearby locations */
		for (y = fy - d; y <= fy + d; y++)
		{
			for (x = fx - d; x <= fx + d; x++)
			{
				/* Skip illegal locations */
				if (!in_bounds(y, x)) continue;

				/* Skip locations in a wall */
				if (!cave_floor_bold(y, x)) continue;

				/* Check distance */
				if (distance(y, x, fy, fx) != d) continue;

				/* Check for "availability" (if monsters can flow) */
				if (options->flow_by_sound)
				{
					/* Ignore grids very far from the player */
					if (cave[y][x].when < cave[p_ptr->py][p_ptr->px].when) continue;

					/* Ignore too-distant grids */
					if (cave[y][x].cost > cave[fy][fx].cost + 2 * d) continue;
				}

				/* Check for absence of shot */
				if (!projectable(y, x, p_ptr->py, p_ptr->px))
				{
					/* Calculate distance from player */
					dis = distance(y, x, p_ptr->py, p_ptr->px);

					/* Remember if further than previous */
					if (dis > gdis)
					{
						gy = y;
						gx = x;
						gdis = dis;
					}
				}
			}
		}

		/* Check for success */
		if (gdis > 0)
		{
			/* Good location */
			(*yp) = fy - gy;
			(*xp) = fx - gx;

			/* Found safe place */
			return true;
		}
	}

	/* No safe place */
	return false;
}


/*
 * Choose a good hiding place near a monster for it to run toward.
 *
 * Pack monsters will use this to "ambush" the player and lure him out
 * of corridors into open space so they can swarm him.
 *
 * Return true if a good location is available.
 */
static bool find_hiding(int m_idx, int *yp, int *xp)
{
	monster_type *m_ptr = &m_list[m_idx];

	int fy = m_ptr->fy;
	int fx = m_ptr->fx;

	int y, x, d, dis;
	int gy = 0, gx = 0, gdis = 999, min;

	/* Closest distance to get */
	min = distance(p_ptr->py, p_ptr->px, fy, fx) * 3 / 4 + 2;

	/* Start with adjacent locations, spread further */
	for (d = 1; d < 10; d++)
	{
		/* Check nearby locations */
		for (y = fy - d; y <= fy + d; y++)
		{
			for (x = fx - d; x <= fx + d; x++)
			{
				/* Skip illegal locations */
				if (!in_bounds(y, x)) continue;

				/* Skip locations in a wall */
				if (!cave_floor_bold(y, x)) continue;

				/* Check distance */
				if (distance(y, x, fy, fx) != d) continue;

				/* Check for hidden, available grid */
				if (!player_can_see_bold(y, x) && clean_shot(fy, fx, y, x))
				{
					/* Calculate distance from player */
					dis = distance(y, x, p_ptr->py, p_ptr->px);

					/* Remember if closer than previous */
					if (dis < gdis && dis >= min)
					{
						gy = y;
						gx = x;
						gdis = dis;
					}
				}
			}
		}

		/* Check for success */
		if (gdis < 999)
		{
			/* Good location */
			(*yp) = fy - gy;
			(*xp) = fx - gx;

			/* Found good place */
			return true;
		}
	}

	/* No good place */
	return false;
}


/* Find an appropriate corpse */
void find_corpse(monster_type *m_ptr, int *y, int *x)
{
	auto const &r_info = game->edit_data.r_info;

	int last = -1;

	for (int k = 0; k < max_o_idx; k++)
	{
		object_type *o_ptr = &o_list[k];

		if (!o_ptr->k_ptr)
		{
			continue;
		}

		if (o_ptr->tval != TV_CORPSE)
		{
			continue;
		}

		if ((o_ptr->sval != SV_CORPSE_CORPSE) && (o_ptr->sval != SV_CORPSE_SKELETON))
		{
			continue;
		}

		auto rt_ptr = &r_info[o_ptr->pval2];

		/* Cannot incarnate into a higher level monster */
		if (rt_ptr->level > m_ptr->level)
		{
			continue;
		}

		/* Must be in LOS */
		if (!los(m_ptr->fy, m_ptr->fx, o_ptr->iy, o_ptr->ix))
		{
			continue;
		}

		if (last != -1)
		{
			auto rt2_ptr = &r_info[o_list[last].pval2];

			if (rt_ptr->level > rt2_ptr->level)
			{
				last = k;
			}
			else
			{
				continue;
			}
		}
		else
		{
			last = k;
		}
	}

	/* Must be ok now */
	if (last != -1)
	{
		*y = o_list[last].iy;
		*x = o_list[last].ix;
	}
}

/*
 * Choose target
 */
static void get_target_monster(int m_idx)
{
	auto const &r_info = game->edit_data.r_info;

	monster_type *m_ptr = &m_list[m_idx];
	int i, t = -1, d = 9999;

	/* Process the monsters (backwards) */
	for (i = m_max - 1; i >= 1; i--)
	{
		/* Access the monster */
		monster_type *t_ptr = &m_list[i];
		/* hack should call the function for ego monsters ... but no_target i not meant to be added by ego and it speeds up the code */
		auto rt_ptr = &r_info[t_ptr->r_idx];
		int dd;

		/* Ignore "dead" monsters */
		if (!t_ptr->r_idx) continue;

		if (m_idx == i) continue;

		/* Cannot be targeted */
		if (rt_ptr->flags & RF_NO_TARGET) continue;

		if (is_enemy(m_ptr, t_ptr) && (los(m_ptr->fy, m_ptr->fx, t_ptr->fy, t_ptr->fx) &&
		                               ((dd = distance(m_ptr->fy, m_ptr->fx, t_ptr->fy, t_ptr->fx)) < d)))
		{
			t = i;
			d = dd;
		}
	}
	/* Hack */
	if ((is_friend(m_ptr) < 0) && los(m_ptr->fy, m_ptr->fx, p_ptr->py, p_ptr->px) && (distance(m_ptr->fy, m_ptr->fx, p_ptr->py, p_ptr->px) < d)) t = 0;

	m_ptr->target = t;
}

/*
 * Choose "logical" directions for monster movement
 */
static bool get_moves(int m_idx, int *mm)
{
	monster_type *m_ptr = &m_list[m_idx];

	int move_val = 0;

	int y2 = p_ptr->py;
	int x2 = p_ptr->px;
	bool done = false;

	/* Oups get nearer */
	if ((is_friend(m_ptr) > 0) && (m_ptr->cdis > p_ptr->pet_follow_distance))
	{
		y2 = p_ptr->py;
		x2 = p_ptr->px;
	}
	/* Use the target */
	else if (!m_ptr->target)
	{
		y2 = p_ptr->py;
		x2 = p_ptr->px;
	}
	else if (m_ptr->target > 0)
	{
		y2 = m_list[m_ptr->target].fy;
		x2 = m_list[m_ptr->target].fx;
	}

	/* Hack doppleganger confuses monsters(even pets) */
	if (doppleganger)
	{
		if (magik(70))
		{
			y2 = m_list[doppleganger].fy;
			x2 = m_list[doppleganger].fx;
		}
	}

	/* Get the race */
	const auto r_ptr = m_ptr->race();

	/* A possessor is not interrested in the player, it only wants a corpse */
	if (r_ptr->flags & RF_POSSESSOR)
	{
		find_corpse(m_ptr, &y2, &x2);
	}

	/* Let quests redefine AI */
	if (r_ptr->flags & RF_AI_SPECIAL)
	{
		struct hook_monster_ai_in in = { m_idx, &m_list[m_idx] };
		struct hook_monster_ai_out out = { 0, 0 };
		if (process_hooks_new(HOOK_MONSTER_AI, &in, &out))
		{
			y2 = out.y;
			x2 = out.x;
		}
	}

	if (m_idx == p_ptr->control)
	{
		if ((r_ptr->flags & RF_AI_PLAYER) || magik(85))
		{
			if (distance(p_ptr->py, p_ptr->px, m_ptr->fy, m_ptr->fx) < 50)
			{
				y2 = m_ptr->fy + ddy[p_ptr->control_dir];
				x2 = m_ptr->fx + ddx[p_ptr->control_dir];
			}
		}
	}

	/* Extract the "pseudo-direction" */
	int y = m_ptr->fy - y2;
	int x = m_ptr->fx - x2;

	/* Tease the player */
	if (r_ptr->flags & RF_AI_ANNOY)
	{
		if (distance(m_ptr->fy, m_ptr->fx, y2, x2) < 4)
		{
			y = -y;
			x = -x;
		}
	}

	/* Death orbs .. */
	if (r_ptr->flags & RF_DEATH_ORB)
	{
		if (!los(m_ptr->fy, m_ptr->fx, y2, x2))
		{
			return false;
		}
	}

	if (is_friend(m_ptr) < 0)
	{
		int tx = x2, ty = y2;

		/*
		* Animal packs try to get the player out of corridors
		* (...unless they can move through walls -- TY)
		*/
		if ((r_ptr->flags & RF_FRIENDS) &&
		                (r_ptr->flags & RF_ANIMAL) &&
		                !((r_ptr->flags & RF_PASS_WALL) ||
		                  (r_ptr->flags & RF_KILL_WALL)))
		{
			int i, room = 0;

			/* Count room grids next to player */
			for (i = 0; i < 8; i++)
			{
				/* Check grid */
				if (cave[ty + ddy_ddd[i]][tx + ddx_ddd[i]].info & (CAVE_ROOM))
				{
					/* One more room grid */
					room++;
				}
			}

			/* Not in a room and strong player */
			if ((room < 8) && (p_ptr->chp > ((p_ptr->mhp * 3) / 4)))
			{
				/* Find hiding place */
				if (find_hiding(m_idx, &y, &x)) done = true;
			}
		}

		/* Monster groups try to surround the player */
		if (!done && (r_ptr->flags & RF_FRIENDS))
		{
			int i;

			/* Find an empty square near the target to fill */
			for (i = 0; i < 8; i++)
			{
				/* Pick squares near target (semi-randomly) */
				y2 = ty + ddy_ddd[(m_idx + i) & 7];
				x2 = tx + ddx_ddd[(m_idx + i) & 7];

				/* Already there? */
				if ((m_ptr->fy == y2) && (m_ptr->fx == x2))
				{
					/* Attack the target */
					y2 = ty;
					x2 = tx;

					break;
				}

				/* Ignore filled grids */
				if (!cave_empty_bold(y2, x2)) continue;

				/* Try to fill this hole */
				break;
			}

			/* Extract the new "pseudo-direction" */
			y = m_ptr->fy - y2;
			x = m_ptr->fx - x2;

			/* Done */
			done = true;
		}
	}

	/* Apply fear if possible and necessary */
	if (is_friend(m_ptr) > 0)
	{
		if (mon_will_run(m_idx))
		{
			/* XXX XXX Not very "smart" */
			y = ( -y), x = ( -x);
		}
	}
	else
	{
		if (!done && mon_will_run(m_idx))
		{
			/* Try to find safe place */
			if (!find_safety(m_idx, &y, &x))
			{
				/* This is not a very "smart" method XXX XXX */
				y = ( -y);
				x = ( -x);
			}
			else
			{
				/* Attempt to avoid the player */
				if (options->flow_by_sound)
				{
					/* Adjust movement */
					get_fear_moves_aux(m_idx, &y, &x);
				}
			}
		}
	}


	/* Check for no move */
	if (!x && !y) return false;

	/* Extract the "absolute distances" */
	int ay = ABS(y);
	int ax = ABS(x);

	/* Do something weird */
	if (y < 0) move_val += 8;
	if (x > 0) move_val += 4;

	/* Prevent the diamond maneuvre */
	if (ay > (ax << 1))
	{
		move_val++;
		move_val++;
	}
	else if (ax > (ay << 1))
	{
		move_val++;
	}

	/* Extract some directions */
	switch (move_val)
	{
	case 0:
		mm[0] = 9;
		if (ay > ax)
		{
			mm[1] = 8;
			mm[2] = 6;
			mm[3] = 7;
			mm[4] = 3;
		}
		else
		{
			mm[1] = 6;
			mm[2] = 8;
			mm[3] = 3;
			mm[4] = 7;
		}
		break;
	case 1:
	case 9:
		mm[0] = 6;
		if (y < 0)
		{
			mm[1] = 3;
			mm[2] = 9;
			mm[3] = 2;
			mm[4] = 8;
		}
		else
		{
			mm[1] = 9;
			mm[2] = 3;
			mm[3] = 8;
			mm[4] = 2;
		}
		break;
	case 2:
	case 6:
		mm[0] = 8;
		if (x < 0)
		{
			mm[1] = 9;
			mm[2] = 7;
			mm[3] = 6;
			mm[4] = 4;
		}
		else
		{
			mm[1] = 7;
			mm[2] = 9;
			mm[3] = 4;
			mm[4] = 6;
		}
		break;
	case 4:
		mm[0] = 7;
		if (ay > ax)
		{
			mm[1] = 8;
			mm[2] = 4;
			mm[3] = 9;
			mm[4] = 1;
		}
		else
		{
			mm[1] = 4;
			mm[2] = 8;
			mm[3] = 1;
			mm[4] = 9;
		}
		break;
	case 5:
	case 13:
		mm[0] = 4;
		if (y < 0)
		{
			mm[1] = 1;
			mm[2] = 7;
			mm[3] = 2;
			mm[4] = 8;
		}
		else
		{
			mm[1] = 7;
			mm[2] = 1;
			mm[3] = 8;
			mm[4] = 2;
		}
		break;
	case 8:
		mm[0] = 3;
		if (ay > ax)
		{
			mm[1] = 2;
			mm[2] = 6;
			mm[3] = 1;
			mm[4] = 9;
		}
		else
		{
			mm[1] = 6;
			mm[2] = 2;
			mm[3] = 9;
			mm[4] = 1;
		}
		break;
	case 10:
	case 14:
		mm[0] = 2;
		if (x < 0)
		{
			mm[1] = 3;
			mm[2] = 1;
			mm[3] = 6;
			mm[4] = 4;
		}
		else
		{
			mm[1] = 1;
			mm[2] = 3;
			mm[3] = 4;
			mm[4] = 6;
		}
		break;
	case 12:
		mm[0] = 1;
		if (ay > ax)
		{
			mm[1] = 2;
			mm[2] = 4;
			mm[3] = 3;
			mm[4] = 7;
		}
		else
		{
			mm[1] = 4;
			mm[2] = 2;
			mm[3] = 7;
			mm[4] = 3;
		}
		break;
	}



	/* Wants to move... */
	return true;
}


int check_hit2(int power, int level, int ac)
{
	int i, k;

	/* Percentile dice */
	k = rand_int(100);

	/* Hack -- Always miss or hit */
	if (k < 10) return (k < 5);

	/* Calculate the "attack quality" */
	i = (power + (level * 3));

	/* Power and Level compete against Armor */
	if ((i > 0) && (randint(i) > ((ac * 3) / 4))) return true;

	/* Assume miss */
	return false;
}


/* Monster attacks monster */
static bool monst_attack_monst(int m_idx, int t_idx)
{
	char temp[80];
	bool blinked = false;
	bool touched = false;
	bool explode = false;
	monster_type *t_ptr = &m_list[t_idx];
	byte y_saver = t_ptr->fy;
	byte x_saver = t_ptr->fx;

	/* Get the racial information on the two monsters */
	monster_type *m_ptr = &m_list[m_idx];
	const auto r_ptr = m_ptr->race();
	const auto tr_ptr = t_ptr->race();

	/* Not allowed to attack */
	if (r_ptr->flags & RF_NEVER_BLOW) return false;

	/* Total armor */
	const int ac = t_ptr->ac;

	/* Extract the effective monster level */
	const int rlev = ((m_ptr->level >= 1) ? m_ptr->level : 1);

	/* Get the monster name (or "it") */
	char m_name[80];
	monster_desc(m_name, m_ptr, 0);

	/* Get the monster name (or "it") */
	char t_name[80];
	monster_desc(t_name, t_ptr, 0);

	/* Get the "died from" information (i.e. "a kobold") */
	char ddesc[80];
	monster_desc(ddesc, m_ptr, 0x88);

	/* Assume no blink */
	blinked = false;

	if (!(m_ptr->ml || t_ptr->ml))
	{
		monster_msg("You hear noise.");
	}

	/* Scan through all four blows */
	for (int ap_cnt = 0; ap_cnt < 4; ap_cnt++)
	{
		int power = 0;
		int damage = 0;

		const char *act = NULL;

		/* Extract the attack infomation */
		int effect = m_ptr->blow[ap_cnt].effect;
		int method = m_ptr->blow[ap_cnt].method;
		int d_dice = m_ptr->blow[ap_cnt].d_dice;
		int d_side = m_ptr->blow[ap_cnt].d_side;

		if (t_ptr == m_ptr) /* Paranoia */
		{
			if (wizard)
				monster_msg("Monster attacking self?");
			break;
		}

		/* Stop attacking if the target dies! */
		if (t_ptr->fx != x_saver || t_ptr->fy != y_saver)
			break;

		/* Hack -- no more attacks */
		if (!method) break;

		if (blinked) /* Stop! */
		{
			/* break; */
		}

		/* Extract the attack "power" */
		power = get_attack_power(effect);


		/* Monster hits*/
		if (!effect || check_hit2(power, rlev, ac))
		{
			/* Always disturbing */
			disturb_on_other();

			/* Describe the attack method */
			switch (method)
			{
			case RBM_HIT:
				{
					act = "hits %s.";
					touched = true;
					break;
				}

			case RBM_TOUCH:
				{
					act = "touches %s.";
					touched = true;
					break;
				}

			case RBM_PUNCH:
				{
					act = "punches %s.";
					touched = true;
					break;
				}

			case RBM_KICK:
				{
					act = "kicks %s.";
					touched = true;
					break;
				}

			case RBM_CLAW:
				{
					act = "claws %s.";
					touched = true;
					break;
				}

			case RBM_BITE:
				{
					act = "bites %s.";
					touched = true;
					break;
				}

			case RBM_STING:
				{
					act = "stings %s.";
					touched = true;
					break;
				}

			case RBM_XXX1:
				{
					act = "XXX1's %s.";
					break;
				}

			case RBM_BUTT:
				{
					act = "butts %s.";
					touched = true;
					break;
				}

			case RBM_CRUSH:
				{
					act = "crushes %s.";
					touched = true;
					break;
				}

			case RBM_ENGULF:
				{
					act = "engulfs %s.";
					touched = true;
					break;
				}

			case RBM_CHARGE:
				{
					act = "charges %s.";
					touched = true;
					break;
				}

			case RBM_CRAWL:
				{
					act = "crawls on %s.";
					touched = true;
					break;
				}

			case RBM_DROOL:
				{
					act = "drools on %s.";
					touched = false;
					break;
				}

			case RBM_SPIT:
				{
					act = "spits on %s.";
					touched = false;
					break;
				}

			case RBM_EXPLODE:
				{
					act = "explodes.";
					explode = true;
					touched = false;
					break;
				}

			case RBM_GAZE:
				{
					act = "gazes at %s.";
					touched = false;
					break;
				}

			case RBM_WAIL:
				{
					act = "wails at %s.";
					touched = false;
					break;
				}

			case RBM_SPORE:
				{
					act = "releases spores at %s.";
					touched = false;
					break;
				}

			case RBM_XXX4:
				{
					act = "projects XXX4's at %s.";
					touched = false;
					break;
				}

			case RBM_BEG:
				{
					act = "begs %s for money.";
					touched = false;
					t_ptr->csleep = 0;
					break;
				}

			case RBM_INSULT:
				{
					act = "insults %s.";
					touched = false;
					t_ptr->csleep = 0;
					break;
				}

			case RBM_MOAN:
				{
					act = "moans at %s.";
					touched = false;
					t_ptr->csleep = 0;
					break;
				}

			case RBM_SHOW:
				{
					act = "sings to %s.";
					touched = false;
					t_ptr->csleep = 0;
					break;
				}
			}

			/* Message */
			if (act)
			{
				strnfmt(temp, sizeof(temp), act, t_name);
				if (m_ptr->ml || t_ptr->ml)
					monster_msg("%^s %s", m_name, temp);

			}

			/* Roll out the damage */
			damage = damroll(d_dice, d_side);

			/* Hack need more punch against monsters */
			damage *= 3;

			int pt = GF_MISSILE;

			/* Apply appropriate damage */
			switch (effect)
			{
			case 0:
				{
					damage = 0;
					pt = 0;
					break;
				}

			case RBE_HURT:
			case RBE_SANITY:
				{
					damage -= (damage * ((ac < 150) ? ac : 150) / 250);
					break;
				}

			case RBE_POISON:
			case RBE_DISEASE:
				{
					pt = GF_POIS;
					break;
				}

			case RBE_UN_BONUS:
			case RBE_UN_POWER:
			case RBE_ABOMINATION:
				{
					pt = GF_DISENCHANT;
					break;
				}

			case RBE_EAT_FOOD:
			case RBE_EAT_LITE:
				{
					pt = damage = 0;
					break;
				}

			case RBE_EAT_ITEM:
			case RBE_EAT_GOLD:
				{
					pt = damage = 0;
					if (randint(2) == 1) blinked = true;
					break;
				}

			case RBE_ACID:
				{
					pt = GF_ACID;
					break;
				}

			case RBE_ELEC:
				{
					pt = GF_ELEC;
					break;
				}

			case RBE_FIRE:
				{
					pt = GF_FIRE;
					break;
				}

			case RBE_COLD:
				{
					pt = GF_COLD;
					break;
				}

			case RBE_BLIND:
				{
					break;
				}

			case RBE_HALLU:
			case RBE_CONFUSE:
				{
					pt = GF_CONFUSION;
					break;
				}

			case RBE_TERRIFY:
				{
					pt = GF_TURN_ALL;
					break;
				}

			case RBE_PARALYZE:
				{
					pt = GF_OLD_SLEEP;  /* sort of close... */
					break;
				}

			case RBE_LOSE_STR:
			case RBE_LOSE_INT:
			case RBE_LOSE_WIS:
			case RBE_LOSE_DEX:
			case RBE_LOSE_CON:
			case RBE_LOSE_CHR:
			case RBE_LOSE_ALL:
			case RBE_PARASITE:
				{
					break;
				}
			case RBE_SHATTER:
				{
					if (damage > 23)
					{
						/* Prevent destruction of quest levels and town */
						if (!is_quest(dun_level) && dun_level)
							earthquake(m_ptr->fy, m_ptr->fx, 8);
					}
					break;
				}
			case RBE_EXP_10:
			case RBE_EXP_20:
			case RBE_EXP_40:
			case RBE_EXP_80:
				{
					pt = GF_NETHER;
					break;
				}
			case RBE_TIME:
				{
					pt = GF_TIME;
					break;
				}
			default:
				{
					pt = 0;
					break;
				}
			}

			if (pt)
			{
				/* Do damage if not exploding */
				if (!explode)
				{
					project(m_idx, 0, t_ptr->fy, t_ptr->fx,
					        (pt == GF_OLD_SLEEP ? m_ptr->level : damage), pt, PROJECT_KILL | PROJECT_STOP);
				}

				if (touched)
				{
					/* Aura fire */
					if ((tr_ptr->flags & RF_AURA_FIRE) &&
					                !(r_ptr->flags & RF_IM_FIRE))
					{
						if (m_ptr->ml || t_ptr->ml)
						{
							blinked = false;
							monster_msg("%^s is suddenly very hot!", m_name);
						}
						project(t_idx, 0, m_ptr->fy, m_ptr->fx,
						        damroll (1 + ((t_ptr->level) / 26),
						                 1 + ((t_ptr->level) / 17)),
						        GF_FIRE, PROJECT_KILL | PROJECT_STOP);
					}

					/* Aura elec */
					if ((tr_ptr->flags & RF_AURA_ELEC) && !(r_ptr->flags & RF_IM_ELEC))
					{
						if (m_ptr->ml || t_ptr->ml)
						{
							blinked = false;
							monster_msg("%^s gets zapped!", m_name);
						}
						project(t_idx, 0, m_ptr->fy, m_ptr->fx,
						        damroll (1 + ((t_ptr->level) / 26),
						                 1 + ((t_ptr->level) / 17)),
						        GF_ELEC, PROJECT_KILL | PROJECT_STOP);
					}

				}
			}
		}

		/* Monster missed player */
		else
		{
			/* Analyze failed attacks */
			switch (method)
			{
			case RBM_HIT:
			case RBM_TOUCH:
			case RBM_PUNCH:
			case RBM_KICK:
			case RBM_CLAW:
			case RBM_BITE:
			case RBM_STING:
			case RBM_XXX1:
			case RBM_BUTT:
			case RBM_CRUSH:
			case RBM_ENGULF:
			case RBM_CHARGE:
				{
					/* Visible monsters */
					if (m_ptr->ml)
					{
						/* Disturbing */
						disturb();

						/* Message */
						monster_msg("%^s misses %s.", m_name, t_name);
					}

					break;
				}
			}
		}
	}

	if (explode)
	{
		mon_take_hit_mon(m_idx, m_idx, m_ptr->hp + 1, " explodes into tiny shreds.");

		blinked = false;
	}


	/* Blink away */
	if (blinked)
	{
		if (m_ptr->ml)
		{
			monster_msg("The thief flees laughing!");
		}
		else
		{
			monster_msg("You hear laughter!");
		}

		teleport_away(m_idx, MAX_SIGHT * 2 + 5);
	}

	return true;
}


/*
 * Hack -- local "player stealth" value (see below)
 */
static u32b noise = 0L;

/* Determine whether the player is invisible to a monster */
static bool player_invis(monster_type * m_ptr)
{
	const auto r_ptr = m_ptr->race();
	s16b inv = p_ptr->invis;
	s16b mlv = m_ptr->level;

	if (r_ptr->flags & RF_NO_SLEEP)
		mlv += 10;
	if (r_ptr->flags & RF_DRAGON)
		mlv += 20;
	if (r_ptr->flags & RF_UNDEAD)
		mlv += 15;
	if (r_ptr->flags & RF_DEMON)
		mlv += 15;
	if (r_ptr->flags & RF_ANIMAL)
		mlv += 15;
	if (r_ptr->flags & RF_ORC)
		mlv -= 15;
	if (r_ptr->flags & RF_TROLL)
		mlv -= 10;
	if (r_ptr->flags & RF_STUPID)
		mlv /= 2;
	if (r_ptr->flags & RF_SMART)
		mlv = (mlv * 5) / 4;
	if (m_ptr->mflag & MFLAG_QUEST)
		inv = 0;
	if (r_ptr->flags & RF_INVISIBLE)
		inv = 0;
	if (m_ptr->mflag & MFLAG_CONTROL)
		inv = 0;
	if (mlv < 1)
		mlv = 1;
	return (inv >= randint(mlv*2));
}

/*
 * Process a monster
 *
 * The monster is known to be within 100 grids of the player
 *
 * In several cases, we directly update the monster lore
 *
 * Note that a monster is only allowed to "reproduce" if there
 * are a limited number of "reproducing" monsters on the current
 * level.  This should prevent the level from being "swamped" by
 * reproducing monsters.  It also allows a large mass of mice to
 * prevent a louse from multiplying, but this is a small price to
 * pay for a simple multiplication method.
 *
 * XXX Monster fear is slightly odd, in particular, monsters will
 * fixate on opening a door even if they cannot open it.  Actually,
 * the same thing happens to normal monsters when they hit a door
 *
 * XXX XXX XXX In addition, monsters which *cannot* open or bash
 * down a door will still stand there trying to open it...
 *
 * XXX Technically, need to check for monster in the way
 * combined with that monster being in a wall (or door?)
 *
 * A "direction" of "5" means "pick a random direction".
 */
static void process_monster(int m_idx)
{
	auto const &f_info = game->edit_data.f_info;

	int i, d, oy, ox, ny, nx;

	int mm[8];

	monster_type *m_ptr = &m_list[m_idx];
	const bool inv = player_invis(m_ptr);

	auto const r_ptr = m_ptr->race();
	if (r_ptr->flags & RF_DOPPLEGANGER) doppleganger = m_idx;

	/* Handle "bleeding" */
	if (m_ptr->bleeding)
	{
		int d = 1 + (m_ptr->maxhp / 50);
		if (d > m_ptr->bleeding) d = m_ptr->bleeding;

		/* Exit if the monster dies */
		if (mon_take_hit(m_idx, d, nullptr, " bleeds to death.")) return;

		/* Hack -- Recover from bleeding */
		if (m_ptr->bleeding > d)
		{
			/* Recover somewhat */
			m_ptr->bleeding -= d;
		}

		/* Fully recover */
		else
		{
			/* Recover fully */
			m_ptr->bleeding = 0;

			/* Message if visible */
			if (m_ptr->ml)
			{
				char m_name[80];

				/* Get the monster name */
				monster_desc(m_name, m_ptr, 0);

				/* Dump a message */
				msg_format("%^s is no longer bleeding.", m_name);

				/* Hack -- Update the health bar */
				if (health_who == m_idx) p_ptr->redraw |= (PR_FRAME);
			}
		}
	}

	/* Handle "poisoned" */
	if (m_ptr->poisoned)
	{
		int d = (m_ptr->poisoned) / 10;
		if (d < 1) d = 1;

		/* Exit if the monster dies */
		if (mon_take_hit(m_idx, d, nullptr, " dies of poison.")) return;

		/* Hack -- Recover from bleeding */
		if (m_ptr->poisoned > d)
		{
			/* Recover somewhat */
			m_ptr->poisoned -= d;
		}

		/* Fully recover */
		else
		{
			/* Recover fully */
			m_ptr->poisoned = 0;

			/* Message if visible */
			if (m_ptr->ml)
			{
				char m_name[80];

				/* Get the monster name */
				monster_desc(m_name, m_ptr, 0);

				/* Dump a message */
				msg_format("%^s is no longer poisoned.", m_name);

				/* Hack -- Update the health bar */
				if (health_who == m_idx) p_ptr->redraw |= (PR_FRAME);
			}
		}
	}

	/* Handle "sleep" */
	if (m_ptr->csleep)
	{
		u32b notice = 0;

		/* Hack -- handle non-aggravation */
		if (!p_ptr->aggravate) notice = rand_int(1024);

		/* Hack -- See if monster "notices" player */
		if ((notice * notice * notice) <= noise)
		{
			/* Hack -- amount of "waking" */
			int d = 1;

			/* Wake up faster near the player */
			if (m_ptr->cdis < 50) d = (100 / m_ptr->cdis);

			/* Hack -- handle aggravation */
			if (p_ptr->aggravate) d = m_ptr->csleep;

			/* Still asleep */
			if (m_ptr->csleep > d)
			{
				/* Monster wakes up "a little bit" */
				m_ptr->csleep -= d;
			}
			/* Just woke up */
			else
			{
				/* Reset sleep counter */
				m_ptr->csleep = 0;

				/* Notice the "waking up" */
				if (m_ptr->ml)
				{
					char m_name[80];

					/* Acquire the monster name */
					monster_desc(m_name, m_ptr, 0);

					/* Dump a message */
					msg_format("%^s wakes up.", m_name);
				}
			}
		}

		/* Still sleeping */
		if (m_ptr->csleep) return;
	}


	/* Handle "stun" */
	if (m_ptr->stunned)
	{
		int d = 1;

		/* Make a "saving throw" against stun */
		if (rand_int(5000) <= m_ptr->level * m_ptr->level)
		{
			/* Recover fully */
			d = m_ptr->stunned;
		}

		/* Hack -- Recover from stun */
		if (m_ptr->stunned > d)
		{
			/* Recover somewhat */
			m_ptr->stunned -= d;
		}

		/* Fully recover */
		else
		{
			/* Recover fully */
			m_ptr->stunned = 0;

			/* Message if visible */
			if (m_ptr->ml)
			{
				char m_name[80];

				/* Acquire the monster name */
				monster_desc(m_name, m_ptr, 0);

				/* Dump a message */
				msg_format("%^s is no longer stunned.", m_name);
			}
		}

		/* Still stunned */
		if (m_ptr->stunned) return;
	}


	/* Handle confusion */
	if (m_ptr->confused)
	{
		/* Amount of "boldness" */
		int d = randint(m_ptr->level / 10 + 1);

		/* Still confused */
		if (m_ptr->confused > d)
		{
			/* Reduce the confusion */
			m_ptr->confused -= d;
		}

		/* Recovered */
		else
		{
			/* No longer confused */
			m_ptr->confused = 0;

			/* Message if visible */
			if (m_ptr->ml)
			{
				char m_name[80];

				/* Acquire the monster name */
				monster_desc(m_name, m_ptr, 0);

				/* Dump a message */
				msg_format("%^s is no longer confused.", m_name);
			}
		}
	}

	/* Do the monster get angry? */
	bool gets_angry = false;

	/* No one wants to be your friend if you're aggravating */
	if ((m_ptr->status > MSTATUS_NEUTRAL) && (m_ptr->status < MSTATUS_COMPANION) && (p_ptr->aggravate) && !(r_ptr->flags & RF_PET))
		gets_angry = true;

	/* Paranoia... no friendly uniques outside wizard mode -- TY */
	if ((m_ptr->status > MSTATUS_NEUTRAL) && (m_ptr->status < MSTATUS_COMPANION) && !(wizard) &&
	                (r_ptr->flags & RF_UNIQUE) && !(r_ptr->flags & RF_PET))
		gets_angry = true;

	if (gets_angry)
	{
		char m_name[80];
		monster_desc(m_name, m_ptr, 0);
		switch (is_friend(m_ptr))
		{
		case 1:
			msg_format("%^s suddenly becomes hostile!", m_name);
			change_side(m_ptr);
			break;
		}
	}

	/* Handle "fear" */
	if (m_ptr->monfear)
	{
		/* Amount of "boldness" */
		int d = randint(m_ptr->level / 10 + 1);

		/* Still afraid */
		if (m_ptr->monfear > d)
		{
			/* Reduce the fear */
			m_ptr->monfear -= d;
		}

		/* Recover from fear, take note if seen */
		else
		{
			/* No longer afraid */
			m_ptr->monfear = 0;

			/* Visual note */
			if (m_ptr->ml)
			{
				char m_name[80];
				char m_poss[80];

				/* Acquire the monster name/poss */
				monster_desc(m_name, m_ptr, 0);
				monster_desc(m_poss, m_ptr, 0x22);

				/* Dump a message */
				msg_format("%^s recovers %s courage.", m_name, m_poss);
			}
		}
	}

	/* Get the origin */
	oy = m_ptr->fy;
	ox = m_ptr->fx;

	/* Attempt to "multiply" if able and allowed */
	if ((r_ptr->spells & SF_MULTIPLY) && (num_repro < MAX_REPRO))
	{
		if (ai_multiply(m_idx)) return;
	}

	if (randint(SPEAK_CHANCE) == 1)
	{
		if (player_has_los_bold(oy, ox) && (r_ptr->flags & RF_CAN_SPEAK))
		{
			char m_name[80];
			char monmessage[1024];

			/* Acquire the monster name/poss */
			if (m_ptr->ml)
				monster_desc(m_name, m_ptr, 0);
			else
				strcpy(m_name, "It");

			/* xtra_line function by Matt Graham--allow uniques to */
			/* say "unique" things based on their monster index.   */
			/* Try for the unique's lines in "monspeak.txt" first. */
			/* 0 is SUCCESS, of course....                         */

			struct hook_mon_speak_in in = { m_idx, m_name };
			if (!process_hooks_new(HOOK_MON_SPEAK, &in, NULL))
			{
				if (get_xtra_line("monspeak.txt", m_ptr, monmessage) != 0)
				{
					/* Get a message from old defaults if new don't work */

					if (is_friend(m_ptr) > 0)
						get_rnd_line("speakpet.txt", monmessage);
					else if (m_ptr->monfear)
						get_rnd_line("monfear.txt", monmessage);
					else
						get_rnd_line("bravado.txt", monmessage);
				}
				msg_format("%^s %s", m_name, monmessage);
			}
		}
	}

	/* Need a new target ? */
	if ((m_ptr->target == -1) || magik(10)) get_target_monster(m_idx);


	/* Attempt to cast a spell */
	if (make_attack_spell(m_idx)) return;

	/*
	 * Attempt to cast a spell at an enemy other than the player
	 * (may slow the game a smidgeon, but I haven't noticed.)
	 */
	hack_message_pain_may_silent = true;
	if (monst_spell_monst(m_idx))
	{
		hack_message_pain_may_silent = false;
		return;
	}
	hack_message_pain_may_silent = false;


	/* Hack -- Assume no movement */
	mm[0] = mm[1] = mm[2] = mm[3] = 0;
	mm[4] = mm[5] = mm[6] = mm[7] = 0;

	/* Confused -- 100% random */
	if (m_ptr->confused || (inv == true && m_ptr->target == 0))
	{
		/* Try four "random" directions */
		mm[0] = mm[1] = mm[2] = mm[3] = 5;
	}

	/* 75% random movement */
	else if ((r_ptr->flags & RF_RAND_50) &&
			(r_ptr->flags & RF_RAND_25) &&
	                (rand_int(100) < 75))
	{
		/* Try four "random" directions */
		mm[0] = mm[1] = mm[2] = mm[3] = 5;
	}

	/* 50% random movement */
	else if ((r_ptr->flags & RF_RAND_50) &&
	                (rand_int(100) < 50))
	{
		/* Try four "random" directions */
		mm[0] = mm[1] = mm[2] = mm[3] = 5;
	}

	/* 25% random movement */
	else if ((r_ptr->flags & RF_RAND_25) &&
	                (rand_int(100) < 25))
	{
		/* Try four "random" directions */
		mm[0] = mm[1] = mm[2] = mm[3] = 5;
	}

	/* Normal movement */
	else
	{
		/* Logical moves, may do nothing */
		if (!get_moves(m_idx, mm)) return;
	}

	/* Paranoia -- quest code could delete it */
	cave_type *c_ptr = &cave[m_ptr->fy][m_ptr->fx];
	if (!c_ptr->m_idx) return;

	/* Assume nothing */
	bool do_turn = false;
	bool do_move = false;
	bool do_view = false;

	/* Assume nothing */
	bool did_open_door = false;
	bool did_bash_door = false;

	/* Take a zero-terminated array of "directions" */
	for (i = 0; mm[i]; i++)
	{
		/* Get the direction */
		d = mm[i];

		/* Hack -- allow "randomized" motion */
		if (d == 5) d = ddd[rand_int(8)];

		/* Get the destination */
		ny = oy + ddy[d];
		nx = ox + ddx[d];

		/* Access that cave grid */
		c_ptr = &cave[ny][nx];

		/* Access that cave grid's contents */
		monster_type *y_ptr = &m_list[c_ptr->m_idx];


		/* Floor is open? */
		if (cave_floor_bold(ny, nx))
		{
			/* Go ahead and move */
			do_move = true;
		}

		/* Hack -- check for Glyph of Warding */
		if ((c_ptr->feat == FEAT_GLYPH) &&
		                !(r_ptr->flags & RF_NEVER_BLOW))
		{
			/* Assume no move allowed */
			do_move = false;

			/* Break the ward */
			if (randint(BREAK_GLYPH) < m_ptr->level)
			{
				/* Describe observable breakage */
				if (c_ptr->info & CAVE_MARK)
				{
					msg_print("The rune of protection is broken!");
				}

				/* Forget the rune */
				c_ptr->info &= ~(CAVE_MARK);

				/* Break the rune */
				place_floor_convert_glass(ny, nx);

				/* Allow movement */
				do_move = true;
			}
		}

		/* Hack -- trees are obstacle */
		else if ((cave[ny][nx].feat == FEAT_TREES) && (r_ptr->flags & RF_KILL_TREES))
		{
			do_move = true;

			/* Forget the tree */
			c_ptr->info &= ~(CAVE_MARK);

			/* Notice */
			cave_set_feat(ny, nx, FEAT_GRASS);
		}

		/* Hack -- player 'in' wall */
		else if ((ny == p_ptr->py) && (nx == p_ptr->px))
		{
			do_move = true;
		}

		else if (c_ptr->m_idx)
		{
			/* Possibly a monster to attack */
			do_move = true;
		}

		/* Permanent wall */
		else if (f_info[c_ptr->feat].flags & FF_PERMANENT)
		{
			/* Nothing */
		}


		/* Some monsters can fly */
		else if ((f_info[c_ptr->feat].flags & FF_CAN_LEVITATE) && (r_ptr->flags & RF_CAN_FLY))
		{
			/* Pass through walls/doors/rubble */
			do_move = true;
		}

		/* Some monsters can fly */
		else if ((f_info[c_ptr->feat].flags & FF_CAN_FLY) && (r_ptr->flags & RF_CAN_FLY))
		{
			/* Pass through trees/... */
			do_move = true;
		}

		/* Monster moves through walls (and doors) */
		else if ((f_info[c_ptr->feat].flags & FF_CAN_PASS) && (r_ptr->flags & RF_PASS_WALL))
		{
			/* Pass through walls/doors/rubble */
			do_move = true;
		}

		/* Monster destroys walls (and doors) */
		else if ((f_info[c_ptr->feat].flags & FF_CAN_PASS) && (r_ptr->flags & RF_KILL_WALL))
		{
			/* Eat through walls/doors/rubble */
			do_move = true;

			if (randint(GRINDNOISE) == 1)
			{
				msg_print("There is a grinding sound.");
			}

			/* Forget the wall */
			c_ptr->info &= ~(CAVE_MARK);

			/* Notice */
			cave_set_feat(ny, nx, FEAT_FLOOR);

			/* Note changes to viewable region */
			if (player_has_los_bold(ny, nx)) do_view = true;
		}

		/* Monster moves through walls (and doors) */
		else if ((f_info[c_ptr->feat].flags & FF_CAN_PASS) && (r_ptr->flags & RF_PASS_WALL))
		{
			/* Pass through walls/doors/rubble */
			do_move = true;
		}

		/* Monster moves through webs */
		else if ((f_info[c_ptr->feat].flags & FF_WEB) &&
		                (r_ptr->flags & RF_SPIDER))
		{
			/* Pass through webs */
			do_move = true;
		}

		/* Handle doors and secret doors */
		else if (((c_ptr->feat >= FEAT_DOOR_HEAD) &&
		                (c_ptr->feat <= FEAT_DOOR_TAIL)) ||
		                (c_ptr->feat == FEAT_SECRET))
		{
			bool may_bash = true;

			/* Take a turn */
			do_turn = true;

			if ((r_ptr->flags & RF_OPEN_DOOR) &&
			                ((is_friend(m_ptr) <= 0) || p_ptr->pet_open_doors))
			{
				/* Closed doors and secret doors */
				if ((c_ptr->feat == FEAT_DOOR_HEAD) ||
				                (c_ptr->feat == FEAT_SECRET))
				{
					/* The door is open */
					did_open_door = true;

					/* Do not bash the door */
					may_bash = false;
				}

				/* Locked doors (not jammed) */
				else if (c_ptr->feat < FEAT_DOOR_HEAD + 0x08)
				{
					int k;

					/* Door power */
					k = ((c_ptr->feat - FEAT_DOOR_HEAD) & 0x07);

					/* Try to unlock it XXX XXX XXX */
					if (rand_int(m_ptr->hp / 10) > k)
					{
						/* Unlock the door */
						cave_set_feat(ny, nx, FEAT_DOOR_HEAD + 0x00);

						/* Do not bash the door */
						may_bash = false;
					}
				}
			}

			/* Stuck doors -- attempt to bash them down if allowed */
			if (may_bash && (r_ptr->flags & RF_BASH_DOOR) &&
			                ((is_friend(m_ptr) <= 0) || p_ptr->pet_open_doors))
			{
				int k;

				/* Door power */
				k = ((c_ptr->feat - FEAT_DOOR_HEAD) & 0x07);

				/* Attempt to Bash XXX XXX XXX */
				if (rand_int(m_ptr->hp / 10) > k)
				{
					/* Message */
					msg_print("You hear a door burst open!");

					/* Disturb (sometimes) */
					if (options->disturb_minor)
					{
						disturb();
					}

					/* The door was bashed open */
					did_bash_door = true;

					/* Hack -- fall into doorway */
					do_move = true;
				}
			}


			/* Deal with doors in the way */
			if (did_open_door || did_bash_door)
			{
				/* It's no longer hidden */
				cave[ny][nx].mimic = 0;

				/* Break down the door */
				if (did_bash_door && (rand_int(100) < 50))
				{
					cave_set_feat(ny, nx, FEAT_BROKEN);
				}

				/* Open the door */
				else
				{
					cave_set_feat(ny, nx, FEAT_OPEN);
				}

				/* Handle viewable doors */
				if (player_has_los_bold(ny, nx)) do_view = true;
			}
		}
		else if (do_move && (c_ptr->feat == FEAT_MINOR_GLYPH)
		                && !(r_ptr->flags & RF_NEVER_BLOW))
		{
			/* Assume no move allowed */
			do_move = false;

			/* Break the ward */
			if (randint(BREAK_MINOR_GLYPH) < m_ptr->level)
			{
				/* Describe observable breakage */
				if (c_ptr->info & CAVE_MARK)
				{
					if (ny == p_ptr->py && nx == p_ptr->px)
					{
						msg_print("The rune explodes!");
						fire_ball(GF_MANA, 0,
						          2 * ((p_ptr->lev / 2) + damroll(7, 7)), 2);
					}
					else
						msg_print("An explosive rune was disarmed.");
				}

				/* Forget the rune */
				c_ptr->info &= ~(CAVE_MARK);

				/* Break the rune */
				place_floor_convert_glass(ny, nx);

				/* Allow movement */
				do_move = true;
			}
		}

		/* Hack -- the Between teleport the monsters too */
		else if (cave[ny][nx].feat == FEAT_BETWEEN)
		{
			nx = cave[ny][nx].special & 255;
			ny = cave[ny][nx].special >> 8;
			get_pos_player(10, &ny, &nx);

			/* Access that cave grid */
			c_ptr = &cave[ny][nx];

			/* Access that cave grid's contents */
			y_ptr = &m_list[c_ptr->m_idx];

			if (!(r_ptr->flags & RF_IM_COLD))
			{
				if ((m_ptr->hp - distance(ny, nx, oy, ox)*2) <= 0)
				{
					ny = oy + ddy[d];
					nx = ox + ddx[d];
					do_move = false;
				}
				else
				{
					m_ptr->hp -= distance(ny, nx, oy, ox) * 2;
					do_move = true;
				}
			}
			else
			{
				do_move = true;
			}
		}

		/* Execute the inscription -- MEGA HACK -- */
		if ((c_ptr->inscription) && (c_ptr->inscription != INSCRIP_CHASM))
		{
			if (inscription_info[c_ptr->inscription].when & INSCRIP_EXEC_MONST_WALK)
			{
				bool t;
				t = execute_inscription(c_ptr->inscription, ny, nx);
				if (!t && do_move)
				{
					/* Hack -- attack the player even if on the inscription */
					if ((ny == p_ptr->py) && (nx == p_ptr->px))
						do_move = true;
					else
						do_move = false;
				}
			}
		}

		/* Some monsters never attack */
		if (do_move && (ny == p_ptr->py) && (nx == p_ptr->px) &&
		                (r_ptr->flags & RF_NEVER_BLOW))
		{
			/* Do not move */
			do_move = false;
		}

		/* The player is in the way.  Attack him. */
		if (do_move && (ny == p_ptr->py) && (nx == p_ptr->px))
		{
			/* Do the attack */
			make_attack_normal(m_idx, 1);

			/* Do not move */
			do_move = false;

			/* Took a turn */
			do_turn = true;
		}

		/* A monster is in the way */
		if (do_move && c_ptr->m_idx)
		{
			auto z_ptr = y_ptr->race();
			monster_type *m2_ptr = &m_list[c_ptr->m_idx];

			/* Assume no movement */
			do_move = false;

			/* Kill weaker monsters */
			if ((r_ptr->flags & RF_KILL_BODY) &&
			    (r_ptr->mexp > z_ptr->mexp) && (cave_floor_bold(ny, nx)) &&
			    /* Friends don't kill friends... */
			    !((is_friend(m_ptr) > 0) && (is_friend(m2_ptr) > 0)) &&
			    /* Uniques aren't faceless monsters in a crowd */
			    !(z_ptr->flags & RF_UNIQUE) &&
			    /* Don't wreck quests */
			    !(m2_ptr->mflag & (MFLAG_QUEST | MFLAG_QUEST2)) &&
			    /* Don't punish summoners for relying on their friends */
			    (is_friend(m2_ptr) <= 0))
			{
				/* Allow movement */
				do_move = true;

				/* Kill the monster */
				delete_monster(ny, nx);

				/* Hack -- get the empty monster */
				y_ptr = &m_list[c_ptr->m_idx];
			}

			/* Attack 'enemies' */
			else if (is_enemy(m_ptr, m2_ptr) || m_ptr->confused)
			{
				do_move = false;
				/* attack */
				if (m2_ptr->r_idx && (m2_ptr->hp >= 0))
				{
					hack_message_pain_may_silent = true;
					if (monst_attack_monst(m_idx, c_ptr->m_idx))
					{
						hack_message_pain_may_silent = false;
						return;
					}
					hack_message_pain_may_silent = false;
				}
			}

			/* Push past weaker monsters (unless leaving a wall) */
			else if ((r_ptr->flags & RF_MOVE_BODY) &&
			                (r_ptr->mexp > z_ptr->mexp) && cave_floor_bold(ny, nx) &&
			                (cave_floor_bold(m_ptr->fy, m_ptr->fx)))
			{
				/* Allow movement */
				do_move = true;
			}
		}

		/*
		 * Check if monster can cross terrain
		 * This is checked after the normal attacks
		 * to allow monsters to attack an enemy,
		 * even if it can't enter the terrain.
		 */
		if (do_move && !monster_can_cross_terrain(c_ptr->feat, r_ptr))
		{
			/* Assume no move allowed */
			do_move = false;
		}

		/* Some monsters never move */
		if (do_move && (r_ptr->flags & RF_NEVER_MOVE))
		{
			/* Do not move */
			do_move = false;
		}



		/* Creature has been allowed move */
		if (do_move)
		{
			/* Take a turn */
			do_turn = true;

			/* Hack -- Update the old location */
			cave[oy][ox].m_idx = c_ptr->m_idx;

			/* Mega-Hack -- move the old monster, if any */
			if (c_ptr->m_idx)
			{
				/* Move the old monster */
				y_ptr->fy = oy;
				y_ptr->fx = ox;

				/* Update the old monster */
				update_mon(c_ptr->m_idx, true);

				/* Wake up the moved monster */
				m_list[c_ptr->m_idx].csleep = 0;

				/*
				 * Update monster light -- I'm too lazy to check flags
				 * here, and those ego monster_race functions aren't
				 * re-entrant XXX XXX XXX
				 */
				p_ptr->update |= (PU_MON_LITE);
			}

			/* Hack -- Update the new location */
			c_ptr->m_idx = m_idx;

			/* Move the monster */
			m_ptr->fy = ny;
			m_ptr->fx = nx;

			/* Update the monster */
			update_mon(m_idx, true);

			/* Redraw the old grid */
			lite_spot(oy, ox);

			/* Redraw the new grid */
			lite_spot(ny, nx);

			/* Execute the inscription -- MEGA HACK -- */
			if (c_ptr->inscription == INSCRIP_CHASM)
			{
				if (inscription_info[c_ptr->inscription].when & INSCRIP_EXEC_MONST_WALK)
				{
					execute_inscription(c_ptr->inscription, ny, nx);
				}
			}

			/* Possible disturb */
			if (m_ptr->ml && (options->disturb_move ||
			                  ((m_ptr->mflag & (MFLAG_VIEW)) &&
			                   options->disturb_near)))
			{
				/* Disturb */
				if ((is_friend(m_ptr) < 0) || options->disturb_pets)
					disturb();
			}


			/* Copy list of objects; we need a copy because we're mutating the list. */
			auto const object_idxs(c_ptr->o_idxs);

			/* Scan all objects in the grid */
			for (auto const this_o_idx: object_idxs)
			{
				/* Acquire object */
				object_type * o_ptr = &o_list[this_o_idx];

				/* Skip gold */
				if (o_ptr->tval == TV_GOLD) continue;

				/* Incarnate ? */
				if ((o_ptr->tval == TV_CORPSE) && (r_ptr->flags & RF_POSSESSOR) &&
						((o_ptr->sval == SV_CORPSE_CORPSE) || (o_ptr->sval == SV_CORPSE_SKELETON)))
				{
					ai_possessor(m_idx, this_o_idx);
					return;
				}

				/* Take or Kill objects on the floor */
				/* rr9: Pets will no longer pick up/destroy items */
				if ((((r_ptr->flags & RF_TAKE_ITEM) &&
						((is_friend(m_ptr) <= 0) || p_ptr->pet_pickup_items)) ||
						(r_ptr->flags & RF_KILL_ITEM)) &&
						(is_friend(m_ptr) <= 0))
				{
					char m_name[80];
					char o_name[80];

					/* Extract some flags */
					auto const flags = object_flags(o_ptr);

					/* Acquire the object name */
					object_desc(o_name, o_ptr, true, 3);

					/* Acquire the monster name */
					monster_desc(m_name, m_ptr, 0x04);

					/* React to objects that hurt the monster */
					monster_race_flag_set flg;
					if (flags & TR_KILL_DEMON) flg |= RF_DEMON;
					if (flags & TR_KILL_UNDEAD) flg |= RF_UNDEAD;
					if (flags & TR_SLAY_DRAGON) flg |= RF_DRAGON;
					if (flags & TR_SLAY_TROLL) flg |= RF_TROLL;
					if (flags & TR_SLAY_GIANT) flg |= RF_GIANT;
					if (flags & TR_SLAY_ORC) flg |= RF_ORC;
					if (flags & TR_SLAY_DEMON) flg |= RF_DEMON;
					if (flags & TR_SLAY_UNDEAD) flg |= RF_UNDEAD;
					if (flags & TR_SLAY_ANIMAL) flg |= RF_ANIMAL;
					if (flags & TR_SLAY_EVIL) flg |= RF_EVIL;

					/* The object cannot be picked up by the monster */
					if (artifact_p(o_ptr) || (r_ptr->flags & flg))
					{
						/* Only give a message for "take_item" */
						if (r_ptr->flags & RF_TAKE_ITEM)
						{
							/* Describe observable situations */
							if (m_ptr->ml && player_has_los_bold(ny, nx))
							{
								/* Dump a message */
								msg_format("%^s tries to pick up %s, but fails.",
									   m_name, o_name);
							}
						}
					}

					/* Pick up the item */
					else if (r_ptr->flags & RF_TAKE_ITEM)
					{
						/* Describe observable situations */
						if (player_has_los_bold(ny, nx))
						{
							/* Dump a message */
							msg_format("%^s picks up %s.", m_name, o_name);
						}

						/* Put into inventory of monster */
						{
							/* Excise the object */
							excise_object_idx(this_o_idx);

							/* Forget mark */
							o_ptr->marked = false;

							/* Forget location */
							o_ptr->iy = o_ptr->ix = 0;

							/* Memorize monster */
							o_ptr->held_m_idx = m_idx;

							/* Carry object */
							m_ptr->hold_o_idxs.push_back(this_o_idx);
						}
					}

					/* Destroy the item */
					else
					{
						/* Describe observable situations */
						if (player_has_los_bold(ny, nx))
						{
							/* Dump a message */
							msg_format("%^s crushes %s.", m_name, o_name);
						}

						/* Delete the object */
						delete_object_idx(this_o_idx);
					}
				}
			}

			/* Update monster light */
			if (r_ptr->flags & RF_HAS_LITE) p_ptr->update |= (PU_MON_LITE);
		}

		/* Stop when done */
		if (do_turn) break;
	}


	/* If we haven't done anything, try casting a spell again */
	if (!do_turn && !do_move && !m_ptr->monfear &&
	                (is_friend(m_ptr) < 0))
	{
		/* Cast spell */
		if (make_attack_spell(m_idx)) return;
	}


	/* Notice changes in view */
	if (do_view)
	{
		/* Update some things */
		p_ptr->update |= (PU_VIEW | PU_FLOW | PU_MONSTERS | PU_MON_LITE);
	}


	/* Hack -- get "bold" if out of options */
	if (!do_turn && !do_move && m_ptr->monfear)
	{
		/* No longer afraid */
		m_ptr->monfear = 0;

		/* Message if seen */
		if (m_ptr->ml)
		{
			char m_name[80];

			/* Acquire the monster name */
			monster_desc(m_name, m_ptr, 0);

			/* Dump a message */
			msg_format("%^s turns to fight!", m_name);
		}

		/* XXX XXX XXX Actually do something now (?) */
	}
}


void summon_maint(int m_idx)
{

	monster_type *m_ptr = &m_list[m_idx];

	/* Can you pay? */
	if ((s32b)(p_ptr->maintain_sum / 10000) > p_ptr->csp)
	{
		char m_name[80];

		monster_desc(m_name, m_ptr, 0);

		msg_format("You lose control of %s.", m_name);

		/* Well, then, I guess I'm dead. */
		delete_monster_idx(m_idx);
	}
	else
	{
		s32b cl, ml, floor, cost;

		cl = get_skill_scale(SKILL_SUMMON, 100);
		ml = m_ptr->level * 10000;

		/* Floor = 19 * ml / 990 + 8 / 199
		   This gives a floor of 0.1 at level 1 and a floor of 2 at level 100

		   Since ml is multiplied by 10000 already, we multiply the 8/199 too
		   */
		floor = ml * 19 / 990 + 80000 / 199;
		cost = (ml / cl - 10000) / 4;
		if(cost < floor)
			cost = floor;

		/* Well, then I'll take my wages from you. */
		p_ptr->maintain_sum += cost;
	}
}


/*
 * Process all the "live" monsters, once per game turn.
 *
 * During each game turn, we scan through the list of all the "live" monsters,
 * (backwards, so we can excise any "freshly dead" monsters), energizing each
 * monster, and allowing fully energized monsters to move, attack, pass, etc.
 *
 * Note that monsters can never move in the monster array (except when the
 * "compact_monsters()" function is called by "dungeon()" or "save_player()").
 *
 * This function is responsible for at least half of the processor time
 * on a normal system with a "normal" amount of monsters and a player doing
 * normal things.
 *
 * When the player is resting, virtually 90% of the processor time is spent
 * in this function, and its children, "process_monster()" and "make_move()".
 *
 * Most of the rest of the time is spent in "update_view()" and "lite_spot()",
 * especially when the player is running.
 *
 * Note the special "MFLAG_BORN" flag, which allows us to ignore "fresh"
 * monsters while they are still being "born".  A monster is "fresh" only
 * during the turn in which it is created, and we use the "hack_m_idx" to
 * determine if the monster is yet to be processed during the current turn.
 *
 * Note the special "MFLAG_NICE" flag, which allows the player to get one
 * move before any "nasty" monsters get to use their spell attacks.
 *
 * Note that when the "knowledge" about the currently tracked monster
 * changes (flags, attacks, spells), we induce a redraw of the monster
 * recall window.
 */
void process_monsters()
{
	auto const &r_info = game->edit_data.r_info;

	int i, e;
	int fx, fy;

	bool test;
	bool is_frien = false;

	monster_type *m_ptr;

	/* Check the doppleganger */
	if (doppleganger && !(r_info[m_list[doppleganger].r_idx].flags & RF_DOPPLEGANGER))
		doppleganger = 0;

	/* Hack -- calculate the "player noise" */
	noise = (1L << (30 - p_ptr->skill_stl));


	/* Process the monsters (backwards) */
	for (i = m_max - 1; i >= 1; i--)
	{
		/* Access the monster */
		m_ptr = &m_list[i];

		/* Handle "leaving" */
		if (p_ptr->leaving) break;

		/* Ignore "dead" monsters */
		if (!m_ptr->r_idx) continue;

		/* Calculate "upkeep" for friendly monsters */
		if (m_ptr->status == MSTATUS_PET)
		{
			total_friends++;
			total_friend_levels += m_ptr->level;
		}


		/* Handle "fresh" monsters */
		if (m_ptr->mflag & (MFLAG_BORN))
		{
			/* No longer "fresh" */
			m_ptr->mflag &= ~(MFLAG_BORN);

			/* Skip */
			continue;
		}


		/* Obtain the energy boost */
		e = extract_energy[m_ptr->mspeed];

		/* Give this monster some energy */
		m_ptr->energy += e;


		/* Not enough energy to move */
		if (m_ptr->energy < 100) continue;

		/* Use up "some" energy */
		m_ptr->energy -= 100;


		/* Hack -- Require proximity */
		if (m_ptr->cdis >= 100) continue;


		/* Access the race */
		auto const r_ptr = m_ptr->race();

		/* Access the location */
		fx = m_ptr->fx;
		fy = m_ptr->fy;


		/* Assume no move */
		test = false;

		/* Control monster aint affected by distance */
		if (p_ptr->control == i)
		{
			test = true;
		}

		/* No free upkeep on partial summons just because they're out
		 * of line of sight. */
		else if (m_ptr->mflag & MFLAG_PARTIAL) test = true;

		/* Handle "sensing radius" */
		else if (m_ptr->cdis <= r_ptr->aaf)
		{
			/* We can "sense" the player */
			test = true;
		}

		/* Handle "sight" and "aggravation" */
		else if ((m_ptr->cdis <= MAX_SIGHT) &&
		                (player_has_los_bold(fy, fx) ||
		                 p_ptr->aggravate))
		{
			/* We can "see" or "feel" the player */
			test = true;
		}

		/* Hack -- Monsters can "smell" the player from far away */
		/* Note that most monsters have "aaf" of "20" or so */
		else if (options->flow_by_sound &&
		                (cave[p_ptr->py][p_ptr->px].when == cave[fy][fx].when) &&
		                (cave[fy][fx].cost < MONSTER_FLOW_DEPTH) &&
		                (cave[fy][fx].cost < r_ptr->aaf))
		{
			/* We can "smell" the player */
			test = true;
		}

		/* Running away wont save them ! */
		if (m_ptr->poisoned || m_ptr->bleeding) test = true;

		/* Do nothing */
		if (!test) continue;

		/* Save global index */
		hack_m_idx = i;

		if (is_friend(m_ptr) > 0) is_frien = true;

		/* Process the monster */
		process_monster(i);

		/* Hack -- notice death or departure */
		if (!alive || death) break;

		/* If it's still alive and friendly, charge upkeep. */
		if (m_ptr->mflag & MFLAG_PARTIAL) summon_maint(i);

		/* Notice leaving */
		if (p_ptr->leaving) break;
	}

	/* Reset global index */
	hack_m_idx = 0;
}
