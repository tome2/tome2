#include "angband.h"

#include <assert.h>

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

s32b STONESKIN;
s32b DIG;
s32b STONEPRISON;
s32b STRIKE;
s32b SHAKE;

s32b ERU_SEE;
s32b ERU_LISTEN;
s32b ERU_UNDERSTAND;
s32b ERU_PROT;

s32b GLOBELIGHT;
s32b FIREFLASH;
s32b FIERYAURA;
s32b FIREWALL;
s32b FIREGOLEM;

s32b CALL_THE_ELEMENTS;
s32b CHANNEL_ELEMENTS;
s32b ELEMENTAL_WAVE;
s32b VAPORIZE;
s32b GEOLYSIS;
s32b DRIPPING_TREAD;
s32b GROW_BARRIER;
s32b ELEMENTAL_MINION;

s32b MANATHRUST;
s32b DELCURSES;
s32b RESISTS;
s32b MANASHIELD;

s32b MANWE_SHIELD;
s32b MANWE_AVATAR;
s32b MANWE_BLESS;
s32b MANWE_CALL;

s32b MELKOR_CURSE;
s32b MELKOR_CORPSE_EXPLOSION;
s32b MELKOR_MIND_STEAL;

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

bool_ *earth_stone_skin()
{
	int type;

	type = 0;
	if (get_level_s(STONESKIN, 50) >= 25)
	{
		type = SHIELD_COUNTER;
	}

	set_shield(randint(10) + 10 + get_level_s(STONESKIN, 100),
		   10 + get_level_s(STONESKIN, 50),
		   type,
		   2 + get_level_s(STONESKIN, 5),
		   3 + get_level_s(STONESKIN, 5));
	return CAST;
}

char  *earth_stone_skin_info()
{
	static char buf[128];

	if (get_level_s(STONESKIN, 50) >= 25)
	{
		sprintf(buf,
			"dam %dd%d dur %d+d10 AC %d",
			(2 + get_level_s(STONESKIN, 5)),
			(3 + get_level_s(STONESKIN, 5)),
			(10 + get_level_s(STONESKIN, 100)),
			(10 + get_level_s(STONESKIN, 50)));
	}
	else
	{
		sprintf(buf,
			"dur %d+d10 AC %d",
			(10 + get_level_s(STONESKIN, 100)),
			(10 + get_level_s(STONESKIN, 50)));
	}

	return buf;
}

bool_ *earth_dig()
{
	int dir;
	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	wall_to_mud(dir);
	return CAST;
}

char  *earth_dig_info()
{
	return "";
}

bool_ *earth_stone_prison()
{
	int x,y;

	if (get_level_s(STONEPRISON, 50) >= 10)
	{
		if (!tgt_pt(&x, &y))
		{
			return NO_CAST;
		}
	}
	else
	{
		y = p_ptr->py;
		x = p_ptr->px;
	}

	wall_stone(y, x);
	return CAST;
}

char  *earth_stone_prison_info()
{
	return "";
}

bool_ *earth_strike()
{
	int dir, dmg;

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	dmg = 50 + get_level_s(STRIKE, 50);
	if (get_level_s(STRIKE, 50) >= 12)
	{
		fire_ball(GF_FORCE, dir, dmg, 1);
	}
	else
	{
		fire_ball(GF_FORCE, dir, dmg, 0);
	}

	return CAST;
}

char  *earth_strike_info()
{
	static char buf[128];
	int dmg = 50 + get_level_s(STRIKE, 50);

	if (get_level_s(STRIKE, 50) >= 12)
	{
		sprintf(buf, "dam %d rad 1", dmg);
	}
	else
	{
		sprintf(buf, "dam %d", dmg);
	}

	return buf;
}

bool_ *earth_shake()
{
	int x,y;

	if (get_level_s(SHAKE, 50) >= 10)
	{
		if (!tgt_pt(&x, &y))
		{
			return NO_CAST;
		}
	}
	else
	{
		x = p_ptr->px;
		y = p_ptr->py;
	}
	earthquake(y, x, 4 + get_level_s(SHAKE, 10));
	return CAST;
}

char  *earth_shake_info()
{
	static char buf[128];
	sprintf(buf, "rad %d", (4 + get_level_s(SHAKE, 10)));
	return buf;
}

