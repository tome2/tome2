#include "spells3.hpp"

#include "cave.hpp"
#include "cave_type.hpp"
#include "cmd5.hpp"
#include "feature_flag.hpp"
#include "feature_type.hpp"
#include "game.hpp"
#include "lua_bind.hpp"
#include "mimic.hpp"
#include "monster2.hpp"
#include "monster3.hpp"
#include "monster_race.hpp"
#include "monster_race_flag.hpp"
#include "monster_type.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "object_kind.hpp"
#include "player_type.hpp"
#include "school_book.hpp"
#include "skills.hpp"
#include "spell_type.hpp"
#include "spells1.hpp"
#include "spells2.hpp"
#include "spells4.hpp"
#include "spells5.hpp"
#include "stats.hpp"
#include "tables.hpp"
#include "timer_type.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "xtra2.hpp"
#include "z-rand.hpp"

#include <algorithm>
#include <cassert>
#include <fmt/format.h>
#include <vector>

s32b NOXIOUSCLOUD = -1; /* Identifier */
s32b AIRWINGS = -1; /* Identifier */
s32b INVISIBILITY;
s32b POISONBLOOD;
s32b THUNDERSTORM;
s32b STERILIZE;

s32b BLINK;
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
s32b DEVICE_HOLY_FIRE;
s32b DEVICE_THUNDERLORDS;

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

static void find_position(int y, int x, int *yy, int *xx)
{
	int attempts = 500;

	do
	{
		scatter(yy, xx, y, x, 6);
	}
	while (!(in_bounds(*yy, *xx) && cave_floor_bold(*yy, *xx)) && --attempts);
}

GENERATE_MONSTER_LOOKUP_FN(get_fire_golem, "Fire golem")

// -------------------------------------------------------------

casting_result air_noxious_cloud()
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

std::string air_noxious_cloud_info()
{
	return fmt::format(
		"dam {} rad 3 dur {}",
		7 + get_level_s(NOXIOUSCLOUD, 150),
		5 + get_level_s(NOXIOUSCLOUD, 40));
}

casting_result air_wings_of_winds()
{
	if (get_level_s(AIRWINGS, 50) >= 16)
	{
		if (p_ptr->tim_fly == 0)
		{
			set_tim_fly(randint(10) + 5 + get_level_s(AIRWINGS, 25));
		}
	}
	else
	{
		if (p_ptr->tim_ffall == 0)
		{
			set_tim_ffall(randint(10) + 5 + get_level_s(AIRWINGS, 25));
		}
	}

	return CAST;
}

std::string air_wings_of_winds_info()
{
	return fmt::format("dur {}+d10", 5 + get_level_s(AIRWINGS, 25));
}

casting_result air_invisibility()
{
	if (p_ptr->tim_invisible == 0)
	{
		set_invis(randint(20) + 15 + get_level_s(INVISIBILITY, 50), 20 + get_level_s(INVISIBILITY, 50));
	}

	return CAST;
}

std::string air_invisibility_info()
{
	return fmt::format(
		"dur {}+d20 power {}",
		15 + get_level_s(INVISIBILITY, 50),
		20 + get_level_s(INVISIBILITY, 50));
}

casting_result air_poison_blood()
{
	casting_result result = NO_CAST;

	if (p_ptr->oppose_pois == 0)
	{
		set_oppose_pois(randint(30) + 25 + get_level_s(POISONBLOOD, 25));
		result = CAST;
	}

	if ((p_ptr->tim_poison == 0) &&
	    (get_level_s(POISONBLOOD, 50) >= 15))
	{
		set_poison(randint(30) + 25 + get_level_s(POISONBLOOD, 25));
		result = CAST;
	}

	return result;
}

std::string air_poison_blood_info()
{
	return fmt::format(
		"dur {}+d30",
		25 + get_level_s(POISONBLOOD, 25));
}

casting_result air_thunderstorm()
{
	if (p_ptr->tim_thunder == 0)
	{
		set_tim_thunder(randint(10) + 10 + get_level_s(THUNDERSTORM, 25), 5 + get_level_s(THUNDERSTORM, 10), 10 + get_level_s(THUNDERSTORM, 25));
	}

	return CAST;
}

std::string air_thunderstorm_info()
{
	return fmt::format(
		"dam {}d{} dur {}+d10",
		5 + get_level_s(THUNDERSTORM, 10),
		10 + get_level_s(THUNDERSTORM, 25),
		10 + get_level_s(THUNDERSTORM, 25));
}

casting_result air_sterilize()
{
	set_no_breeders((30) + 20 + get_level_s(STERILIZE, 70));
	return CAST;
}

std::string air_sterilize_info()
{
	return fmt::format(
		"dur {}+d30",
		20 + get_level_s(STERILIZE, 70));
}

casting_result convey_blink()
{
	if (get_level_s(BLINK, 50) >= 30)
	{
		int oy = p_ptr->py;
		int ox = p_ptr->px;

		teleport_player(10 + get_level_s(BLINK, 8));
		create_between_gate(0, oy, ox);
	}
	else
	{
		teleport_player(10 + get_level_s(BLINK, 8));
	}

	return CAST;
}

std::string convey_blink_info()
{
	return fmt::format(
		"distance {}",
		10 + get_level_s(BLINK, 8));
}

casting_result convey_teleport()
{
	p_ptr->energy -= (25 - get_level_s(TELEPORT, 50));
	teleport_player(100 + get_level_s(TELEPORT, 100));
	return CAST;
}

std::string convey_teleport_info()
{
	return fmt::format(
		"distance {}",
		100 + get_level_s(TELEPORT, 100));
}

casting_result convey_teleport_away()
{
	if (get_level_s(TELEAWAY, 50) >= 20)
	{
		project_hack(GF_AWAY_ALL, 100);
	}
	else if (get_level_s(TELEAWAY, 50) >= 10)
	{
		int dir;
		if (!get_aim_dir(&dir))
		{
			return NO_CAST;
		}

		fire_ball(GF_AWAY_ALL, dir, 100, 3 + get_level_s(TELEAWAY, 4));
	}
	else
	{
		int dir;
		if (!get_aim_dir(&dir))
		{
			return NO_CAST;
		}
		teleport_monster(dir);
	}

	return CAST;
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

casting_result convey_recall()
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
	else if (!c_ptr->o_idxs.empty())
	{
		// Set the target
		target_who = -1;
		target_col = x;
		target_row = y;
		// Fetch item
		if (get_level_s(RECALL, 50) >= 15)
		{
			fetch(5, 10 + get_level_s(RECALL, 150), false);
		}
		else
		{
			fetch(5, 10 + get_level_s(RECALL, 150), true);
		}
		return CAST;
	}
	else
	{
		return NO_CAST;
	}
}

std::string convey_recall_info()
{
	int d = recall_get_d();
	int f = recall_get_f();

	return fmt::format(
		"dur {}+d{} weight {}lb",
		f, d, 1 + get_level_s(RECALL, 15));
}

casting_result convey_probability_travel()
{
	set_prob_travel(randint(20) + get_level_s(PROBABILITY_TRAVEL, 60));
	return CAST;
}

std::string convey_probability_travel_info()
{
	return fmt::format(
		"dur {}+d20",
		get_level_s(PROBABILITY_TRAVEL, 60));
}

casting_result demonology_demon_blade()
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

	set_project(
		randint(20) + get_level_s(DEMON_BLADE, 80),
		type,
		4 + get_level_s(DEMON_BLADE, 40),
		rad,
		PROJECT_STOP | PROJECT_KILL);
	return CAST;
}

std::string demonology_demon_blade_info()
{
	return fmt::format(
		"dur {}+d20 dam {}/blow",
		get_level_s(DEMON_BLADE, 80),
		4 + get_level_s(DEMON_BLADE, 40));
}

casting_result demonology_demon_madness()
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

	// Calculate the coordinates of arrival
	{
		// Use the given direction
		int tx = p_ptr->px + (ddx[dir] * 100);
		int ty = p_ptr->py + (ddy[dir] * 100);

		// Hack -- Use an actual "target"
		if ((dir == 5) && target_okay())
		{
			tx = target_col;
			ty = target_row;
		}

		y1 = ty;
		x1 = tx;
	}

	// Calculate the appropriate place
	y2 = p_ptr->py - (y1 - p_ptr->py);
	x2 = p_ptr->px - (x1 - p_ptr->px);

	project(
		0, 1 + get_level(DEMON_MADNESS, 4),
		y1, x1,
		20 + get_level_s(DEMON_MADNESS, 200),
		type,
		PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL);
	project(
		0, 1 + get_level(DEMON_MADNESS, 4),
		y2, x2,
		20 + get_level_s(DEMON_MADNESS, 200),
		type,
		PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL);

	return CAST;
}

