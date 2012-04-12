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

s32b DEMON_BLADE;
s32b DEMON_MADNESS;
s32b DEMON_FIELD;
s32b DOOM_SHIELD;
s32b UNHOLY_WORD;
s32b DEMON_CLOAK;
s32b DEMON_SUMMON;
s32b DISCHARGE_MINION;
s32b CONTROL_DEMON;

s32b STARIDENTIFY;
s32b IDENTIFY;
s32b VISION;
s32b SENSEHIDDEN;
s32b REVEALWAYS;
s32b SENSEMONSTERS;

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

bool_ *demonology_demon_blade()
{
	int rad, type;

	type = GF_FIRE;
	if (get_level_s(DEMON_BLADE, 50) >= 30)
	{
		type = GF_HELL_FIRE;
	}

	rad = 0;
	if (get_level_s(DEMON_BLADE, 50) >= 45)
	{
		rad = 1;
	}

	set_project(randint(20) + get_level_s(DEMON_BLADE, 80),
		    type,
		    4 + get_level_s(DEMON_BLADE, 40),
		    rad,
		    PROJECT_STOP | PROJECT_KILL);
	return CAST;
}

char  *demonology_demon_blade_info()
{
	static char buf[128];
	sprintf(buf,
		"dur %d+d20 dam %d/blow",
		(get_level_s(DEMON_BLADE, 80)),
		(4 + get_level_s(DEMON_BLADE, 40)));
	return buf;
}

bool_ *demonology_demon_madness()
{
	int dir, type, y1, x1, y2, x2;

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	type = GF_CHAOS;
	if (magik(33))
	{
		type = GF_CONFUSION;
	}
	if (magik(33))
	{
		type = GF_CHARM;
	}

	/* Calc the coordinates of arrival */
	get_target(dir, &y1, &x1);
	y2 = p_ptr->py - (y1 - p_ptr->py);
	x2 = p_ptr->px - (x1 - p_ptr->px);

	project(0, 1 + get_level(DEMON_MADNESS, 4, 0),
		y1, x1,
		20 + get_level_s(DEMON_MADNESS, 200),
		type,
		PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL);
	project(0, 1 + get_level(DEMON_MADNESS, 4, 0),
		y2, x2,
		20 + get_level_s(DEMON_MADNESS, 200),
		type,
		PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL);

	return CAST;
}

char  *demonology_demon_madness_info()
{
	static char buf[128];
	sprintf(buf,
		"dam %d rad %d",
		(20 + get_level_s(DEMON_MADNESS, 200)),
		(1 + get_level(DEMON_MADNESS, 4, 0)));
	return buf;
}

bool_ *demonology_demon_field()
{
	int dir;

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}
	
	fire_cloud(GF_NEXUS,
		   dir,
		   20 + get_level_s(DEMON_FIELD, 70),
		   7,
		   30 + get_level_s(DEMON_FIELD, 100));
	return CAST;
}

char  *demonology_demon_field_info()
{
	static char buf[128];
	sprintf(buf,
		"dam %d dur %d",
		(20 + get_level_s(DEMON_FIELD, 70)),
		(30 + get_level_s(DEMON_FIELD, 100)));
	return buf;
}

bool_ *demonology_doom_shield()
{
	set_shield(randint(10) + 20 + get_level_s(DOOM_SHIELD, 100),
		   -300 + get_level_s(DOOM_SHIELD, 100),
		   SHIELD_COUNTER,
		   1 + get_level_s(DOOM_SHIELD, 14),
		   10 + get_level_s(DOOM_SHIELD, 15));
	return CAST;
}

char  *demonology_doom_shield_info()
{
	static char buf[128];
	sprintf(buf,
		"dur %d+d10 dam %dd%d",
		(20 + get_level_s(DOOM_SHIELD, 100)),
		(1 + get_level_s(DOOM_SHIELD, 14)),
		(10 + get_level_s(DOOM_SHIELD, 15)));
	return buf;
}

