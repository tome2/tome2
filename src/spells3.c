#include "angband.h"

s32b NOXIOUSCLOUD = -1; /* Identifier */
s32b AIRWINGS = -1; /* Identifier */
s32b INVISIBILITY;
s32b POISONBLOOD;
s32b THUNDERSTORM;
s32b STERILIZE;

s32b BLINK;
s32b DISARM;
s32b TELEPORT;
s32b TELEAWAY;
s32b RECALL;
s32b PROBABILITY_TRAVEL;

/* FIXME: Hackish workaround while we're still tied to Lua. This lets
 us return Lua's "nil" and a non-nil value (which is all the s_aux.lua
 cares about). */
bool_ *NO_CAST = NULL;
bool_ CAST_VAL = 0xca; /* Any value will do */
bool_ *CAST = &CAST_VAL;

static s32b get_level_s(int sp, int max)
{
	return get_level(sp, max, 1);
}

bool_ *air_noxious_cloud()
{
	int dir, type;

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	if (get_level_s(NOXIOUSCLOUD, 50) >= 30)
	{
		type = GF_UNBREATH;
	}
	else
	{
		type = GF_POIS;
	}

	fire_cloud(type, dir, 7 + get_level_s(NOXIOUSCLOUD, 150), 3, 5 + get_level_s(NOXIOUSCLOUD, 40));
	return CAST;
}

char *air_noxious_cloud_info()
{
	static char buf[128];
	sprintf(buf,
		"dam %d rad 3 dur %d",
		(7 + get_level_s(NOXIOUSCLOUD, 150)),
		(5 + get_level_s(NOXIOUSCLOUD, 40)));
	return buf;
}

bool_ *air_wings_of_winds()
{
	if (get_level_s(AIRWINGS, 50) >= 16)
	{
		if (p_ptr->tim_fly == 0)
		{
			set_tim_fly(randint(10) + 5 + get_level_s(AIRWINGS, 25));
			return CAST;
		}
		else if (p_ptr->tim_ffall == 0)
		{
			set_tim_ffall(randint(10) + 5 + get_level_s(AIRWINGS, 25));
			return CAST;
		}
	}

	return NO_CAST;
}

char *air_wings_of_winds_info()
{
	static char buf[128];
	sprintf(buf, "dur %d+d10", (5 + get_level_s(AIRWINGS, 25)));
	return buf;
}

bool_ *air_invisibility()
{
	if (p_ptr->tim_invisible == 0)
	{
		set_invis(randint(20) + 15 + get_level_s(INVISIBILITY, 50), 20 + get_level_s(INVISIBILITY, 50));
		return CAST;
	}

	return NO_CAST;
}

char *air_invisibility_info()
{
	static char buf[128];
	sprintf(buf, "dur %d+d20 power %d",
		(15 + get_level_s(INVISIBILITY, 50)),
		(20 + get_level_s(INVISIBILITY, 50)));
	return buf;
}

bool_ *air_poison_blood()
{
	bool_ *cast = NO_CAST;

	if (p_ptr->oppose_pois == 0)
	{
		set_oppose_pois(randint(30) + 25 + get_level_s(POISONBLOOD, 25));
		cast = CAST;
	}

	if ((p_ptr->tim_poison == 0) &&
	    (get_level_s(POISONBLOOD, 50) >= 15))
	{
		set_poison(randint(30) + 25 + get_level_s(POISONBLOOD, 25));
		cast = CAST;
	}

	return cast;
}

char *air_poison_blood_info()
{
	static char buf[128];
	sprintf(buf,
		"dur %d+d30",
		(25 + get_level_s(POISONBLOOD, 25)));
	return buf;
}

bool_ *air_thunderstorm()
{
	if (p_ptr->tim_thunder == 0)
	{
		set_tim_thunder(randint(10) + 10 + get_level_s(THUNDERSTORM, 25), 5 + get_level_s(THUNDERSTORM, 10), 10 + get_level_s(THUNDERSTORM, 25));
		return CAST;
	}

	return NO_CAST;
}

char *air_thunderstorm_info()
{
	static char buf[128];
	sprintf(buf,
		"dam %dd%d dur %d+d10",
		(5 + get_level_s(THUNDERSTORM, 10)),
		(10 + get_level_s(THUNDERSTORM, 25)),
		(10 + get_level_s(THUNDERSTORM, 25)));
	return buf;
}