std::string demonology_demon_madness_info()
{
	return fmt::format(
		"dam {} rad {}",
		20 + get_level_s(DEMON_MADNESS, 200),
		1 + get_level(DEMON_MADNESS, 4));
}

casting_result demonology_demon_field()
{
	int dir;

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}
	
	fire_cloud(
		GF_NEXUS,
		dir,
		20 + get_level_s(DEMON_FIELD, 70),
		7,
		30 + get_level_s(DEMON_FIELD, 100));
	return CAST;
}

std::string demonology_demon_field_info()
{
	return fmt::format(
		"dam {} dur {}",
		20 + get_level_s(DEMON_FIELD, 70),
		30 + get_level_s(DEMON_FIELD, 100));
}

casting_result demonology_doom_shield()
{
	set_shield(
		randint(10) + 20 + get_level_s(DOOM_SHIELD, 100),
		-300 + get_level_s(DOOM_SHIELD, 100),
		SHIELD_COUNTER,
		1  + get_level_s(DOOM_SHIELD, 14),
		10 + get_level_s(DOOM_SHIELD, 15));
	return CAST;
}

std::string demonology_doom_shield_info()
{
	return fmt::format(
		"dur {}+d10 dam {}d{}",
		20 + get_level_s(DOOM_SHIELD, 100),
		1 + get_level_s(DOOM_SHIELD, 14),
		10 + get_level_s(DOOM_SHIELD, 15));
}

casting_result demonology_unholy_word()
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
		if (magik(30 - get_level(UNHOLY_WORD, 25)))
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
			heal = ((30 + get_level(UNHOLY_WORD, 50)) * heal) / 100;

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

std::string demonology_unholy_word_info()
{
	return fmt::format(
		"heal mhp% of {}%",
		30 + get_level(UNHOLY_WORD, 50));
}

casting_result demonology_demon_cloak()
{
	set_tim_reflect(randint(5) + 5 + get_level(DEMON_CLOAK, 15));
	return CAST;
}

std::string demonology_demon_cloak_info()
{
	return fmt::format(
		"dur {}+d5",
		5 + get_level(DEMON_CLOAK, 15));
}

casting_result demonology_summon_demon()
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

	if (!summon_specific_friendly(p_ptr->py, p_ptr->px, level, type, true))
	{
		msg_print("Something blocks your summoning!");
	}

	return CAST;
}

std::string demonology_summon_demon_info()
{
	return fmt::format(
		"level {}",
		5 + get_level_s(DEMON_SUMMON, 100));
}

casting_result demonology_discharge_minion()
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
		dam = (dam * (20 + get_level(DISCHARGE_MINION, 60))) / 100;
		if (dam > 100 + get_level(DISCHARGE_MINION, 500))
		{
			dam = 100 + get_level(DISCHARGE_MINION, 500);
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

std::string demonology_discharge_minion_info()
{
	return fmt::format(
		"dam {}% max {}",
		20 + get_level(DISCHARGE_MINION, 60),
		100 + get_level(DISCHARGE_MINION, 500));
}

casting_result demonology_control_demon()
{
	int dir;
	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	fire_ball(GF_CONTROL_DEMON, dir, 50 + get_level_s(CONTROL_DEMON, 250), 0);
	return CAST;
}

std::string demonology_control_demon_info()
{
	return fmt::format(
		"power {}",
		50 + get_level_s(CONTROL_DEMON, 250));
}

casting_result divination_vision()
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

casting_result divination_sense_hidden()
{
	set_tim_invis(10 + randint(20) + get_level_s(SENSEHIDDEN, 40));
	return CAST;
}

std::string divination_sense_hidden_info()
{
	if (get_level_s(SENSEHIDDEN, 50) >= 15)
	{
		return fmt::format(
			"rad {} dur {}+d20",
			15 + get_level_s(SENSEHIDDEN, 40),
			10 + get_level_s(SENSEHIDDEN, 40));
	}
	else
	{
		return fmt::format(
			"rad {}",
			15 + get_level_s(SENSEHIDDEN, 40));
	}
}

casting_result divination_reveal_ways()
{
	detect_doors(10 + get_level(REVEALWAYS, 40));
	detect_stairs(10 + get_level(REVEALWAYS, 40));
	return CAST;
}

std::string divination_reveal_ways_info()
{
	return fmt::format(
		"rad {}",
		10 + get_level_s(REVEALWAYS, 40));
}

casting_result divination_sense_monsters()
{
	detect_monsters_normal(10 + get_level(SENSEMONSTERS, 40));
	if (get_level_s(SENSEMONSTERS, 50) >= 30)
	{
		set_tim_esp(10 + randint(10) + get_level_s(SENSEMONSTERS, 20));
	}
	return CAST;
}

std::string divination_sense_monsters_info()
{
	if (get_level_s(SENSEMONSTERS, 50) >= 30)
	{
		return fmt::format(
			"rad {} dur {}+d10",
			10 + get_level_s(SENSEMONSTERS, 40),
			10 + get_level_s(SENSEMONSTERS, 20));
	}
	else
	{
		return fmt::format(
			"rad {}",
			10 + get_level_s(SENSEMONSTERS, 40));
	}
}

casting_result earth_stone_skin()
{
	int type = 0;
	if (get_level_s(STONESKIN, 50) >= 25)
	{
		type = SHIELD_COUNTER;
	}

	set_shield(
		randint(10) + 10 + get_level_s(STONESKIN, 100),
		10 + get_level_s(STONESKIN, 50),
		type,
		2 + get_level_s(STONESKIN, 5),
		3 + get_level_s(STONESKIN, 5));
	return CAST;
}

std::string earth_stone_skin_info()
{
	if (get_level_s(STONESKIN, 50) >= 25)
	{
		return fmt::format(
			"dam {}d{} dur {}+d10 AC {}",
			2 + get_level_s(STONESKIN, 5),
			3 + get_level_s(STONESKIN, 5),
			10 + get_level_s(STONESKIN, 100),
			10 + get_level_s(STONESKIN, 50));
	}
	else
	{
		return fmt::format(
			"dur {}+d10 AC {}",
			10 + get_level_s(STONESKIN, 100),
			10 + get_level_s(STONESKIN, 50));
	}
}

casting_result earth_dig()
{
	int dir;
	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	wall_to_mud(dir);
	return CAST;
}

casting_result earth_stone_prison()
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

casting_result earth_strike()
{
	int dir;
	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	int dmg = 50 + get_level_s(STRIKE, 50);
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

std::string earth_strike_info()
{
	int dmg = 50 + get_level_s(STRIKE, 50);

	if (get_level_s(STRIKE, 50) >= 12)
	{
		return fmt::format("dam {} rad 1", dmg);
	}
	else
	{
		return fmt::format("dam {}", dmg);
	}
}

casting_result earth_shake()
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

std::string earth_shake_info()
{
	return fmt::format("rad {}", 4 + get_level_s(SHAKE, 10));
}

casting_result eru_see_the_music()
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

std::string eru_see_the_music_info()
{
	return fmt::format(
		"dur {}+d20",
		10 + get_level_s(ERU_SEE, 100));
}

casting_result eru_listen_to_the_music()
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

casting_result eru_lay_of_protection()
{
	fire_ball(GF_MAKE_GLYPH, 0, 1, 1 + get_level(ERU_PROT, 2));
	return CAST;
}

std::string eru_lay_of_protection_info()
{
	return fmt::format(
		"rad {}",
		1 + get_level(ERU_PROT, 2));
}

casting_result fire_globe_of_light()
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
		fire_ball(
			GF_LITE,
			0,
			10 + get_level_s(GLOBELIGHT, 100),
			5 + get_level_s(GLOBELIGHT, 6));
		p_ptr->update |= PU_VIEW;
	}

	return CAST;
}

std::string fire_globe_of_light_info()
{
	if (get_level_s(GLOBELIGHT, 50) >= 15)
	{
		return fmt::format(
			"dam {} rad {}",
			(10 + get_level_s(GLOBELIGHT, 100)),
			(5 + get_level_s(GLOBELIGHT, 6)));
	}
	else
	{
		return "";
	}
}