bool_ *eru_see_the_music()
{
	set_tim_invis(randint(20) + 10 + get_level_s(ERU_SEE, 100));

	if (get_level_s(ERU_SEE, 50) >= 30)
	{
		wiz_lite_extra();
	}
	else if (get_level_s(ERU_SEE, 50) >= 10)
	{
		map_area();
	}

	if (get_level_s(ERU_SEE, 50) >= 20)
	{
		set_blind(0);
	}

	return CAST;
}

char  *eru_see_the_music_info()
{
	static char buf[128];
	sprintf(buf,
		"dur %d+d20",
		(10 + get_level_s(ERU_SEE, 100)));
	return buf;
}

bool_ *eru_listen_to_the_music()
{
	if (get_level_s(ERU_LISTEN, 50) >= 30)
	{
		ident_all();
		identify_pack();
	}
	else if (get_level_s(ERU_LISTEN, 50) >= 14)
	{
		identify_pack();
	}
	else
	{
		ident_spell();
	}
	return CAST;
}

char  *eru_listen_to_the_music_info()
{
	return "";
}

bool_ *eru_know_the_music()
{
	if (get_level_s(ERU_UNDERSTAND, 50) >= 10)
	{
		identify_pack_fully();
	}
	else
	{
		identify_fully();
	}
	return CAST;
}

char  *eru_know_the_music_info()
{
	return "";
}

bool_ *eru_lay_of_protection()
{
	fire_ball(GF_MAKE_GLYPH, 0, 1, 1 + get_level(ERU_PROT, 2, 0));
	return CAST;
}

char  *eru_lay_of_protection_info()
{
	static char buf[128];
	sprintf(buf,
		"rad %d",
		(1 + get_level(ERU_PROT, 2, 0)));
	return buf;
}

bool_ *fire_globe_of_light()
{
	if (get_level_s(GLOBELIGHT, 50) >= 3)
	{
		lite_area(10, 4);
	}
	else
	{
		lite_room(p_ptr->py, p_ptr->px);
	}

	if (get_level_s(GLOBELIGHT, 50) >= 15)
	{
		fire_ball(GF_LITE,
			  0,
			  10 + get_level_s(GLOBELIGHT, 100),
			  5 + get_level_s(GLOBELIGHT, 6));
		p_ptr->update |= PU_VIEW;
	}
	return CAST;
}

char  *fire_globe_of_light_info()
{
	static char buf[128];

	if (get_level_s(GLOBELIGHT, 50) >= 15)
	{
		sprintf(buf, "dam %d rad %d",
			(10 + get_level_s(GLOBELIGHT, 100)),
			(5 + get_level_s(GLOBELIGHT, 6)));
	}
	else
	{
		buf[0] = '\0';
	}

	return buf;
}

bool_ *fire_fireflash()
{
	int dir;
	int type = GF_FIRE;

	if (get_level_s(FIREFLASH, 50) >= 20)
	{
		type = GF_HOLY_FIRE;
	}

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	fire_ball(type, dir,
		  20 + get_level_s(FIREFLASH, 500),
		  2 + get_level_s(FIREFLASH, 5));
	return CAST;
}

char  *fire_fireflash_info()
{
	static char buf[128];
	sprintf(buf,
		"dam %d rad %d",
		(20 + get_level_s(FIREFLASH, 500)),
		(2 + get_level_s(FIREFLASH, 5)));
	return buf;
}

bool_ *fire_fiery_shield()
{
	int type = SHIELD_FIRE;
	if (get_level_s(FIERYAURA, 50) >= 8)
	{
		type = SHIELD_GREAT_FIRE;
	}

	set_shield(randint(20) + 10 + get_level_s(FIERYAURA, 70),
		   10,
		   type,
		   5 + get_level_s(FIERYAURA, 10),
		   5 + get_level_s(FIERYAURA, 7));
	return CAST;
}

char  *fire_fiery_shield_info()
{
	static char buf[128];
  	sprintf(buf,
		"dam %dd%d dur %d+d20",
		(5 + get_level_s(FIERYAURA, 15)),
		(5 + get_level_s(FIERYAURA, 7)),
		(10 + get_level_s(FIERYAURA, 70)));
	return buf;
}

