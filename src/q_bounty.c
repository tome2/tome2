#undef cquest
#define cquest (quest[QUEST_BOUNTY])

#define bounty_quest_monster (cquest.data[0])

static bool_ bounty_item_tester_hook(object_type *o_ptr)
{
	if ((o_ptr->tval == TV_CORPSE) && (o_ptr->pval2 == bounty_quest_monster))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

bool_ quest_bounty_init_hook(int dummy)
{
	return FALSE;
}

bool_ quest_bounty_drop_item()
{
	char mdesc[512];
	char msg[512];

	if (cquest.status == QUEST_STATUS_UNTAKEN)
	{
		cquest.status = QUEST_STATUS_TAKEN;
		bounty_quest_monster = lua_get_new_bounty_monster(3 + (p_ptr->lev * 3) / 2);

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
	return FALSE;
}

bool_ quest_bounty_get_item()
{
	if (cquest.status != QUEST_STATUS_TAKEN)
	{
		msg_print("You do not have any bounty quest yet.");
		return FALSE;
	}

	// Get the corpse.
	item_tester_hook = bounty_item_tester_hook;
	int item = -1;
	bool_ ret =
		get_item(&item,
			 "What corpse to return?",
			 "You have no corpse to return.",
			 USE_INVEN);
	if (!ret) {
		return FALSE;
	}

	// Take the corpse from the inventory
	inven_item_increase(item, -1);
	inven_item_optimize(item);

	msg_print("Ah well done adventurer!");
	msg_print("As a reward I will teach you a bit of monster lore.");

	skill_type *lore = &s_info[SKILL_LORE];
	skill_type *preservation = &s_info[SKILL_PRESERVATION];

	if (lore->mod == 0) {
		lore->mod = 900;
		lore->dev = TRUE;
	}
	lore->value += lore->mod;

	if (preservation->mod == 0) {
		preservation->value = 800;
		preservation->mod = 800;
		preservation->dev = TRUE;
		msg_print("I see you don't know the corpse preservation skill, I shall teach you it too.");
	}

	// Need to ask for new quest.
	cquest.status = QUEST_STATUS_UNTAKEN;
	bounty_quest_monster = 0;
	return FALSE;
}

bool_ quest_bounty_describe(FILE *fff)
{
	char mdesc[512];

	if (cquest.status == QUEST_STATUS_TAKEN)
	{
		monster_race_desc(mdesc, bounty_quest_monster, 0);

		fprintf(fff, "#####yBounty quest!\n");
		fprintf(fff, "You must bring back %s corpse to the beastmaster.\n", mdesc);
		fprintf(fff, "\n");

		return TRUE;
	}

	return FALSE;
}