casting_result fire_fireflash()
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

	fire_ball(
		type,
		dir,
		20 + get_level_s(FIREFLASH, 500),
		2 + get_level_s(FIREFLASH, 5));
	return CAST;
}

std::string fire_fireflash_info()
{
	return fmt::format(
		"dam {} rad {}",
		20 + get_level_s(FIREFLASH, 500),
		2 + get_level_s(FIREFLASH, 5));
}

casting_result fire_fiery_shield()
{
	int type = SHIELD_FIRE;
	if (get_level_s(FIERYAURA, 50) >= 8)
	{
		type = SHIELD_GREAT_FIRE;
	}

	set_shield(
		randint(20) + 10 + get_level_s(FIERYAURA, 70),
		10,
		type,
		5 + get_level_s(FIERYAURA, 10),
		5 + get_level_s(FIERYAURA, 7));
	return CAST;
}

std::string fire_fiery_shield_info()
{
	return fmt::format(
		"dam {}d{} dur {}+d20",
		5 + get_level_s(FIERYAURA, 15),
		5 + get_level_s(FIERYAURA, 7),
		10 + get_level_s(FIERYAURA, 70));
}

casting_result fire_firewall()
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

std::string fire_firewall_info()
{
	return fmt::format(
		"dam {} dur {}",
		40 + get_level_s(FIREWALL, 150),
		10 + get_level_s(FIREWALL, 14));
}

object_filter_t const &item_tester_hook_fire_golem()
{
	using namespace object_filter;
	static auto instance = And(
		TVal(TV_LITE),
		Or(
			SVal(SV_LITE_TORCH),
			SVal(SV_LITE_LANTERN)));
	return instance;
}

casting_result fire_golem()
{
	/* Can we reconnect ? */
	if (do_control_reconnect())
	{
		msg_print("Control re-established.");
		return NO_CAST;
	}

	int item;
	if (!get_item(&item,
		      "Which light source do you want to use to create the golem?",
		      "You have no light source for the golem",
		      USE_INVEN | USE_EQUIP,
		      item_tester_hook_fire_golem()))
	{
		return NO_CAST;
	}

	/* Destroy the source object */
	inc_stack_size(item, -1);

	/* Find a place for it */
	int x, y;
	find_position(p_ptr->py, p_ptr->px, &y, &x);

	/* Summon it */
	int r_idx = get_fire_golem();
	m_allow_special[r_idx] = true;
	int m_idx = place_monster_one(y, x, r_idx, 0, false, MSTATUS_FRIEND);
	m_allow_special[r_idx] = false;

	/* level it */
	if (m_idx != 0)
	{
		monster_set_level(m_idx, 7 + get_level_s(FIREGOLEM, 70));
		p_ptr->control = m_idx;
		m_list[m_idx].mflag |= MFLAG_CONTROL;
	}

	return CAST;
}

std::string fire_golem_info()
{
	return fmt::format(
		"golem level {}",
		7 + get_level_s(FIREGOLEM, 70));
}

casting_result geomancy_call_the_elements()
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
		  1 + get_level(CALL_THE_ELEMENTS, 5));

	return CAST;
}

std::string geomancy_call_the_elements_info()
{
	return fmt::format(
		"rad {}",
		1 + get_level(CALL_THE_ELEMENTS, 5));
}

casting_result geomancy_channel_elements()
{
	channel_the_elements(p_ptr->py, p_ptr->px, get_level_s(CHANNEL_ELEMENTS, 50));
	return CAST;
}

typedef struct eff_type eff_type;
struct eff_type {
	s16b feat;
	s16b low_effect;
	s16b high_effect;
	int damage;
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
		assert(false);
	}
	/* Default */
	return 0;
}

casting_result geomancy_elemental_wave()
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

		return CAST;
	}
}

casting_result geomancy_vaporize()
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

std::string geomancy_vaporize_info()
{
	return fmt::format(
		"rad {} dur {}",
		1 + get_level_s(VAPORIZE, 4),
		10 + get_level_s(VAPORIZE, 20));
}

bool geomancy_vaporize_depends()
{
	return get_skill(SKILL_AIR) >= 4;
}

casting_result geomancy_geolysis()
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

std::string geomancy_geolysis_info()
{
	return fmt::format(
		"length {}",
		5 + get_level_s(GEOLYSIS, 12));
}

bool geomancy_geolysis_depends()
{
	return get_skill(SKILL_EARTH) >= 7;
}

casting_result geomancy_dripping_tread()
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

std::string geomancy_dripping_tread_info()
{
	return fmt::format(
		"dur {}+d15 movs",
		10 + get_level_s(DRIPPING_TREAD, 50));
}

bool geomancy_dripping_tread_depends()
{
	return get_skill(SKILL_WATER) >= 10;
}

casting_result geomancy_grow_barrier()
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
	return CAST;
}

bool geomancy_grow_barrier_depends()
{
	return get_skill(SKILL_EARTH) >= 12;
}

typedef struct geo_summon geo_summon;
struct geo_summon {
	s16b feat;
	s16b skill_idx;
	const char **summon_names;
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

int geomancy_count_elements(const char **elements)
{
	int i;
	for (i = 0; elements[i] != NULL; i++)
	{
	}
	return i;
}

casting_result geomancy_elemental_minion()
{
	int dir = 0;
	int x = 0, y = 0;
	geo_summon *summon_ptr = NULL;
	const char *earth_summons[] = {
		"Earth elemental",
		"Xorn",
		"Xaren",
		NULL
	};
	const char *air_summons[] = {
		"Air elemental",
		"Ancient blue dragon",
		"Great Storm Wyrm",
		"Sky Drake",
		NULL
	};
	const char *fire_summons[] = {
		"Fire elemental",
		"Ancient red dragon",
		NULL
	};
	const char *water_summons[] = {
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
		const char **names = summon_ptr->summon_names;
		int max = get_skill_scale(summon_ptr->skill_idx,
					  geomancy_count_elements(names));
		int r_idx = test_monster_name(names[rand_int(max)]);
		int mx, my, m_idx;

		/* Summon it */
		find_position(y, x, &my, &mx);
		m_idx = place_monster_one(my, mx, r_idx, 0, false, MSTATUS_FRIEND);

		/* Level it */
		if (m_idx)
		{
			monster_set_level(m_idx, 10 + get_level_s(ELEMENTAL_MINION, 120));
		}

		cave_set_feat(y, x, FEAT_FLOOR);

		return CAST;
	}
}

std::string geomancy_elemental_minion_info()
{
	return fmt::format(
		"min level {}",
		10 + get_level_s(ELEMENTAL_MINION, 120));
}

static void get_manathrust_dam(s16b *num, s16b *sides)
{
	*num = 3 + get_level_s(MANATHRUST, 50);
	*sides = 1 + get_level_s(MANATHRUST, 20);
}

casting_result mana_manathrust()
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

std::string mana_manathrust_info()
{
	s16b num = 0;
	s16b sides = 0;

	get_manathrust_dam(&num, &sides);
	return fmt::format("dam {}d{}", num, sides);
}

casting_result mana_remove_curses()
{
	auto fn = (get_level_s(DELCURSES, 50) >= 20)
		? remove_all_curse
		: remove_curse;

	if (fn())
	{
		msg_print("The curse is broken!");
	}

	return CAST;
}

casting_result mana_elemental_shield()
{
	casting_result res = NO_CAST;

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

std::string mana_elemental_shield_info()
{
	return fmt::format(
		"dur {}+d10",
		15 + get_level_s(RESISTS, 50));
}

casting_result mana_disruption_shield()
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

std::string mana_disruption_shield_info()
{
	return fmt::format(
		"dur {}+d5",
		3 + get_level_s(MANASHIELD, 10));
}

casting_result manwe_wind_shield()
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

		set_shield(
			dur,
			get_level_s(MANWE_SHIELD, 30),
			type,
			1 + get_level_s(MANWE_SHIELD, 2),
			1 + get_level_s(MANWE_SHIELD, 6));
	}

	return CAST;
}

std::string manwe_wind_shield_info()
{
	auto buf = fmt::format(
		"dur {}+d20",
		get_level_s(MANWE_SHIELD, 50) + 10);

	if (get_level_s(MANWE_SHIELD, 50) >= 10)
	{
		buf += fmt::format(
			" AC {}", get_level_s(MANWE_SHIELD, 30));
	}

	if (get_level_s(MANWE_SHIELD, 50) >= 20)
	{
		buf += fmt::format(
			" dam {}d{}",
			1 + get_level_s(MANWE_SHIELD, 2),
			1 + get_level_s(MANWE_SHIELD, 6));
	}

	return buf;
}

