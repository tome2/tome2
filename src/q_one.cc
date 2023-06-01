#include "q_one.hpp"

#include "artifact_type.hpp"
#include "cave.hpp"
#include "cave_type.hpp"
#include "game.hpp"
#include "gods.hpp"
#include "hook_calculate_hp_in.hpp"
#include "hook_calculate_hp_out.hpp"
#include "hook_chardump_in.hpp"
#include "hook_drop_in.hpp"
#include "hook_identify_in.hpp"
#include "hook_monster_death_in.hpp"
#include "hook_move_in.hpp"
#include "hook_wield_in.hpp"
#include "hooks.hpp"
#include "monster2.hpp"
#include "monster_type.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "player_type.hpp"
#include "tables.hpp"
#include "town_type.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "z-rand.hpp"
#include "z-term.hpp"

#define cquest (quest[QUEST_ONE])

static bool quest_one_move_hook(void *, void *in_, void *)
{
	struct hook_move_in *in = static_cast<struct hook_move_in *>(in_);
	s32b y = in->y;
	s32b x = in->x;
	cave_type *c_ptr = &cave[y][x];

	if (cquest.status == QUEST_STATUS_UNTAKEN)
	{
		if (quest[QUEST_NECRO].status < QUEST_STATUS_FINISHED)
		{
			return false;
		}

		/* The mirror of Galadriel */
		if ((c_ptr->feat != FEAT_SHOP) || (c_ptr->special != 23))
		{
			return false;
		}

		cmsg_print(TERM_YELLOW, "You meet Galadriel; she seems worried.");
		cmsg_print(TERM_YELLOW, "'So it was Sauron that lurked in Dol Guldur...'");
		cmsg_print(TERM_YELLOW, "'The Enemy is growing in power. Morgoth will be unreachable as long'");
		cmsg_print(TERM_YELLOW, "'as his most powerful servant, Sauron, lives. But the power of Sauron'");
		cmsg_print(TERM_YELLOW, "'lies in the One Ring. Our only hope is that you find it'");
		cmsg_print(TERM_YELLOW, "'and destroy it. I know it will tempt you, but *NEVER* use it'");
		cmsg_print(TERM_YELLOW, "'or it will corrupt you forever.'");

		if (p_ptr->pgod == GOD_ERU)
		{
			cmsg_print(TERM_YELLOW, "'Also, Eru will abandon you if you wear it.'");
		}

		if (p_ptr->pgod == GOD_MANWE)
		{
			cmsg_print(TERM_YELLOW, "'Also, Manwe will abandon you if you wear it.'");
		}

		if (p_ptr->pgod == GOD_TULKAS)
		{
			cmsg_print(TERM_YELLOW, "'Also, Tulkas will abandon you if you wear it.'");
		}

		if (p_ptr->pgod == GOD_YAVANNA)
		{
			cmsg_print(TERM_YELLOW, "'Also, Yavanna will abandon you if you wear it.'");
		}

		cmsg_print(TERM_YELLOW, "'Without the destruction of the ring, Sauron's death can only be temporary'");
		cmsg_print(TERM_YELLOW, "'When you have it, bring it to Mount Doom, in Mordor,'");
		cmsg_print(TERM_YELLOW, "'to destroy it in the Great Fire where it was forged.'");
		cmsg_print(TERM_YELLOW, "'I do not know where to find it. Seek it through Middle-earth. Maybe there'");
		cmsg_print(TERM_YELLOW, "'are other people that might know.'");
		cmsg_print(TERM_YELLOW, "'Do not forget: the Ring must be cast back into the fires of Mount Doom!'");

		if (p_ptr->pgod == GOD_MELKOR)
		{
			cmsg_print(TERM_YELLOW, "'Melkor will abandon you when you do, but you must do it anyway!'");
		}

		/* Continue the plot */
		cquest.status = QUEST_STATUS_TAKEN;
		cquest.init();

		return true;
	}

	return false;
}

static bool quest_one_drop_hook(void *, void *in_, void *)
{
	struct hook_drop_in *in = static_cast<struct hook_drop_in *>(in_);
	s32b o_idx = in->o_idx;
	object_type *o_ptr = &p_ptr->inventory[o_idx];

	if (cquest.status != QUEST_STATUS_TAKEN)
	{
		return false;
	}

	if (o_ptr->name1 != ART_POWER)
	{
		return false;
	}

	if (cave[p_ptr->py][p_ptr->px].feat != FEAT_GREAT_FIRE)
	{
		return false;
	}

	cmsg_print(TERM_YELLOW, "You throw the One Ring into the #RGreat Fire#y; it is rapidly consumed");
	cmsg_print(TERM_YELLOW, "by the searing flames.");
	cmsg_print(TERM_YELLOW, "You feel the powers of evil weakening.");
	cmsg_print(TERM_YELLOW, "Now you can go onto the hunt for Sauron!");

	inc_stack_size_ex(o_idx, -99, OPTIMIZE, NO_DESCRIBE);

	abandon_god(GOD_MELKOR);

	/* Continue the plot */
	cquest.status = QUEST_STATUS_FINISHED;
	*(quest[QUEST_ONE].plot) = QUEST_SAURON;
	quest[*(quest[QUEST_ONE].plot)].status = QUEST_STATUS_TAKEN;
	quest[*(quest[QUEST_ONE].plot)].init();

	return true;
}