bool_ *fire_firewall()
{
	int dir;
	int type = GF_FIRE;
	if (get_level_s(FIREWALL, 50) >= 6)
	{
		type = GF_HELL_FIRE;
	}

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	fire_wall(type, dir,
		  40 + get_level_s(FIREWALL, 150),
		  10 + get_level_s(FIREWALL, 14));
	return CAST;
}

char  *fire_firewall_info()
{
	static char buf[128];
	sprintf(buf,
		"dam %d dur %d",
		(40 + get_level_s(FIREWALL, 150)),
		(10 + get_level_s(FIREWALL, 14)));
	return buf;
}

bool_ item_tester_hook_fire_golem(object_type *o_ptr)
{
	return ((o_ptr->tval == TV_LITE) &&
		((o_ptr->sval == SV_LITE_TORCH) ||
		 (o_ptr->sval == SV_LITE_LANTERN)));
}

bool_ *fire_golem()
{
	int item, x, y, m_idx;

	/* Can we reconnect ? */
	if (do_control_reconnect())
	{
		msg_print("Control re-established.");
		return NO_CAST;
	}

	item_tester_hook = item_tester_hook_fire_golem;
	if (!get_item(&item,
		      "Which light source do you want to use to create the golem?",
		      "You have no light source for the golem",
		      USE_INVEN | USE_EQUIP))
	{
		return NO_CAST;
	}

	/* Destroy the source object */
	inc_stack_size(item, -1);

	/* Summon it */
	m_allow_special[1043 + 1] = TRUE;
	find_position(p_ptr->py, p_ptr->px, &y, &x);
	m_idx = place_monster_one(y, x, 1043, 0, FALSE, MSTATUS_FRIEND);
	m_allow_special[1043 + 1] = FALSE;

	/* level it */
	if (m_idx != 0)
	{
		monster_set_level(m_idx, 7 + get_level_s(FIREGOLEM, 70));
		p_ptr->control = m_idx;
		m_list[m_idx].mflag |= MFLAG_CONTROL;
	}

	return CAST;
}

char  *fire_golem_info()
{
	static char buf[128];
	sprintf(buf,
		"golem level %d",
		(7 + get_level_s(FIREGOLEM, 70)));
	return buf;
}

bool_ *geomancy_call_the_elements()
{
	int dir = 0;

	if (get_level_s(CALL_THE_ELEMENTS, 50) >= 17)
	{
		if (!get_aim_dir(&dir))
		{
			return FALSE;
		}
	}

	fire_ball(GF_ELEMENTAL_GROWTH,
		  dir,
		  1,
		  1 + get_level(CALL_THE_ELEMENTS, 5, 0));

	return CAST;
}

char *geomancy_call_the_elements_info()
{
	static char buf[128];
	sprintf(buf,
		"rad %d",
		(1 + get_level(CALL_THE_ELEMENTS, 5, 0)));
	return buf;
}

bool_ *geomancy_channel_elements()
{
	channel_the_elements(p_ptr->py, p_ptr->px, get_level_s(CHANNEL_ELEMENTS, 50));
	return CAST;
}

char *geomancy_channel_elements_info()
{
	return "";
}

typedef struct eff_type eff_type;
struct eff_type {
	s16b feat;
	s16b low_effect;
	s16b high_effect;
	s16b damage;
};

static eff_type *geomancy_find_effect(eff_type effs[], int feat)
{
	int i;
	for (i = 0; effs[i].feat >= 0; i++)
	{
		eff_type *p = &effs[i];
		if (p->feat == feat)
		{
			return p;
		}
	}
	return NULL;
}

