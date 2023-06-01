/*
 * Copyright (c) 2001 James E. Wilson, Robert A. Koeneke, DarkGod
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "powers.hpp"

#include "cave.hpp"
#include "cave_type.hpp"
#include "cmd1.hpp"
#include "cmd2.hpp"
#include "cmd5.hpp"
#include "cmd7.hpp"
#include "dungeon_flag.hpp"
#include "feature_flag.hpp"
#include "feature_type.hpp"
#include "files.hpp"
#include "game.hpp"
#include "hooks.hpp"
#include "mimic.hpp"
#include "monster2.hpp"
#include "monster3.hpp"
#include "monster_race.hpp"
#include "monster_race_flag.hpp"
#include "monster_type.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "object_kind.hpp"
#include "options.hpp"
#include "player_type.hpp"
#include "spells1.hpp"
#include "spells2.hpp"
#include "stats.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "xtra2.hpp"
#include "z-rand.hpp"

#include <fmt/format.h>

static bool power_chance(power_activation const &x_ref)
{
	auto x_ptr = &x_ref;
	bool use_hp = false;
	int diff = x_ptr->diff;

	/* Always true ? */
	if (!x_ptr->cost)
	{
		return true;
	}

	/* Not enough mana - use hp */
	if (p_ptr->csp < x_ptr->cost)
	{
		use_hp = true;
	}

	/* Power is not available yet */
	if (p_ptr->lev < x_ptr->level)
	{
		msg_format("You need to attain level %d to use this power.", x_ptr->level);
		energy_use = 0;
		return false;
	}

	/* Too confused */
	else if (p_ptr->confused)
	{
		msg_print("You are too confused to use this power.");
		energy_use = 0;
		return false;
	}

	/* Risk death? */
	else if (use_hp && (p_ptr->chp < x_ptr->cost))
	{
		if (!(get_check("Really use the power in your weakened state? ")))
		{
			energy_use = 0;
			return false;
		}
	}

	/* Else attempt to do it! */

	if (p_ptr->stun)
	{
		diff += p_ptr->stun;
	}
	else if (p_ptr->lev > x_ptr->level)
	{
		int const lev_adj =
			std::min((p_ptr->lev - x_ptr->level) / 3, 10);

		diff -= lev_adj;
	}

	diff = std::max(diff, 5);

	/* take time and pay the price */
	if (use_hp)
	{
		take_hit(((x_ptr->cost / 2) + (randint(x_ptr->cost / 2))),
		         "concentrating too hard");
	}
	else
	{
		p_ptr->csp -= (x_ptr->cost / 2 ) + (randint(x_ptr->cost / 2));
	}
	energy_use = 100;

	/* Redraw mana and hp */
	p_ptr->redraw |= (PR_FRAME);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	/* Success? */
	if (randint(p_ptr->stat_cur[x_ptr->stat]) >=
	                ((diff / 2) + randint(diff / 2)))
	{
		return true;
	}

	flush_on_failure();
	msg_print("You've failed to concentrate hard enough.");

	return false;
}