bool_ *demonology_unholy_word()
{
	int x, y;
	cave_type *c_ptr = NULL;
	
	if (!tgt_pt(&x, &y))
	{
		return NO_CAST;
	}

	c_ptr = &cave[y][x];
	if (c_ptr->m_idx > 0)
	{
		monster_type *m_ptr = &m_list[c_ptr->m_idx];

		if (m_ptr->status != MSTATUS_PET)
		{
			msg_print("You can only target a pet.");
			return NO_CAST;
		}

		/* Oops he is angry now */
		if (magik(30 - get_level(UNHOLY_WORD, 25, 0)))
		{
			char buf[128];
			monster_desc(buf, m_ptr, 0);
			if (buf[0] != '\0')
			{
				buf[0] = toupper(buf[0]);
			}

			msg_format("%s turns against you.", buf);
		}
		else
		{
			char buf[128];
			s32b heal;

			monster_desc(buf, m_ptr, 0);
			msg_format("You consume %s.", buf);

			heal = (m_ptr->hp * 100) / m_ptr->maxhp;
			heal = ((30 + get_level(UNHOLY_WORD, 50, 0)) * heal) / 100;

			hp_player(heal);

			delete_monster_idx(c_ptr->m_idx);
		}
		
		return CAST;
	}
	else
	{
		return NO_CAST;
	}
}

char  *demonology_unholy_word_info()
{
	static char buf[128];
	sprintf(buf,
		"heal mhp%% of %d%%",
		(30 + get_level(UNHOLY_WORD, 50, 0)));
	return buf;
}

bool_ *demonology_demon_cloak()
{
	set_tim_reflect(randint(5) + 5 + get_level(DEMON_CLOAK, 15, 0));
	return CAST;
}

char  *demonology_demon_cloak_info()
{
	static char buf[128];
	sprintf(buf,
		"dur %d+d5",
		(5 + get_level(DEMON_CLOAK, 15, 0)));
	return buf;
}

bool_ *demonology_summon_demon()
{
	int type, level, minlevel;

	level = dun_level;

	minlevel = 4;
	if (level < minlevel)
	{
		level = minlevel;
	}

	summon_specific_level = 5 + get_level_s(DEMON_SUMMON, 100);

	type = SUMMON_DEMON;
	if (get_level_s(DEMON_SUMMON, 50) >= 35)
	{
		type = SUMMON_HI_DEMON;
	}

	if (!summon_specific_friendly(p_ptr->py, p_ptr->px, level, type, TRUE))
	{
		msg_print("Something blocks your summoning!");
	}

	return CAST;
}

char  *demonology_summon_demon_info()
{
	static char buf[128];
	sprintf(buf,
		"level %d",
		(5 + get_level_s(DEMON_SUMMON, 100)));
	return buf;
}

bool_ *demonology_discharge_minion()
{
	cave_type *c_ptr;
	int x, y;

	if (!tgt_pt(&x, &y))
	{
		return NO_CAST;
	}

	c_ptr = &cave[y][x];
	if (c_ptr->m_idx > 0)
	{
		s32b dam;
		monster_type *m_ptr = &m_list[c_ptr->m_idx];

		if (m_ptr->status != MSTATUS_PET)
		{
			msg_print("You can only target a pet.");
			return NO_CAST;
		}

		delete_monster_idx(c_ptr->m_idx);
		
		dam = m_ptr->hp;
		dam = (dam * (20 + get_level(DISCHARGE_MINION, 60, 0))) / 100;
		if (dam > 100 + get_level(DISCHARGE_MINION, 500, 0))
		{
			dam = 100 + get_level(DISCHARGE_MINION, 500, 0);
		}

		/* We use project instead of fire_ball because we must tell it exactly where to land */
		project(0, 2, y, x, dam,
			GF_GRAVITY,
			PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL);
		return CAST;
	}
	else
	{
		return NO_CAST;
	}
}

char  *demonology_discharge_minion_info()
{
	static char buf[128];
	sprintf(buf,
		"dam %d%% max %d",
		(20 + get_level(DISCHARGE_MINION, 60, 0)),
		(100 + get_level(DISCHARGE_MINION, 500, 0)));
	return buf;
}