bool_ *geomancy_elemental_wave()
{
	int dir = 0, y = 0, x = 0;
	eff_type *eff_ptr = NULL;
	eff_type t[] =
	{
		/* Earth */
		{ FEAT_GRASS, GF_POIS, GF_POIS, 10 + get_skill_scale(SKILL_EARTH, 200) },
		{ FEAT_FLOWER, GF_POIS, GF_POIS, 10 + get_skill_scale(SKILL_EARTH, 300) },

		/* Water */
		{ FEAT_SHAL_WATER, GF_WATER, GF_WATER, 10 + get_skill_scale(SKILL_WATER, 200) },
		{ FEAT_DEEP_WATER, GF_WATER, GF_WATER, 10 + get_skill_scale(SKILL_WATER, 300) },
		{ FEAT_ICE, GF_ICE, GF_ICE, 10 + get_skill_scale(SKILL_WATER, 200) },

		/* Fire */
		{ FEAT_SAND, GF_LITE, GF_LITE, 10 + get_skill_scale(SKILL_FIRE, 400) },
		{ FEAT_SHAL_LAVA, GF_FIRE, GF_HOLY_FIRE, 10 + get_skill_scale(SKILL_FIRE, 200) },
		{ FEAT_DEEP_LAVA, GF_FIRE, GF_HOLY_FIRE, 10 + get_skill_scale(SKILL_FIRE, 300) },
		{ -1, -1, -1, -1 },
	};

	if (!get_rep_dir(&dir))
	{
		return FALSE;
	}

	y = ddy[dir] + p_ptr->py;
	x = ddx[dir] + p_ptr->px;

	eff_ptr = geomancy_find_effect(t, cave[y][x].feat);

	if (!eff_ptr)
	{
		msg_print("You cannot channel this area.");
		return NO_CAST;
	}
	else
	{
		s16b typ = eff_ptr->low_effect;
		char buf[16];
		s32b EFF_DIR;

		sprintf(buf, "EFF_DIR%d", dir);
		EFF_DIR = get_lua_int(buf);

		if (get_level_s(ELEMENTAL_WAVE, 50) >= 20)
		{
			typ = eff_ptr->high_effect;
		}

		cave_set_feat(y, x, FEAT_FLOOR);

		fire_wave(typ,
			  0,
			  eff_ptr->damage,
			  0,
			  6 + get_level_s(ELEMENTAL_WAVE, 20),
			  EFF_WAVE + EFF_LAST + EFF_DIR);

		return CAST;
	}
}

char *geomancy_elemental_wave_info()
{
	return "";
}

bool_ *geomancy_vaporize()
{
	eff_type *eff_ptr = NULL;
	eff_type t[] = {
		/* Earth stuff */
		{ FEAT_GRASS, GF_POIS, GF_POIS, 5 + get_skill_scale(SKILL_EARTH, 100) },
		{ FEAT_FLOWER, GF_POIS, GF_POIS, 5 + get_skill_scale(SKILL_EARTH, 150) },
		{ FEAT_DARK_PIT, GF_DARK, GF_DARK, 5 + get_skill_scale(SKILL_EARTH, 200) },
		/* Water stuff */
		{ FEAT_SHAL_WATER, GF_WATER, GF_WATER, 5 + get_skill_scale(SKILL_WATER, 100) },
		{ FEAT_DEEP_WATER, GF_WATER, GF_WATER, 5 + get_skill_scale(SKILL_WATER, 150) },
		{ FEAT_ICE, GF_ICE, GF_ICE, 5 + get_skill_scale(SKILL_WATER, 100) },
		/* Fire stuff */
		{ FEAT_SAND, GF_LITE, GF_LITE, 5 + get_skill_scale(SKILL_FIRE, 200) },
		{ FEAT_SHAL_LAVA, GF_FIRE, GF_HOLY_FIRE, 5 + get_skill_scale(SKILL_FIRE, 100) },
		{ FEAT_DEEP_LAVA, GF_FIRE, GF_HOLY_FIRE, 5 + get_skill_scale(SKILL_FIRE, 150) },
		{ -1, -1, -1, -1 },
	};

	eff_ptr = geomancy_find_effect(t, cave[p_ptr->py][p_ptr->px].feat);

	if (!eff_ptr)
	{
		msg_print("You cannot channel this area.");
		return NO_CAST;
	}
	else
	{
		s16b typ = eff_ptr->low_effect;
		if (get_level_s(VAPORIZE, 50) >= 20)
		{
			typ = eff_ptr->high_effect;
		}

		cave_set_feat(p_ptr->py, p_ptr->px, FEAT_FLOOR);

		fire_cloud(typ,
			   0,
			   eff_ptr->damage,
			   1 + get_level_s(VAPORIZE, 4),
			   10 + get_level_s(VAPORIZE, 20));

		return CAST;
	}
}

