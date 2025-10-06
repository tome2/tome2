#include "q_bounty.hpp"

#include "format_ext.hpp"
#include "game.hpp"
#include "monster2.hpp"
#include "monster_race.hpp"
#include "monster_race_flag.hpp"
#include "monster_spell_flag.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "object_type.hpp"
#include "player_type.hpp"
#include "skill_type.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "variable.hpp"

#include <fmt/format.h>

#define cquest (quest[QUEST_BOUNTY])

#define bounty_quest_monster (cquest.data[0])

static bool lua_mon_hook_bounty(monster_race const *r_ptr)
{
	/* Reject uniques */
	if (r_ptr->flags & RF_UNIQUE) return false;

	/* Reject those who cannot leave anything */
	if (!(r_ptr->flags & RF_DROP_CORPSE)) return false;

	/* Accept only monsters that can be generated */
	if (r_ptr->flags & RF_SPECIAL_GENE) return false;
	if (r_ptr->flags & RF_NEVER_GENE) return false;

	/* Reject pets */
	if (r_ptr->flags & RF_PET) return false;

	/* Reject friendly creatures */
	if (r_ptr->flags & RF_FRIENDLY) return false;

	/* Accept only monsters that are not breeders */
	if (r_ptr->spells & SF_MULTIPLY) return false;

	/* Forbid joke monsters */
	if (r_ptr->flags & RF_JOKEANGBAND) return false;

	/* Accept only monsters that are not good */
	if (r_ptr->flags & RF_GOOD) return false;

	/* The rest are acceptable */
	return true;
}

static int get_new_bounty_monster(int lev)
{
	int r_idx;

	/*
	 * Set up the hooks -- no bounties on uniques or monsters
	 * with no corpses
	 */
	get_monster_hook = lua_mon_hook_bounty;
	get_mon_num_prep();

	/* Set up the quest monster. */
	r_idx = get_mon_num(lev);

	/* Undo the filters */
	get_monster_hook = NULL;
	get_mon_num_prep();

	return r_idx;
}

static bool bounty_item_tester_hook(object_type const *o_ptr)
{
	return ((o_ptr->tval == TV_CORPSE) && (o_ptr->pval2 == bounty_quest_monster));
}

void quest_bounty_init_hook()
{
	// Initialized by building action
}

void quest_bounty_drop_item()
{
	char mdesc[512];
	char msg[512];

	if (cquest.status == QUEST_STATUS_UNTAKEN)
	{
		cquest.status = QUEST_STATUS_TAKEN;
		bounty_quest_monster = get_new_bounty_monster(3 + (p_ptr->lev * 3) / 2);

		monster_race_desc(mdesc, bounty_quest_monster, 0);
		snprintf(msg, sizeof(msg), "You must bring me back %s corpse.", mdesc);
		msg_print(msg);
	}
	else
	{
		monster_race_desc(mdesc, bounty_quest_monster, 0);
		snprintf(msg, sizeof(msg), "You still must bring me back %s corpse.", mdesc);
		msg_print(msg);
	}
}

void quest_bounty_get_item()
{
	auto &s_info = game->s_info;

	if (cquest.status != QUEST_STATUS_TAKEN)
	{
		msg_print("You do not have any bounty quest yet.");
		return;
	}

	// Get the corpse.
	int item = -1;
	if (!get_item(&item,
		"What corpse to return?",
		"You have no corpse to return.",
		 USE_INVEN,
		bounty_item_tester_hook))
	{
		return;
	}

	// Take the corpse from the inventory
	inven_item_increase(item, -1);
	inven_item_optimize(item);

	msg_print("Ah well done adventurer!");
	msg_print("As a reward I will teach you a bit of monster lore.");

	skill_type *lore = &s_info[SKILL_LORE];
	skill_type *preservation = &s_info[SKILL_PRESERVATION];

	if (lore->mod == 0)
	{
		lore->mod = 900;
		lore->dev = true;
	}

	lore->value += lore->mod;

	if (preservation->mod == 0)
	{
		preservation->value = 800;
		preservation->mod = 800;
		preservation->dev = true;
		msg_print("I see you don't know the corpse preservation skill, I shall teach you it too.");
	}

	// Need to ask for new quest.
	cquest.status = QUEST_STATUS_UNTAKEN;
	bounty_quest_monster = 0;
}

std::string quest_bounty_describe()
{
	char mdesc[512];
	fmtMemoryWriter w;

	if (cquest.status == QUEST_STATUS_TAKEN)
	{
		monster_race_desc(mdesc, bounty_quest_monster, 0);

		w.write("#####yBounty quest!\n");
		w.write("You must bring back {} corpse to the beastmaster.", mdesc);
	}

	return w.str();
}
