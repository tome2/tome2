#include "angband.h"

#include <assert.h>

#include "spell_type.h"

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

s32b RECHARGE;
s32b SPELLBINDER;
s32b DISPERSEMAGIC;
s32b TRACKER;
s32b INERTIA_CONTROL;
timer_type *TIMER_INERTIA_CONTROL = 0;

s32b CHARM;
s32b CONFUSE;
s32b ARMOROFFEAR;
s32b STUN;

s32b MAGELOCK;
s32b SLOWMONSTER;
s32b ESSENCESPEED;
s32b BANISHMENT;

s32b TULKAS_AIM;
s32b TULKAS_WAVE;
s32b TULKAS_SPIN;

s32b DRAIN;
s32b GENOCIDE;
s32b WRAITHFORM;
s32b FLAMEOFUDUN;

s32b TIDALWAVE;
s32b ICESTORM;
s32b ENTPOTION;
s32b VAPOR;
s32b GEYSER;

s32b YAVANNA_CHARM_ANIMAL;
s32b YAVANNA_GROW_GRASS;
s32b YAVANNA_TREE_ROOTS;
s32b YAVANNA_WATER_BITE;
s32b YAVANNA_UPROOT;

s32b GROWTREE;
s32b HEALING;
s32b RECOVERY;
s32b REGENERATION;
s32b SUMMONANNIMAL;
s32b GROW_ATHELAS = -1;

s32b DEVICE_HEAL_MONSTER;
s32b DEVICE_SPEED_MONSTER;
s32b DEVICE_WISH;
s32b DEVICE_SUMMON;
s32b DEVICE_MANA;
s32b DEVICE_NOTHING;
s32b DEVICE_LEBOHAUM;
s32b DEVICE_MAGGOT;
s32b DEVICE_HOLY_FIRE;
s32b DEVICE_ETERNAL_FLAME;
s32b DEVICE_DURANDIL;
s32b DEVICE_THUNDERLORDS;
s32b DEVICE_RADAGAST = -1;
s32b DEVICE_VALAROMA = -1;

s32b MUSIC_STOP;
s32b MUSIC_HOLD;
s32b MUSIC_CONF;
s32b MUSIC_STUN;
s32b MUSIC_LITE;
s32b MUSIC_HEAL;
s32b MUSIC_HERO;
s32b MUSIC_TIME;
s32b MUSIC_MIND;
s32b MUSIC_BLOW;
s32b MUSIC_WIND;
s32b MUSIC_YLMIR;
s32b MUSIC_AMBARKANTA;

s32b AULE_FIREBRAND;
s32b AULE_ENCHANT_WEAPON;
s32b AULE_ENCHANT_ARMOUR;
s32b AULE_CHILD;

s32b MANDOS_TEARS_LUTHIEN = -1;
s32b MANDOS_SPIRIT_FEANTURI = -1;
s32b MANDOS_TALE_DOOM = -1;
s32b MANDOS_CALL_HALLS = -1;

s32b ULMO_BELEGAER;
s32b ULMO_DRAUGHT_ULMONAN;
s32b ULMO_CALL_ULUMURI;
s32b ULMO_WRATH;

s32b VARDA_LIGHT_VALINOR;
s32b VARDA_CALL_ALMAREN;
s32b VARDA_EVENSTAR;
s32b VARDA_STARKINDLER;

s32b get_level_s(int sp, int max)
{
	return get_level(sp, max, 1);
}

static casting_result cast(bool_ effect)
{
	return effect ? CAST_OBVIOUS : CAST_HIDDEN;
}

static casting_result cplus(casting_result old, bool_ effect)
{
	if (old == NO_CAST)
	{
		return cast(effect);
	}
	else
	{
		if ((old == CAST_OBVIOUS) || (effect == TRUE)) {
			return CAST_OBVIOUS;
		}
		else
		{
			return CAST_HIDDEN;
		}
	}
}

casting_result air_noxious_cloud(int item)
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
	return CAST_OBVIOUS;
}

char *air_noxious_cloud_info()
{
	static char buf[128];
	sprintf(buf,
		"dam " FMTs32b " rad 3 dur " FMTs32b,
		(7 + get_level_s(NOXIOUSCLOUD, 150)),
		(5 + get_level_s(NOXIOUSCLOUD, 40)));
	return buf;
}

casting_result air_wings_of_winds(int item)
{
	if (get_level_s(AIRWINGS, 50) >= 16)
	{
		if (p_ptr->tim_fly == 0)
		{
			return cast(set_tim_fly(randint(10) + 5 + get_level_s(AIRWINGS, 25)));
		}
	}
	else
	{
		if (p_ptr->tim_ffall == 0)
		{
			return cast(set_tim_ffall(randint(10) + 5 + get_level_s(AIRWINGS, 25)));
		}
	}

	return CAST_HIDDEN;
}

char *air_wings_of_winds_info()
{
	static char buf[128];
	sprintf(buf, "dur " FMTs32b "+d10", (5 + get_level_s(AIRWINGS, 25)));
	return buf;
}

casting_result air_invisibility(int item)
{
	if (p_ptr->tim_invisible == 0)
	{
		return cast(set_invis(randint(20) + 15 + get_level_s(INVISIBILITY, 50), 20 + get_level_s(INVISIBILITY, 50)));
	}

	return CAST_HIDDEN;
}

char *air_invisibility_info()
{
	static char buf[128];
	sprintf(buf, "dur " FMTs32b "+d20 power " FMTs32b,
		(15 + get_level_s(INVISIBILITY, 50)),
		(20 + get_level_s(INVISIBILITY, 50)));
	return buf;
}

casting_result air_poison_blood(int item)
{
	casting_result result = NO_CAST;

	if (p_ptr->oppose_pois == 0)
	{
		result = cplus(result, set_oppose_pois(randint(30) + 25 + get_level_s(POISONBLOOD, 25)));
	}

	if ((p_ptr->tim_poison == 0) &&
	    (get_level_s(POISONBLOOD, 50) >= 15))
	{
		result = cplus(result, set_poison(randint(30) + 25 + get_level_s(POISONBLOOD, 25)));
	}

	return result;
}

char *air_poison_blood_info()
{
	static char buf[128];
	sprintf(buf,
		"dur " FMTs32b "+d30",
		(25 + get_level_s(POISONBLOOD, 25)));
	return buf;
}

casting_result air_thunderstorm(int item)
{
	if (p_ptr->tim_thunder == 0)
	{
		return cast(set_tim_thunder(randint(10) + 10 + get_level_s(THUNDERSTORM, 25), 5 + get_level_s(THUNDERSTORM, 10), 10 + get_level_s(THUNDERSTORM, 25)));
	}

	return CAST_HIDDEN;
}

char *air_thunderstorm_info()
{
	static char buf[128];
	sprintf(buf,
		"dam " FMTs32b "d" FMTs32b " dur " FMTs32b "+d10",
		(5 + get_level_s(THUNDERSTORM, 10)),
		(10 + get_level_s(THUNDERSTORM, 25)),
		(10 + get_level_s(THUNDERSTORM, 25)));
	return buf;
}

casting_result air_sterilize(int item)
{
	set_no_breeders((30) + 20 + get_level_s(STERILIZE, 70));
	return CAST_OBVIOUS;
}

char *air_sterilize_info()
{
	static char buf[128];
	sprintf(buf,
		"dur " FMTs32b "+d30",
		(20 + get_level_s(STERILIZE, 70)));
	return buf;
}

casting_result convey_blink(int item)
{
	if (get_level_s(BLINK, 50) >= 30)
	{
		int oy = p_ptr->py;
		int ox = p_ptr->px;

		teleport_player(10 + get_level_s(BLINK, 8));
		create_between_gate(0, oy, ox);
		return CAST_OBVIOUS;
	}
	else
	{
		teleport_player(10 + get_level_s(BLINK, 8));
		return CAST_OBVIOUS;
	}
}

char *convey_blink_info()
{
	static char buf[128];
	sprintf(buf,
		"distance " FMTs32b,
		(10 + get_level_s(BLINK, 8)));
	return buf;
}

casting_result convey_disarm(int item)
{
	casting_result result = NO_CAST;

	result = cplus(result, destroy_doors_touch());
	if (get_level_s(DISARM, 50) >= 10)
	{
		result = cplus(result, destroy_traps_touch());
	}

	return result;
}

char *convey_disarm_info()
{
	return "";
}

casting_result convey_teleport(int item)
{
	p_ptr->energy -= (25 - get_level_s(TELEPORT, 50));
	teleport_player(100 + get_level_s(TELEPORT, 100));
	return CAST_OBVIOUS;
}

char *convey_teleport_info()
{
	static char buf[128];
	sprintf(buf,
		"distance " FMTs32b,
		(100 + get_level_s(TELEPORT, 100)));
	return buf;
}

casting_result convey_teleport_away(int item)
{
	if (get_level_s(TELEAWAY, 50) >= 20)
	{
		return cast(project_hack(GF_AWAY_ALL, 100));
	}
	else if (get_level_s(TELEAWAY, 50) >= 10)
	{
		int dir;
		if (!get_aim_dir(&dir))
		{
			return NO_CAST;
		}

		return cast(fire_ball(GF_AWAY_ALL, dir, 100, 3 + get_level_s(TELEAWAY, 4)));
	}
	else
	{
		int dir;
		if (!get_aim_dir(&dir))
		{
			return NO_CAST;
		}
		return cast(teleport_monster(dir));
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

casting_result convey_recall(int item)
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
		return CAST_OBVIOUS;
	}
	else if (c_ptr->m_idx > 0)
	{
		swap_position(y, x);
		return CAST_OBVIOUS;
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
		return CAST_OBVIOUS;
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
		"dur %d+d%d weight " FMTs32b "lb",
		f, d, (1 + get_level_s(RECALL, 15)));
	return buf;
}

casting_result convey_probability_travel(int item)
{
	return cast(set_prob_travel(randint(20) + get_level_s(PROBABILITY_TRAVEL, 60)));
}

char *convey_probability_travel_info()
{
	static char buf[128];
	sprintf(buf,
		"dur " FMTs32b "+d20",
		get_level_s(PROBABILITY_TRAVEL, 60));
	return buf;
}

casting_result demonology_demon_blade(int item)
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

	return cast(set_project(randint(20) + get_level_s(DEMON_BLADE, 80),
				type,
				4 + get_level_s(DEMON_BLADE, 40),
				rad,
				PROJECT_STOP | PROJECT_KILL));
}

char  *demonology_demon_blade_info()
{
	static char buf[128];
	sprintf(buf,
		"dur " FMTs32b "+d20 dam " FMTs32b "/blow",
		(get_level_s(DEMON_BLADE, 80)),
		(4 + get_level_s(DEMON_BLADE, 40)));
	return buf;
}

casting_result demonology_demon_madness(int item)
{
	casting_result result = NO_CAST;
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

	result = cplus(result,
		       project(0, 1 + get_level(DEMON_MADNESS, 4, 0),
			       y1, x1,
			       20 + get_level_s(DEMON_MADNESS, 200),
			       type,
			       PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL));
	result = cplus(result,
		       project(0, 1 + get_level(DEMON_MADNESS, 4, 0),
			       y2, x2,
			       20 + get_level_s(DEMON_MADNESS, 200),
			       type,
			       PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL));

	return result;
}

char  *demonology_demon_madness_info()
{
	static char buf[128];
	sprintf(buf,
		"dam " FMTs32b " rad " FMTs32b,
		(20 + get_level_s(DEMON_MADNESS, 200)),
		(1 + get_level(DEMON_MADNESS, 4, 0)));
	return buf;
}

casting_result demonology_demon_field(int item)
{
	int dir;

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}
	
	return cast(fire_cloud(GF_NEXUS,
			       dir,
			       20 + get_level_s(DEMON_FIELD, 70),
			       7,
			       30 + get_level_s(DEMON_FIELD, 100)));
}

char  *demonology_demon_field_info()
{
	static char buf[128];
	sprintf(buf,
		"dam " FMTs32b " dur " FMTs32b,
		(20 + get_level_s(DEMON_FIELD, 70)),
		(30 + get_level_s(DEMON_FIELD, 100)));
	return buf;
}

casting_result demonology_doom_shield(int item)
{
	return cast(set_shield(randint(10) + 20 + get_level_s(DOOM_SHIELD, 100),
			       -300 + get_level_s(DOOM_SHIELD, 100),
			       SHIELD_COUNTER,
			       1 + get_level_s(DOOM_SHIELD, 14),
			       10 + get_level_s(DOOM_SHIELD, 15)));
}