char *geomancy_vaporize_info()
{
	static char buf[128];
	sprintf(buf,
		"rad %d dur %d",
		(1 + get_level_s(VAPORIZE, 4)),
		(10 + get_level_s(VAPORIZE, 20)));
	return buf;
}

bool_ *geomancy_geolysis()
{
	int dir = 0;

	if (!get_rep_dir(&dir))
	{
		return NO_CAST;
	}

	msg_print("Elements recombine before you, laying down an open path.");
	geomancy_dig(p_ptr->py, p_ptr->px, dir, 5 + get_level_s(GEOLYSIS, 12));

	return CAST;
}

char *geomancy_geolysis_info()
{
	static char buf[128];
	sprintf(buf,
		"length %d",
		(5 + get_level_s(GEOLYSIS, 12)));
	return buf;
}

bool_ *geomancy_dripping_tread()
{
	if (p_ptr->dripping_tread == 0)
	{
		p_ptr->dripping_tread = randint(15) + 10 + get_level_s(DRIPPING_TREAD, 50);
		msg_print("You start dripping raw elemental energies.");
	}
	else
	{
		p_ptr->dripping_tread = 0;
		msg_print("You stop dripping raw elemental energies.");
	}

	return CAST;
}

char *geomancy_dripping_tread_info()
{
	static char buf[128];
	sprintf(buf, 
		"dur %d+d15 movs",
		(10 + get_level_s(DRIPPING_TREAD, 50)));
	return buf;
}

bool_ *geomancy_grow_barrier()
{
	int dir = 0;

	if (get_level_s(GROW_BARRIER, 50) >= 20)
	{
		if (!get_aim_dir(&dir))
		{
			return FALSE;
		}
	}

	fire_ball(GF_ELEMENTAL_WALL, dir, 1, 1);
	return CAST;
}

char *geomancy_grow_barrier_info()
{
	return "";
}

typedef struct geo_summon geo_summon;
struct geo_summon {
	s16b feat;
	s16b skill_idx;
	cptr *summon_names;
};

geo_summon *geomancy_find_summon(geo_summon summons[], int feat)
{
	int i;
	for (i = 0; summons[i].feat >= 0; i++)
	{
		geo_summon *summon = &summons[i];
		if (summon->feat == feat)
		{
			return summon;
		}
	}
	return NULL;
}

int geomancy_count_elements(cptr *elements)
{
	int i;
	for (i = 0; elements[i] != NULL; i++)
	{
	}
	return i;
}