casting_result manwe_avatar()
{
	s16b mimic_idx = resolve_mimic_name("Maia");
	assert(mimic_idx >= 0);

	set_mimic(
		get_level_s(MANWE_AVATAR, 20) + randint(10),
		mimic_idx,
		p_ptr->lev);
	return CAST;
}

std::string manwe_avatar_info()
{
	return fmt::format(
		"dur {}+d10",
		get_level_s(MANWE_AVATAR, 20));
}

casting_result manwe_blessing()
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

std::string manwe_blessing_info()
{
	return fmt::format(
		"dur {}+d40",
		get_level_s(MANWE_BLESS, 70) + 30);
}

casting_result manwe_call()
{
	int y = 0, x = 0, m_idx = -1, r_idx = -1;

	find_position(p_ptr->py, p_ptr->px, &y, &x);

	r_idx = test_monster_name("Great eagle");
	assert(r_idx >= 1);

	m_idx = place_monster_one(y, x, r_idx, 0, false, MSTATUS_FRIEND);

	if (m_idx > 0)
	{
		monster_set_level(m_idx, 20 + get_level(MANWE_CALL, 70));
		return CAST;
	}
	else
	{
		return NO_CAST;
	}
}

std::string manwe_call_info()
{
	return fmt::format(
		"level {}",
		get_level_s(MANWE_CALL, 70) + 20);
}

void do_melkor_curse(int m_idx)
{
	assert(m_idx >= 0);

	monster_type *m_ptr = &m_list[m_idx];

	if (get_level_s(MELKOR_CURSE, 50) >= 35)
	{
		auto const r_ptr = m_ptr->race();

		m_ptr->maxhp = m_ptr->maxhp - r_ptr->hside;
		if (m_ptr->maxhp < 1)
		{
			m_ptr->maxhp = 1;
		}
		if (m_ptr->hp > m_ptr->maxhp)
		{
			m_ptr->hp = m_ptr->maxhp;
		}

		p_ptr->redraw |= PR_FRAME;
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

casting_result melkor_curse()
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

casting_result melkor_corpse_explosion()
{
	fire_ball(
		GF_CORPSE_EXPL,
		0,
		20 + get_level_s(MELKOR_CORPSE_EXPLOSION, 70),
		2 + get_level_s(MELKOR_CORPSE_EXPLOSION, 5));
	return CAST;
}

std::string melkor_corpse_explosion_info()
{
	return fmt::format(
		"dam {}%",
		20 + get_level_s(MELKOR_CORPSE_EXPLOSION, 70));
}

casting_result melkor_mind_steal()
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

		char buf[128];
		monster_desc(buf, m_ptr, 0);
		buf[0] = toupper(buf[0]);

		auto const r_ptr = m_ptr->race();
		if ((randint(m_ptr->level) < chance) &&
		    ((r_ptr->flags & RF_UNIQUE).empty()))
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

std::string melkor_mind_steal_info()
{
	return fmt::format(
		"chance 1d(mlvl)<{}",
		get_level_s(MELKOR_MIND_STEAL, 50));
}

casting_result meta_recharge()
{
	recharge(60 + get_level_s(RECHARGE, 140));
	return CAST;
}

std::string meta_recharge_info()
{
	return fmt::format(
		"power {}",
		60 + get_level_s(RECHARGE, 140));
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

casting_result meta_spellbinder()
{
	auto spellbinder = &p_ptr->spellbinder;

	if (spellbinder->spell_idxs.size() > 0)
	{
		struct trigger {
			int idx;
			const char *desc;
		};
		struct trigger triggers[] = {
			{ SPELLBINDER_HP75, "75% HP", },
			{ SPELLBINDER_HP50, "50% HP", },
			{ SPELLBINDER_HP25, "25% HP", },
			{ -1, NULL, },
		};
		int trigger_idx = -1;

		assert(spellbinder->trigger >= 0);

		for (trigger_idx = 0; triggers[trigger_idx].idx >= 0; trigger_idx++)
		{
			if (triggers[trigger_idx].idx == spellbinder->trigger)
			{
				break;
			}
		}

		msg_print("The spellbinder is already active.");
		msg_format("It will trigger at %s.", triggers[trigger_idx].desc);
		msg_print("With the spells: ");
		for (auto spell_idx : spellbinder->spell_idxs)
		{ 
			msg_print(spell_type_name(spell_at(spell_idx)));
		}

		/* Doesn't cost anything */
		return NO_CAST;
	}
	else
	{
		char c;

		if (!get_com("Trigger at [a]75% hp [b]50% hp [c]25% hp?", &c))
		{
			return NO_CAST;
		}
		
		switch (c)
		{
		case 'a':
			spellbinder->trigger = SPELLBINDER_HP75;
			break;
		case 'b':
			spellbinder->trigger = SPELLBINDER_HP50;
			break;
		case 'c':
			spellbinder->trigger = SPELLBINDER_HP25;
			break;
		default:
			return NO_CAST;
			
		}

		std::size_t n = get_spellbinder_max();
		while (n > 0)
		{
			s32b s = get_school_spell("bind", 0);

			if (s == -1)
			{
				spellbinder->trigger = 0;
				spellbinder->spell_idxs.clear();
				return CAST;
			}
			else
			{
				if (spell_type_skill_level(spell_at(s)) > 7 + get_level_s(SPELLBINDER, 35))
				{
					msg_print(fmt::format(
						"You are only allowed spells with a base level of {}.",
						7 + get_level_s(SPELLBINDER, 35)));
					return CAST;
				}
			}

			spellbinder->spell_idxs.push_back(s);
			n--;
		}
		
		p_ptr->energy = p_ptr->energy - 3100;
		msg_print("Spellbinder ready.");
		return CAST;
	}
}

std::string meta_spellbinder_info()
{
	return fmt::format(
		"number {} max level {}",
		get_spellbinder_max(),
		7 + get_level_s(SPELLBINDER, 35));
}

casting_result meta_disperse_magic()
{
	set_blind(0);
	set_lite(0);
	if (get_level_s(DISPERSEMAGIC, 50) >= 5)
	{
		set_confused(0);
		set_image(0);
	}
	if (get_level_s(DISPERSEMAGIC, 50) >= 10)
	{
		set_slow(0);
		set_fast(0, 0);
		set_light_speed(0);
	}
	if (get_level_s(DISPERSEMAGIC, 50) >= 15)
	{
		set_stun(0);
		set_cut(0);
	}
	if (get_level_s(DISPERSEMAGIC, 50) >= 20)
	{
		set_hero(0);
		set_shero(0);
		set_blessed(0);
		set_shield(0, 0, 0, 0, 0);
		set_afraid(0);
		set_parasite(0, 0);
		set_mimic(0, 0, 0);
	}
	return CAST;
}

casting_result meta_tracker()
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
	return CAST;
}

static void stop_inertia_controlled_spell()
{
	assert(TIMER_INERTIA_CONTROL != NULL);

	p_ptr->inertia_controlled_spell = -1;
	TIMER_INERTIA_CONTROL->disable();
	p_ptr->update = p_ptr->update | PU_MANA;
}

void meta_inertia_control_hook_birth_objects()
{
	stop_inertia_controlled_spell();
}

casting_result meta_inertia_control()
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
		msg_print(fmt::format(
			"This spell inertia flow({}) is too strong to be controlled by your current spell.",
			difficulty));
		stop_inertia_controlled_spell();
		return NO_CAST;
	}

	p_ptr->inertia_controlled_spell = s;
	TIMER_INERTIA_CONTROL->set_delay_and_reset(delay);
	TIMER_INERTIA_CONTROL->enable();
	p_ptr->update |= PU_MANA;
	msg_format("Inertia flow controlling spell %s.", spell_type_name(spell_at(s)));
	return CAST;
}

std::string meta_inertia_control_info()
{
	return fmt::format(
		"level {}",
		get_level_s(INERTIA_CONTROL, 10));
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
		lua_cast_school_spell(p_ptr->inertia_controlled_spell, true);
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

casting_result mind_charm()
{
	int pwr = mind_charm_power();
	int level = get_level_s(CHARM, 50);

	if (level >= 35)
	{
		project_hack(GF_CHARM, pwr);
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
			fire_ball(GF_CHARM, dir, pwr, 3);
		}
		else
		{
			fire_bolt(GF_CHARM, dir, pwr);
		}

	}

	return CAST;
}