char  *demonology_doom_shield_info()
{
	static char buf[128];
	sprintf(buf,
		"dur " FMTs32b "+d10 dam " FMTs32b "d" FMTs32b,
		(20 + get_level_s(DOOM_SHIELD, 100)),
		(1 + get_level_s(DOOM_SHIELD, 14)),
		(10 + get_level_s(DOOM_SHIELD, 15)));
	return buf;
}

casting_result demonology_unholy_word(int item)
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
		
		return CAST_OBVIOUS;
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
		"heal mhp%% of " FMTs32b "%%",
		(30 + get_level(UNHOLY_WORD, 50, 0)));
	return buf;
}

casting_result demonology_demon_cloak(int item)
{
	return cast(set_tim_reflect(randint(5) + 5 + get_level(DEMON_CLOAK, 15, 0)));
}

char  *demonology_demon_cloak_info()
{
	static char buf[128];
	sprintf(buf,
		"dur " FMTs32b "+d5",
		(5 + get_level(DEMON_CLOAK, 15, 0)));
	return buf;
}

casting_result demonology_summon_demon(int item)
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

	if (summon_specific_friendly(p_ptr->py, p_ptr->px, level, type, TRUE))
	{
		return CAST_OBVIOUS;
	}
	else
	{
		msg_print("Something blocks your summoning!");
		return CAST_HIDDEN;
	}
}

char  *demonology_summon_demon_info()
{
	static char buf[128];
	sprintf(buf,
		"level " FMTs32b,
		(5 + get_level_s(DEMON_SUMMON, 100)));
	return buf;
}

casting_result demonology_discharge_minion(int item)
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
		return cast(project(0, 2, y, x, dam,
				    GF_GRAVITY,
				    PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL));
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
		"dam " FMTs32b "%% max " FMTs32b,
		(20 + get_level(DISCHARGE_MINION, 60, 0)),
		(100 + get_level(DISCHARGE_MINION, 500, 0)));
	return buf;
}

casting_result demonology_control_demon(int item)
{
	int dir;
	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	return cast(fire_ball(GF_CONTROL_DEMON, dir, 50 + get_level_s(CONTROL_DEMON, 250), 0));
}

char  *demonology_control_demon_info()
{
	static char buf[128];
	sprintf(buf,
		"power " FMTs32b,
		(50 + get_level_s(CONTROL_DEMON, 250)));
	return buf;
}

casting_result divination_greater_identify(int item)
{
	if (get_check("Cast on yourself?"))
	{
		self_knowledge(NULL);
	}
	else
	{
		identify_fully();
	}
	return CAST_OBVIOUS;
}

char  *divination_greater_identify_info()
{
	return "";
}

casting_result divination_identify(int item)
{
	if (get_level_s(IDENTIFY, 50) >= 27)
	{
		casting_result result = NO_CAST;
		result = cplus(result, identify_pack());
		result = cplus(result, fire_ball(GF_IDENTIFY, 0, 1, get_level_s(IDENTIFY, 3)));
		return result;
	}
	else if (get_level_s(IDENTIFY, 50) >= 17)
	{
		casting_result result = NO_CAST;
		result = cplus(result, identify_pack());
		result = cplus(result, fire_ball(GF_IDENTIFY, 0, 1, 0));
		return result;
	}
	else if (ident_spell())
	{
		return CAST_OBVIOUS;
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
		sprintf(buf, "rad " FMTs32b, get_level_s(IDENTIFY, 3));
		return buf;
	}
	else
	{
		return "";
	}
}

casting_result divination_vision(int item)
{
	if (get_level_s(VISION, 50) >= 25)
	{
		wiz_lite_extra();
	}
	else
	{
		map_area();
	}
	return CAST_OBVIOUS;

}

char  *divination_vision_info()
{
	return "";
}

casting_result divination_sense_hidden(int item)
{
	casting_result result = NO_CAST;

	result = cplus(result, detect_traps(15 + get_level(SENSEHIDDEN, 40, 0)));
	if (get_level_s(SENSEHIDDEN, 50) >= 15)
	{
		result = cplus(result, set_tim_invis(10 + randint(20) + get_level_s(SENSEHIDDEN, 40)));
	}

	return result;
}

char  *divination_sense_hidden_info()
{
	static char buf[128];

	if (get_level_s(SENSEHIDDEN, 50) >= 15)
	{
		sprintf(buf,
			"rad " FMTs32b " dur " FMTs32b "+d20",
			(15 + get_level_s(SENSEHIDDEN, 40)),
			(10 + get_level_s(SENSEHIDDEN, 40)));
	}
	else
	{
		sprintf(buf,
			"rad " FMTs32b,
			(15 + get_level_s(SENSEHIDDEN, 40)));
	}

	return buf;
}

casting_result divination_reveal_ways(int item)
{
	casting_result result = NO_CAST;
	result = cplus(result, detect_doors(10 + get_level(REVEALWAYS, 40, 0)));
	result = cplus(result, detect_stairs(10 + get_level(REVEALWAYS, 40, 0)));
	return result;
}

char  *divination_reveal_ways_info()
{
	static char buf[128];
	sprintf(buf,
		"rad " FMTs32b,
		(10 + get_level_s(REVEALWAYS, 40)));
	return buf;
}

casting_result divination_sense_monsters(int item)
{
	casting_result result = NO_CAST;

	result = cplus(result, detect_monsters_normal(10 + get_level(SENSEMONSTERS, 40, 0)));
	if (get_level_s(SENSEMONSTERS, 50) >= 30)
	{
		result = cplus(result, set_tim_esp(10 + randint(10) + get_level_s(SENSEMONSTERS, 20)));
	}
	return result;
}

char  *divination_sense_monsters_info()
{
	static char buf[128];

	if (get_level_s(SENSEMONSTERS, 50) >= 30)
	{
		sprintf(buf,
			"rad " FMTs32b " dur " FMTs32b "+d10",
			(10 + get_level_s(SENSEMONSTERS, 40)),
			(10 + get_level_s(SENSEMONSTERS, 20)));
	}
	else
	{
		sprintf(buf,
			"rad " FMTs32b,
			(10 + get_level_s(SENSEMONSTERS, 40)));
	}

	return buf;
}

casting_result earth_stone_skin(int item)
{
	int type;

	type = 0;
	if (get_level_s(STONESKIN, 50) >= 25)
	{
		type = SHIELD_COUNTER;
	}

	return cast(set_shield(randint(10) + 10 + get_level_s(STONESKIN, 100),
			       10 + get_level_s(STONESKIN, 50),
			       type,
			       2 + get_level_s(STONESKIN, 5),
			       3 + get_level_s(STONESKIN, 5)));
}

char  *earth_stone_skin_info()
{
	static char buf[128];

	if (get_level_s(STONESKIN, 50) >= 25)
	{
		sprintf(buf,
			"dam " FMTs32b "d" FMTs32b " dur " FMTs32b "+d10 AC " FMTs32b,
			(2 + get_level_s(STONESKIN, 5)),
			(3 + get_level_s(STONESKIN, 5)),
			(10 + get_level_s(STONESKIN, 100)),
			(10 + get_level_s(STONESKIN, 50)));
	}
	else
	{
		sprintf(buf,
			"dur " FMTs32b "+d10 AC " FMTs32b,
			(10 + get_level_s(STONESKIN, 100)),
			(10 + get_level_s(STONESKIN, 50)));
	}

	return buf;
}

casting_result earth_dig(int item)
{
	int dir;
	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	return cast(wall_to_mud(dir));
}

char  *earth_dig_info()
{
	return "";
}

casting_result earth_stone_prison(int item)
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
	return CAST_OBVIOUS;
}

char  *earth_stone_prison_info()
{
	return "";
}

casting_result earth_strike(int item)
{
	int dir, dmg;

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	dmg = 50 + get_level_s(STRIKE, 50);
	if (get_level_s(STRIKE, 50) >= 12)
	{
		return cast(fire_ball(GF_FORCE, dir, dmg, 1));
	}
	else
	{
		return cast(fire_ball(GF_FORCE, dir, dmg, 0));
	}
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

casting_result earth_shake(int item)
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
	return CAST_OBVIOUS;
}

char  *earth_shake_info()
{
	static char buf[128];
	sprintf(buf, "rad " FMTs32b, (4 + get_level_s(SHAKE, 10)));
	return buf;
}

casting_result eru_see_the_music(int item)
{
	casting_result result = NO_CAST;

	result = cplus(result, set_tim_invis(randint(20) + 10 + get_level_s(ERU_SEE, 100)));

	if (get_level_s(ERU_SEE, 50) >= 30)
	{
		wiz_lite_extra();
		result = CAST_OBVIOUS;
	}
	else if (get_level_s(ERU_SEE, 50) >= 10)
	{
		map_area();
		result = CAST_OBVIOUS;
	}

	if (get_level_s(ERU_SEE, 50) >= 20)
	{
		result = cplus(result, set_blind(0));
	}

	return result;
}

char  *eru_see_the_music_info()
{
	static char buf[128];
	sprintf(buf,
		"dur " FMTs32b "+d20",
		(10 + get_level_s(ERU_SEE, 100)));
	return buf;
}

casting_result eru_listen_to_the_music(int item)
{
	casting_result result = NO_CAST;

	if (get_level_s(ERU_LISTEN, 50) >= 30)
	{
		result = cplus(result, ident_all());
		result = cplus(result, identify_pack());
	}
	else if (get_level_s(ERU_LISTEN, 50) >= 14)
	{
		result = cplus(result, identify_pack());
	}
	else
	{
		result = cplus(result, ident_spell());
	}

	return result;
}

char  *eru_listen_to_the_music_info()
{
	return "";
}

casting_result eru_know_the_music(int item)
{
	if (get_level_s(ERU_UNDERSTAND, 50) >= 10)
	{
		identify_pack_fully();
		return CAST_OBVIOUS;
	}
	else
	{
		return cast(identify_fully());
	}
}

char  *eru_know_the_music_info()
{
	return "";
}

casting_result eru_lay_of_protection(int item)
{
	return cast(fire_ball(GF_MAKE_GLYPH, 0, 1, 1 + get_level(ERU_PROT, 2, 0)));
}

char  *eru_lay_of_protection_info()
{
	static char buf[128];
	sprintf(buf,
		"rad " FMTs32b,
		(1 + get_level(ERU_PROT, 2, 0)));
	return buf;
}

casting_result fire_globe_of_light(int item)
{
	casting_result result = NO_CAST;

	if (get_level_s(GLOBELIGHT, 50) >= 3)
	{
		result = cplus(result, lite_area(10, 4));
	}
	else
	{
		lite_room(p_ptr->py, p_ptr->px);
		result = CAST_OBVIOUS;
	}

	if (get_level_s(GLOBELIGHT, 50) >= 15)
	{
		result = cplus(result,
			       fire_ball(GF_LITE,
					 0,
					 10 + get_level_s(GLOBELIGHT, 100),
					 5 + get_level_s(GLOBELIGHT, 6)));
		p_ptr->update |= PU_VIEW;
	}

	return result;
}

char  *fire_globe_of_light_info()
{
	static char buf[128];

	if (get_level_s(GLOBELIGHT, 50) >= 15)
	{
		sprintf(buf, "dam " FMTs32b " rad " FMTs32b,
			(10 + get_level_s(GLOBELIGHT, 100)),
			(5 + get_level_s(GLOBELIGHT, 6)));
	}
	else
	{
		buf[0] = '\0';
	}

	return buf;
}

casting_result fire_fireflash(int item)
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

	return cast(fire_ball(type, dir,
			      20 + get_level_s(FIREFLASH, 500),
			      2 + get_level_s(FIREFLASH, 5)));
}

char  *fire_fireflash_info()
{
	static char buf[128];
	sprintf(buf,
		"dam " FMTs32b " rad " FMTs32b,
		(20 + get_level_s(FIREFLASH, 500)),
		(2 + get_level_s(FIREFLASH, 5)));
	return buf;
}

casting_result fire_fiery_shield(int item)
{
	int type = SHIELD_FIRE;
	if (get_level_s(FIERYAURA, 50) >= 8)
	{
		type = SHIELD_GREAT_FIRE;
	}

	return cast(set_shield(randint(20) + 10 + get_level_s(FIERYAURA, 70),
			       10,
			       type,
			       5 + get_level_s(FIERYAURA, 10),
			       5 + get_level_s(FIERYAURA, 7)));
}