bool_ *air_sterilize()
{
	set_no_breeders((30) + 20 + get_level_s(STERILIZE, 70));
	return CAST;
}

char *air_sterilize_info()
{
	static char buf[128];
	sprintf(buf,
		"dur %d+d30",
		(20 + get_level_s(STERILIZE, 70)));
	return buf;
}

bool_ *convey_blink()
{
	if (get_level_s(BLINK, 50) >= 30)
	{
		int oy = p_ptr->py;
		int ox = p_ptr->px;

		teleport_player(10 + get_level_s(BLINK, 8));
		create_between_gate(0, oy, ox);
		return CAST;
	}
	else
	{
		teleport_player(10 + get_level_s(BLINK, 8));
		return CAST;
	}
}

char *convey_blink_info()
{
	static char buf[128];
	sprintf(buf,
		"distance %d",
		(10 + get_level_s(BLINK, 8)));
	return buf;
}

bool_ *convey_disarm()
{
	destroy_doors_touch();
	if (get_level_s(DISARM, 50) >= 10)
	{
		destroy_traps_touch();
	}

	return CAST;
}

char *convey_disarm_info()
{
	return "";
}

bool_ *convey_teleport()
{
	p_ptr->energy -= (25 - get_level_s(TELEPORT, 50));
	teleport_player(100 + get_level_s(TELEPORT, 100));
	return CAST;
}

char *convey_teleport_info()
{
	static char buf[128];
	sprintf(buf,
		"distance %d",
		(100 + get_level_s(TELEPORT, 100)));
	return buf;
}

bool_ *convey_teleport_away()
{
	if (get_level_s(TELEAWAY, 50) >= 20)
	{
		project_hack(GF_AWAY_ALL, 100);
		return CAST;
	}
	else if (get_level_s(TELEAWAY, 50) >= 10)
	{
		int dir;
		if (!get_aim_dir(&dir))
		{
			return NO_CAST;
		}

		fire_ball(GF_AWAY_ALL, dir, 100, 3 + get_level_s(TELEAWAY, 4));
		return CAST;
	}
	else
	{
		int dir;
		if (!get_aim_dir(&dir))
		{
			return NO_CAST;
		}
		teleport_monster(dir);
		return CAST;
	}
}

char *convey_teleport_away_info()
{
	return "";
}

static int recall_get_d()
{
	int d = 21 - get_level_s(RECALL, 15);
	if (d < 0)
	{
		d = 0;
	}
	return d;
}

static int recall_get_f()
{
	int f = 15 - get_level_s(RECALL, 10);
	if (f < 1)
	{
		f = 1;
	}
	return f;
}

bool_ *convey_recall()
{
	int x,y;
	cave_type *c_ptr;

	if (!tgt_pt(&x, &y))
	{
		return NO_CAST;
	}

	c_ptr = &cave[y][x];

	if ((y == p_ptr->py) &&
	    (x == p_ptr->px))
	{
		int d = recall_get_d();
		int f = recall_get_f();
		recall_player(d, f);
		return CAST;
	}
	else if (c_ptr->m_idx > 0)
	{
		swap_position(y, x);
		return CAST;
	}
	else if (c_ptr->o_idx > 0)
	{
		set_target(y, x);
		if (get_level_s(RECALL, 50) >= 15)
		{
			fetch(5, 10 + get_level_s(RECALL, 150), FALSE);
		}
		else
		{
			fetch(5, 10 + get_level_s(RECALL, 150), TRUE);
		}
		return CAST;
	}
	else
	{
		return NO_CAST;
	}
}

char *convey_recall_info()
{
	static char buf[128];
	int d = recall_get_d();
	int f = recall_get_f();

	sprintf(buf,
		"dur %d+d%d weight %dlb",
		f, d, (1 + get_level_s(RECALL, 15)));
	return buf;
}

bool_ *convey_probability_travel()
{
	set_prob_travel(randint(20) + get_level_s(PROBABILITY_TRAVEL, 60));
	return CAST;
}

char *convey_probability_travel_info()
{
	static char buf[128];
	sprintf(buf,
		"dur %d+d20",
		get_level_s(PROBABILITY_TRAVEL, 60));
	return buf;
}