static bool quest_one_wield_hook(void *, void *in_, void *)
{
	struct hook_wield_in *in = static_cast<struct hook_wield_in *>(in_);
	object_type *o_ptr = in->o_ptr;

	if (cquest.status != QUEST_STATUS_TAKEN)
	{
		return false;
	}

	if (o_ptr->name1 != ART_POWER)
	{
		return false;
	}

	// Make sure player is **really** warned
	flush();
	if (!get_check("You were warned not to wear it; are you sure?"))
	{
		return true;
	}

	flush();
	if (!get_check("You were warned not to wear it; are you *REALLY* sure?"))
	{
		return true;
	}

	flush();
	if (!get_check("You were *WARNED* not to wear it; are you *R*E*A*L*L*Y* sure?"))
	{
		return true;
	}

	// Ok, that's enough warning
	cmsg_print(TERM_YELLOW, "As you put it on your finger you feel #Ddark powers #ysapping your soul.");
	cmsg_print(TERM_YELLOW, "The ring firmly binds to your finger!");
	cmsg_print(TERM_YELLOW, "You feel you are drawn to the shadow world! Your material form weakens.");

	abandon_god(GOD_ERU);
	abandon_god(GOD_MANWE);
	abandon_god(GOD_TULKAS);
	abandon_god(GOD_YAVANNA);

	/*
	 * Ok now we are evil, right ?
	 * Towns aren't, right ?
	 * So let's destroy them !
	 */
	town_info[1].destroyed = true;
	town_info[2].destroyed = true;
	town_info[3].destroyed = true;
	town_info[4].destroyed = true;
	town_info[5].destroyed = true;

	/* Continue the plot */
	cquest.status = QUEST_STATUS_FAILED_DONE;
	*(quest[QUEST_ONE].plot) = QUEST_SAURON;
	quest[*(quest[QUEST_ONE].plot)].status = QUEST_STATUS_TAKEN;
	quest[*(quest[QUEST_ONE].plot)].init();

	/* Ok lets reset the lives counter */
	p_ptr->lives = 0;

	return false;
}

static bool quest_one_hp_hook(void *, void *in_, void *out_)
{
	struct hook_calculate_hp_in *in = static_cast<struct hook_calculate_hp_in *>(in_);
	struct hook_calculate_hp_out *out = static_cast<struct hook_calculate_hp_out *>(out_);

	if (cquest.status == QUEST_STATUS_FAILED_DONE)
	{
		s32b mhp = in->mhp;

		for (int i = 0; i < p_ptr->lives + 1; i++)
		{
			mhp = (mhp * 2) / 3;
		}

		out->mhp = mhp;
		return true;
	}

	return false;
}

static bool quest_one_die_hook(void *, void *, void *)
{
	if (cquest.status == QUEST_STATUS_FAILED_DONE)
	{
		if (p_ptr->mhp > 1)
		{
			cmsg_print(TERM_YELLOW, "You feel the power of the One Ring sustaining your life,");
			cmsg_print(TERM_YELLOW, "but it drags you even more into the shadow world.");
			return true;
		}
		else
		{
			cmsg_print(TERM_YELLOW, "The One Ring finally drags you totally to the shadow world.");
			cmsg_print(TERM_YELLOW, "Your mortal existence ends there.");
			game->died_from = "being drawn to the shadow world";
		}
	}
	return false;
}

static bool quest_one_identify_hook(void *, void *in_, void *)
{
	struct hook_identify_in *in = static_cast<struct hook_identify_in *>(in_);
	object_type *o_ptr = in->o_ptr;

	if (cquest.status == QUEST_STATUS_TAKEN)
	{
		if ((o_ptr->name1 == ART_POWER) && (!object_known_p(o_ptr)))
		{
			cmsg_print(TERM_YELLOW, "You finally found the One Ring, source of Sauron's power, and key to");
			cmsg_print(TERM_YELLOW, "its destruction. Remember, bring it to Mount Doom and destroy it.");
			cmsg_print(TERM_YELLOW, "And *NEVER* use it.");
		}
	}

	return false;
}