char  *fire_fiery_shield_info()
{
	static char buf[128];
  	sprintf(buf,
		"dam " FMTs32b "d" FMTs32b " dur " FMTs32b "+d20",
		(5 + get_level_s(FIERYAURA, 15)),
		(5 + get_level_s(FIERYAURA, 7)),
		(10 + get_level_s(FIERYAURA, 70)));
	return buf;
}

casting_result fire_firewall(int item)
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
	return CAST_OBVIOUS;
}

char  *fire_firewall_info()
{
	static char buf[128];
	sprintf(buf,
		"dam " FMTs32b " dur " FMTs32b,
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

casting_result fire_golem(int ignored)
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

	return CAST_OBVIOUS;
}

char  *fire_golem_info()
{
	static char buf[128];
	sprintf(buf,
		"golem level " FMTs32b,
		(7 + get_level_s(FIREGOLEM, 70)));
	return buf;
}

casting_result geomancy_call_the_elements(int item)
{
	int dir = 0;

	if (get_level_s(CALL_THE_ELEMENTS, 50) >= 17)
	{
		if (!get_aim_dir(&dir))
		{
			return NO_CAST;
		}
	}

	fire_ball(GF_ELEMENTAL_GROWTH,
		  dir,
		  1,
		  1 + get_level(CALL_THE_ELEMENTS, 5, 0));

	return CAST_OBVIOUS;
}

char *geomancy_call_the_elements_info()
{
	static char buf[128];
	sprintf(buf,
		"rad " FMTs32b,
		(1 + get_level(CALL_THE_ELEMENTS, 5, 0)));
	return buf;
}

casting_result geomancy_channel_elements(int item)
{
	channel_the_elements(p_ptr->py, p_ptr->px, get_level_s(CHANNEL_ELEMENTS, 50));
	return CAST_OBVIOUS;
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

static u32b dir_to_eff_flags(int dir)
{
	assert(dir >= 1);
	assert(dir <= 9);

	switch (dir)
	{
	case 1: return EFF_DIR1;
	case 2: return EFF_DIR2;
	case 3: return EFF_DIR3;
	case 4: return EFF_DIR4;
	case 5: return 0;
	case 6: return EFF_DIR6;
	case 7: return EFF_DIR7;
	case 8: return EFF_DIR8;
	case 9: return EFF_DIR9;
	default:
		assert(FALSE);
	}
	/* Default */
	return 0;
}

casting_result geomancy_elemental_wave(int item)
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
		return NO_CAST;
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
		u32b dir_flag = dir_to_eff_flags(dir);

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
			  EFF_WAVE + EFF_LAST + dir_flag);

		return CAST_OBVIOUS;
	}
}

char *geomancy_elemental_wave_info()
{
	return "";
}

casting_result geomancy_vaporize(int item)
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

		return CAST_OBVIOUS;
	}
}

char *geomancy_vaporize_info()
{
	static char buf[128];
	sprintf(buf,
		"rad " FMTs32b " dur " FMTs32b,
		(1 + get_level_s(VAPORIZE, 4)),
		(10 + get_level_s(VAPORIZE, 20)));
	return buf;
}

bool_ geomancy_vaporize_depends()
{
	return get_skill(SKILL_AIR) >= 4;
}

casting_result geomancy_geolysis(int item)
{
	int dir = 0;

	if (!get_rep_dir(&dir))
	{
		return NO_CAST;
	}

	msg_print("Elements recombine before you, laying down an open path.");
	geomancy_dig(p_ptr->py, p_ptr->px, dir, 5 + get_level_s(GEOLYSIS, 12));

	return CAST_OBVIOUS;
}

char *geomancy_geolysis_info()
{
	static char buf[128];
	sprintf(buf,
		"length " FMTs32b,
		(5 + get_level_s(GEOLYSIS, 12)));
	return buf;
}

bool_ geomancy_geolysis_depends()
{
	return get_skill(SKILL_EARTH) >= 7;
}

casting_result geomancy_dripping_tread(int item)
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

	return CAST_OBVIOUS;
}

char *geomancy_dripping_tread_info()
{
	static char buf[128];
	sprintf(buf, 
		"dur " FMTs32b "+d15 movs",
		(10 + get_level_s(DRIPPING_TREAD, 50)));
	return buf;
}

bool_ geomancy_dripping_tread_depends()
{
	return get_skill(SKILL_WATER) >= 10;
}

casting_result geomancy_grow_barrier(int item)
{
	int dir = 0;

	if (get_level_s(GROW_BARRIER, 50) >= 20)
	{
		if (!get_aim_dir(&dir))
		{
			return NO_CAST;
		}
	}

	fire_ball(GF_ELEMENTAL_WALL, dir, 1, 1);
	return CAST_OBVIOUS;
}

char *geomancy_grow_barrier_info()
{
	return "";
}

bool_ geomancy_grow_barrier_depends()
{
	return get_skill(SKILL_EARTH) >= 12;
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

casting_result geomancy_elemental_minion(int item)
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

		return CAST_OBVIOUS;
	}
}

char *geomancy_elemental_minion_info()
{
	static char buf[128];
	sprintf(buf,
		"min level " FMTs32b,
		(10 + get_level_s(ELEMENTAL_MINION, 120)));
	return buf;
}

static void get_manathrust_dam(s16b *num, s16b *sides)
{
	*num = 3 + get_level_s(MANATHRUST, 50);
	*sides = 1 + get_level_s(MANATHRUST, 20);
}

casting_result mana_manathrust(int item)
{
	int dir;
	s16b num = 0;
	s16b sides = 0;

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	get_manathrust_dam(&num, &sides);
	return cast(fire_bolt(GF_MANA, dir, damroll(num, sides)));
}

char *mana_manathrust_info()
{
	s16b num = 0;
	s16b sides = 0;
	static char buf[128];

	get_manathrust_dam(&num, &sides);
	sprintf(buf,
		"dam " FMTs16b "d" FMTs16b,
		num,
		sides);
	return buf;
}

casting_result mana_remove_curses(int item)
{
	casting_result result = NO_CAST;

	if (get_level_s(DELCURSES, 50) >= 20)
	{
		result = cplus(result, remove_all_curse());
	}
	else
	{
		result = cplus(result, remove_curse());
	}

	if (result == CAST_OBVIOUS)
	{
		msg_print("The curse is broken!");
	}

	return result;
}

char *mana_remove_curses_info()
{
	return "";
}

casting_result mana_elemental_shield(int item)
{
	casting_result res = NO_CAST;

	if (p_ptr->oppose_fire == 0)
	{
		res = cplus(res, set_oppose_fire(randint(10) + 15 + get_level_s(RESISTS, 50)));
	}

	if (p_ptr->oppose_cold == 0)
	{
		res = cplus(res, set_oppose_cold(randint(10) + 15 + get_level_s(RESISTS, 50)));
	}

	if (p_ptr->oppose_elec == 0)
	{
		res = cplus(res, set_oppose_elec(randint(10) + 15 + get_level_s(RESISTS, 50)));
	}

	if (p_ptr->oppose_acid == 0)
	{
		res = cplus(res, set_oppose_acid(randint(10) + 15 + get_level_s(RESISTS, 50)));
	}

	return res;
}

char *mana_elemental_shield_info()
{
	static char buf[128];
	sprintf(buf,
		"dur " FMTs32b "+d10",
		(15 + get_level_s(RESISTS, 50)));
	return buf;
}

casting_result mana_disruption_shield(int item)
{
	if (get_level_s(MANASHIELD, 50) >= 5)
	{
		if (p_ptr->invuln == 0)
		{
			return cast(set_invuln(randint(5) + 3 + get_level_s(MANASHIELD, 10)));
		}
	}
	else if (p_ptr->disrupt_shield == 0)
	{
		return cast(set_disrupt_shield(randint(5) + 3 + get_level_s(MANASHIELD, 10)));
	}

	return NO_CAST;
}

char *mana_disruption_shield_info()
{
	static char buf[128];
	sprintf(buf,
		"dur " FMTs32b "+d5",
		(3 + get_level_s(MANASHIELD, 10)));
	return buf;
}

casting_result manwe_wind_shield(int item)
{
	casting_result res = NO_CAST;
	s32b dur = get_level_s(MANWE_SHIELD, 50) + 10 + randint(20);

	res = cplus(res, set_protevil(dur));

	if (get_level_s(MANWE_SHIELD, 50) >= 10)
	{
		int type = 0;
		if (get_level_s(MANWE_SHIELD, 50) >= 20)
		{
			type = SHIELD_COUNTER;
		}

		res = cplus(res,
			    set_shield(dur,
				       get_level_s(MANWE_SHIELD, 30),
				       type,
				       1 + get_level_s(MANWE_SHIELD, 2),
				       1 + get_level_s(MANWE_SHIELD, 6)));
	}

	return res;
}

char  *manwe_wind_shield_info()
{
	static char buf[128];

	sprintf(buf,
		"dur " FMTs32b "+d20",
		(get_level_s(MANWE_SHIELD, 50) + 10));

	if (get_level_s(MANWE_SHIELD, 50) >= 10)
	{
		char tmp[128];
		sprintf(tmp, " AC " FMTs32b, get_level_s(MANWE_SHIELD, 30));
		strcat(buf, tmp);
	}

	if (get_level_s(MANWE_SHIELD, 50) >= 20)
	{
		char tmp[128];
		sprintf(tmp, " dam " FMTs32b "d" FMTs32b,
			(1 + get_level_s(MANWE_SHIELD, 2)),
			(1 + get_level_s(MANWE_SHIELD, 6)));
		strcat(buf, tmp);
	}

	return buf;
}

casting_result manwe_avatar(int item)
{
	s16b mimic_idx = resolve_mimic_name("Maia");
	assert(mimic_idx >= 0);

	return cast(set_mimic(get_level_s(MANWE_AVATAR, 20) + randint(10),
			      mimic_idx,
			      p_ptr->lev));
}

char  *manwe_avatar_info()
{
	static char buf[128];
	sprintf(buf,
		"dur " FMTs32b "+d10",
		get_level_s(MANWE_AVATAR, 20));
	return buf;
}

casting_result manwe_blessing(int item)
{
	casting_result res = NO_CAST;
	s32b dur = get_level_s(MANWE_BLESS, 70) + 30 + randint(40);

	res = cplus(res, set_blessed(dur));
	res = cplus(res, set_afraid(0));
	res = cplus(res, set_lite(0));

	if (get_level_s(MANWE_BLESS, 50) >= 10)
	{
		res = cplus(res, set_hero(dur));
	}
	if (get_level_s(MANWE_BLESS, 50) >= 20)
	{
		res = cplus(res, set_shero(dur));
	}
	if (get_level_s(MANWE_BLESS, 50) >= 30)
	{
		res = cplus(res, set_holy(dur));
	}

	return res;
}

char  *manwe_blessing_info()
{
	static char buf[128];
	sprintf(buf,
		"dur " FMTs32b "+d40",
		get_level_s(MANWE_BLESS, 70) + 30);
	return buf;
}

casting_result manwe_call(int item)
{
	int y = 0, x = 0, m_idx = -1, r_idx = -1;

	find_position(p_ptr->py, p_ptr->px, &y, &x);

	r_idx = test_monster_name("Great eagle");
	assert(r_idx >= 1);

	m_idx = place_monster_one(y, x, r_idx, 0, FALSE, MSTATUS_FRIEND);

	if (m_idx > 0)
	{
		monster_set_level(m_idx, 20 + get_level(MANWE_CALL, 70, 0));
		return CAST_OBVIOUS;
	}

	return NO_CAST;
}

char  *manwe_call_info()
{
	static char buf[128];
	sprintf(buf,
		"level " FMTs32b,
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

casting_result melkor_curse(int item)
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
		return CAST_OBVIOUS;
	}
}

char  *melkor_curse_info()
{
	return "";
}

casting_result melkor_corpse_explosion(int item)
{
	return cast(fire_ball(GF_CORPSE_EXPL,
			      0,
			      20 + get_level_s(MELKOR_CORPSE_EXPLOSION, 70),
			      2 + get_level_s(MELKOR_CORPSE_EXPLOSION, 5)));
}

char  *melkor_corpse_explosion_info()
{
	static char buf[128];
	sprintf(buf,
		"dam " FMTs32b "%%",
		20 + get_level_s(MELKOR_CORPSE_EXPLOSION, 70));
	return buf;
}

casting_result melkor_mind_steal(int item)
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
		return CAST_OBVIOUS;
	}
}