static void power_activate(int power)
{
	auto const &f_info = game->edit_data.f_info;
	auto const &dungeon_flags = game->dungeon_flags;

	s16b plev = p_ptr->lev;
	char ch = 0;
	int amber_power = 0;
	int dir = 0, dummy;
	object_type *q_ptr;
	object_type forge;
	int ii = 0, ij = 0;
	const char *q;
	const char *s;
	int x, y;
	cave_type *c_ptr;

	/* Break goi/manashield */
	if (p_ptr->invuln)
	{
		set_invuln(0);
	}
	if (p_ptr->disrupt_shield)
	{
		set_disrupt_shield(0);
	}

	/* Check that we activate successfully */
	if (!power_chance(game->powers.at(power)->activation))
	{
		return;
	}

	switch (power)
	{
	case PWR_BALROG:
		{
			set_mimic(p_ptr->lev / 2, resolve_mimic_name("Balrog"), p_ptr->lev);
		}
		break;
	case PWR_BEAR:
		{
			set_mimic(150 + (p_ptr->lev * 10) , resolve_mimic_name("Bear"), p_ptr->lev);
		}
		break;
	case PWR_COMPANION:
		{
			if (!can_create_companion())
			{
				msg_print("You cannot have more companions.");
			}
			else
			{
				monster_type *m_ptr;
				int ii, jj;

				msg_print("Select the friendly monster:");
				if (!tgt_pt(&ii, &jj)) return;

				if (cave[jj][ii].m_idx)
				{
					m_ptr = &m_list[cave[jj][ii].m_idx];

					if (m_ptr->status != MSTATUS_PET)
					{
						msg_print("You cannot convert this monster.");
						return;
					}

					m_ptr->status = MSTATUS_COMPANION;
				}
			}
		}
		break;
	case PWR_MAGIC_MAP:
		{
			msg_print("You sense the world around you.");
			map_area();
		}
		break;

	case PWR_PASSWALL:
		{
			if (!get_aim_dir(&dir))
				break;
			if (passwall(dir, true))
				msg_print("A passage opens, and you step through.");
			else
				msg_print("There is no wall there!");
		}
		break;

	case PWR_COOK_FOOD:
		{
			/* Get local object */
			q_ptr = &forge;

			/* Create the item */
			object_prep(q_ptr, 21);

			/* Drop the object from heaven */
			drop_near(q_ptr, -1, p_ptr->py, p_ptr->px);
			msg_print("You cook some food.");
		}
		break;

	case PWR_UNFEAR:
		{
			msg_print("You play tough.");
			set_afraid(0);
		}
		break;

	case PWR_BERSERK:
		{
			msg_print("RAAAGH!");
			set_afraid(0);

			set_shero(p_ptr->shero + 10 + randint(plev));
			hp_player(30);
		}
		break;

	case PWR_EXPL_RUNE:
		{
			msg_print("You carefully set an explosive rune...");
			explosive_rune();
		}
		break;

	case PWR_STM:
		{
			if (!get_aim_dir(&dir)) break;
			msg_print("You bash at a stone wall.");
			wall_to_mud(dir);
		}
		break;

	case PWR_ROHAN:
		/* Select power to use */
		while (true)
		{
			if (!get_com("Use [F]lash aura or [L]ight speed jump? ", &ch))
			{
				amber_power = 0;
				break;
			}

			if (ch == 'F' || ch == 'f')
			{
				amber_power = 1;
				break;
			}

			if (ch == 'L' || ch == 'l')
			{
				amber_power = 2;
				break;
			}
		}

		if (amber_power == 1)
		{
			power_activation x_ptr_foo;
			x_ptr_foo.level = 1;
			x_ptr_foo.cost = 9;
			x_ptr_foo.stat = A_CHR;
			x_ptr_foo.diff = 7;
			if (power_chance(x_ptr_foo))
			{
				if (!(get_aim_dir(&dir))) break;
				msg_print("You flash a bright aura.");
				if (p_ptr->lev < 10)
					fire_bolt(GF_CONFUSION, dir, plev*2);
				else
					fire_ball(GF_CONFUSION, dir, plev*2, 2);
			}
		}
		if (amber_power == 2)
		{
			power_activation x_ptr_foo;
			x_ptr_foo.level = 30;
			x_ptr_foo.cost = 30;
			x_ptr_foo.stat = A_WIS;
			x_ptr_foo.diff = 7;
			if (power_chance(x_ptr_foo))
			{
				set_light_speed(p_ptr->lightspeed + 3);
			}
		}
		break;


	case PWR_POIS_DART:
		{
			if (!get_aim_dir(&dir)) break;
			msg_print("You throw a dart of poison.");
			fire_bolt(GF_POIS, dir, plev);
		}
		break;

	case PWR_DETECT_TD:
		{
			msg_print("You examine your surroundings.");
			detect_doors(DEFAULT_RADIUS);
			detect_stairs(DEFAULT_RADIUS);
		}
		break;

	case PWR_MAGIC_MISSILE:
		{
			if (!get_aim_dir(&dir)) break;
			msg_print("You cast a magic missile.");
			fire_bolt_or_beam(10, GF_MISSILE, dir,
			                  damroll(3 + ((plev - 1) / 5), 4));
		}
		break;

	case PWR_THUNDER:
		/* Select power to use */
		while (true)
		{
			if (!get_com("Use [T]hunder strike, [R]ide the straight road, go [B]ack in town? ", &ch))
			{
				amber_power = 0;
				break;
			}

			if (ch == 'T' || ch == 't')
			{
				amber_power = 1;
				break;
			}

			if (ch == 'R' || ch == 'r')
			{
				amber_power = 2;
				break;
			}

			if (ch == 'B' || ch == 'b')
			{
				amber_power = 3;
				break;
			}

		}

		if (amber_power == 1)
		{
			power_activation x_ptr_foo;
			x_ptr_foo.level = 1;
			x_ptr_foo.cost = p_ptr->lev;
			x_ptr_foo.stat = A_CON;
			x_ptr_foo.diff = 6;
			if (power_chance(x_ptr_foo))
			{
				if (!get_aim_dir(&dir)) break;
				msg_format("You conjure up thunder!");
				fire_beam(GF_ELEC, dir, p_ptr->lev * 2);
				fire_beam(GF_SOUND, dir, p_ptr->lev * 2);
				p_ptr->energy -= 100;
			}
		}
		if (amber_power == 2)
		{
			if (dungeon_flags & DF_NO_TELEPORT)
			{
				msg_print("No teleport on special levels ...");
				break;
			}

			power_activation x_ptr_foo;
			x_ptr_foo.level = 3;
			x_ptr_foo.cost = 15;
			x_ptr_foo.stat = A_CON;
			x_ptr_foo.diff = 6;
			if (power_chance(x_ptr_foo))
			{
				msg_print("You enter the straight road and fly beside the world. Where to exit?");
				if (!tgt_pt(&ii, &ij)) return;
				p_ptr->energy -= 60 - plev;
				if (!cave_empty_bold(ij, ii) || (cave[ij][ii].info & CAVE_ICKY) ||
				                (distance(ij, ii, p_ptr->py, p_ptr->px) > plev*20 + 2) || !(cave[ij][ii].info & CAVE_MARK))
				{
					msg_print("You fail to exit correctly!");
					p_ptr->energy -= 100;
					teleport_player(10);
				}
				else teleport_player_to(ij, ii);
			}

		}
		if (amber_power == 3)
		{
			if (dungeon_flags & DF_NO_TELEPORT)
			{
				msg_print("No recall on special levels..");
				break;
			}

			power_activation x_ptr_foo;
			x_ptr_foo.level = 7;
			x_ptr_foo.cost = 30;
			x_ptr_foo.stat = A_CON;
			x_ptr_foo.diff = 6;
			if (power_chance(x_ptr_foo))
			{
				if (dun_level == 0)
					msg_print("You are already in town!");
				else
				{
					msg_print("You enter the straight road and fly beside the world.");
					p_ptr->energy -= 100;
					p_ptr->word_recall = 1;
				}
			}
		}
		break;

	case PWR_GROW_TREE:
		{
			msg_print("You make the trees grow!");
			grow_trees((plev / 8 < 1) ? 1 : plev / 8);
		}
		break;

	case PWR_DEATHMOLD:
		do_cmd_immovable_special();
		break;

	case PWR_BR_COLD:
		{
			msg_print("You breathe cold...");
			if (get_aim_dir(&dir))
				fire_ball(GF_COLD, dir, p_ptr->lev * 2, 1 + (p_ptr->lev / 20));
		}
		break;

	case PWR_BR_CHAOS:
		{
			msg_print("You breathe chaos...");
			if (get_aim_dir(&dir))
				fire_ball(GF_CHAOS, dir, p_ptr->lev * 2, 1 + (p_ptr->lev / 20));
		}
		break;

	case PWR_BR_ELEM:
		{
			if (!get_aim_dir(&dir)) break;
			msg_format("You breathe the elements.");
			fire_ball(GF_MISSILE, dir, (p_ptr->lev)*2,
			          ((p_ptr->lev) / 15) + 1);
		}
		break;

	case PWR_SUMMON_MONSTER:
		{
			do_cmd_beastmaster();
		}
		break;

	case PWR_WRECK_WORLD:
		msg_print("The power of Eru Iluvatar flows through you!");
		msg_print("The world changes!");

		autosave_checkpoint();
		/* Leaving */
		p_ptr->leaving = true;
		break;

	case PWR_VAMPIRISM:
		{
			/* Only works on adjacent monsters */
			if (!get_rep_dir(&dir)) break;    /* was get_aim_dir */
			y = p_ptr->py + ddy[dir];
			x = p_ptr->px + ddx[dir];
			c_ptr = &cave[y][x];

			if (!(c_ptr->m_idx))
			{
				msg_print("You bite into thin air!");
				break;
			}

			msg_print("You grin and bare your fangs...");
			dummy = plev + randint(plev) * std::max(1, plev / 10);    /* Dmg */
			if (drain_life(dir, dummy))
			{
				if (p_ptr->food < PY_FOOD_FULL)
					/* No heal if we are "full" */
					hp_player(dummy);
				else
					msg_print("You were not hungry.");
				/* Gain nutritional sustenance: 150/hp drained */
				/* A Food ration gives 5000 food points (by contrast) */
				/* Don't ever get more than "Full" this way */
				/* But if we ARE Gorged,  it won't cure us */
				dummy = p_ptr->food + std::min(5000, 100 * dummy);
				if (p_ptr->food < PY_FOOD_MAX)   /* Not gorged already */
					set_food(dummy >= PY_FOOD_MAX ? PY_FOOD_MAX - 1 : dummy);
			}
			else
				msg_print("Yechh. That tastes foul.");
		}
		break;

	case PWR_SCARE:
		{
			msg_print("You emit an eldritch howl!");
			if (!get_aim_dir(&dir)) break;
			fear_monster(dir, plev);
		}
		break;

	case PWR_REST_LIFE:
		{
			msg_print("You attempt to restore your lost energies.");
			restore_level();
		}
		break;

	case PWR_HYPNO:
		{
			int dir, x, y;
			cave_type *c_ptr;
			monster_type *m_ptr;
			object_type *q_ptr;
			object_type forge;

			msg_print("Hypnotize which pet?");
			if (!get_rep_dir(&dir)) return;
			y = p_ptr->py + ddy[dir];
			x = p_ptr->px + ddx[dir];
			c_ptr = &cave[y][x];
			if (c_ptr->m_idx)
			{
				m_ptr = &m_list[c_ptr->m_idx];
				auto const r_ptr = m_ptr->race();

				if ((r_ptr->flags & RF_NEVER_MOVE) && (m_ptr->status == MSTATUS_PET) && (!(r_ptr->flags & RF_SPECIAL_GENE)))
				{
					q_ptr = &forge;
					object_prep(q_ptr, lookup_kind(TV_HYPNOS, 1));
					q_ptr->number = 1;
					q_ptr->pval = m_ptr->r_idx;
					q_ptr->pval2 = m_ptr->hp;
					object_aware(q_ptr);
					object_known(q_ptr);

					drop_near(q_ptr, 0, y, x);

					delete_monster(y, x);
					health_who = 0;
				}
				else
					msg_print("You can only hypnotize monsters that can't move.");
			}
			else msg_print("There is no pet here!");
		}
		break;

	case PWR_UNHYPNO:
		{
			monster_type *m_ptr;
			int m_idx;
			int item, x, y, d;
			object_type *o_ptr;

			const char *q;
			const char *s;

			/* Get an item */
			q = "Awaken which monster? ";
			s = "You have no monster to awaken.";
			if (!get_item(&item, q, s, (USE_FLOOR), object_filter::TVal(TV_HYPNOS))) return;

			o_ptr = &o_list[0 - item];

			d = 2;
			while (d < 100)
			{
				scatter(&y, &x, p_ptr->py, p_ptr->px, d);

				if (cave_floor_bold(y, x) && (!cave[y][x].m_idx)) break;

				d++;
			}

			if (d >= 100) return;

			if ((m_idx = place_monster_one(y, x, o_ptr->pval, 0, false, MSTATUS_PET)) == 0) return;

			m_ptr = &m_list[m_idx];
			m_ptr->hp = o_ptr->pval2;

			floor_item_increase(0 - item, -1);
			floor_item_describe(0 - item);
			floor_item_optimize(0 - item);
		}
		break;

	case PWR_NECRO:
		{
			do_cmd_necromancer();
			break;
		}
	case PWR_INCARNATE:
		{
			do_cmd_integrate_body();
			break;
		}

	case PWR_SPIT_ACID:
		{
			msg_print("You spit acid...");
			if (get_aim_dir(&dir))
				fire_ball(GF_ACID, dir, p_ptr->lev, 1 + (p_ptr->lev / 30));
		}
		break;

	case PWR_BR_FIRE:
		{
			msg_print("You breathe fire...");
			if (get_aim_dir(&dir))
				fire_ball(GF_FIRE, dir, p_ptr->lev * 2, 1 + (p_ptr->lev / 20));
		}
		break;

	case PWR_HYPN_GAZE:
		{
			msg_print("Your eyes look mesmerising...");
			if (get_aim_dir(&dir))
				charm_monster(dir, p_ptr->lev);
		}
		break;

	case PWR_TELEKINES:
		{
			msg_print("You concentrate...");
			if (get_aim_dir(&dir))
				fetch(dir, p_ptr->lev * 10, true);
		}
		break;

	case PWR_VTELEPORT:
		{
			msg_print("You concentrate...");
			teleport_player(10 + 4*(p_ptr->lev));
		}
		break;

	case PWR_MIND_BLST:
		{
			msg_print("You concentrate...");
			if (!get_aim_dir(&dir)) return;
			fire_bolt(GF_PSI, dir, damroll(3 + ((p_ptr->lev - 1) / 5), 3));
		}
		break;

	case PWR_RADIATION:
		{
			msg_print("Radiation flows from your body!");
			fire_ball(GF_NUKE, 0, (p_ptr->lev * 2), 3 + (p_ptr->lev / 20));
		}
		break;

	case PWR_SMELL_MET:
		{
			detect_treasure(DEFAULT_RADIUS);
		}
		break;

	case PWR_SMELL_MON:
		{
			detect_monsters_normal(DEFAULT_RADIUS);
		}
		break;

	case PWR_BLINK:
		{
			teleport_player(10);
		}
		break;

	case PWR_EAT_ROCK:
		{
			int x, y, ox, oy;
			cave_type *c_ptr;

			if (!get_rep_dir(&dir)) break;
			y = p_ptr->py + ddy[dir];
			x = p_ptr->px + ddx[dir];
			c_ptr = &cave[y][x];
			if (cave_floor_bold(y, x))
			{
				msg_print("You bite into thin air!");
				break;
			}
			else if ((f_info[c_ptr->feat].flags & FF_PERMANENT) || (c_ptr->feat == FEAT_MOUNTAIN))
			{
				msg_print("Ouch!  This wall is harder than your teeth!");
				break;
			}
			else if (c_ptr->m_idx)
			{
				msg_print("There's something in the way!");
				break;
			}
			else if (c_ptr->feat == FEAT_TREES)
			{
				msg_print("You don't like the woody taste!");
				break;
			}
			else
			{
				if ((c_ptr->feat >= FEAT_DOOR_HEAD) &&
				                (c_ptr->feat <= FEAT_RUBBLE))
				{
					set_food(p_ptr->food + 3000);
				}
				else if ((c_ptr->feat >= FEAT_MAGMA) &&
				                (c_ptr->feat <= FEAT_QUARTZ_K))
				{
					set_food(p_ptr->food + 5000);
				}
				else if ((c_ptr->feat >= FEAT_SANDWALL) &&
				                (c_ptr->feat <= FEAT_SANDWALL_K))
				{
					set_food(p_ptr->food + 500);
				}
				else
				{
					msg_print("This granite is very filling!");
					set_food(p_ptr->food + 10000);
				}
			}
			wall_to_mud(dir);

			oy = p_ptr->py;
			ox = p_ptr->px;

			p_ptr->py = y;
			p_ptr->px = x;

			lite_spot(p_ptr->py, p_ptr->px);
			lite_spot(oy, ox);

			verify_panel();

			p_ptr->update |= (PU_VIEW | PU_FLOW | PU_MON_LITE);
			p_ptr->update |= (PU_DISTANCE);
			p_ptr->window |= (PW_OVERHEAD);
		}
		break;

	case PWR_SWAP_POS:
		{
			if (!get_aim_dir(&dir)) return;
			teleport_swap(dir);
		}
		break;

	case PWR_SHRIEK:
		{
			fire_ball(GF_SOUND, 0, 4 * p_ptr->lev, 8);
			aggravate_monsters(0);
		}
		break;

	case PWR_ILLUMINE:
		{
			lite_area(damroll(2, (p_ptr->lev / 2)), (p_ptr->lev / 10) + 1);
		}
		break;

	case PWR_POLYMORPH:
		{
			do_poly_self();
		}
		break;

	case PWR_MIDAS_TCH:
		{
			alchemy();
		}
		break;

	case PWR_GROW_MOLD:
		{
			int i;
			for (i = 0; i < 8; i++)
			{
				summon_specific_friendly(p_ptr->py, p_ptr->px, p_ptr->lev, SUMMON_BIZARRE1, false);
			}
		}
		break;

	case PWR_RESIST:
		{
			int num = p_ptr->lev / 10;
			int dur = randint(20) + 20;

			if (rand_int(5) < num)
			{
				set_oppose_acid(p_ptr->oppose_acid + dur);
				num--;
			}
			if (rand_int(4) < num)
			{
				set_oppose_elec(p_ptr->oppose_elec + dur);
				num--;
			}
			if (rand_int(3) < num)
			{
				set_oppose_fire(p_ptr->oppose_fire + dur);
				num--;
			}
			if (rand_int(2) < num)
			{
				set_oppose_cold(p_ptr->oppose_cold + dur);
				num--;
			}
			if (num)
			{
				set_oppose_pois(p_ptr->oppose_pois + dur);
				num--;
			}
		}
		break;

	case PWR_EARTHQUAKE:
		{
			/* Prevent destruction of quest levels and town */
			if (!is_quest(dun_level) && dun_level)
				earthquake(p_ptr->py, p_ptr->px, 10);
		}
		break;

	case PWR_EAT_MAGIC:
		{
			object_type * o_ptr;
			int lev, item;

			/* Get an item */
			q = "Drain which item? ";
			s = "You have nothing to drain.";
			if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR), item_tester_hook_recharge())) break;

			o_ptr = get_object(item);

			lev = o_ptr->k_ptr->level;

			if (o_ptr->tval == TV_ROD_MAIN)
			{
				if (!o_ptr->timeout)
				{
					msg_print("You can't absorb energy from a discharged rod.");
				}
				else
				{
					p_ptr->csp += o_ptr->timeout;
					o_ptr->timeout = 0;
				}
			}
			else
			{
				if (o_ptr->pval > 0)
				{
					p_ptr->csp += o_ptr->pval * lev;
					o_ptr->pval = 0;
				}
				else
				{
					msg_print("There's no energy there to absorb!");
				}
			}

			if (p_ptr->csp > p_ptr->msp)
			{
				p_ptr->csp = p_ptr->msp;
			}

			p_ptr->notice |= (PN_COMBINE | PN_REORDER);
			p_ptr->window |= (PW_INVEN);
		}
		break;

	case PWR_WEIGH_MAG:
		{
			report_magics();
		}
		break;

	case PWR_STERILITY:
		{
			/* Fake a population explosion. */
			msg_print("You suddenly have a headache!");
			take_hit(randint(30) + 30, "the strain of forcing abstinence");
			num_repro += MAX_REPRO;
		}
		break;

	case PWR_PANIC_HIT:
		{
			int x, y;

			if (!get_rep_dir(&dir)) return;
			y = p_ptr->py + ddy[dir];
			x = p_ptr->px + ddx[dir];
			if (cave[y][x].m_idx)
			{
				py_attack(y, x, -1);
				teleport_player(30);
			}
			else
			{
				msg_print("You don't see any monster in this direction.");
				msg_print(NULL);
			}
		}
		break;

	case PWR_DAZZLE:
		{
			stun_monsters(p_ptr->lev * 4);
			confuse_monsters(p_ptr->lev * 4);
			turn_monsters(p_ptr->lev * 4);
		}
		break;

	case PWR_DARKRAY:
		{
			if (!get_aim_dir(&dir)) return;
			fire_beam(GF_LITE, dir, 2*p_ptr->lev);
		}
		break;

	case PWR_RECALL:
		{
			if (!(dungeon_flags & DF_ASK_LEAVE) || ((dungeon_flags & DF_ASK_LEAVE) && !get_check("Leave this unique level forever? ")))
			{
				recall_player(21, 15);
			}
		}
		break;

	case PWR_BANISH:
		{
			if (!get_rep_dir(&dir)) return;
			const int x = p_ptr->px + ddx[dir];
			const int y = p_ptr->py + ddy[dir];
			cave_type *c_ptr = &cave[y][x];
			if (!(c_ptr->m_idx))
			{
				msg_print("You sense no evil there!");
				break;
			}

			monster_type *m_ptr = &m_list[c_ptr->m_idx];
			auto const r_ptr = m_ptr->race();

			if (r_ptr->flags & RF_EVIL)
			{
				/* Delete the monster, rather than killing it. */
				delete_monster_idx(c_ptr->m_idx);
				msg_print("The evil creature vanishes in a puff of sulphurous smoke!");
			}
			else
			{
				msg_print("Your invocation is ineffectual!");
			}
		}
		break;

	case PWR_COLD_TOUCH:
		{
			int x, y;
			cave_type *c_ptr;

			if (!get_rep_dir(&dir)) return;
			y = p_ptr->py + ddy[dir];
			x = p_ptr->px + ddx[dir];
			c_ptr = &cave[y][x];
			if (!(c_ptr->m_idx))
			{
				msg_print("You wave your hands in the air.");
				break;
			}
			fire_bolt(GF_COLD, dir, 2 * (p_ptr->lev));
		}
		break;

	case PWR_LAUNCHER:
		{
			/* Gives a multiplier of 2 at first, up to 5 at 48th */
			p_ptr->throw_mult = 2 + (p_ptr->lev / 16);
			do_cmd_throw();
			p_ptr->throw_mult = 1;
		}
		break;

	case PWR_DODGE:
		use_ability_blade();
		break;

	case POWER_INVISIBILITY:
		set_invis(20 + randint(30), 30);
		break;

	case POWER_WEB:
		/* Warning, beware of f_info changes .. I hate to do that .. */
		grow_things(16, 1 + (p_ptr->lev / 10));
		break;

	case POWER_COR_SPACE_TIME:
		if (p_ptr->corrupt_anti_teleport_stopped)
		{
			p_ptr->corrupt_anti_teleport_stopped = false;
			msg_print("You stop controlling your corruption.");
			p_ptr->update |= PU_BONUS;
		}
		else
		{
			p_ptr->corrupt_anti_teleport_stopped = true;
			msg_print("You start controlling your corruption, teleportation works once more.");
			p_ptr->update |= PU_BONUS;
		}
		break;

	default:
		abort();
		break;
	}

	p_ptr->redraw |= (PR_FRAME);
	p_ptr->window |= (PW_PLAYER);
}