bool_ *geomancy_elemental_minion()
{
	int dir = 0;
	int x = 0, y = 0;
	geo_summon *summon_ptr = NULL;
	cptr earth_summons[] = {
		"Earth elemental",
		"Xorn",
		"Xaren",
		NULL
	};
	cptr air_summons[] = {
		"Air elemental",
		"Ancient blue dragon",
		"Great Storm Wyrm",
		"Sky Drake",
		NULL
	};
	cptr fire_summons[] = {
		"Fire elemental",
		"Ancient red dragon",
		NULL
	};
	cptr water_summons[] = {
		"Water elemental",
		"Water troll",
		"Water demon",
		NULL
	};
	geo_summon summons[] = {
		{ FEAT_WALL_EXTRA, SKILL_EARTH, earth_summons },
		{ FEAT_WALL_OUTER, SKILL_EARTH, earth_summons },
		{ FEAT_WALL_INNER, SKILL_EARTH, earth_summons },
		{ FEAT_WALL_SOLID, SKILL_EARTH, earth_summons },
		{ FEAT_MAGMA, SKILL_EARTH, earth_summons },
		{ FEAT_QUARTZ, SKILL_EARTH, earth_summons },
		{ FEAT_MAGMA_H, SKILL_EARTH, earth_summons },
		{ FEAT_QUARTZ_H, SKILL_EARTH, earth_summons },
		{ FEAT_MAGMA_K, SKILL_EARTH, earth_summons },
		{ FEAT_QUARTZ_K, SKILL_EARTH, earth_summons },

		{ FEAT_DARK_PIT, SKILL_AIR, air_summons },

		{ FEAT_SANDWALL, SKILL_FIRE, fire_summons },
		{ FEAT_SANDWALL_H, SKILL_FIRE, fire_summons },
		{ FEAT_SANDWALL_K, SKILL_FIRE, fire_summons },
		{ FEAT_SHAL_LAVA, SKILL_FIRE, fire_summons },
		{ FEAT_DEEP_LAVA, SKILL_FIRE, fire_summons },

		{ FEAT_ICE_WALL, SKILL_WATER, water_summons },
		{ FEAT_SHAL_WATER, SKILL_WATER, water_summons },
		{ FEAT_DEEP_WATER, SKILL_WATER, water_summons },

		{ -1, -1, NULL },
	};

	if (!get_rep_dir(&dir))
	{
		return NO_CAST;
	}

	y = ddy[dir] + p_ptr->py;
	x = ddx[dir] + p_ptr->px;

	summon_ptr = geomancy_find_summon(summons, cave[y][x].feat);
	
	if (!summon_ptr)
	{
		msg_print("You cannot summon from this area.");
		return NO_CAST;
	}
	else
	{
		cptr *names = summon_ptr->summon_names;
		int max = get_skill_scale(summon_ptr->skill_idx,
					  geomancy_count_elements(names));
		int r_idx = test_monster_name(names[rand_int(max)]);
		int mx, my, m_idx;

		/* Summon it */
		find_position(y, x, &my, &mx);
		m_idx = place_monster_one(my, mx, r_idx, 0, FALSE, MSTATUS_FRIEND);

		/* Level it */
		if (m_idx)
		{
			monster_set_level(m_idx, 10 + get_level_s(ELEMENTAL_MINION, 120));
		}

		cave_set_feat(y, x, FEAT_FLOOR);

		return CAST;
	}
}

char *geomancy_elemental_minion_info()
{
	static char buf[128];
	sprintf(buf,
		"min level %d",
		(10 + get_level_s(ELEMENTAL_MINION, 120)));
	return buf;
}

static void get_manathrust_dam(s16b *num, s16b *sides)
{
	*num = 3 + get_level_s(MANATHRUST, 50);
	*sides = 1 + get_level_s(MANATHRUST, 20);
}

bool_ *mana_manathrust()
{
	int dir;
	s16b num = 0;
	s16b sides = 0;

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	get_manathrust_dam(&num, &sides);
	fire_bolt(GF_MANA, dir, damroll(num, sides));
	return CAST;
}

char *mana_manathrust_info()
{
	s16b num = 0;
	s16b sides = 0;
	static char buf[128];

	get_manathrust_dam(&num, &sides);
	sprintf(buf,
		"dam %dd%d",
		num,
		sides);
	return buf;
}

bool_ *mana_remove_curses()
{
	bool_ done = FALSE;

	if (get_level_s(DELCURSES, 50) >= 20)
	{
		done = remove_all_curse();
	}
	else
	{
		done = remove_curse();
	}

	if (done)
	{
		msg_print("The curse is broken!");
	}

	return CAST;
}

char *mana_remove_curses_info()
{
	return "";
}

bool_ *mana_elemental_shield()
{
	bool_ *res = NO_CAST;

	if (p_ptr->oppose_fire == 0)
	{
		set_oppose_fire(randint(10) + 15 + get_level_s(RESISTS, 50));
		res = CAST;
	}

	if (p_ptr->oppose_cold == 0)
	{
		set_oppose_cold(randint(10) + 15 + get_level_s(RESISTS, 50));
		res = CAST;
	}

	if (p_ptr->oppose_elec == 0)
	{
		set_oppose_elec(randint(10) + 15 + get_level_s(RESISTS, 50));
		res = CAST;
	}

	if (p_ptr->oppose_acid == 0)
	{
		set_oppose_acid(randint(10) + 15 + get_level_s(RESISTS, 50));
		res = CAST;
	}

	return res;
}

char *mana_elemental_shield_info()
{
	static char buf[128];
	sprintf(buf,
		"dur %d+d10",
		(15 + get_level_s(RESISTS, 50)));
	return buf;
}