std::string mind_charm_info()
{
	return fmt::format(
		"power {}",
		mind_charm_power());
}

static int mind_confuse_power()
{
	return 10 + get_level_s(CONFUSE, 150);
}

casting_result mind_confuse()
{
	int pwr = mind_confuse_power();
	int level = get_level_s(CONFUSE, 50);

	if (level >= 35)
	{
		project_hack(GF_OLD_CONF, pwr);
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
			fire_ball(GF_OLD_CONF, dir, pwr, 3);
		}
		else
		{
			fire_bolt(GF_OLD_CONF, dir, pwr);
		}
	}

	return CAST;
}

std::string mind_confuse_info()
{
	return fmt::format(
		"power {}",
		mind_confuse_power());
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

casting_result mind_armor_of_fear()
{
	set_shield(
		randint(10) + mind_armor_of_fear_base_duration(),
		10,
		SHIELD_FEAR,
		mind_armor_of_fear_power_sides(),
		mind_armor_of_fear_power_dice());
	return CAST;
}

std::string mind_armor_of_fear_info()
{
	return fmt::format(
		"dur {}+d10 power {}d{}",
		mind_armor_of_fear_base_duration(),
		mind_armor_of_fear_power_sides(),
		mind_armor_of_fear_power_dice());
}

static int mind_stun_power()
{
	return 10 + get_level_s(STUN, 150);
}

casting_result mind_stun()
{
	int dir;

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	if (get_level_s(STUN, 50) >= 20)
	{
		fire_ball(GF_STUN, dir, mind_stun_power(), 3);
	}
	else
	{
		fire_bolt(GF_STUN, dir, mind_stun_power());
	}
	return CAST;
}

std::string mind_stun_info()
{
	return fmt::format(
		"power {}",
		mind_stun_power());
}

casting_result tempo_magelock()
{
	auto const &f_info = game->edit_data.f_info;

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

			if ((!(f_info[c_ptr->feat].flags | FF_FLOOR)) ||
			    (f_info[c_ptr->feat].flags | FF_PERMANENT) ||
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
	} else {
		int dir;
		if (!get_aim_dir(&dir))
		{
			return NO_CAST;
		}
		wizard_lock(dir);
	}
	return CAST;
}

static s32b tempo_slow_monster_power()
{
	return 40 + get_level_s(SLOWMONSTER, 160);
}

casting_result tempo_slow_monster()
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
		fire_ball(GF_OLD_SLOW, dir, pwr, 1);
	}
	else
	{
		fire_bolt(GF_OLD_SLOW, dir, pwr);
	}
	return CAST;
}

std::string tempo_slow_monster_info()
{
	s32b pwr = tempo_slow_monster_power();

	if (get_level_s(SLOWMONSTER, 50) >= 20)
	{
		return fmt::format("power {} rad 1", pwr);
	}
	else
	{
		return fmt::format("power {}", pwr);
	}
}

static s32b tempo_essence_of_speed_base_duration()
{
	return 10 + get_level_s(ESSENCESPEED, 50);
}

static s32b tempo_essence_of_speed_bonus()
{
	return 5 + get_level_s(ESSENCESPEED, 20);
}

casting_result tempo_essence_of_speed()
{
	if (p_ptr->fast == 0)
	{
		set_fast(
			randint(10) + tempo_essence_of_speed_base_duration(),
			tempo_essence_of_speed_bonus());
		return CAST;
	}
	return NO_CAST;
}

std::string tempo_essence_of_speed_info()
{
	return fmt::format(
		"dur {}+d10 speed {}",
		tempo_essence_of_speed_base_duration(),
		tempo_essence_of_speed_bonus());
}

static s32b tempo_banishment_power()
{
	return 40 + get_level_s(BANISHMENT, 160);
}

casting_result tempo_banishment()
{
	s32b pwr = tempo_banishment_power();

	project_hack(GF_AWAY_ALL, pwr);

	if (get_level_s(BANISHMENT, 50) >= 15)
	{
		project_hack(GF_STASIS, 20 + get_level_s(BANISHMENT, 120));
	}
	
	return CAST;
}

std::string tempo_banishment_info()
{
	return fmt::format(
		"power {}",
		tempo_banishment_power());
}

casting_result tulkas_divine_aim()
{
	s32b dur = get_level_s(TULKAS_AIM, 50) + randint(10);

	set_strike(dur);
	if (get_level_s(TULKAS_AIM, 50) >= 20)
	{
		set_tim_deadly(dur);
	}

	return CAST;
}

std::string tulkas_divine_aim_info()
{
	return fmt::format(
		"dur {}+d10",
		get_level_s(TULKAS_AIM, 50));
}

casting_result tulkas_wave_of_power()
{
	int dir;

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	fire_bolt(GF_ATTACK, dir, get_level_s(TULKAS_WAVE, p_ptr->num_blow));
	return CAST;
}

std::string tulkas_wave_of_power_info()
{
	return fmt::format(
		"blows {}",
		get_level_s(TULKAS_WAVE, p_ptr->num_blow));
}

casting_result tulkas_whirlwind()
{
	fire_ball(GF_ATTACK, 0, 1, 1);
	return CAST;
}

/* Return the number of Udun/Melkor spells in a given book */
int udun_in_book(s32b sval, s32b pval)
{
	int count = 0;

	random_book_setup(sval, pval);

	/* Get the school book */
	school_book *school_book = school_books_at(sval);

	/* Go through spells */
	for (auto spell_idx : school_book->spell_idxs) {
		spell_type *spell = spell_at(spell_idx);
		for (auto school_idx : spell_type_get_schools(spell))
		{
			if ((school_idx == SCHOOL_UDUN) ||
			    (school_idx == SCHOOL_MELKOR))
			{
				count++;
			}
		}
	}

	return count;
}

int levels_in_book(s32b sval, s32b pval)
{
	int levels = 0;

	random_book_setup(sval, pval);

	/* Get the school book */
	school_book *school_book = school_books_at(sval);

	/* Parse all spells */
	for (auto spell_idx : school_book->spell_idxs)
	{
		spell_type *spell = spell_at(spell_idx);
		levels += spell_type_skill_level(spell);
	}

	return levels;
}

static object_filter_t const &udun_object_is_drainable()
{
	using namespace object_filter;
	static auto instance = Or(
		TVal(TV_WAND),
		TVal(TV_ROD_MAIN),
		TVal(TV_STAFF));
	return instance;
}

casting_result udun_drain()
{
	/* Ask for an item */
	int item;
	if (!get_item(&item,
		      "What item to drain?",
		      "You have nothing you can drain",
		      USE_INVEN,
		      udun_object_is_drainable()))
	{
		return NO_CAST;
	}

	/* Drain */
	object_type *o_ptr = get_object(item);

	switch (o_ptr->tval)
	{
	case TV_STAFF:
	case TV_WAND:
	{
		/* Generate mana */
		increase_mana(o_ptr->pval * o_ptr->k_ptr->level * o_ptr->number);

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
		assert(false);
	}

	return CAST;
}

casting_result udun_genocide()
{
	if (get_level_s(GENOCIDE, 50) < 10)
	{
		genocide();
	}
	else
	{
		if (get_check("Genocide all monsters near you? "))
		{
			mass_genocide();
		}
		else
		{
			genocide();
		}
	}

	return CAST;
}

static int udun_wraithform_base_duration()
{
	return 20 + get_level_s(WRAITHFORM, 40);
}

casting_result udun_wraithform()
{
	set_shadow(randint(30) + udun_wraithform_base_duration());
	return CAST;
}

std::string udun_wraithform_info()
{
	return fmt::format(
		"dur {}+d30",
		udun_wraithform_base_duration());
}

static int udun_flame_of_udun_base_duration()
{
	return 5 + get_level_s(FLAMEOFUDUN, 30);
}

casting_result udun_flame_of_udun()
{
	set_mimic(
		randint(15) + udun_flame_of_udun_base_duration(),
		resolve_mimic_name("Balrog"),
		get_level_s(FLAMEOFUDUN, 50));
	return CAST;
}

std::string udun_flame_of_udun_info()
{
	return fmt::format(
		"dur {}+d15",
		udun_flame_of_udun_base_duration());
}

static int tidal_wave_damage()
{
	return 40 + get_level_s(TIDALWAVE, 200);
}

static int tidal_wave_duration()
{
	return 6 + get_level_s(TIDALWAVE, 10);
}

casting_result water_tidal_wave()
{
	fire_wave(GF_WAVE,
		  0,
		  tidal_wave_damage(),
		  0,
		  tidal_wave_duration(),
		  EFF_WAVE);
	return CAST;
}

std::string water_tidal_wave_info()
{
	return fmt::format(
		"dam {} dur {}",
		tidal_wave_damage(),
		tidal_wave_duration());
}

static int water_ice_storm_damage()
{
	return 80 + get_level_s(ICESTORM, 200);
}

static int water_ice_storm_radius()
{
	return 1 + get_level(ICESTORM, 3);
}

static int water_ice_storm_duration()
{
	return 20 + get_level_s(ICESTORM, 70);
}

casting_result water_ice_storm()
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

	return CAST;
}