char  *melkor_mind_steal_info()
{
	static char buf[128];
	sprintf(buf,
		"chance 1d(mlvl)<" FMTs32b,
		get_level_s(MELKOR_MIND_STEAL, 50));
	return buf;
}

casting_result meta_recharge(int item)
{
	return cast(recharge(60 + get_level_s(RECHARGE, 140)));
}

char *meta_recharge_info()
{
	static char buf[128];
	sprintf(buf,
		"power " FMTs32b,
		60 + get_level_s(RECHARGE, 140));
	return buf;
}

static int get_spellbinder_max()
{
	int i = get_level_s(SPELLBINDER, 4);
	if (i > 4)
	{
		i = 4;
	}
	return i;
}

casting_result meta_spellbinder(int item)
{
	if (p_ptr->spellbinder_num != 0)
	{
		typedef struct trigger trigger;
		struct trigger {
			int idx;
			cptr desc;
		};
		trigger triggers[] = {
			{ SPELLBINDER_HP75, "75% HP", },
			{ SPELLBINDER_HP50, "50% HP", },
			{ SPELLBINDER_HP25, "25% HP", },
			{ -1, NULL, },
		};
		int trigger_idx = -1;
		int i;

		assert(p_ptr->spellbinder_trigger >= 0);

		for (trigger_idx = 0; triggers[trigger_idx].idx >= 0; trigger_idx++)
		{
			if (triggers[trigger_idx].idx == p_ptr->spellbinder_trigger)
			{
				break;
			}
		}

		msg_print("The spellbinder is already active.");
		msg_format("It will trigger at %s.", triggers[trigger_idx].desc);
		msg_print("With the spells: ");
		for (i = 0; i < p_ptr->spellbinder_num; i++)
		{ 
			msg_print(spell_type_name(spell_at(p_ptr->spellbinder[i])));
		}

		/* Doesn't cost anything */
		return NO_CAST;
	}
	else
	{
		char c;
		int i;

		if (!get_com("Trigger at [a]75% hp [b]50% hp [c]25% hp?", &c))
		{
			return NO_CAST;
		}
		
		switch (c)
		{
		case 'a':
			p_ptr->spellbinder_trigger = SPELLBINDER_HP75;
			break;
		case 'b':
			p_ptr->spellbinder_trigger = SPELLBINDER_HP50;
			break;
		case 'c':
			p_ptr->spellbinder_trigger = SPELLBINDER_HP25;
			break;
		default:
			return NO_CAST;
			
		}

		p_ptr->spellbinder_num = get_spellbinder_max();
		i = p_ptr->spellbinder_num;
		while (i > 0)
		{
			s32b s = get_school_spell("bind", 0);
			if (s == -1)
			{
				p_ptr->spellbinder_trigger = 0;
				p_ptr->spellbinder_num = 0;
				return CAST_OBVIOUS;
			} else {
				if (spell_type_skill_level(spell_at(s)) > 7 + get_level_s(SPELLBINDER, 35))
				{
					msg_format("You are only allowed spells with a base level of " FMTs32b ".", (7 + get_level_s(SPELLBINDER, 35)));
					return CAST_OBVIOUS;
				}
			}

			p_ptr->spellbinder[i] = s;
			i = i - 1;
		}
		
		p_ptr->energy = p_ptr->energy - 3100;
		msg_print("Spellbinder ready.");
		return CAST_OBVIOUS;
	}
}

char *meta_spellbinder_info()
{
	static char buf[128];
	sprintf(buf,
		"number %d max level " FMTs32b,
		get_spellbinder_max(),
		(7 + get_level_s(SPELLBINDER, 35)));
	return buf;
}

casting_result meta_disperse_magic(int item)
{
	casting_result res = NO_CAST;

	res = cplus(res, set_blind(0));
	res = cplus(res, set_lite(0));
	if (get_level_s(DISPERSEMAGIC, 50) >= 5)
	{
		res = cplus(res, set_confused(0));
		res = cplus(res, set_image(0));
	}
	if (get_level_s(DISPERSEMAGIC, 50) >= 10)
	{
		res = cplus(res, set_slow(0));
		res = cplus(res, set_fast(0, 0));
		res = cplus(res, set_light_speed(0));
	}
	if (get_level_s(DISPERSEMAGIC, 50) >= 15)
	{
		res = cplus(res, set_stun(0));
		res = cplus(res, set_meditation(0));
		res = cplus(res, set_cut(0));
	}
	if (get_level_s(DISPERSEMAGIC, 50) >= 20)
	{
		res = cplus(res, set_hero(0));
		res = cplus(res, set_shero(0));
		res = cplus(res, set_blessed(0));
		res = cplus(res, set_shield(0, 0, 0, 0, 0));
		res = cplus(res, set_afraid(0));
		res = cplus(res, set_parasite(0, 0));
		res = cplus(res, set_mimic(0, 0, 0));
	}
	return res;
}

char *meta_disperse_magic_info()
{
	return "";
}

casting_result meta_tracker(int item)
{
	if ((last_teleportation_y < 0) ||
	    (last_teleportation_x < 0))
	{
		msg_print("There has not been any teleporatation here.");
	}
	else
	{
		teleport_player_to(last_teleportation_y, last_teleportation_x);
	}
	return CAST_OBVIOUS;
}

char *meta_tracker_info()
{
	return "";
}

static void stop_inertia_controlled_spell()
{
	assert(TIMER_INERTIA_CONTROL != NULL);

	p_ptr->inertia_controlled_spell = -1;
	TIMER_INERTIA_CONTROL->enabled = FALSE;
	p_ptr->update = p_ptr->update | PU_MANA;
}

void meta_inertia_control_hook_birth_objects()
{
	stop_inertia_controlled_spell();
}

casting_result meta_inertia_control(int item)
{
	s32b s, difficulty, delay;
	spell_type *spell;

	if (p_ptr->inertia_controlled_spell != -1)
	{
		msg_print("You cancel your inertia flow control.");
		stop_inertia_controlled_spell();
		return NO_CAST;
	}

	s = get_school_spell("control", 0);
	if (s == -1)
	{
		stop_inertia_controlled_spell();
		return NO_CAST;
	}

	spell = spell_at(s);

	if (!spell_type_inertia(spell, &difficulty, &delay))
	{
		msg_print("This spell inertia flow can not be controlled.");
		stop_inertia_controlled_spell();
		return NO_CAST;
	}

	if (difficulty > get_level_s(INERTIA_CONTROL, 10))
	{
		msg_format("This spell inertia flow(" FMTs32b ") is too strong to be controlled by your current spell.", difficulty);
		stop_inertia_controlled_spell();
		return NO_CAST;
	}

	p_ptr->inertia_controlled_spell = s;
	TIMER_INERTIA_CONTROL->enabled = TRUE;
	TIMER_INERTIA_CONTROL->delay = delay;
	TIMER_INERTIA_CONTROL->countdown = delay;
	p_ptr->update |= PU_MANA;
	msg_format("Inertia flow controlling spell %s.", spell_type_name(spell_at(s)));
	return CAST_OBVIOUS;
}

char *meta_inertia_control_info()
{
	static char buf[128];
	sprintf(buf,
		"level " FMTs32b,
		get_level_s(INERTIA_CONTROL, 10));
	return buf;
}

void meta_inertia_control_timer_callback()
{
	/* Don't cast a controlled spell in wilderness mode */
	if (p_ptr->antimagic)
	{
		msg_print("Your anti-magic field disrupts any magic attempts.");
	}
	else if (p_ptr->anti_magic)
	{
		msg_print("Your anti-magic shell disrupts any magic attempts.");
	}
	else if ((p_ptr->inertia_controlled_spell != -1) &&
		 (!p_ptr->wild_mode))
	{
		lua_cast_school_spell(p_ptr->inertia_controlled_spell, TRUE);
	}
}

void meta_inertia_control_calc_mana(int *msp)
{
	if (p_ptr->inertia_controlled_spell != -1)
	{
		*msp = *msp - (get_mana(p_ptr->inertia_controlled_spell) * 4);
		if (*msp < 0)
		{
			*msp = 0;
		}
	}
}

static int mind_charm_power()
{
	return 10 + get_level_s(CHARM, 150);
}

casting_result mind_charm(int item)
{
	int pwr = mind_charm_power();
	int level = get_level_s(CHARM, 50);

	if (level >= 35)
	{
		return cast(project_hack(GF_CHARM, pwr));
	}
	else
	{
		int dir;
		if (!get_aim_dir(&dir))
		{
			return NO_CAST;
		}

		if (level >= 15)
		{
			return cast(fire_ball(GF_CHARM, dir, pwr, 3));
		}
		else
		{
			return cast(fire_bolt(GF_CHARM, dir, pwr));
		}
	}
}

char *mind_charm_info()
{
	static char buf[128];
	sprintf(buf,
		"power %d",
		mind_charm_power());
	return buf;
}

static int mind_confuse_power()
{
	return 10 + get_level_s(CONFUSE, 150);
}

casting_result mind_confuse(int item)
{
	int pwr = mind_confuse_power();
	int level = get_level_s(CONFUSE, 50);

	if (level >= 35)
	{
		return cast(project_hack(GF_OLD_CONF, pwr));
	}
	else
	{
		int dir;
		if (!get_aim_dir(&dir))
		{
			return NO_CAST;
		}
		
		if (level >= 15)
		{
			return cast(fire_ball(GF_OLD_CONF, dir, pwr, 3));
		}
		else
		{
			return cast(fire_bolt(GF_OLD_CONF, dir, pwr));
		}
	}
}

char *mind_confuse_info()
{
	static char buf[128];
	sprintf(buf,
		"power %d",
		mind_confuse_power());
	return buf;
}

static int mind_armor_of_fear_base_duration()
{
	return 10 + get_level_s(ARMOROFFEAR, 100);
}

static int mind_armor_of_fear_power_sides()
{
	return 1 + get_level_s(ARMOROFFEAR, 7);
}

static int mind_armor_of_fear_power_dice()
{
	return 5 + get_level_s(ARMOROFFEAR, 20);
}

casting_result mind_armor_of_fear(int item)
{
	return cast(set_shield(randint(10) + mind_armor_of_fear_base_duration(),
			       10,
			       SHIELD_FEAR,
			       mind_armor_of_fear_power_sides(),
			       mind_armor_of_fear_power_dice()));
}

char  *mind_armor_of_fear_info()
{
	static char buf[128];
	sprintf(buf,
		"dur %d+d10 power %dd%d",
		mind_armor_of_fear_base_duration(),
		mind_armor_of_fear_power_sides(),
		mind_armor_of_fear_power_dice());
	return buf;
}

static int mind_stun_power()
{
	return 10 + get_level_s(STUN, 150);
}

casting_result mind_stun(int item)
{
	int dir;

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	if (get_level_s(STUN, 50) >= 20)
	{
		return cast(fire_ball(GF_STUN, dir, mind_stun_power(), 3));
	}
	else
	{
		return cast(fire_bolt(GF_STUN, dir, mind_stun_power()));
	}
}

char  *mind_stun_info()
{
	static char buf[128];
	sprintf(buf,
		"power %d",
		mind_stun_power());
	return buf;
}

casting_result tempo_magelock(int item)
{
	if (get_level_s(MAGELOCK, 50) >= 30)
	{
		int x,y;

		if (get_level_s(MAGELOCK, 50) >= 40)
		{
			cave_type *c_ptr = NULL;

			if (!tgt_pt(&x, &y))
			{
				return NO_CAST;
			}

			c_ptr = &cave[y][x];

			if ((!(f_info[c_ptr->feat].flags1 | FF1_FLOOR)) ||
			    (f_info[c_ptr->feat].flags1 | FF1_PERMANENT) ||
			    (!los(p_ptr->py, p_ptr->px, y, x)))
			{
				msg_print("You cannot place it there.");
				return NO_CAST;
			}
		} else {
			y = p_ptr->py;
			x = p_ptr->px;
		}
		cave_set_feat(y, x, 3);
		return CAST_OBVIOUS;
	} else {
		int dir;
		if (!get_aim_dir(&dir))
		{
			return NO_CAST;
		}
		return cast(wizard_lock(dir));
	}
}

char *tempo_magelock_info()
{
	return "";
}

static s32b tempo_slow_monster_power()
{
	return 40 + get_level_s(SLOWMONSTER, 160);
}