bool_ *mana_disruption_shield()
{
	if (get_level_s(MANASHIELD, 50) >= 5)
	{
		if (p_ptr->invuln == 0)
		{
			set_invuln(randint(5) + 3 + get_level_s(MANASHIELD, 10));
			return CAST;
		}
	}
	else if (p_ptr->disrupt_shield == 0)
	{
		set_disrupt_shield(randint(5) + 3 + get_level_s(MANASHIELD, 10));
		return CAST;
	}

	return NO_CAST;
}

char *mana_disruption_shield_info()
{
	static char buf[128];
	sprintf(buf,
		"dur %d+d5",
		(3 + get_level_s(MANASHIELD, 10)));
	return buf;
}

bool_ *manwe_wind_shield()
{
	s32b dur = get_level_s(MANWE_SHIELD, 50) + 10 + randint(20);

	set_protevil(dur);
	if (get_level_s(MANWE_SHIELD, 50) >= 10)
	{
		int type = 0;
		if (get_level_s(MANWE_SHIELD, 50) >= 20)
		{
			type = SHIELD_COUNTER;
		}

		set_shield(dur,
			   get_level_s(MANWE_SHIELD, 30),
			   type,
			   1 + get_level_s(MANWE_SHIELD, 2),
			   1 + get_level_s(MANWE_SHIELD, 6));
	}

	return CAST;
}

char  *manwe_wind_shield_info()
{
	static char buf[128];

	sprintf(buf,
		"dur %d+d20",
		(get_level_s(MANWE_SHIELD, 50) + 10));

	if (get_level_s(MANWE_SHIELD, 50) >= 10)
	{
		char tmp[128];
		sprintf(tmp, " AC %d", get_level_s(MANWE_SHIELD, 30));
		strcat(buf, tmp);
	}

	if (get_level_s(MANWE_SHIELD, 50) >= 20)
	{
		char tmp[128];
		sprintf(tmp, " dam %dd%d",
			(1 + get_level_s(MANWE_SHIELD, 2)),
			(1 + get_level_s(MANWE_SHIELD, 6)));
		strcat(buf, tmp);
	}

	return buf;
}

bool_ *manwe_avatar()
{
	s16b mimic_idx = resolve_mimic_name("Maia");
	assert(mimic_idx >= 0);

	set_mimic(get_level_s(MANWE_AVATAR, 20) + randint(10),
		  mimic_idx,
		  p_ptr->lev);
	return CAST;
}

char  *manwe_avatar_info()
{
	static char buf[128];
	sprintf(buf,
		"dur %d+d10",
		get_level_s(MANWE_AVATAR, 20));
	return buf;
}

bool_ *manwe_blessing()
{
	s32b dur = get_level_s(MANWE_BLESS, 70) + 30 + randint(40);

	set_blessed(dur);
	set_afraid(0);
	set_lite(0);

	if (get_level_s(MANWE_BLESS, 50) >= 10)
	{
		set_hero(dur);
	}
	if (get_level_s(MANWE_BLESS, 50) >= 20)
	{
		set_shero(dur);
	}
	if (get_level_s(MANWE_BLESS, 50) >= 30)
	{
		set_holy(dur);
	}

	return CAST;
}

char  *manwe_blessing_info()
{
	static char buf[128];
	sprintf(buf,
		"dur %d+d40",
		get_level_s(MANWE_BLESS, 70) + 30);
	return buf;
}

bool_ *manwe_call()
{
	int y = 0, x = 0, m_idx = -1, r_idx = -1;

	find_position(p_ptr->py, p_ptr->px, &y, &x);

	r_idx = test_monster_name("Great eagle");
	assert(r_idx >= 1);

	m_idx = place_monster_one(y, x, r_idx, 0, FALSE, MSTATUS_FRIEND);

	if (m_idx > 0)
	{
		monster_set_level(m_idx, 20 + get_level(MANWE_CALL, 70, 0));
		return CAST;
	}

	return NO_CAST;
}

char  *manwe_call_info()
{
	static char buf[128];
	sprintf(buf,
		"level %d",
		get_level_s(MANWE_CALL, 70) + 20);
	return buf;
}