std::string water_ice_storm_info()
{
	return fmt::format(
		"dam {} rad {} dur {}",
		water_ice_storm_damage(),
		water_ice_storm_radius(),
		water_ice_storm_duration());
}

static int water_ent_potion_base_duration()
{
	return 25 + get_level_s(ENTPOTION, 40);;
}

casting_result water_ent_potion()
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

	return CAST;
}

std::string water_ent_potion_info()
{
	if (get_level_s(ENTPOTION, 50) >= 12)
	{
		return fmt::format(
			"dur {}+d25",
			water_ent_potion_base_duration());
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
	return 3 + get_level(VAPOR, 9);
}

static int water_vapor_duration()
{
	return 5;
}

casting_result water_vapor()
{
	fire_cloud(GF_WATER,
		   0,
		   water_vapor_damage(),
		   water_vapor_radius(),
		   water_vapor_duration());
	return CAST;
}

std::string water_vapor_info()
{
	return fmt::format(
		"dam {} rad {} dur {}",
		water_vapor_damage(),
		water_vapor_radius(),
		water_vapor_duration());
}

static void get_geyser_damage(int *dice, int *sides)
{
	assert(dice != NULL);
	assert(sides != NULL);

	*dice = get_level_s(GEYSER, 10);
	*sides = 3 + get_level_s(GEYSER, 35);
}

casting_result water_geyser()
{
	int dir, dice, sides;

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	get_geyser_damage(&dice, &sides);
	fire_bolt_or_beam(
		2 * get_level_s(GEYSER, 85),
		GF_WATER,
		dir,
		damroll(dice, sides));
	return CAST;
}

std::string water_geyser_info()
{
	int dice, sides;
	get_geyser_damage(&dice, &sides);

	return fmt::format(
		"dam {}d{}",
		dice,
		sides);
}

static int charm_animal_power()
{
	return 10 + get_level_s(YAVANNA_CHARM_ANIMAL, 170);
}

static int charm_animal_radius()
{
	return get_level_s(YAVANNA_CHARM_ANIMAL, 2);
}

casting_result yavanna_charm_animal()
{
	int dir;

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	fire_ball(
		GF_CONTROL_ANIMAL,
		dir,
		charm_animal_power(),
		charm_animal_radius());
	return CAST;
}

std::string yavanna_charm_animal_info()
{
	return fmt::format(
		"power {} rad {}",
		charm_animal_power(),
		charm_animal_radius());
}

static int yavanna_grow_grass_radius()
{
	return get_level_s(YAVANNA_GROW_GRASS, 4);
}

casting_result yavanna_grow_grass()
{
	grow_grass(yavanna_grow_grass_radius());
	return CAST;
}

std::string yavanna_grow_grass_info()
{
	return fmt::format(
		"rad {}",
		yavanna_grow_grass_radius());
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

casting_result yavanna_tree_roots()
{
	set_roots(
		tree_roots_duration(),
		tree_roots_ac(),
		tree_roots_damage());
	return CAST;
}

std::string yavanna_tree_roots_info()
{
	return fmt::format(
		"dur {} AC {} dam {}",
		tree_roots_duration(),
		tree_roots_ac(),
		tree_roots_damage());
}

static int water_bite_base_duration()
{
	return 30 + get_level_s(YAVANNA_WATER_BITE, 150);
}

static int water_bite_damage()
{
	return 10 + get_level_s(YAVANNA_WATER_BITE, 50);
}

casting_result yavanna_water_bite()
{
	int rad = 0;

	if (get_level_s(YAVANNA_WATER_BITE, 50) >= 25)
	{
		rad = 1;
	}

	set_project(
		randint(30) + water_bite_base_duration(),
		GF_WATER,
		water_bite_damage(),
		rad,
		PROJECT_STOP | PROJECT_KILL);
	return CAST;
}

std::string yavanna_water_bite_info()
{
	return fmt::format(
		"dur {}+d30 dam {}/blow",
		water_bite_base_duration(),
		water_bite_damage());
}

static int uproot_mlevel()
{
	return 30 + get_level_s(YAVANNA_UPROOT, 70);
}

casting_result yavanna_uproot()
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
		m_idx = place_monster_one(y, x, test_monster_name("Ent"), 0, false, MSTATUS_FRIEND);

		/* level it */
		if (m_idx != 0)
		{
			monster_set_level(m_idx, uproot_mlevel());
		}

		msg_print("The tree awakes!");
		return CAST;
	}
	else
	{
		msg_print("There is no tree there.");
		return NO_CAST;
	}
}

std::string yavanna_uproot_info()
{
	return fmt::format(
		"lev {}",
		uproot_mlevel());
}

static int nature_grow_trees_radius()
{
	return 2 + get_level_s(GROWTREE, 7);
}

casting_result nature_grow_trees()
{
	grow_trees(nature_grow_trees_radius());
	return CAST;
}

std::string nature_grow_trees_info()
{
	return fmt::format(
		"rad {}",
		nature_grow_trees_radius());
}

static int nature_healing_percentage()
{
	return 15 + get_level_s(HEALING, 35);
}

static int nature_healing_hp()
{
	return p_ptr->mhp * nature_healing_percentage() / 100;
}

casting_result nature_healing()
{
	hp_player(nature_healing_hp());
	return CAST;
}

std::string nature_healing_info()
{
	return fmt::format(
		"heal {}% = {}hp",
		nature_healing_percentage(),
		nature_healing_hp());
}

casting_result nature_recovery()
{
	set_poisoned(p_ptr->poisoned / 2);

	if (get_level_s(RECOVERY, 50) >= 5)
	{
		set_poisoned(0);
		set_cut(0);
	}

	if (get_level_s(RECOVERY, 50) >= 10)
	{
		do_res_stat(A_STR, true);
		do_res_stat(A_CON, true);
		do_res_stat(A_DEX, true);
		do_res_stat(A_WIS, true);
		do_res_stat(A_INT, true);
		do_res_stat(A_CHR, true);
	}

	if (get_level_s(RECOVERY, 50) >= 15)
	{
		restore_level();
	}

	return CAST;
}

static int regeneration_base_duration()
{
	return 5 + get_level_s(REGENERATION, 50);
}

static int regeneration_power()
{
	return 300 + get_level_s(REGENERATION, 700);
}

casting_result nature_regeneration()
{
	if (p_ptr->tim_regen == 0)
	{
		set_tim_regen(
			randint(10) + regeneration_base_duration(),
			regeneration_power());
		return CAST;
	}
	return NO_CAST;
}

std::string nature_regeneration_info()
{
	return fmt::format(
		"dur {}+d10 power {}",
		regeneration_base_duration(),
		regeneration_power());
}

static int summon_animal_level()
{
	return 25 + get_level_s(SUMMONANNIMAL, 50);
}

casting_result nature_summon_animal()
{
	summon_specific_level = summon_animal_level();
	summon_specific_friendly(
		p_ptr->py,
		p_ptr->px,
		dun_level,
		SUMMON_ANIMAL,
		true);
	return CAST;
}

std::string nature_summon_animal_info()
{
	return fmt::format(
		"level {}",
		summon_animal_level());
}

casting_result nature_grow_athelas()
{
        if (p_ptr->black_breath)
	{
		msg_print("The hold of the Black Breath on you is broken!");
		p_ptr->black_breath = false;
	}

	return CAST;
}

static int device_heal_monster_hp()
{
	return 20 + get_level_s(DEVICE_HEAL_MONSTER, 380);
}

casting_result device_heal_monster()
{
	int dir;

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	fire_ball(GF_OLD_HEAL, dir, device_heal_monster_hp(), 0);
	return CAST;
}