casting_result tempo_slow_monster(int item)
{
	int dir;
	s32b pwr;

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	pwr = tempo_slow_monster_power();
	if (get_level_s(SLOWMONSTER, 50) >= 20)
	{
		return cast(fire_ball(GF_OLD_SLOW, dir, pwr, 1));
	}
	else
	{
		return cast(fire_bolt(GF_OLD_SLOW, dir, pwr));
	}
}

char *tempo_slow_monster_info()
{
	static char buf[128];
	s32b pwr = tempo_slow_monster_power();

	if (get_level_s(SLOWMONSTER, 50) >= 20)
	{
		sprintf(buf, "power " FMTs32b " rad 1", pwr);
	}
	else
	{
		sprintf(buf, "power " FMTs32b, pwr);
	}
	return buf;
}

static s32b tempo_essence_of_speed_base_duration()
{
	return 10 + get_level_s(ESSENCESPEED, 50);
}

static s32b tempo_essence_of_speed_bonus()
{
	return 5 + get_level_s(ESSENCESPEED, 20);
}

casting_result tempo_essence_of_speed(int item)
{
	if (p_ptr->fast == 0)
	{
		return cast(set_fast(randint(10) + tempo_essence_of_speed_base_duration(),
				     tempo_essence_of_speed_bonus()));
	}
	return NO_CAST;
}

char *tempo_essence_of_speed_info()
{
	static char buf[128];
	sprintf(buf,
		"dur " FMTs32b "+d10 speed " FMTs32b,
		tempo_essence_of_speed_base_duration(),
		tempo_essence_of_speed_bonus());
	return buf;
}

static s32b tempo_banishment_power()
{
	return 40 + get_level_s(BANISHMENT, 160);
}

casting_result tempo_banishment(int item)
{
	casting_result result = NO_CAST;
	s32b pwr = tempo_banishment_power();

	result = cplus(result, project_hack(GF_AWAY_ALL, pwr));

	if (get_level_s(BANISHMENT, 50) >= 15)
	{
		result = cplus(result,
			       project_hack(GF_STASIS, 20 + get_level_s(BANISHMENT, 120)));
	}
	
	return result;
}

char *tempo_banishment_info()
{
	static char buf[128];
	sprintf(buf,
		"power " FMTs32b,
		tempo_banishment_power());
	return buf;
}

casting_result tulkas_divine_aim(int item)
{
	casting_result result = NO_CAST;
	s32b dur = get_level_s(TULKAS_AIM, 50) + randint(10);

	result = cplus(result, set_strike(dur));
	if (get_level_s(TULKAS_AIM, 50) >= 20)
	{
		result = cplus(result, set_tim_deadly(dur));
	}

	return result;
}

char *tulkas_divine_aim_info()
{
	static char buf[128];
	sprintf(buf,
		"dur " FMTs32b "+d10",
		get_level_s(TULKAS_AIM, 50));
	return buf;
}

casting_result tulkas_wave_of_power(int item)
{
	int dir;

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	return cast(fire_bolt(GF_ATTACK, dir, get_level_s(TULKAS_WAVE, p_ptr->num_blow)));
}

char *tulkas_wave_of_power_info()
{
	static char buf[128];
	sprintf(buf,
		"blows " FMTs32b,
		get_level_s(TULKAS_WAVE, p_ptr->num_blow));
	return buf;
}

casting_result tulkas_whirlwind(int item)
{
	return cast(fire_ball(GF_ATTACK, 0, 1, 1));
}

char *tulkas_whirlwind_info()
{
	return "";
}

static bool_ check_school_is_udun(void *data, s32b school_idx)
{
	int *count = (int *) data;

	if ((school_idx == SCHOOL_UDUN) ||
	    (school_idx == SCHOOL_MELKOR))
	{
		(*count)++;
	}

	/* Keep going */
	return TRUE;
}

/* Return the number of Udun/Melkor spells in a given book */
int udun_in_book(s32b sval, s32b pval)
{
	int count = 0;
	school_book_type *school_book;
	spell_idx_list *spell_idx = NULL;
	struct sglib_spell_idx_list_iterator it;

	random_book_setup(sval, pval);

	/* Get the school book */
	school_book = school_books_at(sval);

	/* Go through spells */
	for (spell_idx = sglib_spell_idx_list_it_init(&it, school_book->spell_idx_list);
	     spell_idx != NULL;
	     spell_idx = sglib_spell_idx_list_it_next(&it))
	{
		spell_type *spell = spell_at(spell_idx->i);
		spell_type_school_foreach(spell, check_school_is_udun, &count);
	}

	return count;
}

int levels_in_book(s32b sval, s32b pval)
{
	int levels = 0;
	school_book_type *school_book;
	spell_idx_list *spell_idx = NULL;
	struct sglib_spell_idx_list_iterator it;

	random_book_setup(sval, pval);

	/* Get the school book */
	school_book = school_books_at(sval);

	/* Parse all spells */
	for (spell_idx = sglib_spell_idx_list_it_init(&it, school_book->spell_idx_list);
	     spell_idx != NULL;
	     spell_idx = sglib_spell_idx_list_it_next(&it))
	{
		s32b s = spell_idx->i;
		spell_type *spell = spell_at(s);

		levels += spell_type_skill_level(spell);
	}

	return levels;
}

static bool_ udun_object_is_drainable(object_type *o_ptr)
{
	return ((o_ptr->tval == TV_WAND) ||
		(o_ptr->tval == TV_ROD_MAIN) ||
		(o_ptr->tval == TV_STAFF));
}

casting_result udun_drain(int ignored)
{
	int item;
	object_type *o_ptr = NULL;

	/* Ask for an item */
	item_tester_hook = udun_object_is_drainable;
	if (!get_item(&item,
		      "What item to drain?",
		      "You have nothing you can drain",
		      USE_INVEN))
	{
		return NO_CAST;
	}

	/* Drain */

	/* get the item */
	o_ptr = get_object(item);

	switch (o_ptr->tval)
	{
	case TV_STAFF:
	case TV_WAND:
	{
		object_kind *k_ptr = &k_info[o_ptr->k_idx];

		/* Generate mana */
		increase_mana(o_ptr->pval * k_ptr->level * o_ptr->number);

		/* Destroy it */
		inc_stack_size(item, -99);

		break;
	}

	case TV_ROD_MAIN:
	{
		/* Generate mana */
		increase_mana(o_ptr->timeout);

		/* Drain it */
		o_ptr->timeout = 0;

		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= PN_COMBINE | PN_REORDER;
		p_ptr->window |= PW_INVEN | PW_EQUIP | PW_PLAYER;
		break;
	}

	default:
		assert(FALSE);
	}

	return CAST_OBVIOUS;
}

char *udun_drain_info()
{
	return "";
}

casting_result udun_genocide(int item)
{
	if (get_level_s(GENOCIDE, 50) < 10)
	{
		genocide(TRUE);
	}
	else
	{
		if (get_check("Genocide all monsters near you? "))
		{
			mass_genocide(TRUE);
		}
		else
		{
			genocide(TRUE);
		}
	}

	return CAST_OBVIOUS;
}

char *udun_genocide_info()
{
	return "";
}

static int udun_wraithform_base_duration()
{
	return 20 + get_level_s(WRAITHFORM, 40);
}

casting_result udun_wraithform(int item)
{
	return cast(set_shadow(randint(30) + udun_wraithform_base_duration()));
}

char *udun_wraithform_info()
{
	static char buf[128];
	sprintf(buf,
		"dur %d+d30",
		udun_wraithform_base_duration());
	return buf;
}

static int udun_flame_of_udun_base_duration()
{
	return 5 + get_level_s(FLAMEOFUDUN, 30);
}

casting_result udun_flame_of_udun(int item)
{
	return cast(set_mimic(randint(15) + udun_flame_of_udun_base_duration(),
			      resolve_mimic_name("Balrog"),
			      get_level_s(FLAMEOFUDUN, 50)));
}

char *udun_flame_of_udun_info()
{
	static char buf[128];
	sprintf(buf,
		"dur %d+d15",
		udun_flame_of_udun_base_duration());
	return buf;
}

static int tidal_wave_damage()
{
	return 40 + get_level_s(TIDALWAVE, 200);
}

static int tidal_wave_duration()
{
	return 6 + get_level_s(TIDALWAVE, 10);
}

casting_result water_tidal_wave(int item)
{
	fire_wave(GF_WAVE,
		  0,
		  tidal_wave_damage(),
		  0,
		  tidal_wave_duration(),
		  EFF_WAVE);
	return CAST_OBVIOUS;
}

char *water_tidal_wave_info()
{
	static char buf[128];
	sprintf(buf,
		"dam %d dur %d",
		tidal_wave_damage(),
		tidal_wave_duration());
	return buf;
}

static int water_ice_storm_damage()
{
	return 80 + get_level_s(ICESTORM, 200);
}

static int water_ice_storm_radius()
{
	return 1 + get_level(ICESTORM, 3, 0);
}

static int water_ice_storm_duration()
{
	return 20 + get_level_s(ICESTORM, 70);
}

casting_result water_ice_storm(int item)
{
	int type = GF_COLD;

	if (get_level_s(ICESTORM, 50) >= 10)
	{
		type = GF_ICE;
	}

	fire_wave(type,
		  0,
		  water_ice_storm_damage(),
		  water_ice_storm_radius(),
		  water_ice_storm_duration(),
		  EFF_STORM);

	return CAST_OBVIOUS;
}

char *water_ice_storm_info()
{
	static char buf[128];
	sprintf(buf,
		"dam %d rad %d dur %d",
		water_ice_storm_damage(),
		water_ice_storm_radius(),
		water_ice_storm_duration());
	return buf;
}

static int water_ent_potion_base_duration()
{
	return 25 + get_level_s(ENTPOTION, 40);;
}

casting_result water_ent_potion(int item)
{
	set_food(PY_FOOD_MAX - 1);
	msg_print("The Ent's Potion fills your stomach.");
	
	if (get_level_s(ENTPOTION, 50) >= 5)
	{
		set_afraid(0);
	}
	if (get_level_s(ENTPOTION, 50) >= 12)
	{
		set_hero(p_ptr->hero + randint(25) + water_ent_potion_base_duration());
	}

	return CAST_OBVIOUS;
}

char *water_ent_potion_info()
{
	if (get_level_s(ENTPOTION, 50) >= 12)
	{
		static char buf[128];
		sprintf(buf,
			"dur %d+d25",
			water_ent_potion_base_duration());
		return buf;
	}
	else
	{
		return "";
	}
}

static int water_vapor_damage()
{
	return 3 + get_level_s(VAPOR, 20);
}

static int water_vapor_radius()
{
	return 3 + get_level(VAPOR, 9, 0);
}

static int water_vapor_duration()
{
	return 5;
}

casting_result water_vapor(int item)
{
	fire_cloud(GF_WATER,
		   0,
		   water_vapor_damage(),
		   water_vapor_radius(),
		   water_vapor_duration());
	return CAST_OBVIOUS;
}

char *water_vapor_info()
{
	static char buf[128];
	sprintf(buf,
		"dam %d rad %d dur %d",
		water_vapor_damage(),
		water_vapor_radius(),
		water_vapor_duration());
	return buf;
}

static void get_geyser_damage(int *dice, int *sides)
{
	assert(dice != NULL);
	assert(sides != NULL);

	*dice = get_level_s(GEYSER, 10);
	*sides = 3 + get_level_s(GEYSER, 35);
}

casting_result water_geyser(int item)
{
	int dir, dice, sides;

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	get_geyser_damage(&dice, &sides);
	return cast(fire_bolt_or_beam(2 * get_level_s(GEYSER, 85),
				      GF_WATER,
				      dir,
				      damroll(dice, sides)));
}

char *water_geyser_info()
{
	static char buf[128];
	int dice, sides;

	get_geyser_damage(&dice, &sides);

	sprintf(buf,
		"dam %dd%d",
		dice,
		sides);
	return buf;
}

static int charm_animal_power()
{
	return 10 + get_level_s(YAVANNA_CHARM_ANIMAL, 170);
}

static int charm_animal_radius()
{
	return get_level_s(YAVANNA_CHARM_ANIMAL, 2);
}

casting_result yavanna_charm_animal(int item)
{
	int dir;

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	return cast(fire_ball(GF_CONTROL_ANIMAL,
			      dir,
			      charm_animal_power(),
			      charm_animal_radius()));
}

char *yavanna_charm_animal_info()
{
	static char buf[128];
	sprintf(buf,
		"power %d rad %d",
		charm_animal_power(),
		charm_animal_radius());
	return buf;
}

