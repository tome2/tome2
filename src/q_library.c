#undef cquest

#define MONSTER_LICH 518
#define MONSTER_MONASTIC_LICH 611
#define MONSTER_FLESH_GOLEM 256
#define MONSTER_CLAY_GOLEM 261
#define MONSTER_IRON_GOLEM 367
#define MONSTER_MITHRIL_GOLEM 464

static s16b library_quest_place_random(int minY, int minX, int maxY, int maxX, int r_idx)
{
	int y = randint(maxY - minY + 1) + minY;
	int x = randint(maxX - minX + 1) + minX;
	return place_monster_one(y, x, r_idx, 0, TRUE, MSTATUS_ENEMY);
}

static void library_quest_place_nrandom(int minY, int minX, int maxY, int maxX, int r_idx, int n)
{
	while(n > 0)
	{
		if (0 < library_quest_place_random(minY, minX, maxY, maxX, r_idx))
		{
			n--;
		}
	}
}

void quest_library_gen_hook()
{
	library_quest_place_nrandom(
		4, 4, 14, 37, MONSTER_LICH, damroll(4,2));

	library_quest_place_nrandom(
		14, 34, 37, 67, MONSTER_MONASTIC_LICH, damroll(1, 2));

	library_quest_place_nrandom(
		4, 34, 14, 67, MONSTER_MONASTIC_LICH, damroll(1, 2) - 1);

	library_quest_place_nrandom(
		14, 4, 37, 34, MONSTER_MONASTIC_LICH, damroll(1, 2) - 1);

	library_quest_place_nrandom(
		10, 10, 37, 67, MONSTER_FLESH_GOLEM, 2);

	library_quest_place_nrandom(
		10, 10, 37, 67, MONSTER_CLAY_GOLEM, 2);

	library_quest_place_nrandom(
		10, 10, 37, 67, MONSTER_IRON_GOLEM, 2);

	library_quest_place_nrandom(
		10, 10, 37, 67, MONSTER_MITHRIL_GOLEM, 1);
}