std::string device_heal_monster_info()
{
	return fmt::format(
		"heal {}",
		device_heal_monster_hp());
}

casting_result device_haste_monster()
{
	int dir;

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	fire_ball(GF_OLD_SPEED, dir, 1, 0);
	return CAST;
}

std::string device_haste_monster_info()
{
	return "speed +10";
}

casting_result device_wish()
{
	make_wish();
	return CAST;
}

casting_result device_summon_monster()
{
	for (int i = 0; i < 4 + get_level_s(DEVICE_SUMMON, 30); i++)
	{
		summon_specific(p_ptr->py, p_ptr->px, dun_level, 0);
	}

	return CAST;
}

static int device_mana_pct()
{
	return 20 + get_level_s(DEVICE_MANA, 50);
}

casting_result device_mana()
{
	increase_mana((p_ptr->msp * device_mana_pct()) / 100);
	return CAST;
}

std::string device_mana_info()
{
	return fmt::format(
		"restore {}%",
		device_mana_pct());
}

casting_result device_nothing()
{
	return CAST;
}

static int holy_fire_damage()
{
	return 50 + get_level_s(DEVICE_HOLY_FIRE, 300);
}

casting_result device_holy_fire()
{
	project_hack(GF_HOLY_FIRE, holy_fire_damage());
	return CAST;
}

std::string device_holy_fire_info()
{
	return fmt::format(
		"dam {}",
		holy_fire_damage());
}

casting_result device_thunderlords()
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
			return CAST;
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
			return CAST;
		}

	default:
		assert(false);
		return NO_CAST;
	}
}

void static start_lasting_spell(int spl)
{
	p_ptr->music_extra = -spl;
}