static int yavanna_grow_grass_radius()
{
	return get_level_s(YAVANNA_GROW_GRASS, 4);
}

casting_result yavanna_grow_grass(int item)
{
	grow_grass(yavanna_grow_grass_radius());
	return CAST_OBVIOUS;
}

char *yavanna_grow_grass_info()
{
	static char buf[128];
	sprintf(buf,
		"rad %d",
		yavanna_grow_grass_radius());
	return buf;
}

static int tree_roots_duration()
{
	return 10 + get_level_s(YAVANNA_TREE_ROOTS, 30);
}

static int tree_roots_ac()
{
	return 10 + get_level_s(YAVANNA_TREE_ROOTS, 60);
}

static int tree_roots_damage()
{
	return 10 + get_level_s(YAVANNA_TREE_ROOTS, 20);
}

casting_result yavanna_tree_roots(int item)
{
	return cast(set_roots(tree_roots_duration(),
			      tree_roots_ac(),
			      tree_roots_damage()));
}

char *yavanna_tree_roots_info()
{
	static char buf[128];
	sprintf(buf,
		"dur %d AC %d dam %d",
		tree_roots_duration(),
		tree_roots_ac(),
		tree_roots_damage());
	return buf;
}

static int water_bite_base_duration()
{
	return 30 + get_level_s(YAVANNA_WATER_BITE, 150);
}

static int water_bite_damage()
{
	return 10 + get_level_s(YAVANNA_WATER_BITE, 50);
}

casting_result yavanna_water_bite(int item)
{
	int rad = 0;

	if (get_level_s(YAVANNA_WATER_BITE, 50) >= 25)
	{
		rad = 1;
	}

	return cast(set_project(randint(30) + water_bite_base_duration(),
				GF_WATER,
				water_bite_damage(),
				rad,
				PROJECT_STOP | PROJECT_KILL));
}

char *yavanna_water_bite_info()
{
	static char buf[128];
	sprintf(buf,
		"dur %d+d30 dam %d/blow",
		water_bite_base_duration(),
		water_bite_damage());
	return buf;
}

static int uproot_mlevel()
{
	return 30 + get_level_s(YAVANNA_UPROOT, 70);
}

casting_result yavanna_uproot(int item)
{
	int dir, x, y;
	cave_type *c_ptr;

	if (!get_rep_dir(&dir))
	{
		return NO_CAST;
	}

	y = ddy[dir];
	x = ddx[dir];

	y += p_ptr->py;
	x += p_ptr->px;

	c_ptr = &cave[y][x];

	if (c_ptr->feat == FEAT_TREES)
	{
		s16b m_idx;

		cave_set_feat(y, x, FEAT_GRASS);

		/* Summon it */
		find_position(y, x, &y, &x);
		m_idx = place_monster_one(y, x, test_monster_name("Ent"), 0, FALSE, MSTATUS_FRIEND);

		/* level it */
		if (m_idx != 0)
		{
			monster_set_level(m_idx, uproot_mlevel());
		}

		msg_print("The tree awakes!");
		return CAST_OBVIOUS;
	}
	else
	{
		msg_print("There is no tree there.");
		return NO_CAST;
	}
}

char *yavanna_uproot_info()
{
	static char buf[128];
	sprintf(buf,
		"lev %d",
		uproot_mlevel());
	return buf;
}

static int nature_grow_trees_radius()
{
	return 2 + get_level_s(GROWTREE, 7);
}

casting_result nature_grow_trees(int item)
{
	grow_trees(nature_grow_trees_radius());
	return CAST_OBVIOUS;
}

char *nature_grow_trees_info()
{
	static char buf[128];
	sprintf(buf,
		"rad %d",
		nature_grow_trees_radius());
	return buf;
}

static int nature_healing_percentage()
{
	return 15 + get_level_s(HEALING, 35);
}

static int nature_healing_hp()
{
	return p_ptr->mhp * nature_healing_percentage() / 100;
}

casting_result nature_healing(int item)
{
	return cast(hp_player(nature_healing_hp()));
}

char *nature_healing_info()
{
	static char buf[128];
	sprintf(buf,
		"heal %d%% = %dhp",
		nature_healing_percentage(),
		nature_healing_hp());
	return buf;
}

casting_result nature_recovery(int item)
{
	casting_result result = NO_CAST;

	result = cplus(result, set_poisoned(p_ptr->poisoned / 2));
	if (get_level_s(RECOVERY, 50) >= 5)
	{
		result = cplus(result, set_poisoned(0));
		result = cplus(result, set_cut(0));
	}
	if (get_level_s(RECOVERY, 50) >= 10)
	{
		result = cplus(result, do_res_stat(A_STR, TRUE));
		result = cplus(result, do_res_stat(A_CON, TRUE));
		result = cplus(result, do_res_stat(A_DEX, TRUE));
		result = cplus(result, do_res_stat(A_WIS, TRUE));
		result = cplus(result, do_res_stat(A_INT, TRUE));
		result = cplus(result, do_res_stat(A_CHR, TRUE));
	}
	if (get_level_s(RECOVERY, 50) >= 15)
	{
		result = cplus(result, restore_level());
	}

	return result;
}

char *nature_recovery_info()
{
	return "";
}

static int regeneration_base_duration()
{
	return 5 + get_level_s(REGENERATION, 50);
}

static int regeneration_power()
{
	return 300 + get_level_s(REGENERATION, 700);
}

casting_result nature_regeneration(int item)
{
	if (p_ptr->tim_regen == 0)
	{
		return cast(set_tim_regen(randint(10) + regeneration_base_duration(),
					  regeneration_power()));
	}
	return NO_CAST;
}

char *nature_regeneration_info()
{
	static char buf[128];
	sprintf(buf,
		"dur %d+d10 power %d",
		regeneration_base_duration(),
		regeneration_power());
	return buf;
}

static int summon_animal_level()
{
	return 25 + get_level_s(SUMMONANNIMAL, 50);
}

casting_result nature_summon_animal(int item)
{
	summon_specific_level = summon_animal_level();
	return cast(summon_specific_friendly(p_ptr->py,
					     p_ptr->px,
					     dun_level,
					     SUMMON_ANIMAL,
					     TRUE));
}

char *nature_summon_animal_info()
{
	static char buf[128];
	sprintf(buf,
		"level %d",
		summon_animal_level());
	return buf;
}

casting_result nature_grow_athelas(int item)
{
        if (p_ptr->black_breath)
	{
		msg_print("The hold of the Black Breath on you is broken!");
		p_ptr->black_breath = FALSE;
		return CAST_OBVIOUS;
	}

	return CAST_HIDDEN;
}

char *nature_grow_athelas_info()
{
	return "";
}

static int device_heal_monster_hp()
{
	return 20 + get_level_s(DEVICE_HEAL_MONSTER, 380);
}

casting_result device_heal_monster(int item)
{
	int dir;

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	return cast(fire_ball(GF_OLD_HEAL, dir, device_heal_monster_hp(), 0));
}

char  *device_heal_monster_info()
{
	static char buf[128];
	sprintf(buf,
		"heal %d",
		device_heal_monster_hp());
	return buf;
}

casting_result device_haste_monster(int item)
{
	int dir;

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	return cast(fire_ball(GF_OLD_SPEED, dir, 1, 0));
}

char  *device_haste_monster_info()
{
	return "speed +10";
}

casting_result device_wish(int item)
{
	make_wish();
	return CAST_OBVIOUS;
}

char  *device_wish_info()
{
	return "";
}

casting_result device_summon_monster(int item)
{
	casting_result result = NO_CAST;
	int i;

	for (i = 0; i < 4 + get_level_s(DEVICE_SUMMON, 30); i++)
	{
		result = cplus(result, summon_specific(p_ptr->py, p_ptr->px, dun_level, 0));
	}

	return result;
}

char  *device_summon_monster_info()
{
	return "";
}

static int device_mana_pct()
{
	return 20 + get_level_s(DEVICE_MANA, 50);
}

casting_result device_mana(int item)
{
	increase_mana((p_ptr->msp * device_mana_pct()) / 100);
	return CAST_OBVIOUS;
}

char  *device_mana_info()
{
	static char buf[128];
	sprintf(buf,
		"restore %d%%",
		device_mana_pct());
	return buf;
}

casting_result device_nothing(int item)
{
	return CAST_HIDDEN;
}

char  *device_nothing_info()
{
	return "";
}

casting_result device_lebohaum(int item)
{
	msg_print("You hear a little song in your head in some unknown tongue:");
	msg_print("'Avec le casque Lebohaum y a jamais d'anicroches, je parcours les dongeons,");
	msg_print("j'en prend plein la caboche. Avec le casque Lebohaum, tout ces monstres a la");
	msg_print("con, je leur met bien profond: c'est moi le maitre du dongeon!'");
	return CAST_OBVIOUS;
}

char  *device_lebohaum_info()
{
	return "";
}

casting_result device_maggot(int item)
{
	int dir;

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	return cast(fire_ball(GF_TURN_ALL, dir, 40, 2));
}

char  *device_maggot_info()
{
	return "power 40 rad 2";
}

static int holy_fire_damage()
{
	return 50 + get_level_s(DEVICE_HOLY_FIRE, 300);
}

casting_result device_holy_fire(int item)
{
	return cast(project_hack(GF_HOLY_FIRE, holy_fire_damage()));
}

char  *device_holy_fire_info()
{
	static char buf[128];
	sprintf(buf,
		"dam %d",
		holy_fire_damage());
	return buf;
}

static int get_eternal_artifact_idx(object_type *o_ptr)
{
	if ((o_ptr->tval == TV_SWORD) && (o_ptr->sval == SV_LONG_SWORD)) {
		return 147;
	} else if ((o_ptr->tval == TV_MSTAFF) && (o_ptr->sval == SV_MSTAFF)) {
		return 127;
	} else if ((o_ptr->tval == TV_BOW) && (o_ptr->sval == SV_HEAVY_XBOW)) {
		return 152;
	} else if ((o_ptr->tval == TV_DRAG_ARMOR) && (o_ptr->sval == SV_DRAGON_POWER)) {
		return 17;
	}

	if (game_module_idx == MODULE_THEME)
	{
		if ((o_ptr->tval == TV_HAFTED) && (o_ptr->sval == SV_LUCERN_HAMMER)) {
			return 241;
		} else if ((o_ptr->tval == TV_POLEARM) && (o_ptr->sval == SV_TRIDENT)) {
			return 242;
		} else if ((o_ptr->tval == TV_AXE) && (o_ptr->sval == SV_BROAD_AXE)) {
			return 243;
		} else if ((o_ptr->tval == TV_BOW) && (o_ptr->sval == SV_LONG_BOW)) {
			return 245;
		} else if ((o_ptr->tval == TV_BOOMERANG) && (o_ptr->sval == SV_BOOM_METAL)) {
			return 247;
		} else if ((o_ptr->tval == TV_BOW) && (o_ptr->sval == SV_SLING)) {
			return 246;
		} else if ((o_ptr->tval == TV_SWORD) && (o_ptr->sval == SV_RAPIER)) {
			return 244;
		} else if ((o_ptr->tval == TV_AMULET) && (o_ptr->sval == SV_AMULET_SPELL)) {
			return 248;
		}
	}

	/* Not usable */
	return -1;
}

static bool_ eternal_flame_item_tester_hook(object_type *o_ptr)
{
	if ((o_ptr->name1 > 0) ||
	    (o_ptr->name2 > 0))
	{
		return FALSE;
	}

	return (get_eternal_artifact_idx(o_ptr) >= 0);
}

casting_result device_eternal_flame(int flame_item)
{
	int item;
	object_type *o_ptr = NULL;
	int artifact_idx = -1;

	item_tester_hook = eternal_flame_item_tester_hook;
	if (!get_item(&item,
		      "Which object do you want to imbue?",
		      "You have no objects to imbue.",
		      USE_INVEN))
	{
		return NO_CAST;
	}

	/* Get the artifact idx */
	artifact_idx = get_eternal_artifact_idx(o_ptr);
	assert(artifact_idx >= 0);

	/* Forge the item */
	o_ptr = get_object(item);
	o_ptr->name1 = artifact_idx;

	apply_magic(o_ptr, -1, TRUE, TRUE, TRUE);

	o_ptr->found = OBJ_FOUND_SELFMADE;

	inven_item_increase(flame_item, -1);
	inven_item_describe(flame_item);
	inven_item_optimize(flame_item);

	return CAST_OBVIOUS;
}