/*
 * Print a batch of power.
 */
static void print_power_batch(std::vector<int> const &power_idxs, int start, int max)
{
	int j = 0;

	prt(fmt::format("{:<10}{:<31} Level Mana Fail", "", "Name"), 1, 19);

	for (int i = start; i < (start + 20); i++)
	{
		if (i >= max)
		{
			break;
		}

		auto spell = game->powers.at(power_idxs.at(i));

		auto buff = fmt::format(
			"  {} -{:>3}) {:<30}  {:>5} {:>4} {}@{}",
			I2C(j),
			power_idxs.at(i) + 1,
			spell->name,
			spell->activation.level,
			spell->activation.cost,
			stat_names[spell->activation.stat],
			spell->activation.diff
		);

		prt(buff, 2 + j, 19);
		j++;
	}

	prt("", 2 + j, 19);
	prt(fmt::format("Select a power (a-{}), +/- to scroll:", I2C(j - 1)), 0, 0);
}


/*
 * List powers and ask to pick one. 
 */

static boost::optional<int> select_power()
{
	// Find selectable power indexes
	std::vector<int> power_idxs;
	std::copy(
		std::begin(p_ptr->powers),
		std::end(p_ptr->powers),
		std::back_inserter(power_idxs));
	std::sort(
		std::begin(power_idxs),
		std::end(power_idxs));

	/* Exit if there aren't powers */
	if (power_idxs.empty())
	{
		msg_print("You don't have any special powers.");
		return boost::none;
	}
	else
	{
		int start = 0;
		int const max = power_idxs.size();
		// Save
		screen_save_no_flush();
		// Loop until we get a result.
		boost::optional<int> result;
		while (true)
		{
			print_power_batch(power_idxs, start, max);
			char which = inkey();

			if (which == ESCAPE)
			{
				break;
			}
			else if (which == '+')
			{
				start += 20;
				if (start >= max) start -= 20;
				screen_load_no_flush();
			}
			else if (which == '-')
			{
				start -= 20;
				if (start < 0) start += 20;
				screen_load_no_flush();
			}
			else
			{
				which = tolower(which);
				if (start + A2I(which) >= max)
				{
					bell();
					continue;
				}
				if (start + A2I(which) < 0)
				{
					bell();
					continue;
				}

				result = power_idxs[start + A2I(which)];
				break;
			}
		}

		screen_load_no_flush();

		return result;
	}
}

/* Ask & execute a power */
void do_cmd_power()
{
	int x_idx;
	bool push = true;

	/* Get the skill, if available */
	if (repeat_pull(&x_idx))
	{
		if (!game->powers.count(x_idx))
		{
			return;
		}

		push = false;
	}
	else if (!command_arg)
	{
		if (auto i = select_power())
		{
			x_idx = *i;
		}
		else
		{
			return;
		}
	}
	else
	{
		x_idx = command_arg - 1;

		if (!game->powers.count(x_idx))
		{
			return;
		}
	}

	if (push)
	{
		repeat_push(x_idx);
	}

	if (p_ptr->powers.count(x_idx))
	{
		power_activate(x_idx);
	}
	else
	{
		msg_print("You do not have access to this power.");
	}
}