bool_ *demonology_control_demon()
{
	int dir;
	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	fire_ball(GF_CONTROL_DEMON, dir, 50 + get_level_s(CONTROL_DEMON, 250), 0);
	return CAST;
}

char  *demonology_control_demon_info()
{
	static char buf[128];
	sprintf(buf,
		"power %d",
		(50 + get_level_s(CONTROL_DEMON, 250)));
	return buf;
}

bool_ *divination_greater_identify()
{
	if (get_check("Cast on yourself?"))
	{
		self_knowledge(NULL);
	}
	else
	{
		identify_fully();
	}
	return CAST;
}

char  *divination_greater_identify_info()
{
	return "";
}

bool_ *divination_identify()
{
	if (get_level_s(IDENTIFY, 50) >= 27)
	{
		identify_pack();
		fire_ball(GF_IDENTIFY, 0, 1, get_level_s(IDENTIFY, 3));
		p_ptr->notice |= (PN_COMBINE | PN_REORDER);
		return CAST;
	}
	else if (get_level_s(IDENTIFY, 50) >= 17)
	{
		identify_pack();
		fire_ball(GF_IDENTIFY, 0, 1, 0);
		p_ptr->notice |= (PN_COMBINE | PN_REORDER);
		return CAST;
	}
	else if (ident_spell() == TRUE)
	{
		return CAST;
	}
	else
	{
		return NO_CAST;
	}
}

char  *divination_identify_info()
{
	static char buf[128];

	if (get_level_s(IDENTIFY, 50) >= 27)
	{
		sprintf(buf, "rad %d", get_level_s(IDENTIFY, 3));
		return buf;
	}
	else
	{
		return "";
	}
}

bool_ *divination_vision()
{
	if (get_level_s(VISION, 50) >= 25)
	{
		wiz_lite_extra();
	}
	else
	{
		map_area();
	}
	return CAST;

}

char  *divination_vision_info()
{
	return "";
}

bool_ *divination_sense_hidden()
{
	detect_traps(15 + get_level(SENSEHIDDEN, 40, 0));
	if (get_level_s(SENSEHIDDEN, 50) >= 15)
	{
		set_tim_invis(10 + randint(20) + get_level_s(SENSEHIDDEN, 40));
	}
	return CAST;
}

char  *divination_sense_hidden_info()
{
	static char buf[128];

	if (get_level_s(SENSEHIDDEN, 50) >= 15)
	{
		sprintf(buf,
			"rad %d dur %d+d20",
			(15 + get_level_s(SENSEHIDDEN, 40)),
			(10 + get_level_s(SENSEHIDDEN, 40)));
	}
	else
	{
		sprintf(buf,
			"rad %d",
			(15 + get_level_s(SENSEHIDDEN, 40)));
	}

	return buf;
}

bool_ *divination_reveal_ways()
{
	detect_doors(10 + get_level(REVEALWAYS, 40, 0));
	detect_stairs(10 + get_level(REVEALWAYS, 40, 0));
	return CAST;
}

char  *divination_reveal_ways_info()
{
	static char buf[128];
	sprintf(buf,
		"rad %d",
		(10 + get_level_s(REVEALWAYS, 40)));
	return buf;
}

bool_ *divination_sense_monsters()
{
	detect_monsters_normal(10 + get_level(SENSEMONSTERS, 40, 0));
	if (get_level_s(SENSEMONSTERS, 50) >= 30)
	{
		set_tim_esp(10 + randint(10) + get_level_s(SENSEMONSTERS, 20));
	}
	return CAST;
}

char  *divination_sense_monsters_info()
{
	static char buf[128];

	if (get_level_s(SENSEMONSTERS, 50) >= 30)
	{
		sprintf(buf,
			"rad %d dur %d+d10",
			(10 + get_level_s(SENSEMONSTERS, 40)),
			(10 + get_level_s(SENSEMONSTERS, 20)));
	}
	else
	{
		sprintf(buf,
			"rad %d",
			(10 + get_level_s(SENSEMONSTERS, 40)));
	}

	return buf;
}