char  *device_eternal_flame_info()
{
	return "";
}

casting_result device_durandil(int item)
{
	msg_print("You hear a little song in your head in some unknown tongue:");
	msg_print("'Les epees Durandils sont forgees dans les mines par des nains.");
	msg_print("Avec ca c'est facile de tuer un troll avec une seule main. Pas besoin");
	msg_print("de super entrainement nis de niveau 28. Quand tu sors l'instrument");
	msg_print("c'est l'ennemi qui prend la fuite! Avec ton epee Durandil quand tu");
	msg_print("parcours les chemins, tu massacre sans peine les brigands et les gobelins,");
	msg_print("les rats geants, les ogres mutants, les zombies et les liches, tu les");
	msg_print("decoupe en tranches comme si c'etait des parts de quiches.");
	msg_print("Les epees Durandil! Les epees Durandil!");
	msg_print("Quand tu la sort dans un dongeon au moins t'as pas l'air debile.");
	msg_print("C'est l'arme des bourins qui savent etre subtils.");
	msg_print("Ne partez pas a l'aventure sans votre epee Durandil!'");
	return CAST_OBVIOUS;
}

char  *device_durandil_info()
{
	return "";
}

casting_result device_thunderlords(int item)
{
	switch (game_module_idx)
	{
	case MODULE_TOME:
		{
			if (dun_level > 0)
			{
				msg_print("As you blow the horn a thunderlord pops out of nowhere and grabs you.");
				recall_player(0, 1);
			}
			else
			{
				msg_print("You cannot use it there.");
			}
			return CAST_OBVIOUS;
		}

	case MODULE_THEME:
		{
			if (dun_level > 0)
			{
				msg_print("As you blow the horn, an Eagle of Manwe appears overhead.");
				recall_player(0, 1);
			}
			else
			{
				msg_print("You cannot use it there.");
			}
			return CAST_OBVIOUS;
		}

	default:
		assert(FALSE);
		return NO_CAST;
	}
}

char  *device_thunderlords_info()
{
	return "";
}

casting_result device_radagast(int item)
{
	cmsg_print(TERM_GREEN, "The staff's power cleanses you completely!");
	remove_all_curse();
	do_res_stat(A_STR, TRUE);
	do_res_stat(A_CON, TRUE);
	do_res_stat(A_DEX, TRUE);
	do_res_stat(A_WIS, TRUE);
	do_res_stat(A_INT, TRUE);
	do_res_stat(A_CHR, TRUE);
	restore_level();
	// clean_corruptions(); TODO: Do we want to implement this?
	hp_player(5000);
	heal_insanity(5000);
	set_poisoned(0);
	set_blind(0);
	set_confused(0);
	set_image(0);
	set_stun(0);
	set_cut(0);
	set_parasite(0, 0);

	if (p_ptr->black_breath)
	{
		msg_print("The hold of the Black Breath on you is broken!");
	}
	p_ptr->black_breath = FALSE;

	p_ptr->update |= PU_BONUS;
	p_ptr->window |= PW_PLAYER;

	return CAST_OBVIOUS;
}

char  *device_radagast_info()
{
	return "";
}

casting_result device_valaroma(int item)
{
	int power = 5 * p_ptr->lev;
	banish_evil(power);
	return CAST_HIDDEN;
}

char  *device_valaroma_info()
{
	return "";
}

void static start_lasting_spell(int spl)
{
	p_ptr->music_extra = -spl;
}

casting_result music_stop_singing_spell(int item)
{
	start_lasting_spell(0);
	return CAST_OBVIOUS;
}

char *music_stop_singing_info()
{
	return "";
}

static int holding_pattern_power()
{
	return 10 + get_level_s(MUSIC_HOLD, 100);
}

int music_holding_pattern_lasting()
{
	project_hack(GF_OLD_SLOW, holding_pattern_power());
	return get_mana(MUSIC_HOLD);
}

casting_result music_holding_pattern_spell(int item)
{
	start_lasting_spell(MUSIC_HOLD);
	return CAST_OBVIOUS;
}

char *music_holding_pattern_info()
{
	static char buf[128];
	sprintf(buf,
		"power %d",
		holding_pattern_power());
	return buf;
}

static int illusion_pattern_power()
{
	return 10 + get_level_s(MUSIC_CONF, 100);
}

int music_illusion_pattern_lasting()
{
	project_hack(GF_OLD_CONF, illusion_pattern_power());
	return get_mana(MUSIC_CONF);
}

casting_result music_illusion_pattern_spell(int item)
{
	start_lasting_spell(MUSIC_CONF);
	return CAST_OBVIOUS;
}

char *music_illusion_pattern_info()
{
	static char buf[128];
	sprintf(buf,
		"power %d",
		illusion_pattern_power());
	return buf;
}

static int stun_pattern_power()
{
	return 10 + get_level_s(MUSIC_STUN, 90);
}

int music_stun_pattern_lasting()
{
	project_hack(GF_STUN, stun_pattern_power());
	return get_mana(MUSIC_STUN);
}

casting_result music_stun_pattern_spell(int item)
{
	start_lasting_spell(MUSIC_STUN);
	return CAST_OBVIOUS;
}

char *music_stun_pattern_info()
{
	static char buf[128];
	sprintf(buf,
		"power %d",
		stun_pattern_power());
	return buf;
}

int music_song_of_the_sun_lasting()
{
	set_lite(5);
	return 1;
}

casting_result music_song_of_the_sun_spell(int item)
{
	start_lasting_spell(MUSIC_LITE);
	return CAST_OBVIOUS;
}

char *music_song_of_the_sun_info()
{
	return "";
}

int flow_of_life_hp()
{
	return 7 + get_level_s(MUSIC_HEAL, 100);
}

int music_flow_of_life_lasting()
{
	hp_player(flow_of_life_hp());
	return get_mana(MUSIC_HEAL);
}

casting_result music_flow_of_life_spell(int item)
{
	start_lasting_spell(MUSIC_HEAL);
	return CAST_OBVIOUS;
}

char *music_flow_of_life_info()
{
	static char buf[128];
	sprintf(buf,
		"heal %d/turn",
		flow_of_life_hp());
	return buf;
}

int music_heroic_ballad_lasting()
{
	set_hero(5);
	if (get_level_s(MUSIC_HERO, 50) >= 10)
	{
		set_shero(5);
	}
	if (get_level_s(MUSIC_HERO, 50) >= 20)
	{
		set_strike(5);
	}
	if (get_level_s(MUSIC_HERO, 50) >= 25)
	{
		set_oppose_cc(5);
	}
	return get_mana(MUSIC_HERO);
}

casting_result music_heroic_ballad_spell(int item)
{
	start_lasting_spell(MUSIC_HERO);
	return CAST_OBVIOUS;
}

char *music_heroic_ballad_info()
{
	return "";
}

int music_hobbit_melodies_lasting()
{
	set_shield(5, 10 + get_level_s(MUSIC_TIME, 50), 0, 0, 0);
	if (get_level_s(MUSIC_TIME, 50) >= 15)
	{
		set_fast(5, 7 + get_level_s(MUSIC_TIME, 10));
	}
	return get_mana(MUSIC_TIME);
}

casting_result music_hobbit_melodies_spell(int item)
{
	start_lasting_spell(MUSIC_TIME);
	return CAST_OBVIOUS;
}

char *music_hobbit_melodies_info()
{
	static char buf[128];
	if (get_level_s(MUSIC_TIME, 50) >= 15)
	{
		sprintf(buf, "AC " FMTs32b " speed " FMTs32b,
			10 + get_level_s(MUSIC_TIME, 50),
			7 + get_level_s(MUSIC_TIME, 10));
	}
	else
	{
		sprintf(buf, "AC " FMTs32b,
			10 + get_level_s(MUSIC_TIME, 50));
	}
	return buf;
}

int music_clairaudience_lasting()
{
	set_tim_esp(5);
	if (get_level_s(MUSIC_MIND, 50) >= 10)
	{
		fire_ball(GF_IDENTIFY, 0, 1, 1 + get_level(MUSIC_MIND, 3, 0));
	}
	return get_mana(MUSIC_MIND);
}

casting_result music_clairaudience_spell(int item)
{
	start_lasting_spell(MUSIC_MIND);
	return CAST_OBVIOUS;
}

char *music_clairaudience_info()
{
	static char buf[128];

	if (get_level_s(MUSIC_MIND, 50) >= 10)
	{
		sprintf(buf, "rad " FMTs32b,
			1 + get_level(MUSIC_MIND, 3, 0));
		return buf;
	}
	else
	{
		return "";
	}
}

casting_result music_blow_spell(int item)
{
	fire_ball(GF_SOUND,
		  0,
		  damroll(2 + get_level(MUSIC_BLOW, 10, 0), 4 + get_level(MUSIC_BLOW, 40, 0)),
		  1 + get_level(MUSIC_BLOW, 12, 0));
	return CAST_OBVIOUS;
}

char *music_blow_info()
{
	static char buf[128];
	sprintf(buf,
		"dam " FMTs32b "d" FMTs32b " rad " FMTs32b,
		2 + get_level(MUSIC_BLOW, 10, 0),
		4 + get_level(MUSIC_BLOW, 40, 0),
		1 + get_level(MUSIC_BLOW, 12, 0));
	return buf;
}

casting_result music_gush_of_wind_spell(int item)
{
	fire_ball(GF_AWAY_ALL,
		  0,
		  10 + get_level(MUSIC_BLOW, 40, 0),
		  1 + get_level(MUSIC_BLOW, 12, 0));
	return CAST_OBVIOUS;
}

char *music_gush_of_wind_info()
{
	static char buf[128];
	sprintf(buf,
		"dist " FMTs32b " rad " FMTs32b,
		10 + get_level(MUSIC_BLOW, 40, 0),
		1 + get_level(MUSIC_BLOW, 12, 0));
	return buf;
}

casting_result music_horns_of_ylmir_spell(int item)
{
	earthquake(p_ptr->py, p_ptr->px, 2 + get_level_s(SHAKE, 10));
	return CAST_OBVIOUS;
}

char *music_horns_of_ylmir_info()
{
	static char buf[128];
	sprintf(buf,
		"rad " FMTs32b,
		2 + get_level_s(SHAKE, 10));
	return buf;
}

casting_result music_ambarkanta_spell(int item)
{
	alter_reality();
	return CAST_OBVIOUS;
}

char *music_ambarkanta_info()
{
	return "";
}

casting_result aule_firebrand_spell(int item)
{
	int rad = 0;
	int type = GF_FIRE;
	s32b level = get_level_s(AULE_FIREBRAND, 50);

	if (level > 30)
	{
		type = GF_HOLY_FIRE;
	}

	if (level >= 15)
	{
		rad = 1;
	}

	return cast(set_project(level + randint(20),
				type,
				4 + level,
				rad,
				PROJECT_STOP | PROJECT_KILL));
}

char  *aule_firebrand_info()
{
	s32b level = get_level_s(AULE_FIREBRAND, 50);
	static char buf[128];

	sprintf(buf,
		"dur " FMTs32b "+d20 dam " FMTs32b "/blow",
		level,
		4 + level);
	return buf;
}

static bool_ aule_enchant_weapon_item_tester(object_type *o_ptr)
{
	if (o_ptr->name1 > 0)
	{
		return FALSE;
	}

	switch (o_ptr->tval)
	{
	case TV_MSTAFF:
	case TV_BOW:
	case TV_HAFTED:
	case TV_POLEARM:
	case TV_SWORD:
	case TV_AXE:
		return TRUE;

	default:
		return FALSE;
	}
}

casting_result aule_enchant_weapon_spell(int ignored)
{
	s32b level = get_level_s(AULE_ENCHANT_WEAPON, 50);
	s16b num_h, num_d, num_p;
	int item;
	object_type *o_ptr = NULL;

	num_h = 1 + randint(level/12);
	num_d = 0;
	num_p = 0;

	if (level >= 5)
	{
		num_d = 1 + randint(level/12);
	}
	if (level >= 45)
	{
		num_p = 1;
	}

	item_tester_hook = aule_enchant_weapon_item_tester;
	if (!get_item(&item,
		      "Which object do you want to enchant?",
		      "You have no objects to enchant.",
		      USE_INVEN))
	{
		return NO_CAST;
	}

	o_ptr = get_object(item);

	o_ptr->to_h = o_ptr->to_h + num_h;
	o_ptr->to_d = o_ptr->to_d + num_d;
	o_ptr->pval = o_ptr->pval + num_p;

	return CAST_OBVIOUS;
}