void do_melkor_curse(int m_idx)
{
	monster_type *m_ptr = NULL;
	assert(m_idx >= 0);

	m_ptr = &m_list[m_idx];

	if (get_level_s(MELKOR_CURSE, 50) >= 35)
	{
		monster_race *r_ptr = race_info_idx(m_ptr->r_idx, m_ptr->ego);

		m_ptr->maxhp = m_ptr->maxhp - r_ptr->hside;
		if (m_ptr->maxhp < 1)
		{
			m_ptr->maxhp = 1;
		}
		if (m_ptr->hp > m_ptr->maxhp)
		{
			m_ptr->hp = m_ptr->maxhp;
		}

		p_ptr->redraw |= PR_HEALTH;
	}

	if (get_level_s(MELKOR_CURSE, 50) >= 25)
	{
		m_ptr->speed  = m_ptr->speed  - get_level_s(MELKOR_CURSE, 7);
		m_ptr->mspeed = m_ptr->mspeed - get_level_s(MELKOR_CURSE, 7);

		if (m_ptr->speed < 70)
		{
			m_ptr->speed = 70;
		}

		if (m_ptr->mspeed < 70)
		{
			m_ptr->mspeed = 70;
		}
	}

	if (get_level_s(MELKOR_CURSE, 50) >= 15)
	{
		m_ptr->ac = m_ptr->ac - get_level_s(MELKOR_CURSE, 50);

		if (m_ptr->ac < -70)
		{
			m_ptr->ac = -70;
		}
	}

	/* Reduce melee too */
	{
		int i;
		int pow = get_level_s(MELKOR_CURSE, 2);

		for (i = 0; i < 4; i++)
		{
			if (m_ptr->blow[i].d_dice <= 0)
			{
				break;
			}

			if (m_ptr->blow[i].d_dice < pow)
			{
				pow = m_ptr->blow[i].d_dice;
			}
			if (m_ptr->blow[i].d_side < pow)
			{
				pow = m_ptr->blow[i].d_side;
			}

			m_ptr->blow[i].d_dice = m_ptr->blow[i].d_dice - pow;
		}
	}

	/* Describe what happened */
	{
		char buf[128];

		monster_desc(buf, m_ptr, 0);
		buf[0] = toupper(buf[0]);

		strcat(buf, " looks weaker.");
		msg_print(buf);
	}

	/* wake it */
	m_ptr->csleep = 0;
}

bool_ *melkor_curse()
{
	int dir = 0;

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	if (target_who < 0)
	{
		msg_print("You must target a monster.");
		return NO_CAST;
	}
	else
	{
		do_melkor_curse(target_who);
		return CAST;
	}
}

char  *melkor_curse_info()
{
	return "";
}

bool_ *melkor_corpse_explosion()
{
	fire_ball(GF_CORPSE_EXPL,
		  0,
		  20 + get_level_s(MELKOR_CORPSE_EXPLOSION, 70),
		  2 + get_level_s(MELKOR_CORPSE_EXPLOSION, 5));
	return CAST;
}

char  *melkor_corpse_explosion_info()
{
	static char buf[128];
	sprintf(buf,
		"dam %d%%",
		20 + get_level_s(MELKOR_CORPSE_EXPLOSION, 70));
	return buf;
}

bool_ *melkor_mind_steal()
{
	int dir = 0;

	if (!get_aim_dir(&dir))
	{
		return FALSE;
	}

	if (target_who < 0)
	{
		msg_print("You must target a monster.");
		return NO_CAST;
	}
	else
	{
		monster_type *m_ptr = &m_list[target_who];
		int chance = get_level_s(MELKOR_MIND_STEAL, 50);
		monster_race *r_ptr = race_info_idx(m_ptr->r_idx, m_ptr->ego);
		char buf[128];

		monster_desc(buf, m_ptr, 0);
		buf[0] = toupper(buf[0]);

		if ((randint(m_ptr->level) < chance) &&
		    ((r_ptr->flags1 & RF1_UNIQUE) == 0))
		{
			p_ptr->control = target_who;
			m_ptr->mflag |= MFLAG_CONTROL;
			strcat(buf, " falls under your control.");
		}
		else
		{
			strcat(buf, " resists.");
		}

		msg_print(buf);
		return CAST;
	}
}

char  *melkor_mind_steal_info()
{
	static char buf[128];
	sprintf(buf,
		"chance 1d(mlvl)<%d",
		get_level_s(MELKOR_MIND_STEAL, 50));
	return buf;
}