casting_result music_stop_singing_spell()
{
	start_lasting_spell(0);
	return CAST;
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

casting_result music_holding_pattern_spell()
{
	start_lasting_spell(MUSIC_HOLD);
	return CAST;
}

std::string music_holding_pattern_info()
{
	return fmt::format(
		"power {}",
		holding_pattern_power());
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

casting_result music_illusion_pattern_spell()
{
	start_lasting_spell(MUSIC_CONF);
	return CAST;
}

std::string music_illusion_pattern_info()
{
	return fmt::format(
		"power {}",
		illusion_pattern_power());
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

casting_result music_stun_pattern_spell()
{
	start_lasting_spell(MUSIC_STUN);
	return CAST;
}

std::string music_stun_pattern_info()
{
	return fmt::format(
		"power {}",
		stun_pattern_power());
}

int music_song_of_the_sun_lasting()
{
	set_lite(5);
	return 1;
}

casting_result music_song_of_the_sun_spell()
{
	start_lasting_spell(MUSIC_LITE);
	return CAST;
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

casting_result music_flow_of_life_spell()
{
	start_lasting_spell(MUSIC_HEAL);
	return CAST;
}

std::string music_flow_of_life_info()
{
	return fmt::format(
		"heal {}/turn",
		flow_of_life_hp());
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

casting_result music_heroic_ballad_spell()
{
	start_lasting_spell(MUSIC_HERO);
	return CAST;
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

casting_result music_hobbit_melodies_spell()
{
	start_lasting_spell(MUSIC_TIME);
	return CAST;
}

std::string music_hobbit_melodies_info()
{
	if (get_level_s(MUSIC_TIME, 50) >= 15)
	{
		return fmt::format(
			"AC {} speed {}",
			10 + get_level_s(MUSIC_TIME, 50),
			7 + get_level_s(MUSIC_TIME, 10));
	}
	else
	{
		return fmt::format(
			"AC {}",
			10 + get_level_s(MUSIC_TIME, 50));
	}
}

int music_clairaudience_lasting()
{
	set_tim_esp(5);
	return get_mana(MUSIC_MIND);
}

casting_result music_clairaudience_spell()
{
	start_lasting_spell(MUSIC_MIND);
	return CAST;
}

std::string music_clairaudience_info()
{
	if (get_level_s(MUSIC_MIND, 50) >= 10)
	{
		return fmt::format(
			"rad {}",
			1 + get_level(MUSIC_MIND, 3));
	}
	else
	{
		return "";
	}
}

casting_result music_blow_spell()
{
	fire_ball(GF_SOUND,
		  0,
		  damroll(2 + get_level(MUSIC_BLOW, 10), 4 + get_level(MUSIC_BLOW, 40)),
		  1 + get_level(MUSIC_BLOW, 12));
	return CAST;
}

std::string music_blow_info()
{
	return fmt::format(
		"dam {}d{} rad {}",
		2 + get_level(MUSIC_BLOW, 10),
		4 + get_level(MUSIC_BLOW, 40),
		1 + get_level(MUSIC_BLOW, 12));
}

casting_result music_gush_of_wind_spell()
{
	fire_ball(GF_AWAY_ALL,
		  0,
		  10 + get_level(MUSIC_BLOW, 40),
		  1 + get_level(MUSIC_BLOW, 12));
	return CAST;
}

std::string music_gush_of_wind_info()
{
	return fmt::format(
		"dist {} rad {}",
		10 + get_level(MUSIC_BLOW, 40),
		1 + get_level(MUSIC_BLOW, 12));
}

casting_result music_horns_of_ylmir_spell()
{
	earthquake(p_ptr->py, p_ptr->px, 2 + get_level_s(MUSIC_YLMIR, 10));
	return CAST;
}

std::string music_horns_of_ylmir_info()
{
	return fmt::format(
		"rad {}",
		2 + get_level_s(MUSIC_YLMIR, 10));
}

casting_result music_ambarkanta_spell()
{
	alter_reality();
	return CAST;
}

casting_result aule_firebrand_spell()
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

	set_project(
		level + randint(20),
		type,
		4 + level,
		rad,
		PROJECT_STOP | PROJECT_KILL);
	return CAST;
}

std::string aule_firebrand_info()
{
	s32b level = get_level_s(AULE_FIREBRAND, 50);

	return fmt::format("dur {}+d20 dam {}/blow",
		level,
		4 + level);
}

static object_filter_t const &aule_enchant_weapon_item_tester()
{
	using namespace object_filter;
	static auto instance = And(
		// Cannot enchant artifacts, spell is probably already too overpowered.
		Not(IsArtifact()),
		// Only weapons which Aule likes
		Or(
			TVal(TV_MSTAFF),
			TVal(TV_BOW),
			TVal(TV_HAFTED),
			TVal(TV_POLEARM),
			TVal(TV_SWORD),
			TVal(TV_AXE)));
	return instance;
}

casting_result aule_enchant_weapon_spell()
{
	s32b level = get_level_s(AULE_ENCHANT_WEAPON, 50);
	s16b num_h, num_d, num_p;

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

	int item;
	if (!get_item(&item,
		      "Which object do you want to enchant?",
		      "You have no objects to enchant.",
		      USE_INVEN,
		      aule_enchant_weapon_item_tester()))
	{
		return NO_CAST;
	}

	object_type *o_ptr = get_object(item);

	o_ptr->to_h = o_ptr->to_h + num_h;
	o_ptr->to_d = o_ptr->to_d + num_d;
	o_ptr->pval = o_ptr->pval + num_p;

	return CAST;
}

std::string aule_enchant_weapon_info()
{
	return fmt::format(
		"tries {}",
		1 + get_level_s(AULE_ENCHANT_WEAPON, 50)/12);
}

static object_filter_t const &aule_enchant_armor_item_tester()
{
	using namespace object_filter;
	static auto instance = And(
		// No enchanting artifacts; the spell is already horribly
		// overpowered.
		Not(IsArtifact()),
		// Only armor-like things can be enchanted
		Or(
			TVal(TV_BOOTS),
			TVal(TV_GLOVES),
			TVal(TV_HELM),
			TVal(TV_CROWN),
			TVal(TV_SHIELD),
			TVal(TV_CLOAK),
			TVal(TV_SOFT_ARMOR),
			TVal(TV_HARD_ARMOR),
			TVal(TV_DRAG_ARMOR)));
	return instance;
}

casting_result aule_enchant_armour_spell()
{
	s32b level = get_level_s(AULE_ENCHANT_ARMOUR, 50);
	s16b num_h, num_d, num_a, num_p;
	int item;

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

	if (!get_item(&item,
		      "Which object do you want to enchant?",
		      "You have no objects to enchant.",
		      USE_INVEN,
		      aule_enchant_armor_item_tester()))
	{
		return NO_CAST;
	}

	object_type *o_ptr = get_object(item);

	o_ptr->to_h = o_ptr->to_h + num_h;
	o_ptr->to_d = o_ptr->to_d + num_d;
	o_ptr->pval = o_ptr->pval + num_p;
	o_ptr->to_a = o_ptr->to_a + num_a;

	return CAST;
}

std::string aule_enchant_armour_info()
{
	return fmt::format(
		"tries {}",
		1 + get_level_s(AULE_ENCHANT_ARMOUR, 50)/10);
}

casting_result aule_child_spell()
{
	int y, x;
	s16b m_idx;

	find_position(p_ptr->py, p_ptr->px, &y, &x);
	m_idx = place_monster_one(y, x, test_monster_name("Dwarven warrior"),
				  0, false, MSTATUS_FRIEND);
 
	if (m_idx)
	{
		monster_set_level(m_idx, 20 + get_level(AULE_CHILD, 70));
		return CAST;
	}
	else
	{
		return NO_CAST;
	}
}

std::string aule_child_info()
{
	return fmt::format(
		"level {}",
		20 + get_level_s(AULE_CHILD, 70));
}

static int tears_of_luthien_hp()
{
	return 10 * get_level_s(MANDOS_TEARS_LUTHIEN, 30);
}

casting_result mandos_tears_of_luthien_spell()
{
	hp_player(tears_of_luthien_hp());
	set_stun(0);
	set_cut(0);
	set_afraid(0);

	return CAST;
}

std::string mandos_tears_of_luthien_info()
{
	return fmt::format(
		"heals {}",
		tears_of_luthien_hp());
}

casting_result mandos_spirit_of_the_feanturi_spell()
{
	s32b level = get_level_s(MANDOS_SPIRIT_FEANTURI, 50);

	set_afraid(0);
	set_confused(0);

	if (level >= 20)
	{
		do_res_stat(A_WIS, true);
		do_res_stat(A_INT, true);
	}
            
	if (level >= 30)
	{
		set_image(0);
		heal_insanity(p_ptr->msane * level / 100);
	}
            
	return CAST;
}

std::string mandos_spirit_of_the_feanturi_info()
{
	s32b level = get_level_s(MANDOS_SPIRIT_FEANTURI, 50) ;
	if (level >= 20)
	{
		return fmt::format("heals {}%", level);
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

casting_result mandos_tale_of_doom_spell()
{
	set_tim_precognition(tale_of_doom_duration());
	return CAST;
}

std::string mandos_tale_of_doom_info()
{
	return fmt::format(
		"dur {}",
		tale_of_doom_duration());
}

int call_to_the_halls_mlev()
{
	return 20 + get_level(MANDOS_CALL_HALLS, 70);
}

casting_result mandos_call_to_the_halls_spell()
{
	int y, x;
	s16b m_idx;
	std::vector<int> summons {
		test_monster_name("Experienced spirit"),
		test_monster_name("Wise spirit")
	};

	int r_idx = summons[rand_int(summons.size())];
	assert(r_idx >= 0);

	find_position(p_ptr->py, p_ptr->px, &y, &x);
	m_idx = place_monster_one(y, x, r_idx, 0, false, MSTATUS_FRIEND);
	if (m_idx)
	{
		monster_set_level(m_idx, call_to_the_halls_mlev());
		return CAST;
	}
	return NO_CAST;
}

std::string mandos_call_to_the_halls_info()
{
	return fmt::format(
		"level {}",
		call_to_the_halls_mlev());
}

static void get_belegaer_damage(int *dice, int *sides)
{
	*dice = get_level_s(ULMO_BELEGAER, 10);
	*sides = 3 + get_level_s(ULMO_BELEGAER, 35);
}

casting_result ulmo_song_of_belegaer_spell()
{
	int dir, dice, sides;

	if (!get_aim_dir(&dir))
	{
		return NO_CAST;
	}

	get_belegaer_damage(&dice, &sides);
	fire_bolt_or_beam(
		2 * get_level_s(ULMO_BELEGAER, 85),
		GF_WATER,
		dir,
		damroll(dice, sides));
	return CAST;
}

std::string ulmo_song_of_belegaer_info()
{
	int dice, sides;
	get_belegaer_damage(&dice, &sides);

	return fmt::format(
		"dam {}d{}",
		dice,
		sides);
}

int draught_of_ulmonan_hp()
{
	return 5 * get_level_s(ULMO_DRAUGHT_ULMONAN, 50);
}

casting_result ulmo_draught_of_ulmonan_spell()
{
	s32b level = get_level_s(ULMO_DRAUGHT_ULMONAN, 50);

	hp_player(draught_of_ulmonan_hp());

	set_poisoned(0);
	set_cut(0);
	set_stun(0);
	set_blind(0);

	if (level >= 10)
	{
		do_res_stat(A_STR, true);
		do_res_stat(A_CON, true);
		do_res_stat(A_DEX, true);
	}

	if (level >= 20)
	{
		set_parasite(0, 0);
		set_mimic(0, 0, 0);
	}

	return CAST;
}

std::string ulmo_draught_of_ulmonan_info()
{
	return fmt::format(
		"cure {}",
		draught_of_ulmonan_hp());
}

static int call_of_the_ulumuri_mlev()
{
	return 30 + get_level(ULMO_CALL_ULUMURI, 70);
}

casting_result ulmo_call_of_the_ulumuri_spell()
{
	int x,y;
	s16b m_idx;
	std::vector<int> summons {
		test_monster_name("Water spirit"),
		test_monster_name("Water elemental")
	};

	int r_idx = summons[rand_int(summons.size())];
	assert(r_idx >= 0);

	find_position(p_ptr->py, p_ptr->px, &y, &x);

	m_idx = place_monster_one(y, x, r_idx, 0, false, MSTATUS_FRIEND);
	if (m_idx)
	{
		monster_set_level(m_idx, call_of_the_ulumuri_mlev());
		return CAST;
	}
	else
	{
		return NO_CAST;
	}
}

std::string ulmo_call_of_the_ulumuri_info()
{
	return fmt::format(
		"level {}",
		call_of_the_ulumuri_mlev());
}

static int wrath_of_ulmo_damage()
{
	return 40 + get_level_s(ULMO_WRATH, 150);
}

static int wrath_of_ulmo_duration()
{
	return 10 + get_level_s(ULMO_WRATH, 14);
}

casting_result ulmo_wrath_of_ulmo_spell()
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
	return CAST;
}

std::string ulmo_wrath_of_ulmo_info()
{
	return fmt::format(
		"dam {} dur {}",
		wrath_of_ulmo_damage(),
		wrath_of_ulmo_duration());
}

static int light_of_valinor_damage()
{
	return 10 + get_level_s(VARDA_LIGHT_VALINOR, 100);
}

static int light_of_valinor_radius()
{
	return 5 + get_level_s(VARDA_LIGHT_VALINOR, 6);
}

casting_result varda_light_of_valinor_spell()
{
	if (get_level_s(VARDA_LIGHT_VALINOR, 50) >= 3)
	{
		lite_area(10, 4);
	}
	else
	{
		lite_room(p_ptr->py, p_ptr->px);
	}

	if (get_level_s(VARDA_LIGHT_VALINOR, 50) >= 15)
	{
		fire_ball(
			GF_LITE,
			0,
			light_of_valinor_damage(),
			light_of_valinor_radius());
	}

	return CAST;
}

std::string varda_light_of_valinor_info()
{
	if (get_level_s(VARDA_LIGHT_VALINOR, 50) >= 15)
	{
		return fmt::format(
			"dam {} rad {}",
			light_of_valinor_damage(),
			light_of_valinor_radius());
	}
	else
	{
		return "";
	}
}

casting_result varda_call_of_almaren_spell()
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
	return CAST;
}

casting_result varda_evenstar_spell()
{
	wiz_lite_extra();
	if (get_level_s(VARDA_EVENSTAR, 50) >= 40)
	{
		identify_pack();
	}

	return CAST;
}

static int star_kindler_bursts()
{
	return p_ptr->lev / 5;
}

static int star_kindler_damage()
{
	return 20 + get_level_s(VARDA_STARKINDLER, 100);
}

casting_result varda_star_kindler_spell()
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

	return CAST;
}

std::string varda_star_kindler_info()
{
	return fmt::format(
		"dam {} bursts {} rad 10",
		star_kindler_damage(),
		star_kindler_bursts());
}