static bool quest_one_death_hook(void *, void *in_, void *)
{
	auto const &a_info = game->edit_data.a_info;

	struct hook_monster_death_in *in = static_cast<struct hook_monster_death_in *>(in_);
	s32b m_idx = in->m_idx;
	s32b r_idx = m_list[m_idx].r_idx;
	bool ok = false;

	if (a_info[ART_POWER].cur_num)
	{
		return false;
	}

	/* Paranoia */
	if (cquest.status != QUEST_STATUS_TAKEN)
	{
		return false;
	}

	if (magik(30) && (r_idx == test_monster_name("Sauron, the Sorcerer")))
	{
		ok = true;
	}
	else if (magik(10) && (r_idx == test_monster_name("Ar-Pharazon the Golden")))
	{
		ok = true;
	}
	else if (magik(10) && (r_idx == test_monster_name("Shelob, Spider of Darkness")))
	{
		ok = true;
	}
	else if (magik(10) && (r_idx == test_monster_name("The Watcher in the Water")))
	{
		ok = true;
	}
	else if (magik(10) && (r_idx == test_monster_name("Glaurung, Father of the Dragons")))
	{
		ok = true;
	}
	else if (magik(10) && (r_idx == test_monster_name("Feagwath, the Undead Sorcerer")))
	{
		ok = true;
	}

	if (ok)
	{
		int i;

		/* Get local object */
		object_type forge, *q_ptr = &forge;

		/* Mega-Hack -- Prepare to make "Grond" */
		object_prep(q_ptr, lookup_kind(TV_RING, SV_RING_POWER));

		/* Mega-Hack -- Mark this item as "the one ring" */
		q_ptr->name1 = ART_POWER;

		/* Mega-Hack -- Actually create "the one ring" */
		apply_magic(q_ptr, -1, true, true, true);

		/* Find a space */
		for (i = 0; i < INVEN_PACK; i++)
		{
			/* Skip non-objects */
			if (!p_ptr->inventory[i].k_ptr)
			{
				break;
			}
		}
		/* Arg, no space ! */
		if (i == INVEN_PACK)
		{
			char o_name[200];

			object_desc(o_name, &p_ptr->inventory[INVEN_PACK - 1], false, 0);

			/* Drop the item */
			inven_drop(INVEN_PACK - 1, 99, p_ptr->py, p_ptr->px, false);

			cmsg_format(TERM_VIOLET, "You feel the urge to drop your %s to make room in your inventory.", o_name);
		}

		/* Carry it */
		cmsg_format(TERM_VIOLET, "You feel the urge to pick up that plain gold ring you see.");
		inven_carry(q_ptr, false);
	}

	return false;
}

static bool quest_one_dump_hook(void *, void *in_, void *)
{
	struct hook_chardump_in *in = static_cast<struct hook_chardump_in *>(in_);
	FILE *f = in->file;

	if (cquest.status == QUEST_STATUS_FINISHED)
	{
		fprintf(f, "\n You destroyed the One Ring, thus weakening Sauron.");
	}
	if (cquest.status == QUEST_STATUS_FAILED_DONE)
	{
		fprintf(f, "\n You fell under the evil influence of the One Ring and decided to wear it.");
	}
	return false;
}

static bool quest_one_gen_hook(void *, void *, void *)
{
	s32b x, y, tries = 10000;

	/* Paranoia */
	if (cquest.status != QUEST_STATUS_TAKEN)
	{
		return false;
	}
	if ((dungeon_type != DUNGEON_ANGBAND) || (dun_level != 99))
	{
		return false;
	}

	/* Find a good position */
	while (tries)
	{
		/* Get a random spot */
		y = randint(cur_hgt - 4) + 2;
		x = randint(cur_wid - 4) + 2;

		/* Is it a good spot ? */
		if (cave_empty_bold(y, x)) break;

		/* One less try */
		tries--;
	}

	if (tries)
	{
		int m_idx = place_monster_one(y, x, test_monster_name("Sauron, the Sorcerer"), 0, false, MSTATUS_ENEMY);
		if (m_idx) m_list[m_idx].mflag |= MFLAG_QUEST;
	}

	return false;
}

void quest_one_init_hook()
{
	if ((cquest.status >= QUEST_STATUS_TAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook_new(HOOK_LEVEL_END_GEN, quest_one_gen_hook,      "one_gen",   NULL);
		add_hook_new(HOOK_MONSTER_DEATH, quest_one_death_hook,    "one_death", NULL);
		add_hook_new(HOOK_DROP,          quest_one_drop_hook,     "one_drop",  NULL);
		add_hook_new(HOOK_WIELD,         quest_one_wield_hook,    "one_wield", NULL);
		add_hook_new(HOOK_IDENTIFY,      quest_one_identify_hook, "one_id",    NULL);
	}
	if (cquest.status == QUEST_STATUS_UNTAKEN)
	{
		add_hook_new(HOOK_MOVE, quest_one_move_hook, "one_move", NULL);
	}
	add_hook_new(HOOK_CHAR_DUMP, quest_one_dump_hook, "one_dump", NULL);
	add_hook_new(HOOK_CALC_HP,   quest_one_hp_hook,   "one_hp",   NULL);
	add_hook_new(HOOK_DIE,       quest_one_die_hook,  "one_die",  NULL);
}