char  *aule_enchant_weapon_info()
{
	static char buf[128];
	sprintf(buf,
		"tries " FMTs32b,
		1 + get_level_s(AULE_ENCHANT_WEAPON, 50)/12);
	return buf;
}

bool_ aule_enchant_armor_item_tester(object_type *o_ptr)
{
	if (o_ptr->name1 > 0)
	{
		return FALSE;
	}

	switch (o_ptr->tval)
	{
	case TV_BOOTS:
	case TV_GLOVES:
	case TV_HELM:
	case TV_CROWN:
	case TV_SHIELD:
	case TV_CLOAK:
	case TV_SOFT_ARMOR:
	case TV_HARD_ARMOR:
	case TV_DRAG_ARMOR:
		return TRUE;

	default:
		return FALSE;
	}
}

casting_result aule_enchant_armour_spell(int ignored)
{
	s32b level = get_level_s(AULE_ENCHANT_ARMOUR, 50);
	s16b num_h, num_d, num_a, num_p;
	int item;
	object_type *o_ptr = NULL;

	num_a = 1 + randint(level/10);
	num_h = 0;
	num_d = 0;
	num_p = 0;
	if (level >= 20)
	{
		num_h = 1;
		num_d = 1;
	}
	if (level >= 40)
	{
		num_p = 1;
	}

	item_tester_hook = aule_enchant_armor_item_tester;
	if (!get_item(&item,
		      "Which object do you want to enchant?",
		      "You have no objects to enchant.",
		      USE_INVEN))
	{
		return NO_CAST;
	}

	o_ptr = get_object(item);

	o_ptr->to_h = o_ptr->to_h + num_h;
	o_ptr->to_d = o_ptr->to_d + num_d;
	o_ptr->pval = o_ptr->pval + num_p;
	o_ptr->to_a = o_ptr->to_a + num_a;

	return CAST_OBVIOUS;
}

char  *aule_enchant_armour_info()
{
	static char buf[128];
	sprintf(buf,
		"tries " FMTs32b,
		1 + get_level_s(AULE_ENCHANT_ARMOUR, 50)/10);
	return buf;
}

casting_result aule_child_spell(int item)
{
	int y, x;
	s16b m_idx;

	find_position(p_ptr->py, p_ptr->px, &y, &x);
	m_idx = place_monster_one(y, x, test_monster_name("Dwarven warrior"),
				  0, FALSE, MSTATUS_FRIEND);
 
	if (m_idx)
	{
		monster_set_level(m_idx, 20 + get_level(AULE_CHILD, 70, 0));
		return CAST_OBVIOUS;
	}
	else
	{
		return NO_CAST;
	}
}

char  *aule_child_info()
{
	static char buf[128];
	sprintf(buf,
		"level " FMTs32b,
		20 + get_level_s(AULE_CHILD, 70));
	return buf;
}

static int tears_of_luthien_hp()
{
	return 10 * get_level_s(MANDOS_TEARS_LUTHIEN, 30);
}

casting_result mandos_tears_of_luthien_spell(int item)
{
	casting_result result = NO_CAST;

	result = cplus(result, hp_player(tears_of_luthien_hp()));
	result = cplus(result, set_stun(0));
	result = cplus(result, set_cut(0));
	result = cplus(result, set_afraid(0));

	return result;
}

char  *mandos_tears_of_luthien_info()
{
	static char buf[128];
	sprintf(buf,
		"heals %d",
		tears_of_luthien_hp());
	return buf;
}

casting_result mandos_spirit_of_the_feanturi_spell(int item)
{
	casting_result result = NO_CAST;
	s32b level = get_level_s(MANDOS_SPIRIT_FEANTURI, 50);

	result = cplus(result, set_afraid(0));
	result = cplus(result, set_confused(0));

	if (level >= 20)
	{
                result = cplus(result, do_res_stat(A_WIS, TRUE));
                result = cplus(result, do_res_stat(A_INT, TRUE));
	}
            
	if (level >= 30)
	{
                result = cplus(result, set_image(0));
                result = cplus(result, heal_insanity(p_ptr->msane * level / 100)); 
	}
            
	return result;
}

char  *mandos_spirit_of_the_feanturi_info()
{
	static char buf[128];
	s32b level = get_level_s(MANDOS_SPIRIT_FEANTURI, 50) ;
	if (level >= 20)
	{
		sprintf(buf, "heals " FMTs32b "%%", level);
		return buf;
	}
	else
	{
		return "";
	}
}

static int tale_of_doom_duration()
{
	return 5 + get_level_s(MANDOS_TALE_DOOM,10);
}

casting_result mandos_tale_of_doom_spell(int item)
{
	return cast(set_tim_precognition(tale_of_doom_duration()));
}

char  *mandos_tale_of_doom_info()
{
	static char buf[128];
	sprintf(buf,
		"dur %d",
		tale_of_doom_duration());
	return buf;
}

int call_to_the_halls_mlev()
{
	return 20 + get_level(MANDOS_CALL_HALLS, 70, 0);
}

casting_result mandos_call_to_the_halls_spell(int item)
{
#define N_SUMMONS 2
	int y, x;
	s16b m_idx, r_idx;
	s16b summons[N_SUMMONS] = {
		test_monster_name("Experienced spirit"),
		test_monster_name("Wise spirit"),
	};

	r_idx = summons[rand_int(N_SUMMONS)];
	assert(r_idx >= 0);

	find_position(p_ptr->py, p_ptr->px, &y, &x);
	m_idx = place_monster_one(y, x, r_idx, 0, FALSE, MSTATUS_FRIEND);
	if (m_idx)
	{
		monster_set_level(m_idx, call_to_the_halls_mlev());
		return CAST_OBVIOUS;
	}
	return NO_CAST;
#undef N_SUMMONS
}

char  *mandos_call_to_the_halls_info()
{
	static char buf[128];
	sprintf(buf,
		"level %d",
		call_to_the_halls_mlev());
	return buf;
}

static void get_belegaer_damage(int *dice, int *sides)
{
	*dice = get_level_s(ULMO_BELEGAER, 10);
	*sides = 3 + get_level_s(ULMO_BELEGAER, 35);
}

casting_result ulmo_song_of_belegaer_spell(int item)
{
	int dir, dice, sides;

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	get_belegaer_damage(&dice, &sides);
	return cast(fire_bolt_or_beam(2 * get_level_s(ULMO_BELEGAER, 85),
				      GF_WATER,
				      dir,
				      damroll(dice, sides)));
}

char *ulmo_song_of_belegaer_info()
{
	static char buf[128];
	int dice, sides;

	get_belegaer_damage(&dice, &sides);
	sprintf(buf,
		"dam %dd%d",
		dice,
		sides);
	return buf;
}

int draught_of_ulmonan_hp()
{
	return 5 * get_level_s(ULMO_DRAUGHT_ULMONAN, 50);
}

casting_result ulmo_draught_of_ulmonan_spell(int item)
{
	casting_result result = NO_CAST;
	s32b level = get_level_s(ULMO_DRAUGHT_ULMONAN, 50);
		
	result = cplus(result, hp_player(draught_of_ulmonan_hp()));

	result = cplus(result, set_poisoned(0));
	result = cplus(result, set_cut(0));
	result = cplus(result, set_stun(0));
	result = cplus(result, set_blind(0));

	if (level >= 10)
	{
		result = cplus(result, do_res_stat(A_STR, TRUE));
		result = cplus(result, do_res_stat(A_CON, TRUE));
		result = cplus(result, do_res_stat(A_DEX, TRUE));
	}

	if (level >= 20)
	{
		result = cplus(result, set_parasite(0, 0));
		result = cplus(result, set_mimic(0, 0, 0));
	}

	return result;
}

char *ulmo_draught_of_ulmonan_info()
{
	static char buf[128];
	sprintf(buf,
		"cure %d",
		draught_of_ulmonan_hp());
	return buf;
}

static int call_of_the_ulumuri_mlev()
{
	return 30 + get_level(ULMO_CALL_ULUMURI, 70, 0);
}

casting_result ulmo_call_of_the_ulumuri_spell(int item)
{
#define N_SUMMONS 2
	int x,y;
	s16b m_idx, r_idx;
	s16b summons[N_SUMMONS] = {
		test_monster_name("Water spirit"),
		test_monster_name("Water elemental")
	};

	r_idx = summons[rand_int(N_SUMMONS)];
	assert(r_idx >= 0);

	find_position(p_ptr->py, p_ptr->px, &y, &x);

	m_idx = place_monster_one(y, x, r_idx, 0, FALSE, MSTATUS_FRIEND);
	if (m_idx)
	{
		monster_set_level(m_idx, call_of_the_ulumuri_mlev());
		return CAST_OBVIOUS;
	}

	return NO_CAST;
#undef N_SUMMONS
}

char *ulmo_call_of_the_ulumuri_info()
{
	static char buf[128];
	sprintf(buf,
		"level %d",
		call_of_the_ulumuri_mlev());
	return buf;
}

static int wrath_of_ulmo_damage()
{
	return 40 + get_level_s(ULMO_WRATH, 150);
}

static int wrath_of_ulmo_duration()
{
	return 10 + get_level_s(ULMO_WRATH, 14);
}

casting_result ulmo_wrath_of_ulmo_spell(int item)
{
	int dir, type = GF_WATER;

	if (get_level_s(ULMO_WRATH, 50) >= 30)
	{
		type = GF_WAVE;
	}

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	fire_wall(type,
		  dir,
		  wrath_of_ulmo_damage(),
		  wrath_of_ulmo_duration());
	return CAST_OBVIOUS;
}

char *ulmo_wrath_of_ulmo_info()
{
	static char buf[128];
	sprintf(buf,
		"dam %d dur %d",
		wrath_of_ulmo_damage(),
		wrath_of_ulmo_duration());
	return buf;
}

static int light_of_valinor_damage()
{
	return 10 + get_level_s(VARDA_LIGHT_VALINOR, 100);
}

static int light_of_valinor_radius()
{
	return 5 + get_level_s(VARDA_LIGHT_VALINOR, 6);
}

casting_result varda_light_of_valinor_spell(int item)
{
	casting_result result = NO_CAST;

	if (get_level_s(VARDA_LIGHT_VALINOR, 50) >= 3)
	{
		result = cplus(result, lite_area(10, 4));
	}
	else
	{
		lite_room(p_ptr->py, p_ptr->px);
		result = CAST_OBVIOUS;
	}

	if (get_level_s(VARDA_LIGHT_VALINOR, 50) >= 15)
	{
		result = cplus(result,
			       fire_ball(GF_LITE,
					 0,
					 light_of_valinor_damage(),
					 light_of_valinor_radius()));
	}

	return result;
}

char *varda_light_of_valinor_info()
{
	static char buf[128];
	if (get_level_s(VARDA_LIGHT_VALINOR, 50) >= 15)
	{
		sprintf(buf,
			"dam %d rad %d",
			light_of_valinor_damage(),
			light_of_valinor_radius());
		return buf;
	}
	else
	{
		return "";
	}
}

casting_result varda_call_of_almaren_spell(int item)
{
	int power = 5 * p_ptr->lev;
	if (get_level_s(VARDA_CALL_ALMAREN, 50) >= 20)
	{
		dispel_evil(power);
	}
	else
	{
		banish_evil(power);
	}
	return CAST_OBVIOUS;
}

char *varda_call_of_almaren_info()
{
	return "";
}

casting_result varda_evenstar_spell(int item)
{
	wiz_lite_extra();
	if (get_level_s(VARDA_EVENSTAR, 50) >= 40)
	{
		identify_pack();
		self_knowledge(NULL);
	}

	return CAST_OBVIOUS;
}

char *varda_evenstar_info()
{
	return "";
}

static int star_kindler_bursts()
{
	return p_ptr->lev / 5;
}

static int star_kindler_damage()
{
	return 20 + get_level_s(VARDA_STARKINDLER, 100);
}

casting_result varda_star_kindler_spell(int item)
{
	int dir, i, n = star_kindler_bursts();

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	for (i = 0; i < n; i++)
	{
		fire_ball(GF_LITE,
			  dir,
			  star_kindler_damage(),
			  10);
	}

	return CAST_OBVIOUS;
}

char *varda_star_kindler_info()
{
	static char buf[128];
	sprintf(buf,
		"dam %d bursts %d rad 10",
		star_kindler_damage(),
		star_kindler_bursts());
	return buf;
}

