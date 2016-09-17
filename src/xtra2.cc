/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "xtra2.hpp"

#include "artifact_type.hpp"
#include "cave.hpp"
#include "cave_type.hpp"
#include "corrupt.hpp"
#include "dungeon_info_type.hpp"
#include "ego_item_type.hpp"
#include "feature_flag.hpp"
#include "feature_type.hpp"
#include "files.hpp"
#include "gods.hpp"
#include "hook_player_level_in.hpp"
#include "hook_monster_death_in.hpp"
#include "hooks.hpp"
#include "melee2.hpp"
#include "mimic.hpp"
#include "monster1.hpp"
#include "monster2.hpp"
#include "monster3.hpp"
#include "monster_ego.hpp"
#include "monster_race.hpp"
#include "monster_race_flag.hpp"
#include "monster_type.hpp"
#include "notes.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "object_flag.hpp"
#include "object_kind.hpp"
#include "options.hpp"
#include "player_class.hpp"
#include "player_race.hpp"
#include "player_race_flag.hpp"
#include "player_race_mod.hpp"
#include "player_type.hpp"
#include "point.hpp"
#include "randart.hpp"
#include "skill_type.hpp"
#include "skills.hpp"
#include "spells1.hpp"
#include "spells2.hpp"
#include "stats.hpp"
#include "store_info_type.hpp"
#include "tables.hpp"
#include "trap_type.hpp"
#include "util.hpp"
#include "util.h"
#include "variable.h"
#include "variable.hpp"
#include "wilderness_map.hpp"
#include "wilderness_type_info.hpp"
#include "wizard2.hpp"
#include "xtra1.hpp"
#include "z-rand.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <cassert>
#include <fmt/format.h>
#include <type_traits>



using boost::algorithm::iequals;

static void corrupt_corrupted(void);

/*
 * Set "p_ptr->parasite" and "p_ptr->parasite_r_idx"
 * notice observable changes
 */
bool_ set_parasite(int v, int r)
{
	bool_ notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->parasite)
		{
			msg_print("You feel something growing in you.");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->parasite)
		{
			if (magik(80))
			{
				char r_name[80];
				int wx, wy;
				int attempts = 500;

				monster_race_desc(r_name, p_ptr->parasite_r_idx, 0);

				do
				{
					scatter(&wy, &wx, p_ptr->py, p_ptr->px, 10);
				}
				while (!(in_bounds(wy, wx) && cave_floor_bold(wy, wx)) && --attempts);

				if (place_monster_one(wy, wx, p_ptr->parasite_r_idx, 0, FALSE, MSTATUS_ENEMY))
				{
					cmsg_format(TERM_L_BLUE, "Your body convulses and spawns %s.", r_name);
					p_ptr->food -= 750;
					if (p_ptr->food < 100) p_ptr->food = 100;
				}
			}
			else
			{
				cmsg_print(TERM_L_BLUE, "The hideous thing growing in you seems to die.");
			}
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->parasite = v;
	p_ptr->parasite_r_idx = r;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	disturb_on_state();

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Result */
	return (TRUE);
}

/*
 * Set a simple player field.
 */
static bool_ set_simple_field(
	s16b *p_field,
	s16b v,
	byte activate_color,
	cptr activate_msg,
	byte deactivate_color,
	cptr deactivate_msg)
{
	bool_ notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!*p_field)
		{
			cmsg_print(activate_color, activate_msg);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (*p_field)
		{
			cmsg_print(deactivate_color, deactivate_msg);
			notice = TRUE;
		}
	}

	/* Use the value */
	*p_field = v;

	/* Nothing to notice */
	if (!notice)
		return (FALSE);

	/* Disturb */
	disturb_on_state();

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->tim_project" and others
 * notice observable changes
 */
bool_ set_project(int v, s16b gf, s16b dam, s16b rad, s16b flag)
{
	bool_ notice = set_simple_field(
		&p_ptr->tim_project, v,
		TERM_WHITE, "Your weapon starts glowing.",
		TERM_WHITE, "Your weapon stops glowing.");

	/* Use the values */
	p_ptr->tim_project_gf = gf;
	p_ptr->tim_project_dam = dam;
	p_ptr->tim_project_rad = rad;
	p_ptr->tim_project_flag = flag;

	/* Result */
	return notice;
}

/*
 * Set "p_ptr->tim_roots" and others
 * notice observable changes
 */
bool_ set_roots(int v, s16b ac, s16b dam)
{
	bool_ notice = set_simple_field(
		&p_ptr->tim_roots, v,
		TERM_WHITE, "Roots dive into the floor from your feet.",
		TERM_WHITE, "The roots of your feet suddenly vanish.");

	/* Use the values */
	p_ptr->tim_roots_dam = dam;
	p_ptr->tim_roots_ac = ac;

	/* Result */
	return notice;
}

/*
 * Set "p_ptr->tim_(magic|water)_breath" and others
 * notice observable changes
 */
bool_ set_tim_breath(int v, bool_ magical)
{
	if (magical)
	{
		return set_simple_field(
			&p_ptr->tim_magic_breath, v,
			TERM_WHITE, "Air seems to fill your lungs without breathing.",
			TERM_WHITE, "You need to breathe again.");
	}
	else
	{
		return set_simple_field(
			&p_ptr->tim_water_breath, v,
			TERM_WHITE, "Water seems to fill your lungs.",
			TERM_WHITE, "The water filling your lungs evaporates.");
	}
}

/*
 * Set timered precognition
 */
bool_ set_tim_precognition(int v)
{
	return set_simple_field(
		&p_ptr->tim_precognition, v,
		TERM_WHITE, "You feel able to predict the future.",
		TERM_WHITE, "You feel less able to predict the future.");
}

/*
 * Set "p_ptr->absorb_soul"
 * notice observable changes
 */
bool_ set_absorb_soul(int v)
{
	return set_simple_field(
		&p_ptr->absorb_soul, v,
		TERM_L_DARK, "You start absorbing the souls of your foes.",
		TERM_L_DARK, "You stop absorbing the souls of dead foes.");
}

/*
 * Set "p_ptr->disrupt_shield"
 * notice observable changes
 */
bool_ set_disrupt_shield(int v)
{
	return set_simple_field(
		&p_ptr->disrupt_shield, v,
		TERM_L_BLUE, "You feel invulnerable.",
		TERM_L_RED, "You are more vulnerable.");
}

/*
 * Set "p_ptr->prob_travel"
 * notice observable changes
 */
bool_ set_prob_travel(int v)
{
	return set_simple_field(
		&p_ptr->prob_travel, v,
		TERM_WHITE, "You feel instable.",
		TERM_WHITE, "You are more stable.");
}

/*
 * Set "p_ptr->tim_invis", and "p_ptr->tim_inv_pow",
 * notice observable changes
 */
bool_ set_invis(int v, int p)
{
	bool_ notice = set_simple_field(
		&p_ptr->tim_invisible, v,
		TERM_WHITE, "You feel your body fade away.",
		TERM_WHITE, "You are no longer invisible.");

	/* Use the power value */
	p_ptr->tim_inv_pow = p;

	/* Result */
	return notice;
}

/*
 * Set "p_ptr->tim_poison",
 * notice observable changes
 */
bool_ set_poison(int v)
{
	return set_simple_field(
		&p_ptr->tim_poison, v,
		TERM_WHITE, "Your hands are dripping with venom.",
		TERM_WHITE, "The venom source dries out.");
}

/*
 * Set "no_breeds"
 */
bool_ set_no_breeders(int v)
{
	return set_simple_field(
		&no_breeds, v,
		TERM_WHITE, "You feel an anti-sexual aura.",
		TERM_WHITE, "You no longer feel an anti-sexual aura.");
}

/*
 * Set "p_ptr->tim_deadly"
 */
bool_ set_tim_deadly(int v)
{
	return set_simple_field(
		&p_ptr->tim_deadly, v,
		TERM_WHITE, "You feel extremely accurate.",
		TERM_WHITE, "You are suddenly much less accurate.");
}

/*
 * Set "p_ptr->tim_ffall"
 */
bool_ set_tim_ffall(int v)
{
	return set_simple_field(
		&p_ptr->tim_ffall, v,
		TERM_WHITE, "You feel very light.",
		TERM_WHITE, "You are suddenly heavier.");
}

/*
 * Set "p_ptr->tim_fly"
 */
bool_ set_tim_fly(int v)
{
	return set_simple_field(
		&p_ptr->tim_fly, v,
		TERM_WHITE, "You feel able to reach the clouds.",
		TERM_WHITE, "You are suddenly a lot heavier.");
}

/*
 * Set "p_ptr->tim_reflect"
 */
bool_ set_tim_reflect(int v)
{
	return set_simple_field(
		&p_ptr->tim_reflect, v,
		TERM_WHITE, "You start reflecting the world around you.",
		TERM_WHITE, "You stop reflecting.");
}

/*
 * Set "p_ptr->strike"
 */
bool_ set_strike(int v)
{
	return set_simple_field(
		&p_ptr->strike, v,
		TERM_WHITE, "You feel very accurate.",
		TERM_WHITE, "You are no longer very accurate.");
}

/*
 * Set "p_ptr->oppose_cc"
 */
bool_ set_oppose_cc(int v)
{
	return set_simple_field(
		&p_ptr->oppose_cc, v,
		TERM_WHITE, "You feel protected against raw chaos.",
		TERM_WHITE, "You are no longer protected against chaos.");
}

/*
 * Set "p_ptr->tim_mimic", and "p_ptr->mimic_form",
 * notice observable changes
 */
bool_ set_mimic(int v, int p, int level)
{
	bool_ notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->tim_mimic)
		{
			msg_print("You feel your body change.");
			p_ptr->mimic_form = p;
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->tim_mimic)
		{
			msg_print("You are no longer transformed.");
			p_ptr->mimic_form = 0;
			notice = TRUE;
			if (p == resolve_mimic_name("Bear"))
			{
				s_info[SKILL_BEAR].hidden = true;
				select_default_melee();
			}
			p = 0;
		}
	}

	/* Use the value */
	p_ptr->tim_mimic = v;
	p_ptr->mimic_level = level;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	disturb_on_state();

	/* Redraw title */
	p_ptr->redraw |= (PR_FRAME);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BODY | PU_BONUS | PU_SANITY);

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->blind", notice observable changes
 *
 * Note the use of "PU_UN_VIEW", which is needed to memorize any terrain
 * features which suddenly become "visible".
 * Note that blindness is currently the only thing which can affect
 * "player_can_see_bold()".
 */
bool_ set_blind(int v)
{
	bool_ notice = set_simple_field(
		&p_ptr->blind, v,
		TERM_WHITE, "You are blind!",
		TERM_WHITE, "You can see again.");

	if (notice)
	{
		/* Fully update the visuals */
		p_ptr->update |= (PU_UN_VIEW | PU_VIEW | PU_MONSTERS | PU_MON_LITE);

		/* Redraw map */
		p_ptr->redraw |= (PR_MAP);

		/* Redraw the "blind" */
		p_ptr->redraw |= (PR_FRAME);

		/* Window stuff */
		p_ptr->window |= (PW_OVERHEAD);

		/* Handle stuff */
		handle_stuff();
	}

	/* Result */
	return notice;
}

/*
 * Set "p_ptr->tim_lite", notice observable changes
 *
 * Note the use of "PU_VIEW", which is needed to
 * memorize any terrain features which suddenly become "visible".
 * Note that blindness is currently the only thing which can affect
 * "player_can_see_bold()".
 */
bool_ set_lite(int v)
{
	bool_ notice = set_simple_field(
		&p_ptr->tim_lite, v,
		TERM_WHITE, "You suddenly seem brighter!",
		TERM_WHITE, "You are no longer bright.");

	if (notice)
	{
		/* Fully update the visuals */
		p_ptr->update |= (PU_VIEW | PU_MONSTERS);

		/* Redraw map */
		p_ptr->redraw |= (PR_MAP);

		/* Window stuff */
		p_ptr->window |= (PW_OVERHEAD);

		/* Handle stuff */
		handle_stuff();
	}

	/* Result */
	return notice;
}

/*
 * Set "p_ptr->confused", notice observable changes
 */
bool_ set_confused(int v)
{
	bool_ notice = 
		set_simple_field(
			&p_ptr->confused, v,
			TERM_WHITE, "You are confused!",
			TERM_WHITE, "You feel less confused now.");

	if (notice)
	{
		/* Redraw the "confused" */
		p_ptr->redraw |= (PR_FRAME);

		/* Handle stuff */
		handle_stuff();
	}

	/* Result */
	return notice;
}


/*
 * Set "p_ptr->poisoned", notice observable changes
 */
bool_ set_poisoned(int v)
{
	bool_ notice = set_simple_field(
		&p_ptr->poisoned, v,
		TERM_WHITE, "You are poisoned!",
		TERM_WHITE, "You are no longer poisoned.");

	if (notice)
	{
		/* Redraw the "poisoned" */
		p_ptr->redraw |= (PR_FRAME);

		/* Handle stuff */
		handle_stuff();
	}

	/* Result */
	return notice;
}


/*
 * Set "p_ptr->afraid", notice observable changes
 */
bool_ set_afraid(int v)
{
	bool_ notice = set_simple_field(
		&p_ptr->afraid, v,
		TERM_WHITE, "You are terrified!",
		TERM_WHITE, "You feel bolder now.");

	if (notice)
	{
		/* Redraw the "afraid" */
		p_ptr->redraw |= (PR_FRAME);

		/* Handle stuff */
		handle_stuff();
	}

	/* Result */
	return notice;
}


/*
 * Mechanics for setting the "paralyzed" field.
 */
static bool_ set_paralyzed_aux(int v)
{
	bool_ notice;

	/* Normal processing */
	notice = set_simple_field(
		&p_ptr->paralyzed, v,
		TERM_WHITE, "You are paralyzed!",
		TERM_WHITE, "You can move again.");

	if (notice)
	{
		/* Redraw the state */
		p_ptr->redraw |= (PR_FRAME);

		/* Handle stuff */
		handle_stuff();
	}

	/* Result */
	return notice;
}

/*
 * Set "p_ptr->paralyzed", notice observable changes
 */
bool_ set_paralyzed(int v)
{
	/* Paralysis effects do not accumulate -- this is to
	   prevent the uninteresting insta-death effect, but
	   still leave paralyzers highly dangerous if they're
	   faster than the player. */

	if (p_ptr->paralyzed > 0) {
		return FALSE;
	}

	/* Normal processing */
	return set_paralyzed_aux(v);
}

/*
 * Decrement "p_ptr->paralyzed", notice observable changes
 */
void dec_paralyzed()
{
	set_paralyzed_aux(p_ptr->paralyzed - 1);
}


/*
 * Set "p_ptr->image", notice observable changes
 *
 * Note that we must redraw the map when hallucination changes.
 */
bool_ set_image(int v)
{
	bool_ notice = set_simple_field(
		&p_ptr->image, v,
		TERM_WHITE, "Oh, wow! Everything looks so cosmic now!",
		TERM_WHITE, "You can see clearly again.");

	if (notice)
	{
		/* Redraw map */
		p_ptr->redraw |= (PR_MAP);

		/* Update monsters */
		p_ptr->update |= (PU_MONSTERS);

		/* Window stuff */
		p_ptr->window |= (PW_OVERHEAD | PW_M_LIST);

		/* Handle stuff */
		handle_stuff();
	}

	/* Result */
	return notice;
}

/*
 * Set "p_ptr->lightspeed", notice observable changes
 */
bool_ set_light_speed(int v)
{
	bool_ notice = 
		set_simple_field(
			&p_ptr->lightspeed, v,
			TERM_WHITE, "You feel as if time has stopped!",
			TERM_WHITE, "You feel time returning to its normal rate.");

	if (notice)
	{
		/* Handle stuff */
		handle_stuff();
	}

	/* Result */
	return notice;
}

bool_ set_fast(int v, int p)
{
	bool_ notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->fast)
		{
			msg_print("You feel yourself moving faster!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->fast)
		{
			msg_print("You feel yourself slow down.");
			p = 0;
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->fast = v;
	p_ptr->speed_factor = p;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	disturb_on_state();

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->slow", notice observable changes
 */
bool_ set_slow(int v)
{
	bool_ notice = set_simple_field(
		&p_ptr->slow, v,
		TERM_WHITE, "You feel yourself moving slower!",
		TERM_WHITE, "You feel yourself speed up.");

	if (notice)
	{
		/* Handle stuff */
		handle_stuff();
	}

	/* Result */
	return notice;
}


/*
 * Set "p_ptr->shield", notice observable changes
 */
bool_ set_shield(int v, int p, s16b o, s16b d1, s16b d2)
{
	bool_ notice = set_simple_field(
		&p_ptr->shield, v,
		TERM_WHITE, "A mystic shield forms around your body!",
		TERM_WHITE, "Your mystic shield crumbles away.");

	/* Use the values */
	p_ptr->shield_power = p;
	p_ptr->shield_opt = o;
	p_ptr->shield_power_opt = d1;
	p_ptr->shield_power_opt2 = d2;

	/* Notice? */
	if (notice)
	{
		/* Handle stuff */
		handle_stuff();
	}

	/* Result */
	return notice;
}



/*
 * Set "p_ptr->blessed", notice observable changes
 */
bool_ set_blessed(int v)
{
	bool_ notice = set_simple_field(
		&p_ptr->blessed, v,
		TERM_WHITE, "You feel righteous!",
		TERM_WHITE, "The prayer has expired.");

	if (notice)
	{
		/* Handle stuff */
		handle_stuff();
	}

	/* Result */
	return notice;
}


/*
 * Set "p_ptr->hero", notice observable changes
 */
bool_ set_hero(int v)
{
	bool_ notice = set_simple_field(
		&p_ptr->hero, v,
		TERM_WHITE, "You feel like a hero!",
		TERM_WHITE, "The heroism wears off.");

	if (notice)
	{
		/* Recalculate hitpoints */
		p_ptr->update |= (PU_HP);

		/* Handle stuff */
		handle_stuff();
	}

	/* Result */
	return notice;
}

/*
 * Set "p_ptr->holy", notice observable changes
 */
bool_ set_holy(int v)
{
	bool_ notice = set_simple_field(
		&p_ptr->holy, v,
		TERM_WHITE, "You feel a holy aura around you!",
		TERM_WHITE, "The holy aura vanishes.");

	if (notice)
	{
		/* Handle stuff */
		handle_stuff();
	}

	/* Result */
	return notice;
}

/*
 * Set "p_ptr->shero", notice observable changes
 */
bool_ set_shero(int v)
{
	bool_ notice = set_simple_field(
		&p_ptr->shero, v,
		TERM_WHITE, "You feel like a killing machine!",
		TERM_WHITE, "You feel less berserk.");

	if (notice)
	{
		/* Redraw map */
		p_ptr->redraw |= (PR_MAP);

		/* Update monsters */
		p_ptr->update |= (PU_MONSTERS | PU_HP);

		/* Window stuff */
		p_ptr->window |= (PW_OVERHEAD);

		/* Handle stuff */
		handle_stuff();
	}

	/* Result */
	return notice;
}


/*
 * Set "p_ptr->protevil", notice observable changes
 */
bool_ set_protevil(int v)
{
	bool_ notice = set_simple_field(
		&p_ptr->protevil, v,
		TERM_WHITE, "You feel safe from evil!",
		TERM_WHITE, "You no longer feel safe from evil.");

	if (notice)
	{
		/* Handle stuff */
		handle_stuff();
	}

	/* Result */
	return notice;
}

/*
 * Set "p_ptr->set_shadow", notice observable changes
 */
bool_ set_shadow(int v)
{
	bool_ notice = set_simple_field(
		&p_ptr->tim_wraith, v,
		TERM_WHITE, "You leave the physical world and turn into a wraith-being!",
		TERM_WHITE, "You feel opaque.");

	if (notice)
	{
		/* Redraw map */
		p_ptr->redraw |= (PR_MAP);

		/* Update monsters */
		p_ptr->update |= (PU_MONSTERS);

		/* Window stuff */
		p_ptr->window |= (PW_OVERHEAD);

		/* Handle stuff */
		handle_stuff();
	}

	/* Result */
	return notice;
}




/*
 * Set "p_ptr->invuln", notice observable changes
 */
bool_ set_invuln(int v)
{
	bool_ notice = set_simple_field(
		&p_ptr->invuln, v,
		TERM_L_BLUE, "Invulnerability!",
		TERM_L_RED, "The invulnerability wears off.");

	if (notice)
	{
		/* Redraw map */
		p_ptr->redraw |= (PR_MAP);
		
		/* Update monsters */
		p_ptr->update |= (PU_MONSTERS);
		
		/* Window stuff */
		p_ptr->window |= (PW_OVERHEAD);

		/* Handle stuff */
		handle_stuff();
	}

	/* Result */
	return notice;
}



/*
 * Set "p_ptr->tim_esp", notice observable changes
 */
bool_ set_tim_esp(int v)
{
	bool_ notice = set_simple_field(
		&p_ptr->tim_esp, v,
		TERM_WHITE, "You feel your consciousness expand!",
		TERM_WHITE, "Your consciousness contracts again.");

	if (notice)
	{
		/* Update the monsters */
		p_ptr->update |= (PU_MONSTERS);

		/* Handle stuff */
		handle_stuff();
	}

	/* Result */
	return notice;
}

/*
 * Set "p_ptr->tim_thunder", notice observable changes
 */
bool_ set_tim_thunder(int v, int p1, int p2)
{
	bool_ notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->tim_thunder)
		{
			msg_print("The air around you charges with lightning!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->tim_thunder)
		{
			msg_print("The air around you discharges.");
			notice = TRUE;
			p1 = p2 = 0;
		}
	}

	/* Use the value */
	p_ptr->tim_thunder = v;
	p_ptr->tim_thunder_p1 = p1;
	p_ptr->tim_thunder_p2 = p2;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	disturb_on_state();

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Update the monsters */
	p_ptr->update |= (PU_MONSTERS);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->tim_invis", notice observable changes
 */
bool_ set_tim_invis(int v)
{
	bool_ notice = set_simple_field(
		&p_ptr->tim_invis, v,
		TERM_WHITE, "Your eyes feel very sensitive!",
		TERM_WHITE, "Your eyes feel less sensitive.");

	if (notice)
	{
		/* Update the monsters */
		p_ptr->update |= (PU_MONSTERS);

		/* Handle stuff */
		handle_stuff();
	}

	/* Result */
	return notice;
}


/*
 * Set "p_ptr->tim_infra", notice observable changes
 */
bool_ set_tim_infra(int v)
{
	bool_ notice = set_simple_field(
		&p_ptr->tim_infra, v,
		TERM_WHITE, "Your eyes begin to tingle!",
		TERM_WHITE, "Your eyes stop tingling.");

	if (notice)
	{
		/* Update the monsters */
		p_ptr->update |= (PU_MONSTERS);

		/* Handle stuff */
		handle_stuff();
	}

	/* Result */
	return notice;
}


/*
 * Set "p_ptr->oppose_acid", notice observable changes
 */
bool_ set_oppose_acid(int v)
{
	bool_ notice = set_simple_field(
		&p_ptr->oppose_acid, v,
		TERM_WHITE, "You feel resistant to acid!",
		TERM_WHITE, "You feel less resistant to acid.");

	if (notice)
	{
		/* Handle stuff */
		handle_stuff();
	}

	/* Result */
	return notice;
}


/*
 * Set "p_ptr->oppose_elec", notice observable changes
 */
bool_ set_oppose_elec(int v)
{
	bool_ notice = set_simple_field(
		&p_ptr->oppose_elec, v,
		TERM_WHITE, "You feel resistant to electricity!",
		TERM_WHITE, "You feel less resistant to electricity.");

	if (notice)
	{
		/* Handle stuff */
		handle_stuff();
	}

	/* Result */
	return notice;
}


/*
 * Set "p_ptr->oppose_fire", notice observable changes
 */
bool_ set_oppose_fire(int v)
{
	bool_ notice = set_simple_field(
		&p_ptr->oppose_fire, v,
		TERM_WHITE, "You feel resistant to fire!",
		TERM_WHITE, "You feel less resistant to fire.");

	if (notice)
	{
		/* Handle stuff */
		handle_stuff();
	}

	/* Result */
	return notice;
}


/*
 * Set "p_ptr->oppose_cold", notice observable changes
 */
bool_ set_oppose_cold(int v)
{
	bool_ notice = set_simple_field(
		&p_ptr->oppose_cold, v,
		TERM_WHITE, "You feel resistant to cold!",
		TERM_WHITE, "You feel less resistant to cold.");

	if (notice)
	{
		/* Handle stuff */
		handle_stuff();
	}

	/* Result */
	return notice;
}


/*
 * Set "p_ptr->oppose_pois", notice observable changes
 */
bool_ set_oppose_pois(int v)
{
	bool_ notice = set_simple_field(
		&p_ptr->oppose_pois, v,
		TERM_WHITE, "You feel resistant to poison!",
		TERM_WHITE, "You feel less resistant to poison.");

	if (notice)
	{
		/* Handle stuff */
		handle_stuff();
	}

	/* Result */
	return notice;
}


/*
 * Set "p_ptr->tim_regen", notice observable changes
 */
bool_ set_tim_regen(int v, int p)
{
	bool_ notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->tim_regen)
		{
			msg_print("Your body regenerates much more quickly!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->tim_regen)
		{
			p = 0;
			msg_print("Your body regenerates much more slowly.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->tim_regen = v;
	p_ptr->tim_regen_pow = p;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	disturb_on_state();

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->stun", notice observable changes
 *
 * Note the special code to only notice "range" changes.
 */
bool_ set_stun(int v)
{
	int old_aux, new_aux;
	bool_ notice = FALSE;


	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	if (race_flags_p(PR_NO_STUN)) v = 0;

	/* Knocked out */
	if (p_ptr->stun > 100)
	{
		old_aux = 3;
	}

	/* Heavy stun */
	else if (p_ptr->stun > 50)
	{
		old_aux = 2;
	}

	/* Stun */
	else if (p_ptr->stun > 0)
	{
		old_aux = 1;
	}

	/* None */
	else
	{
		old_aux = 0;
	}

	/* Knocked out */
	if (v > 100)
	{
		new_aux = 3;
	}

	/* Heavy stun */
	else if (v > 50)
	{
		new_aux = 2;
	}

	/* Stun */
	else if (v > 0)
	{
		new_aux = 1;
	}

	/* None */
	else
	{
		new_aux = 0;
	}

	/* Increase cut */
	if (new_aux > old_aux)
	{
		/* Describe the state */
		switch (new_aux)
		{
			/* Stun */
		case 1:
			msg_print("You have been stunned.");
			break;

			/* Heavy stun */
		case 2:
			msg_print("You have been heavily stunned.");
			break;

			/* Knocked out */
		case 3:
			msg_print("You have been knocked out.");
			break;
		}

		if (randint(1000) < v || randint(16) == 1)
		{

			msg_print("A vicious blow hits your head.");
			if (randint(3) == 1)
			{
				if (!p_ptr->sustain_int)
				{
					(void) do_dec_stat(A_INT, STAT_DEC_NORMAL);
				}
				if (!p_ptr->sustain_wis)
				{
					(void) do_dec_stat(A_WIS, STAT_DEC_NORMAL);
				}
			}
			else if (randint(2) == 1)
			{
				if (!p_ptr->sustain_int)
				{
					(void) do_dec_stat(A_INT, STAT_DEC_NORMAL);
				}
			}
			else
			{
				if (!p_ptr->sustain_wis)
				{
					(void) do_dec_stat(A_WIS, STAT_DEC_NORMAL);
				}
			}
		}

		/* Notice */
		notice = TRUE;
	}

	/* Decrease cut */
	else if (new_aux < old_aux)
	{
		/* Describe the state */
		switch (new_aux)
		{
			/* None */
		case 0:
			msg_print("You are no longer stunned.");
			disturb_on_state();
			break;
		}

		/* Notice */
		notice = TRUE;
	}

	/* Use the value */
	p_ptr->stun = v;

	/* No change */
	if (!notice) return (FALSE);

	/* Disturb */
	disturb_on_state();

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Redraw the "stun" */
	p_ptr->redraw |= (PR_FRAME);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->cut", notice observable changes
 *
 * Note the special code to only notice "range" changes.
 */
bool_ set_cut(int v)
{
	int old_aux, new_aux;

	bool_ notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	if (race_flags_p(PR_NO_CUT)) v = 0;

	/* Mortal wound */
	if (p_ptr->cut > 1000)
	{
		old_aux = 7;
	}

	/* Deep gash */
	else if (p_ptr->cut > 200)
	{
		old_aux = 6;
	}

	/* Severe cut */
	else if (p_ptr->cut > 100)
	{
		old_aux = 5;
	}

	/* Nasty cut */
	else if (p_ptr->cut > 50)
	{
		old_aux = 4;
	}

	/* Bad cut */
	else if (p_ptr->cut > 25)
	{
		old_aux = 3;
	}

	/* Light cut */
	else if (p_ptr->cut > 10)
	{
		old_aux = 2;
	}

	/* Graze */
	else if (p_ptr->cut > 0)
	{
		old_aux = 1;
	}

	/* None */
	else
	{
		old_aux = 0;
	}

	/* Mortal wound */
	if (v > 1000)
	{
		new_aux = 7;
	}

	/* Deep gash */
	else if (v > 200)
	{
		new_aux = 6;
	}

	/* Severe cut */
	else if (v > 100)
	{
		new_aux = 5;
	}

	/* Nasty cut */
	else if (v > 50)
	{
		new_aux = 4;
	}

	/* Bad cut */
	else if (v > 25)
	{
		new_aux = 3;
	}

	/* Light cut */
	else if (v > 10)
	{
		new_aux = 2;
	}

	/* Graze */
	else if (v > 0)
	{
		new_aux = 1;
	}

	/* None */
	else
	{
		new_aux = 0;
	}

	/* Increase cut */
	if (new_aux > old_aux)
	{
		/* Describe the state */
		switch (new_aux)
		{
			/* Graze */
		case 1:
			msg_print("You have been given a graze.");
			break;

			/* Light cut */
		case 2:
			msg_print("You have been given a light cut.");
			break;

			/* Bad cut */
		case 3:
			msg_print("You have been given a bad cut.");
			break;

			/* Nasty cut */
		case 4:
			msg_print("You have been given a nasty cut.");
			break;

			/* Severe cut */
		case 5:
			msg_print("You have been given a severe cut.");
			break;

			/* Deep gash */
		case 6:
			msg_print("You have been given a deep gash.");
			break;

			/* Mortal wound */
		case 7:
			msg_print("You have been given a mortal wound.");
			break;
		}

		/* Notice */
		notice = TRUE;

		if (randint(1000) < v || randint(16) == 1)
		{
			if (!p_ptr->sustain_chr)
			{
				msg_print("You have been horribly scarred.");

				do_dec_stat(A_CHR, STAT_DEC_NORMAL);
			}
		}
	}

	/* Decrease cut */
	else if (new_aux < old_aux)
	{
		/* Describe the state */
		switch (new_aux)
		{
			/* None */
		case 0:
			msg_print("You are no longer bleeding.");
			disturb_on_state();
			break;
		}

		/* Notice */
		notice = TRUE;
	}

	/* Use the value */
	p_ptr->cut = v;

	/* No change */
	if (!notice) return (FALSE);

	/* Disturb */
	disturb_on_state();

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Redraw the "cut" */
	p_ptr->redraw |= (PR_FRAME);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}

void drop_from_wild()
{
	/* Hack -- Not if player were in normal mode in previous turn */
	if (!p_ptr->old_wild_mode) return;

	if (p_ptr->wild_mode && (!dun_level))
	{
		p_ptr->wilderness_x = p_ptr->px;
		p_ptr->wilderness_y = p_ptr->py;
		change_wild_mode();
		p_ptr->energy = 100;
		energy_use = 0;
	}
}

/*
 * Set "p_ptr->food", notice observable changes
 *
 * The "p_ptr->food" variable can get as large as 20000, allowing the
 * addition of the most "filling" item, Elvish Waybread, which adds
 * 7500 food units, without overflowing the 32767 maximum limit.
 *
 * Perhaps we should disturb the player with various messages,
 * especially messages about hunger status changes.  XXX XXX XXX
 *
 * Digestion of food is handled in "dungeon.c", in which, normally,
 * the player digests about 20 food units per 100 game turns, more
 * when "fast", more when "regenerating", less with "slow digestion",
 * but when the player is "gorged", he digests 100 food units per 10
 * game turns, or a full 1000 food units per 100 game turns.
 *
 * Note that the player's speed is reduced by 10 units while gorged,
 * so if the player eats a single food ration (5000 food units) when
 * full (15000 food units), he will be gorged for (5000/100)*10 = 500
 * game turns, or 500/(100/5) = 25 player turns (if nothing else is
 * affecting the player speed).
 */
bool_ set_food(int v)
{
	int old_aux, new_aux;

	bool_ notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 20000) ? 20000 : (v < 0) ? 0 : v;

	/* Fainting / Starving */
	if (p_ptr->food < PY_FOOD_FAINT)
	{
		old_aux = 0;
	}

	/* Weak */
	else if (p_ptr->food < PY_FOOD_WEAK)
	{
		old_aux = 1;
	}

	/* Hungry */
	else if (p_ptr->food < PY_FOOD_ALERT)
	{
		old_aux = 2;
	}

	/* Normal */
	else if (p_ptr->food < PY_FOOD_FULL)
	{
		old_aux = 3;
	}

	/* Full */
	else if (p_ptr->food < PY_FOOD_MAX)
	{
		old_aux = 4;
	}

	/* Gorged */
	else
	{
		old_aux = 5;
	}

	/* Fainting / Starving */
	if (v < PY_FOOD_FAINT)
	{
		new_aux = 0;
	}

	/* Weak */
	else if (v < PY_FOOD_WEAK)
	{
		new_aux = 1;
	}

	/* Hungry */
	else if (v < PY_FOOD_ALERT)
	{
		new_aux = 2;
	}

	/* Normal */
	else if (v < PY_FOOD_FULL)
	{
		new_aux = 3;
	}

	/* Full */
	else if (v < PY_FOOD_MAX)
	{
		new_aux = 4;
	}

	/* Gorged */
	else
	{
		new_aux = 5;
	}

	/* Food increase */
	if (new_aux > old_aux)
	{
		/* Describe the state */
		switch (new_aux)
		{
			/* Weak */
		case 1:
			msg_print("You are still weak.");
			break;

			/* Hungry */
		case 2:
			msg_print("You are still hungry.");
			break;

			/* Normal */
		case 3:
			msg_print("You are no longer hungry.");
			break;

			/* Full */
		case 4:
			msg_print("You are full!");
			break;

			/* Bloated */
		case 5:
			msg_print("You have gorged yourself!");
			break;
		}

		/* Change */
		notice = TRUE;
	}

	/* Food decrease */
	else if (new_aux < old_aux)
	{
		/* Describe the state */
		switch (new_aux)
		{
			/* Fainting / Starving */
		case 0:
			msg_print("You are getting faint from hunger!");
			drop_from_wild();
			break;

			/* Weak */
		case 1:
			msg_print("You are getting weak from hunger!");
			drop_from_wild();
			break;

			/* Hungry */
		case 2:
			msg_print("You are getting hungry.");
			break;

			/* Normal */
		case 3:
			msg_print("You are no longer full.");
			break;

			/* Full */
		case 4:
			msg_print("You are no longer gorged.");
			break;
		}

		/* Change */
		notice = TRUE;
	}

	/* Use the value */
	p_ptr->food = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	disturb_on_state();

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Redraw hunger */
	p_ptr->redraw |= (PR_FRAME);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
 * Advance experience levels and print experience
 */
void check_experience(void)
{
	int gained = 0;
	bool_ level_corruption = FALSE;


	/* Hack -- lower limit */
	if (p_ptr->exp < 0) p_ptr->exp = 0;

	/* Hack -- lower limit */
	if (p_ptr->max_exp < 0) p_ptr->max_exp = 0;

	/* Hack -- upper limit */
	if (p_ptr->exp > PY_MAX_EXP) p_ptr->exp = PY_MAX_EXP;

	/* Hack -- upper limit */
	if (p_ptr->max_exp > PY_MAX_EXP) p_ptr->max_exp = PY_MAX_EXP;

	/* Hack -- maintain "max" experience */
	if (p_ptr->exp > p_ptr->max_exp) p_ptr->max_exp = p_ptr->exp;

	/* Redraw experience */
	p_ptr->redraw |= (PR_FRAME);

	/* Handle stuff */
	handle_stuff();


	/* Lose levels while possible */
	while ((p_ptr->lev > 1) &&
	                (p_ptr->exp < (player_exp[p_ptr->lev - 2] * p_ptr->expfact / 100L)))
	{
		/* Lose a level */
		p_ptr->lev--;
		gained--;
		lite_spot(p_ptr->py, p_ptr->px);

		/* Update some stuff */
		p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS | PU_SANITY);

		/* Redraw some stuff */
		p_ptr->redraw |= (PR_FRAME);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);

		/* Handle stuff */
		handle_stuff();
	}


	/* Gain levels while possible */
	while ((p_ptr->lev < PY_MAX_LEVEL) && (p_ptr->lev < max_plev) &&
	                (p_ptr->exp >= (player_exp[p_ptr->lev - 1] * p_ptr->expfact / 100L)))
	{
		/* Gain a level */
		p_ptr->lev++;
		gained++;
		lite_spot(p_ptr->py, p_ptr->px);

		/* Save the highest level */
		if (p_ptr->lev > p_ptr->max_plv)
		{
			p_ptr->max_plv = p_ptr->lev;
			if ((race_flags_p(PR_CORRUPT)) &&
			                (randint(3) == 1))
			{
				level_corruption = TRUE;
			}
		}

		/* Message */
		cmsg_format(TERM_L_GREEN, "Welcome to level %d.", p_ptr->lev);

		if (p_ptr->skill_last_level < p_ptr->lev)
		{
			p_ptr->skill_last_level = p_ptr->lev;
			p_ptr->skill_points += modules[game_module_idx].skills.skill_per_level;
			cmsg_format(TERM_L_GREEN, "You can increase %d more skills.", p_ptr->skill_points);
			p_ptr->redraw |= PR_FRAME;
		}

		/* Gain this level's abilities */
		apply_level_abilities(p_ptr->lev);

		/* Note level gain */
		{
			char note[80];

			/* Write note */
			sprintf(note, "Reached level %d", p_ptr->lev);
			add_note(note, 'L');
		}

		/* Update some stuff */
		p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS | PU_SANITY);

		/* Redraw some stuff */
		p_ptr->redraw |= (PR_FRAME);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);

		/* Handle stuff */
		handle_stuff();

		if (level_corruption)
		{
			msg_print("You feel different...");
			corrupt_corrupted();
			level_corruption = FALSE;
		}
	}

	/* Hook it! */
	{
		hook_player_level_in in = { gained };
		process_hooks_new(HOOK_PLAYER_LEVEL, &in, NULL);
	}
}

/*
 * Advance experience levels and print experience
 */
void check_experience_obj(object_type *o_ptr)
{
	/* Hack -- lower limit */
	if (o_ptr->exp < 0) o_ptr->exp = 0;

	/* Hack -- upper limit */
	if (o_ptr->exp > PY_MAX_EXP) o_ptr->exp = PY_MAX_EXP;

	/* Gain levels while possible */
	while ((o_ptr->elevel < PY_MAX_LEVEL) &&
			(o_ptr->exp >= calc_object_need_exp(o_ptr)))
	{
		char buf[100];

		/* Add a level */
		o_ptr->elevel++;

		/* Get object name */
		object_desc(buf, o_ptr, 1, 0);
		cmsg_format(TERM_L_BLUE, "%s gains a level!", buf);

		/* What does it gains ? */
		object_gain_level(o_ptr);
	}
}


/*
 * Gain experience
 */
void gain_exp(s32b amount)
{
	if ((p_ptr->max_exp > 0) && (race_flags_p(PR_CORRUPT)))
	{
		if ((randint(p_ptr->max_exp) < amount) || (randint(12000000) < amount))
		{
			msg_print("You feel different...");
			corrupt_corrupted();
		};
		/* 12,000,000 is equal to double Morgoth's raw XP value (60,000 * his Dlev (100))*/
	};

	/* Gain some experience */
	p_ptr->exp += amount;

	/* Slowly recover from experience drainage */
	if (p_ptr->exp < p_ptr->max_exp)
	{
		/* Gain max experience (20%) (was 10%) */
		p_ptr->max_exp += amount / 5;
	}

	/* Check Experience */
	check_experience();
}


/*
 * Lose experience
 */
void lose_exp(s32b amount)
{
	/* Never drop below zero experience */
	if (amount > p_ptr->exp) amount = p_ptr->exp;

	/* Lose some experience */
	p_ptr->exp -= amount;

	/* Check Experience */
	check_experience();
}




/*
 * Hack -- Return the "automatic coin type" of a monster race
 * Used to allocate proper treasure when "Creeping coins" die
 *
 * XXX XXX XXX Note the use of actual "monster names"
 */
int get_coin_type(std::shared_ptr<monster_race const> r_ptr)
{
	cptr name = r_ptr->name;

	/* Analyze "coin" monsters */
	if (r_ptr->d_char == '$')
	{
		/* Look for textual clues */
		if (strstr(name, " copper ")) return (2);
		if (strstr(name, " silver ")) return (5);
		if (strstr(name, " gold ")) return (10);
		if (strstr(name, " mithril ")) return (16);
		if (strstr(name, " adamantite ")) return (17);

		/* Look for textual clues */
		if (strstr(name, "Copper ")) return (2);
		if (strstr(name, "Silver ")) return (5);
		if (strstr(name, "Gold ")) return (10);
		if (strstr(name, "Mithril ")) return (16);
		if (strstr(name, "Adamantite ")) return (17);
	}

	/* Assume nothing */
	return (0);
}

/*
 * This routine handles the production of corpses/skeletons/heads/skulls
 * when a monster is killed.
 */
void place_corpse(monster_type *m_ptr)
{
	const int x = m_ptr->fx;
	const int y = m_ptr->fy;

	/* Get local object */
	object_type object_type_body;
	object_type *i_ptr = &object_type_body;

	/* Get the race information */
	auto const r_ptr = m_ptr->race();

	/* It has a physical form */
	if (r_ptr->flags & RF_DROP_CORPSE)
	{
		/* Wipe the object */
		object_prep(i_ptr, lookup_kind(TV_CORPSE, SV_CORPSE_CORPSE));

		/* Unique corpses are unique */
		if (r_ptr->flags & RF_UNIQUE)
		{
			object_aware(i_ptr);
			i_ptr->name1 = 201;
		}

		/* Calculate length of time before decay */
		i_ptr->pval = r_ptr->weight + rand_int(r_ptr->weight);

		/* Set weight */
		i_ptr->weight = (r_ptr->weight + rand_int(r_ptr->weight) / 10) + 1;

		/* Remember what we are */
		i_ptr->pval2 = m_ptr->r_idx;

		/* Some hp */
		i_ptr->pval3 = ((maxroll(r_ptr->hdice, r_ptr->hside) + p_ptr->mhp) / 2);
		i_ptr->pval3 -= randint(i_ptr->pval3) / 3;

		i_ptr->found = OBJ_FOUND_MONSTER;
		i_ptr->found_aux1 = m_ptr->r_idx;
		i_ptr->found_aux2 = m_ptr->ego;
		i_ptr->found_aux3 = dungeon_type;
		i_ptr->found_aux4 = level_or_feat(dungeon_type, dun_level);

		/* Drop it in the dungeon */
		drop_near(i_ptr, -1, y, x);
	}

	/* The creature is an animated skeleton. */
	if (!(r_ptr->flags & RF_DROP_CORPSE) && (r_ptr->flags & RF_DROP_SKELETON))
	{
		/* Wipe the object */
		object_prep(i_ptr, lookup_kind(TV_CORPSE, SV_CORPSE_SKELETON));

		/* Unique corpses are unique */
		if (r_ptr->flags & RF_UNIQUE)
		{
			object_aware(i_ptr);
			i_ptr->name1 = 201;
		}

		i_ptr->pval = 0;

		/* Set weight */
		i_ptr->weight = (r_ptr->weight / 4 + rand_int(r_ptr->weight) / 40) + 1;

		/* Remember what we are */
		i_ptr->pval2 = m_ptr->r_idx;

		i_ptr->found = OBJ_FOUND_MONSTER;
		i_ptr->found_aux1 = m_ptr->r_idx;
		i_ptr->found_aux2 = m_ptr->ego;
		i_ptr->found_aux3 = dungeon_type;
		i_ptr->found_aux4 = level_or_feat(dungeon_type, dun_level);

		/* Drop it in the dungeon */
		drop_near(i_ptr, -1, y, x);
	}

}


/*
 * Check if monster race is in a given list. The list
 * must be NULL-terminated.
 */
static bool_ monster_race_in_list_p(monster_type *m_ptr, cptr races[])
{
	int i=0;
	for (i=0; races[i] != NULL; i++)
	{
		if (m_ptr->r_idx == test_monster_name(races[i])) {
			return TRUE;
		}
	}
	/* Not found */
	return FALSE;
}

/*
 * Handle the "death" of a monster (Gods)
 */
static void monster_death_gods(int m_idx, monster_type *m_ptr)
{
	if (p_ptr->pgod == GOD_AULE)
	{
		/* TODO: This should really be a racial flag
		   which can be added to the r_info file. */
		cptr DWARVES[] = {
			"Petty-dwarf",
			"Petty-dwarf mage",
			"Dark dwarven warrior",
			"Dark dwarven smith",
			"Dark dwarven lord",
			"Dark dwarven priest",
			"Dwarven warrior",
			NULL,
		};
		cptr UNIQUE_DWARVES[] = {
			"Nar, the Dwarf",
			"Naugladur, Lord of Nogrod",
			"Telchar the Smith",
			"Fundin Bluecloak",
			"Khim, Son of Mim",
			"Ibun, Son of Mim",
			"Mim, Betrayer of Turin",
			NULL,
		};

		/* Aule dislikes the killing of dwarves */
		if (monster_race_in_list_p(m_ptr, DWARVES))
		{
			inc_piety(GOD_ALL, -20);
		}

		/* ... and UNIQUE dwarves */
		if (monster_race_in_list_p(m_ptr, UNIQUE_DWARVES))
		{
			inc_piety(GOD_ALL, -500);
		}
	}

	if (p_ptr->pgod == GOD_ULMO)
	{
		/* He doesn't like it if you kill these monsters */
		cptr MINOR_RACES[] = {
			"Swordfish",
			"Barracuda",
			"Globefish",
			"Aquatic bear",
			"Pike",
			"Electric eel",
			"Giant crayfish",
			"Mermaid",
			"Leviathan",
			"Box jellyfish",
			"Giant piranha",
			"Piranha",
			"Ocean naga",
			"Whale",
			"Octopus",
			"Giant octopus",
			"Drowned soul",
			"Tiger shark",
			"Hammerhead shark",
			"Great white shark",
			"White shark",
			"Stargazer",
			"Flounder",
			"Giant turtle",
			"Killer whale",
			"Water naga",
			"Behemoth",
			NULL,
		};
		/* These monsters earn higher penalties */
		cptr MAJOR_RACES[] = {
			"Seahorse",
			"Aquatic elven warrior",
			"Aquatic elven mage",
			"Wavelord",
			"The Watcher in the Water",
			NULL,
		};

		if (monster_race_in_list_p(m_ptr, MINOR_RACES))
		{
			inc_piety(GOD_ALL, -20);
		}

		if (monster_race_in_list_p(m_ptr, MAJOR_RACES))
		{
			inc_piety(GOD_ALL, -500);
		}
	}

	if (p_ptr->pgod == GOD_MANDOS)
	{
		cptr MINOR_BONUS_RACES[] = {
			"Vampire",
			"Master vampire",
			"Oriental vampire",
			"Vampire lord",
			"Vampire orc",
			"Vampire yeek",
			"Vampire ogre",
			"Vampire troll",
			"Vampire dwarf",
			"Vampire gnome",
			"Elder vampire",
			NULL,
		};
		cptr MAJOR_BONUS_RACES[] = {
			"Vampire elf",
			"Thuringwethil, the Vampire Messenger",
			NULL,
		};
		cptr MINOR_PENALTY[] = {
			"Dark elf",
			"Dark elven druid",
			"Eol, the Dark Elf",
			"Maeglin, the Traitor of Gondolin",
			"Dark elven mage",
			"Dark elven warrior",
			"Dark elven priest",
			"Dark elven lord",
			"Dark elven warlock",
			"Dark elven sorcerer",
			NULL,
		};
		cptr MEDIUM_PENALTY[] = {
			"Glorfindel of Rivendell",
			"Finrod Felagund",
			"Thranduil, King of the Wood Elves",
			"Aquatic elven warrior",
			"Aquatic elven mage",
			"High-elven ranger",
			"Elven archer",
			NULL,
		};
		cptr MAJOR_PENALTY[] = {
			"Child spirit",
			"Young spirit",
			"Mature spirit",
			"Experienced spirit",
			"Wise spirit",
			NULL,
		};

		if (monster_race_in_list_p(m_ptr, MINOR_BONUS_RACES))
		{
			/* He really likes it if you kill Vampires
			 * (but not the adventurer kind :P) */
			inc_piety(GOD_ALL, 50);
		}

		if (monster_race_in_list_p(m_ptr, MAJOR_BONUS_RACES))
		{
			/* He *loves* it if you kill vampire Elves. He
			 * will also thank you extra kindly if you
			 * kill Thuringwethil */
			inc_piety(GOD_ALL, 200);
		}

		if (monster_race_in_list_p(m_ptr, MINOR_PENALTY))
		{
			/* He doesn't like it if you kill normal Elves
			 * (means more work for him :P) */
			inc_piety(GOD_ALL, -20);
		}

		if (monster_race_in_list_p(m_ptr, MEDIUM_PENALTY))
		{
			/* He hates it if you kill coaligned Elves */
			inc_piety(GOD_ALL, -200);
		}

		if (monster_race_in_list_p(m_ptr, MAJOR_PENALTY))
		{
			/* He *hates* it if you kill the coaligned Spirits */
			inc_piety(GOD_ALL, -1000);
		}
	}
}

/*
 * Handle the "death" of a monster.
 *
 * Disperse treasures centered at the monster location based on the
 * various flags contained in the monster flags fields.
 *
 * Check for "Quest" completion when a quest monster is killed.
 *
 * Note that only the player can induce "monster_death()" on Uniques.
 * Thus (for now) all Quest monsters should be Uniques.
 *
 * Note that monsters can now carry objects, and when a monster dies,
 * it drops all of its objects, which may disappear in crowded rooms.
 */
void monster_death(int m_idx)
{
	monster_type *m_ptr = &m_list[m_idx];

	auto const r_ptr = m_ptr->race();

	bool_ create_stairs = FALSE;
	int force_coin = get_coin_type(r_ptr);

	object_type forge;
	object_type *q_ptr;

	/* Get the location */
	int y = m_ptr->fy;
	int x = m_ptr->fx;

	/* Process the appropriate hooks */
	{
		struct hook_monster_death_in in = { m_idx };
		process_hooks_new(HOOK_MONSTER_DEATH, &in, NULL);
	}

	/* Per-god processing */
	monster_death_gods(m_idx, m_ptr);

	/* If companion dies, take note */
	if (m_ptr->status == MSTATUS_COMPANION) p_ptr->companion_killed++;

	/* Handle reviving if undead */
	if ((p_ptr->necro_extra & CLASS_UNDEAD) && p_ptr->necro_extra2)
	{
		p_ptr->necro_extra2--;

		if (!p_ptr->necro_extra2)
		{
			msg_print("Your death has been avenged -- you return to life.");
			p_ptr->necro_extra &= ~CLASS_UNDEAD;

			/* Display the hitpoints */
			p_ptr->update |= (PU_HP);
			p_ptr->redraw |= (PR_FRAME);

			/* Window stuff */
			p_ptr->window |= (PW_PLAYER);
		}
		else
		{
			msg_format("You still have to kill %d monster%s.", p_ptr->necro_extra2, (p_ptr->necro_extra2 == 1) ? "" : "s");
		}
	}

	/* If the doppleganger die, the variable must be set accordingly */
	if (r_ptr->flags & RF_DOPPLEGANGER) doppleganger = 0;

	/* Need copy of object list since we're going to mutate it */
	auto const object_idxs(m_ptr->hold_o_idxs);

	/* Drop objects being carried */
	for (auto const this_o_idx: object_idxs)
	{
		/* Acquire object */
		object_type * o_ptr = &o_list[this_o_idx];

		/* Paranoia */
		o_ptr->held_m_idx = 0;

		/* Get local object */
		q_ptr = &forge;

		/* Copy the object */
		object_copy(q_ptr, o_ptr);

		/* Delete the object */
		delete_object_idx(this_o_idx);

		/* Drop it */
		drop_near(q_ptr, -1, y, x);
	}

	/* Forget objects */
	m_ptr->hold_o_idxs.clear();

	/* Average dungeon and monster levels */
	object_level = (dun_level + m_ptr->level) / 2;

	/* Mega^2-hack -- destroying the Stormbringer gives it us! */
	if (strstr(r_ptr->name, "Stormbringer"))
	{
		/* Get local object */
		q_ptr = &forge;

		/* Prepare to make the Stormbringer */
		object_prep(q_ptr, lookup_kind(TV_SWORD, SV_BLADE_OF_CHAOS));

		/* Mega-Hack -- Name the sword  */

		q_ptr->artifact_name = "'Stormbringer'";
		q_ptr->to_h = 16;
		q_ptr->to_d = 16;
		q_ptr->ds = 6;
		q_ptr->dd = 6;
		q_ptr->pval = 2;

		q_ptr->art_flags |=
			TR_VAMPIRIC |
			TR_STR |
			TR_CON |
			TR_BLOWS |
			TR_FREE_ACT |
			TR_HOLD_LIFE |
			TR_RES_NEXUS |
			TR_RES_CHAOS |
			TR_RES_NETHER |
			TR_RES_CONF |
			TR_IGNORE_ACID |
			TR_IGNORE_ELEC |
			TR_IGNORE_FIRE |
			TR_IGNORE_COLD |
			TR_NO_TELE |
			TR_CURSED |
			TR_HEAVY_CURSE;

		q_ptr->ident |= IDENT_CURSED;
		if (randint(2) == 1)
		{
			q_ptr->art_flags |= TR_DRAIN_EXP;
		}
		else
		{
			q_ptr->art_flags |= TR_AGGRAVATE;
		}

		q_ptr->found = OBJ_FOUND_MONSTER;
		q_ptr->found_aux1 = m_ptr->r_idx;
		q_ptr->found_aux2 = m_ptr->ego;
		q_ptr->found_aux3 = dungeon_type;
		q_ptr->found_aux4 = level_or_feat(dungeon_type, dun_level);

		/* Drop it in the dungeon */
		drop_near(q_ptr, -1, y, x);
	}

	/*
	 * Mega^3-hack: killing a 'Warrior of the Dawn' is likely to
	 * spawn another in the fallen one's place!
	 */
	else if (strstr(r_ptr->name, "the Dawn"))
	{
		if (!(randint(20) == 13))
		{
			int wy = p_ptr->py, wx = p_ptr->px;
			int attempts = 100;

			do
			{
				scatter(&wy, &wx, p_ptr->py, p_ptr->px, 20);
			}
			while (!(in_bounds(wy, wx) && cave_floor_bold(wy, wx)) && --attempts);

			if (attempts > 0)
			{
				if (is_friend(m_ptr) > 0)
				{
					if (summon_specific_friendly(wy, wx, 100, SUMMON_DAWN, FALSE))
					{
						if (player_can_see_bold(wy, wx))
							msg_print ("A new warrior steps forth!");
					}
				}
				else
				{
					if (summon_specific(wy, wx, 100, SUMMON_DAWN))
					{
						if (player_can_see_bold(wy, wx))
							msg_print ("A new warrior steps forth!");
					}
				}
			}
		}
	}

	/* One more ultra-hack: An Unmaker goes out with a big bang! */
	else if (strstr(r_ptr->name, "Unmaker"))
	{
		int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;
		(void)project(m_idx, 6, y, x, 100, GF_CHAOS, flg);
	}
	/* Pink horrors are replaced with 2 Blue horrors */
	else if (strstr(r_ptr->name, "ink horror"))
	{
		for (int i = 0; i < 2; i++)
		{
			int wy = p_ptr->py, wx = p_ptr->px;
			int attempts = 100;

			do
			{
				scatter(&wy, &wx, p_ptr->py, p_ptr->px, 3);
			}
			while (!(in_bounds(wy, wx) && cave_floor_bold(wy, wx)) && --attempts);

			if (attempts > 0)
			{
				if (summon_specific(wy, wx, 100, SUMMON_BLUE_HORROR))
				{
					if (player_can_see_bold(wy, wx))
						msg_print ("A blue horror appears!");
				}
			}
		}
	}

	/* Mega-Hack -- drop "winner" treasures */
	else if (r_ptr->flags & RF_DROP_CHOSEN)
	{
		if (strstr(r_ptr->name, "Morgoth, Lord of Darkness"))
		{
			/* Get local object */
			q_ptr = &forge;

			/* Mega-Hack -- Prepare to make "Grond" */
			object_prep(q_ptr, lookup_kind(TV_HAFTED, SV_GROND));

			/* Mega-Hack -- Mark this item as "Grond" */
			q_ptr->name1 = ART_GROND;

			/* Mega-Hack -- Actually create "Grond" */
			apply_magic(q_ptr, -1, TRUE, TRUE, TRUE);

			/* Drop it in the dungeon */
			drop_near(q_ptr, -1, y, x);

			/* Get local object */
			q_ptr = &forge;

			/* Mega-Hack -- Prepare to make "Morgoth" */
			object_prep(q_ptr, lookup_kind(TV_CROWN, SV_MORGOTH));

			/* Mega-Hack -- Mark this item as "Morgoth" */
			q_ptr->name1 = ART_MORGOTH;

			/* Mega-Hack -- Actually create "Morgoth" */
			apply_magic(q_ptr, -1, TRUE, TRUE, TRUE);

			q_ptr->found = OBJ_FOUND_MONSTER;
			q_ptr->found_aux1 = m_ptr->r_idx;
			q_ptr->found_aux2 = m_ptr->ego;
			q_ptr->found_aux3 = dungeon_type;
			q_ptr->found_aux4 = level_or_feat(dungeon_type, dun_level);

			/* Drop it in the dungeon */
			drop_near(q_ptr, -1, y, x);
		}
		else if (strstr(r_ptr->name, "Smeagol"))
		{
			/* Get local object */
			q_ptr = &forge;

			object_wipe(q_ptr);

			/* Mega-Hack -- Prepare to make a ring of invisibility */
			object_prep(q_ptr, lookup_kind(TV_RING, SV_RING_INVIS));
			q_ptr->number = 1;

			apply_magic(q_ptr, -1, TRUE, TRUE, FALSE);

			q_ptr->found = OBJ_FOUND_MONSTER;
			q_ptr->found_aux1 = m_ptr->r_idx;
			q_ptr->found_aux2 = m_ptr->ego;
			q_ptr->found_aux3 = dungeon_type;
			q_ptr->found_aux4 = level_or_feat(dungeon_type, dun_level);

			/* Drop it in the dungeon */
			drop_near(q_ptr, -1, y, x);
		}
		else if (r_ptr->flags & RF_NAZGUL)
		{
			/* Get local object */
			q_ptr = &forge;

			object_wipe(q_ptr);

			/* Mega-Hack -- Prepare to make a Ring of Power */
			object_prep(q_ptr, lookup_kind(TV_RING, SV_RING_SPECIAL));
			q_ptr->number = 1;

			apply_magic(q_ptr, -1, TRUE, TRUE, FALSE);

			/* Create a random artifact */
			create_artifact(q_ptr, TRUE, FALSE);

			/* Save the inscription */
			q_ptr->artifact_name = fmt::format("of {}", r_ptr->name);

			q_ptr->found = OBJ_FOUND_MONSTER;
			q_ptr->found_aux1 = m_ptr->r_idx;
			q_ptr->found_aux2 = m_ptr->ego;
			q_ptr->found_aux3 = dungeon_type;
			q_ptr->found_aux4 = level_or_feat(dungeon_type, dun_level);

			/* Drop it in the dungeon */
			drop_near(q_ptr, -1, y, x);
		}
		else
		{
			byte a_idx = r_ptr->artifact_idx;
			int chance = r_ptr->artifact_chance;

			if ((a_idx > 0) && ((randint(99) < chance) || (wizard)))
			{
				if (a_info[a_idx].cur_num == 0)
				{
					artifact_type *a_ptr = &a_info[a_idx];

					/* Get local object */
					q_ptr = &forge;

					/* Wipe the object */
					object_wipe(q_ptr);

					/* Acquire the "kind" index */
					int I_kind = lookup_kind(a_ptr->tval, a_ptr->sval);

					/* Create the artifact */
					object_prep(q_ptr, I_kind);

					/* Save the name */
					q_ptr->name1 = a_idx;

					/* Extract the fields */
					q_ptr->pval = a_ptr->pval;
					q_ptr->ac = a_ptr->ac;
					q_ptr->dd = a_ptr->dd;
					q_ptr->ds = a_ptr->ds;
					q_ptr->to_a = a_ptr->to_a;
					q_ptr->to_h = a_ptr->to_h;
					q_ptr->to_d = a_ptr->to_d;
					q_ptr->weight = a_ptr->weight;

					/* Hack -- acquire "cursed" flag */
					if (a_ptr->flags & TR_CURSED) q_ptr->ident |= (IDENT_CURSED);

					random_artifact_resistance(q_ptr);

					a_info[a_idx].cur_num = 1;

					q_ptr->found = OBJ_FOUND_MONSTER;
					q_ptr->found_aux1 = m_ptr->r_idx;
					q_ptr->found_aux2 = m_ptr->ego;
					q_ptr->found_aux3 = dungeon_type;
					q_ptr->found_aux4 = level_or_feat(dungeon_type, dun_level);

					/* Drop the artifact from heaven */
					drop_near(q_ptr, -1, y, x);
				}
			}
		}
	}

	/* Hack - the protected monsters must be advanged */
	else if (r_ptr->flags & RF_WYRM_PROTECT)
	{
		int xx = x, yy = y;
		int attempts = 100;

		cmsg_print(TERM_VIOLET, "This monster was under the protection of a Great Wyrm of Power!");

		do
		{
			scatter(&yy, &xx, y, x, 6);
		}
		while (!(in_bounds(yy, xx) && cave_floor_bold(yy, xx)) && --attempts);

		place_monster_aux(yy, xx, test_monster_name("Great Wyrm of Power"), FALSE, FALSE, m_ptr->status);
	}

	/* Let monsters explode! */
	for (int i = 0; i < 4; i++)
	{
		if (m_ptr->blow[i].method == RBM_EXPLODE)
		{
			int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;
			int typ = GF_MISSILE;
			int d_dice = m_ptr->blow[i].d_dice;
			int d_side = m_ptr->blow[i].d_side;
			int damage = damroll(d_dice, d_side);

			switch (m_ptr->blow[i].effect)
			{
			case RBE_HURT:
				typ = GF_MISSILE;
				break;
			case RBE_POISON:
				typ = GF_POIS;
				break;
			case RBE_UN_BONUS:
				typ = GF_DISENCHANT;
				break;
			case RBE_UN_POWER:
				typ = GF_MISSILE;
				break;  /* ToDo: Apply the correct effects */
			case RBE_EAT_GOLD:
				typ = GF_MISSILE;
				break;
			case RBE_EAT_ITEM:
				typ = GF_MISSILE;
				break;
			case RBE_EAT_FOOD:
				typ = GF_MISSILE;
				break;
			case RBE_EAT_LITE:
				typ = GF_MISSILE;
				break;
			case RBE_ACID:
				typ = GF_ACID;
				break;
			case RBE_ELEC:
				typ = GF_ELEC;
				break;
			case RBE_FIRE:
				typ = GF_FIRE;
				break;
			case RBE_COLD:
				typ = GF_COLD;
				break;
			case RBE_BLIND:
				typ = GF_MISSILE;
				break;
			case RBE_HALLU:
				typ = GF_CONFUSION;
				break;
			case RBE_CONFUSE:
				typ = GF_CONFUSION;
				break;
			case RBE_TERRIFY:
				typ = GF_MISSILE;
				break;
			case RBE_PARALYZE:
				typ = GF_MISSILE;
				break;
			case RBE_LOSE_STR:
				typ = GF_MISSILE;
				break;
			case RBE_LOSE_DEX:
				typ = GF_MISSILE;
				break;
			case RBE_LOSE_CON:
				typ = GF_MISSILE;
				break;
			case RBE_LOSE_INT:
				typ = GF_MISSILE;
				break;
			case RBE_LOSE_WIS:
				typ = GF_MISSILE;
				break;
			case RBE_LOSE_CHR:
				typ = GF_MISSILE;
				break;
			case RBE_LOSE_ALL:
				typ = GF_MISSILE;
				break;
			case RBE_PARASITE:
				typ = GF_MISSILE;
				break;
			case RBE_SHATTER:
				typ = GF_ROCKET;
				break;
			case RBE_EXP_10:
				typ = GF_MISSILE;
				break;
			case RBE_EXP_20:
				typ = GF_MISSILE;
				break;
			case RBE_EXP_40:
				typ = GF_MISSILE;
				break;
			case RBE_EXP_80:
				typ = GF_MISSILE;
				break;
			case RBE_DISEASE:
				typ = GF_POIS;
				break;
			case RBE_TIME:
				typ = GF_TIME;
				break;
			case RBE_SANITY:
				typ = GF_MISSILE;
				break;
			}

			project(m_idx, 3, y, x, damage, typ, flg);
			break;
		}
	}

	if ((!force_coin) && (magik(10 + get_skill_scale(SKILL_PRESERVATION, 75))) && (!(m_ptr->mflag & MFLAG_NO_DROP)))
		place_corpse(m_ptr);

	/* Create a magical staircase */
	if (create_stairs && (dun_level < d_info[dungeon_type].maxdepth))
	{
		for (int i = -1; i <= 1; i++)
		{
			for (int j = -1; j <= 1; j++)
			{
				if (!(f_info[cave[y + j][x + i].feat].flags & FF_PERMANENT))
				{
					cave_set_feat(y + j, x + i, d_info[dungeon_type].floor1);
				}
			}
		}

		/* Stagger around */
		while (!cave_valid_bold(y, x))
		{
			/* Pick a location */
			int ny, nx;
			scatter(&ny, &nx, y, x, 1);

			/* Stagger */
			y = ny;
			x = nx;
		}

		/* Destroy any objects */
		delete_object(y, x);

		/* Explain the staircase */
		msg_print("A magical staircase appears...");

		/* Create stairs down */
		cave_set_feat(y, x, FEAT_MORE);

		/* Remember to update everything */
		p_ptr->update |= (PU_VIEW | PU_FLOW | PU_MONSTERS);
	}
}




/*
 * Decreases monsters hit points, handling monster death.
 *
 * We return TRUE if the monster has been killed (and deleted).
 *
 * We announce monster death (using an optional "death message"
 * if given, and a otherwise a generic killed/destroyed message).
 *
 * Only "physical attacks" can induce the "You have slain" message.
 * Missile and Spell attacks will induce the "dies" message, or
 * various "specialized" messages.  Note that "You have destroyed"
 * and "is destroyed" are synonyms for "You have slain" and "dies".
 *
 * Hack -- unseen monsters yield "You have killed it." message.
 *
 * Added fear (DGK) and check whether to print fear messages -CWS
 *
 * Genericized name, sex, and capitilization -BEN-
 *
 * Hack -- we "delay" fear messages by passing around a "fear" flag.
 *
 * XXX XXX XXX Consider decreasing monster experience over time, say,
 * by using "(m_exp * m_lev * (m_lev)) / (p_lev * (m_lev + n_killed))"
 * instead of simply "(m_exp * m_lev) / (p_lev)", to make the first
 * monster worth more than subsequent monsters.  This would also need
 * to induce changes in the monster recall code.
 */
bool_ mon_take_hit(int m_idx, int dam, bool_ *fear, cptr note)
{
	monster_type *m_ptr = &m_list[m_idx];
	auto const r_idx = m_ptr->r_idx;
	auto const r_ptr = m_ptr->race();

	/* Redraw (later) if needed */
	if (health_who == m_idx) p_ptr->redraw |= (PR_FRAME);

	/* Some mosnters are immune to death */
	if (r_ptr->flags & RF_NO_DEATH) return FALSE;

	/* Wake it up */
	m_ptr->csleep = 0;

	/* Hurt it */
	m_ptr->hp -= dam;

	/* It is dead now */
	if (m_ptr->hp < 0)
	{
		char m_name[80];

		/* Lets face it, you cannot get rid of a possessor that easily */
		if (m_ptr->possessor)
		{
			ai_deincarnate(m_idx);

			return FALSE;
		}

		/* Extract monster name */
		monster_desc(m_name, m_ptr, 0);

		if ((r_ptr->flags & RF_DG_CURSE) && (randint(2) == 1))
		{
			int curses = 2 + randint(5);

			cmsg_format(TERM_VIOLET, "%^s puts a terrible Morgothian curse on you!", m_name);
			curse_equipment_dg(100, 50);

			do
			{
				activate_dg_curse();
			}
			while (--curses);
		}

		if (r_ptr->flags & RF_CAN_SPEAK)
		{
			char line_got[80];
			/* Dump a message */

			get_rnd_line("mondeath.txt", line_got);
			msg_format("%^s says: %s", m_name, line_got);
		}

		/* Death by Missile/Spell attack */
		if (note)
		{
			cmsg_format(TERM_L_RED, "%^s%s", m_name, note);
		}

		/* Death by physical attack -- invisible monster */
		else if (!m_ptr->ml)
		{
			cmsg_format(TERM_L_RED, "You have killed %s.", m_name);
		}

		/* Death by Physical attack -- non-living monster */
		else if ((r_ptr->flags & RF_DEMON) ||
		                (r_ptr->flags & RF_UNDEAD) ||
		                (r_ptr->flags & RF_STUPID) ||
		                (r_ptr->flags & RF_NONLIVING) ||
		                (strchr("Evg", r_ptr->d_char)))
		{
			cmsg_format(TERM_L_RED, "You have destroyed %s.", m_name);
		}

		/* Death by Physical attack -- living monster */
		else
		{
			cmsg_format(TERM_L_RED, "You have slain %s.", m_name);
		}

		/* Maximum player level */
		const s32b div = p_ptr->max_plv;

		if (m_ptr->status < MSTATUS_FRIEND)
		{
			/* Give some experience for the kill */
			int new_exp = ((long)r_ptr->mexp * m_ptr->level) / div;

			/* Handle fractional experience */
			const s32b new_exp_frac = ((((long)r_ptr->mexp * m_ptr->level) % div)
			                * 0x10000L / div) + p_ptr->exp_frac;

			/* Keep track of experience */
			if (new_exp_frac >= 0x10000L)
			{
				new_exp++;
				p_ptr->exp_frac = new_exp_frac - 0x10000L;
			}
			else
			{
				p_ptr->exp_frac = new_exp_frac;
			}

			/* Gain experience */
			gain_exp(new_exp);
		}

		if (!note)
		{
			/* Access the weapon */
			object_type *o_ptr = &p_ptr->inventory[INVEN_WIELD];
			auto const flags = object_flags(o_ptr);

			/* Can the weapon gain levels ? */
			if ((o_ptr->k_idx) && (flags & TR_LEVELS))
			{
				/* Give some experience for the kill */
				const int new_exp = ((long)r_ptr->mexp * m_ptr->level) / (div * 2);

				/* Gain experience */
				o_ptr->exp += new_exp;
				check_experience_obj(o_ptr);
			}
		}

		/* When the player kills a Unique, it stays dead */
		if (r_ptr->flags & RF_UNIQUE)
		{
			r_ptr->max_num = 0;
		}

		/* Generate treasure */
		monster_death(m_idx);

		/* Eru doesn't appreciate good monster death */
		if (r_ptr->flags & RF_GOOD)
		{
			inc_piety(GOD_ERU, -7 * m_ptr->level);
			inc_piety(GOD_MANWE, -10 * m_ptr->level);
			inc_piety(GOD_MELKOR, 3 * m_ptr->level);
		}
		else
		{
			inc_piety(GOD_MELKOR, 1 + m_ptr->level / 2);
		}

		/* Manwe appreciate evil monster death */
		if (r_ptr->flags & RF_EVIL)
		{
			int inc = std::max(1, m_ptr->level / 2);

			if (praying_to(GOD_MANWE))
			{
				inc_piety(GOD_MANWE, inc);
			}

			inc = std::max(2, inc);

			inc_piety(GOD_TULKAS, inc / 2);

			if (praying_to(GOD_TULKAS))
			{
				inc_piety(GOD_TULKAS, inc / 2);
				if (r_ptr->flags & RF_DEMON)
				{
					inc_piety(GOD_TULKAS, inc);
				}
			}
		}

		/* Yavanna likes when corruption is destroyed */
		if ((r_ptr->flags & RF_NONLIVING) || (r_ptr->flags & RF_DEMON) || (r_ptr->flags & RF_UNDEAD))
		{
			int inc = std::max(1, m_ptr->level / 2);
			inc_piety(GOD_YAVANNA, inc);
		}

		/* Yavanna doesnt like any killing in her name */
		if (praying_to(GOD_YAVANNA))
		{
			int inc = std::max(1, m_ptr->level / 2);
			inc_piety(GOD_YAVANNA, -inc);

			/* Killing animals in her name is a VERY bad idea */
			if (r_ptr->flags & RF_ANIMAL)
				inc_piety(GOD_YAVANNA, -(inc * 3));
		}

		/* SHould we absorb its soul? */
		if (p_ptr->absorb_soul && (!(r_ptr->flags & RF_UNDEAD)) && (!(r_ptr->flags & RF_NONLIVING)))
		{
			msg_print("You absorb the life of the dying soul.");
			hp_player(1 + (m_ptr->level / 2) + get_skill_scale(SKILL_NECROMANCY, 40));
		}

		/*
		* XXX XXX XXX Mega-Hack -- Remove random quest rendered
		* impossible
		*/
		if (r_ptr->flags & RF_UNIQUE)
		{
			int i;

			/* Search for all the random quests */
			for (i = 0; i < MAX_RANDOM_QUEST; i++)
			{
				random_quest *q_ptr = &random_quests[i];

				/* Ignore invalid entries */
				if (q_ptr->type == 0) continue;

				/* It's done */
				if (q_ptr->done) continue;

				/*
				 * XXX XXX XXX Not yet completed quest is
				 * to kill a unique you've just killed
				 */
				if (r_idx == q_ptr->r_idx)
				{
					/* Invalidate it */
					q_ptr->type = 0;
				}
			}
		}

		/* Make note of unique kills */
		if (r_ptr->flags & RF_UNIQUE)
		{
			char note[80];

			/* Write note */
			sprintf(note, "Killed %s", r_ptr->name);

			add_note(note, 'U');
		}

		/* Recall even invisible uniques or winners */
		if (m_ptr->ml || (r_ptr->flags & RF_UNIQUE))
		{
			/* Count kills this life */
			if (r_ptr->r_pkills < MAX_SHORT) r_ptr->r_pkills++;

			/* Hack -- Auto-recall */
			monster_race_track(m_ptr->r_idx, m_ptr->ego);
		}

		/* Delete the monster */
		delete_monster_idx(m_idx);

		/* Not afraid */
		(*fear) = FALSE;

		/* Monster is dead */
		return (TRUE);
	}

	/* Apply fear */
	mon_handle_fear(m_ptr, dam, fear);

	/* Not dead yet */
	return (FALSE);
}


/*
 * Get term size and calculate screen size
 */
void get_screen_size(int *wid_p, int *hgt_p)
{
	Term_get_size(wid_p, hgt_p);
	*hgt_p -= ROW_MAP + 1;
	*wid_p -= COL_MAP + 1;
}

/*
 * Calculates current boundaries
 * Called below.
 */
static void panel_bounds(void)
{
	int wid, hgt;

	/* Retrieve current screen size */
	get_screen_size(&wid, &hgt);

	/* + 24 - 1 - 2 =  + 21 */
	panel_row_max = panel_row_min + hgt - 1;
	panel_row_prt = panel_row_min - ROW_MAP;

	/* Paranoia -- Boundary check */
	if (panel_row_max > cur_hgt - 1) panel_row_max = cur_hgt - 1;

	panel_col_max = panel_col_min + wid - 1;
	panel_col_prt = panel_col_min - COL_MAP;

	/* Paranoia -- Boundary check */
	if (panel_col_max > cur_wid - 1) panel_col_max = cur_wid - 1;
}


/*
 * Handle a request to change the current panel
 *
 * Return TRUE if the panel was changed.
 *
 * Also used in do_cmd_locate()
 */
bool_ change_panel(int dy, int dx)
{
	int y, x;
	int wid, hgt;

	/* Get size */
	get_screen_size(&wid, &hgt);

	/* Apply the motion */
	y = panel_row_min + dy * (hgt / 2);
	x = panel_col_min + dx * (wid / 2);

	/* Calculate bounds */
	if (y > cur_hgt - hgt) y = cur_hgt - hgt;
	if (y < 0) y = 0;
	if (x > cur_wid - wid) x = cur_wid - wid;
	if (x < 0) x = 0;

	/* Handle changes */
	if ((y != panel_row_min) || (x != panel_col_min))
	{
		/* Save the new panel info */
		panel_row_min = y;
		panel_col_min = x;

		/* Recalculate the boundaries */
		panel_bounds();

		/* Update stuff */
		p_ptr->update |= (PU_MONSTERS);

		/* Redraw map */
		p_ptr->redraw |= (PR_MAP);

		/* Handle stuff */
		handle_stuff();

		/* Success */
		return (TRUE);
	}

	/* No changes */
	return (FALSE);
}


/*
 * Given an row (y) and col (x), this routine detects when a move
 * off the screen has occurred and figures new borders. -RAK-
 *
 * "Update" forces a "full update" to take place.
 *
 * The map is reprinted if necessary, and "TRUE" is returned.
 */
void verify_panel(void)
{
	int y = p_ptr->py;
	int x = p_ptr->px;

	int wid, hgt;

	int prow_min;
	int pcol_min;

	int panel_hgt, panel_wid;

	int max_prow_min;
	int max_pcol_min;


	/*
	 * Make sure panel_row/col_max have correct values -- now taken care of
	 * by the hook function below, which eliminates glitches, but does it
	 * in a very hackish way XXX XXX XXX
	 */
	/* panel_bounds(); */

	/* Get size */
	get_screen_size(&wid, &hgt);

	/* Calculate scroll amount */
	panel_hgt = hgt / 2;
	panel_wid = wid / 2;

	/* Upper boundary of panel_row/col_min */
	max_prow_min = cur_hgt - hgt;
	max_pcol_min = cur_wid - wid;

	/* Boundary check */
	if (max_prow_min < 0) max_prow_min = 0;
	if (max_pcol_min < 0) max_pcol_min = 0;

	/* An option: center on player */
	if (options->center_player)
	{
		/* Center vertically */
		prow_min = y - panel_hgt;

		/* Boundary check */
		if (prow_min < 0) prow_min = 0;
		else if (prow_min > max_prow_min) prow_min = max_prow_min;

		/* Center horizontally */
		pcol_min = x - panel_wid;

		/* Boundary check */
		if (pcol_min < 0) pcol_min = 0;
		else if (pcol_min > max_pcol_min) pcol_min = max_pcol_min;
	}

	/* No centering */
	else
	{
		prow_min = panel_row_min;
		pcol_min = panel_col_min;

		/* Scroll screen when 2 grids from top/bottom edge */
		if (y > panel_row_max - 2)
		{
			while (y > prow_min + hgt - 2)
			{
				prow_min += panel_hgt;
			}

			if (prow_min > max_prow_min) prow_min = max_prow_min;
		}

		if (y < panel_row_min + 2)
		{
			while (y < prow_min + 2)
			{
				prow_min -= panel_hgt;
			}

			if (prow_min < 0) prow_min = 0;
		}

		/* Scroll screen when 4 grids from left/right edge */
		if (x > panel_col_max - 4)
		{
			while (x > pcol_min + wid - 4)
			{
				pcol_min += panel_wid;
			}

			if (pcol_min > max_pcol_min) pcol_min = max_pcol_min;
		}

		if (x < panel_col_min + 4)
		{
			while (x < pcol_min + 4)
			{
				pcol_min -= panel_wid;
			}

			if (pcol_min < 0) pcol_min = 0;
		}
	}

	/* Check for "no change" */
	if ((prow_min == panel_row_min) && (pcol_min == panel_col_min)) return;

	/* Save the new panel info */
	panel_row_min = prow_min;
	panel_col_min = pcol_min;

	/* Hack -- optional disturb on "panel change" */
	if (options->disturb_panel && !options->center_player)
	{
		disturb(0);
	}

	/* Recalculate the boundaries */
	panel_bounds();

	/* Update stuff */
	p_ptr->update |= (PU_MONSTERS);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);
}


/*
 * Map resizing whenever the main term changes size
 */
void resize_map(void)
{
	/* Only if the dungeon exists */
	if (!character_dungeon) return;

	/* Mega-Hack -- No panel yet, assume illegal panel */
	panel_row_min = cur_hgt;
	panel_row_max = 0;
	panel_col_min = cur_wid;
	panel_col_max = 0;

	/* Select player panel */
	verify_panel();

	/*
	 * The following should be the same as the main window code
	 * in the do_cmd_redraw()
	 */

	/* Combine and reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Update torch */
	p_ptr->update |= (PU_TORCH);

	/* Update stuff */
	p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS | PU_POWERS |
	                  PU_SANITY | PU_BODY);

	/* Forget and update view */
	p_ptr->update |= (PU_UN_VIEW | PU_VIEW | PU_MONSTERS | PU_MON_LITE);

	/* Redraw everything */
	p_ptr->redraw |= (PR_WIPE | PR_FRAME | PR_MAP);

	/* Hack -- update */
	handle_stuff();

	/* Redraw */
	Term_redraw();

	/* Refresh */
	Term_fresh();
}


/*
 * Redraw a term when it is resized
 */
void resize_window(void)
{
	/* Only if the dungeon exists */
	if (!character_dungeon) return;

	/* Hack -- Activate the Angband window for the redraw */
	Term_activate(&term_screen[0]);

	/* Hack -- react to changes */
	Term_xtra(TERM_XTRA_REACT, 0);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

	/* Window stuff */
	p_ptr->window |= (PW_M_LIST | PW_MESSAGE | PW_OVERHEAD |
	                  PW_MONSTER | PW_OBJECT);


	/* Hack -- update */
	handle_stuff();

	/* Refresh */
	Term_fresh();
}




/*
 * Monster health description
 */
static cptr look_mon_desc(int m_idx)
{
	bool_ living = TRUE;

	/* Determine if the monster is "living" (vs "undead") */
	monster_type *m_ptr = &m_list[m_idx];
	auto const r_ptr = m_ptr->race();
	if (r_ptr->flags & RF_UNDEAD) living = FALSE;
	if (r_ptr->flags & RF_DEMON) living = FALSE;
	if (r_ptr->flags & RF_NONLIVING) living = FALSE;
	if (strchr("Egv", r_ptr->d_char)) living = FALSE;


	/* Healthy monsters */
	if (m_ptr->hp >= m_ptr->maxhp)
	{
		/* No damage */
		return (living ? "unhurt" : "undamaged");
	}


	/* Calculate a health "percentage" */
	const int perc = 100L * m_ptr->hp / m_ptr->maxhp;

	if (perc >= 60)
	{
		return (living ? "somewhat wounded" : "somewhat damaged");
	}

	if (perc >= 25)
	{
		return (living ? "wounded" : "damaged");
	}

	if (perc >= 10)
	{
		return (living ? "badly wounded" : "badly damaged");
	}

	return (living ? "almost dead" : "almost destroyed");
}





/*** Targetting Code ***/


/*
 * Determine is a monster makes a reasonable target
 *
 * The concept of "targetting" was stolen from "Morgul" (?)
 *
 * The player can target any location, or any "target-able" monster.
 *
 * Currently, a monster is "target_able" if it is visible, and if
 * the player can hit it with a projection, and the player is not
 * hallucinating.  This allows use of "use closest target" macros.
 *
 * Future versions may restrict the ability to target "trappers"
 * and "mimics", but the semantics is a little bit weird.
 */
static bool target_able(int m_idx)
{
	monster_type *m_ptr = &m_list[m_idx];

	/* Monster must be alive */
	if (!m_ptr->r_idx) return (FALSE);

	/* Monster must be visible */
	if (!m_ptr->ml) return (FALSE);

	/* Monster must be projectable */
	if (!projectable(p_ptr->py, p_ptr->px, m_ptr->fy, m_ptr->fx)) return (FALSE);

	/* Hack -- no targeting hallucinations */
	if (p_ptr->image) return (FALSE);

	/* Dont target pets */
	if (is_friend(m_ptr) > 0) return (FALSE);

	/* Honor flag */
	if (r_info[m_ptr->r_idx].flags & RF_NO_TARGET) return (FALSE);

	/* XXX XXX XXX Hack -- Never target trappers */
	/* if (CLEAR_ATTR && (CLEAR_CHAR)) return (FALSE); */

	/* Assume okay */
	return (TRUE);
}




/*
 * Update (if necessary) and verify (if possible) the target.
 *
 * We return TRUE if the target is "okay" and FALSE otherwise.
 */
bool_ target_okay(void)
{
	/* Accept stationary targets */
	if (target_who < 0) return (TRUE);

	/* Check moving targets */
	if (target_who > 0)
	{
		/* Accept reasonable targets */
		if (target_able(target_who))
		{
			monster_type *m_ptr = &m_list[target_who];

			/* Acquire monster location */
			target_row = m_ptr->fy;
			target_col = m_ptr->fx;

			/* Good target */
			return (TRUE);
		}
	}

	/* Assume no target */
	return (FALSE);
}



/*
 * Hack -- help "select" a location (see below)
 */
static s16b target_pick(point p, int dy, int dx, std::vector<point> const &points)
{
	int b_i = -1, b_v = 9999;

	/* Scan the locations */
	for (std::size_t i = 0; i < points.size(); i++)
	{
		/* Point 2 */
		int x2 = points[i].x();
		int y2 = points[i].y();

		/* Directed distance */
		int x3 = (x2 - p.x());
		int y3 = (y2 - p.y());

		/* Verify quadrant */
		if (dx && (x3 * dx <= 0)) continue;
		if (dy && (y3 * dy <= 0)) continue;

		/* Absolute distance */
		int x4 = ABS(x3);
		int y4 = ABS(y3);

		/* Verify quadrant */
		if (dy && !dx && (x4 > y4)) continue;
		if (dx && !dy && (y4 > x4)) continue;

		/* Approximate Double Distance */
		int v = ((x4 > y4) ? (x4 + x4 + y4) : (y4 + y4 + x4));

		/* XXX XXX XXX Penalize location */

		/* Track best */
		if ((b_i >= 0) && (v >= b_v)) continue;

		/* Track best */
		b_i = i;
		b_v = v;
	}

	/* Result */
	return (b_i);
}


/*
 * Hack -- determine if a given location is "interesting"
 */
static bool_ target_set_accept(int y, int x)
{
	/* Player grid is always interesting */
	if ((y == p_ptr->py) && (x == p_ptr->px)) return (TRUE);


	/* Handle hallucination */
	if (p_ptr->image) return (FALSE);


	/* Examine the grid */
	cave_type *c_ptr = &cave[y][x];

	/* Visible monsters */
	if (c_ptr->m_idx && c_ptr->m_idx < max_r_idx)
	{

		monster_type *m_ptr = &m_list[c_ptr->m_idx];
		/* Visible monsters */
		if (m_ptr->ml) return (TRUE);
	}

	/* Scan all objects in the grid */
	for (auto const this_o_idx: c_ptr->o_idxs)
	{
		/* Acquire object */
		object_type *o_ptr = &o_list[this_o_idx];

		/* Memorized object */
		if (o_ptr->marked)
		{
			return (TRUE);
		}
	}

	/* Interesting memorized features */
	if (c_ptr->info & (CAVE_MARK))
	{
		/* Traps are interesting */
		if (c_ptr->info & (CAVE_TRDT)) return (TRUE);

		/* Hack -- Doors are boring */
		if (c_ptr->feat == FEAT_OPEN) return (FALSE);
		if (c_ptr->feat == FEAT_BROKEN) return (FALSE);
		if ((c_ptr->feat >= FEAT_DOOR_HEAD) &&
		                (c_ptr->feat <= FEAT_DOOR_TAIL)) return (FALSE);

		/* Accept 'naturally' interesting features */
		if (f_info[c_ptr->feat].flags & FF_NOTICE) return (TRUE);
	}

	/* Nope */
	return (FALSE);
}


/*
 * Prepare the "temp" array for "target_set"
 *
 * Return the number of target_able monsters in the set.
 */
static std::vector<point> target_set_prepare(int mode)
{
	std::vector<point> points;

	// Scan the current panel
	for (int y = panel_row_min; y <= panel_row_max; y++)
	{
		for (int x = panel_col_min; x <= panel_col_max; x++)
		{
			cave_type *c_ptr = &cave[y][x];

			// Require "interesting" contents
			if (!target_set_accept(y, x)) continue;

			// Require target_able monsters for "TARGET_KILL"
			if ((mode & (TARGET_KILL)) && !target_able(c_ptr->m_idx)) continue;

			// Save the location
			points.push_back(point(x,y));
		}
	}

	// Sort the points by distance to player; we'll
	// use a stable sort to avoid equidistant targets
	// "swapping" priorities.
	std::stable_sort(
		std::begin(points),
		std::end(points),
		[](point i, point j) -> bool {
			auto di = distance(p_ptr->py, p_ptr->px, i.y(), i.x());
			auto dj = distance(p_ptr->py, p_ptr->px, j.y(), j.x());
			return di < dj;
		}
	);

	return points;
}


bool_ target_object(int y, int x, int mode, cptr info, bool_ *boring,
                   object_type *o_ptr, char *out_val, cptr *s1, cptr *s2, cptr *s3,
                   int *query)
{
	char o_name[80];

	/* Not boring */
	*boring = FALSE;

	/* Obtain an object description */
	object_desc(o_name, o_ptr, TRUE, 3);

	/* Describe the object */
	sprintf(out_val, "%s%s%s%s [%s]", *s1, *s2, *s3, o_name, info);
	prt(out_val, 0, 0);
	move_cursor_relative(y, x);
	*query = inkey();

	/* Always stop at "normal" keys */
	if ((*query != '\r') && (*query != '\n') && (*query != ' ')) return (TRUE);

	/* Sometimes stop at "space" key */
	if ((*query == ' ') && !(mode & (TARGET_LOOK))) return (TRUE);

	/* Change the intro */
	*s1 = "It is ";

	/* Plurals */
	if (o_ptr->number != 1) *s1 = "They are ";

	/* Preposition */
	*s2 = "on ";
	return (FALSE);
}

/*
 * Examine a grid, return a keypress.
 *
 * The "mode" argument contains the "TARGET_LOOK" bit flag, which
 * indicates that the "space" key should scan through the contents
 * of the grid, instead of simply returning immediately.  This lets
 * the "look" command get complete information, without making the
 * "target" command annoying.
 *
 * The "info" argument contains the "commands" which should be shown
 * inside the "[xxx]" text.  This string must never be empty, or grids
 * containing monsters will be displayed with an extra comma.
 *
 * Note that if a monster is in the grid, we update both the monster
 * recall info and the health bar info to track that monster.
 *
 * Eventually, we may allow multiple objects per grid, or objects
 * and terrain features in the same grid. XXX XXX XXX
 *
 * This function must handle blindness/hallucination.
 */
static int target_set_aux(int y, int x, int mode, cptr info)
{
	cave_type *c_ptr = &cave[y][x];

	cptr s1, s2, s3;

	bool_ boring;

	int feat;

	int query;

	char out_val[160];


	/* Repeat forever */
	while (1)
	{
		/* Paranoia */
		query = ' ';

		/* Assume boring */
		boring = TRUE;

		/* Default */
		s1 = "You see ";
		s2 = "";
		s3 = "";

		/* Hack -- under the player */
		if ((y == p_ptr->py) && (x == p_ptr->px))
		{
			/* Description */
			s1 = "You are ";

			/* Preposition */
			s2 = "on ";
		}


		/* Hack -- hallucination */
		if (p_ptr->image)
		{
			cptr name = "something strange";

			/* Display a message */
			sprintf(out_val, "%s%s%s%s [%s]", s1, s2, s3, name, info);
			prt(out_val, 0, 0);
			move_cursor_relative(y, x);
			query = inkey();

			/* Stop on everything but "return" */
			if ((query != '\r') && (query != '\n')) break;

			/* Repeat forever */
			continue;
		}


		/* Actual monsters */
		if (c_ptr->m_idx)
		{
			monster_type *m_ptr = &m_list[c_ptr->m_idx];
			auto const r_ptr = m_ptr->race();

			/* Mimics special treatment -- looks like an object */
			if ((r_ptr->flags & RF_MIMIC) && (m_ptr->csleep))
			{
				/* Acquire object */
				object_type *o_ptr = &o_list[m_ptr->mimic_o_idx()];

				if (o_ptr->marked)
				{
					if (target_object(y, x, mode, info, &boring, o_ptr, out_val, &s1, &s2, &s3, &query))
					{
						break;
					}
				}
			}
			else
			{
				/* Visible */
				if (m_ptr->ml)
				{
					bool_ recall = FALSE;

					char m_name[80];

					/* Not boring */
					boring = FALSE;

					/* Get the monster name ("a kobold") */
					monster_desc(m_name, m_ptr, 0x08);

					/* Hack -- track this monster race */
					monster_race_track(m_ptr->r_idx, m_ptr->ego);

					/* Hack -- health bar for this monster */
					health_track(c_ptr->m_idx);

					/* Hack -- handle stuff */
					handle_stuff();

					/* Interact */
					while (1)
					{
						/* Recall */
						if (recall)
						{
							/* Save */
							character_icky = TRUE;
							Term_save();

							/* Recall on screen */
							screen_roff(m_ptr->r_idx, m_ptr->ego);

							/* Hack -- Complete the prompt (again) */
							Term_addstr( -1, TERM_WHITE, format("  [r,%s]", info));

							/* Command */
							query = inkey();

							/* Restore */
							Term_load();
							character_icky = FALSE;
						}

						/* Normal */
						else
						{
							cptr mstat;

							switch (m_ptr->status)
							{
							case MSTATUS_NEUTRAL:
							case MSTATUS_NEUTRAL_M:
							case MSTATUS_NEUTRAL_P:
								mstat = " (neutral) ";
								break;
							case MSTATUS_PET:
								mstat = " (pet) ";
								break;
							case MSTATUS_FRIEND:
								mstat = " (coaligned) ";
								break;
							case MSTATUS_COMPANION:
								mstat = " (companion) ";
								break;
							default:
								mstat = " ";
								break;
							}
							if (m_ptr->mflag & MFLAG_PARTIAL) mstat = " (partial) ";

							/* Describe, and prompt for recall */
							sprintf(out_val, "%s%s%s%s (level %d, %s%s%s)%s%s[r,%s]",
							        s1, s2, s3, m_name,
							        m_ptr->level, look_mon_desc(c_ptr->m_idx),
								(m_ptr->csleep) ? ", asleep" : "",
							        (m_ptr->mflag & MFLAG_QUEST) ? ", quest" : "",
							        (m_ptr->smart & SM_CLONED ? " (clone)" : ""),
							        (mstat), info);

							prt(out_val, 0, 0);

							/* Place cursor */
							move_cursor_relative(y, x);

							/* Command */
							query = inkey();
						}

						/* Normal commands */
						if (query != 'r') break;

						/* Toggle recall */
						recall = !recall;
					}

					/* Always stop at "normal" keys */
					if ((query != '\r') && (query != '\n') && (query != ' ')) break;

					/* Sometimes stop at "space" key */
					if ((query == ' ') && !(mode & (TARGET_LOOK))) break;

					/* Change the intro */
					s1 = "It is ";

					/* Hack -- take account of gender */
					if (r_ptr->flags & RF_FEMALE) s1 = "She is ";
					else if (r_ptr->flags & RF_MALE) s1 = "He is ";

					/* Use a preposition */
					s2 = "carrying ";

					/* Scan all objects being carried */
					std::size_t i = 0;
					for (; i < m_ptr->hold_o_idxs.size(); i++)
					{
						/* Acquire object */
						auto this_o_idx = m_ptr->hold_o_idxs.at(i);
						object_type *o_ptr = &o_list[this_o_idx];

						/* Obtain an object description */
						char o_name[80];
						object_desc(o_name, o_ptr, TRUE, 3);

						/* Describe the object */
						sprintf(out_val, "%s%s%s%s [%s]", s1, s2, s3, o_name, info);
						prt(out_val, 0, 0);
						move_cursor_relative(y, x);
						query = inkey();

						/* Always stop at "normal" keys */
						if ((query != '\r') && (query != '\n') && (query != ' '))
						{
							break;
						}

						/* Sometimes stop at "space" key */
						if ((query == ' ') && !(mode & (TARGET_LOOK)))
						{
							break;
						}

						/* Change the intro */
						s2 = "also carrying ";
					}

					/* Double break? */
					if (i != m_ptr->hold_o_idxs.size())
					{
						break;
					}

					/* Use a preposition */
					s2 = "on ";
				}
			}
		}

		/* Scan all objects in the grid */
		{
			std::size_t i = 0;
			for (; i < c_ptr->o_idxs.size(); i++)
			{
				/* Acquire object */
				auto this_o_idx = c_ptr->o_idxs.at(i);
				object_type *o_ptr = &o_list[this_o_idx];

				/* Describe it */
				if (o_ptr->marked && target_object(y, x, mode, info, &boring, o_ptr, out_val, &s1, &s2, &s3, &query))
				{
					break;
				}
			}

			/* Double break? */
			if (i != c_ptr->o_idxs.size())
			{
				break;
			}
		}

		/* Actual traps */
		if ((c_ptr->info & (CAVE_TRDT)) && c_ptr->t_idx)
		{
			cptr name = "a trap", s4;

			/* Name trap */
			if (t_info[c_ptr->t_idx].ident)
			{
				s4 = format("(%s)", t_info[c_ptr->t_idx].name);
			}
			else
			{
				s4 = "an unknown trap";
			}

			/* Display a message */
			sprintf(out_val, "%s%s%s%s [%s]", s1, s2, s3, name, s4);
			prt(out_val, 0, 0);
			move_cursor_relative(y, x);
			query = inkey();

			/* Stop on everything but "return" */
			if ((query != '\r') && (query != '\n')) break;

			/* Repeat forever */
			continue;
		}

		/* Feature (apply "mimic") */
		if (c_ptr->mimic)
		{
			feat = c_ptr->mimic;
		}
		else
		{
			feat = f_info[c_ptr->feat].mimic;
		}

		/* Require knowledge about grid, or ability to see grid */
		if (!(c_ptr->info & (CAVE_MARK)) && !player_can_see_bold(y, x))
		{
			/* Forget feature */
			feat = FEAT_NONE;
		}

		/* Terrain feature if needed */
		if (boring || (feat >= FEAT_GLYPH))
		{
			cptr name;

			/* Hack -- special handling for building doors */
			if (feat == FEAT_SHOP)
			{
				name = st_info[c_ptr->special].name;
			}
			else
			{
				name = f_info[feat].name;
			}

			/* Hack -- handle unknown grids */
			if (feat == FEAT_NONE) name = "unknown grid";

			/* Pick a prefix */
			if (*s2 &&
			                (((feat >= FEAT_MINOR_GLYPH) &&
			                  (feat <= FEAT_PATTERN_XTRA2)) ||
			                 (feat == FEAT_DIRT) ||
			                 (feat == FEAT_GRASS) ||
			                 (feat == FEAT_FLOWER))) s2 = "on ";
			else if (*s2 && (feat == FEAT_SMALL_TREES)) s2 = "by ";
			else if (*s2 && (feat >= FEAT_DOOR_HEAD)) s2 = "in ";

			/* Pick proper indefinite article */
			s3 = (is_a_vowel(name[0])) ? "an " : "a ";

			/* Hack -- special introduction for store & building doors */
			if (feat == FEAT_SHOP)
			{
				s3 = "the entrance to the ";
			}

			if ((feat == FEAT_MORE) && c_ptr->special)
			{
				s3 = "";
				name = d_info[c_ptr->special].text;
			}

			if (p_ptr->wild_mode && (feat == FEAT_TOWN))
			{
				auto const &wilderness = *wilderness_ptr;
				auto const &wf = wf_info[wilderness(x, y).feat];

				s3 = "";
				name = format("%s(%s)", wf.name, wf.text);
			}

			if ((feat == FEAT_FOUNTAIN) && (c_ptr->info & CAVE_IDNT))
			{
				int tv, sv;

				if (c_ptr->special <= SV_POTION_LAST)
				{
					tv = TV_POTION;
					sv = c_ptr->special;
				}
				else
				{
					tv = TV_POTION2;
					sv = c_ptr->special - SV_POTION_LAST;
				}

				info = k_info[lookup_kind(tv, sv)].name;
			}

			/* Display a message */
			if (!wizard)
			{
				sprintf(out_val, "%s%s%s%s [%s]", s1, s2, s3, name, info);
			}
			else
			{
				sprintf(out_val, "%s%s%s%s [%s] (%d:%d:%d)",
				        s1, s2, s3, name, info,
				        c_ptr->feat, c_ptr->mimic, c_ptr->special);
			}
			prt(out_val, 0, 0);
			move_cursor_relative(y, x);
			query = inkey();

			/* Always stop at "normal" keys */
			if ((query != '\r') && (query != '\n') && (query != ' ')) break;
		}

		/* Stop on everything but "return" */
		if ((query != '\r') && (query != '\n')) break;
	}

	/* Keep going */
	return (query);
}




/*
 * Handle "target" and "look".
 *
 * Note that this code can be called from "get_aim_dir()".
 *
 * All locations must be on the current panel.  Consider the use of
 * "panel_bounds()" to allow "off-panel" targets, perhaps by using
 * some form of "scrolling" the map around the cursor.  XXX XXX XXX
 * That is, consider the possibility of "auto-scrolling" the screen
 * while the cursor moves around.  This may require changes in the
 * "update_mon()" code to allow "visibility" even if off panel, and
 * may require dynamic recalculation of the "temp" grid set.
 *
 * Hack -- targetting/observing an "outer border grid" may induce
 * problems, so this is not currently allowed.
 *
 * The player can use the direction keys to move among "interesting"
 * grids in a heuristic manner, or the "space", "+", and "-" keys to
 * move through the "interesting" grids in a sequential manner, or
 * can enter "location" mode, and use the direction keys to move one
 * grid at a time in any direction.  The "t" (set target) command will
 * only target a monster (as opposed to a location) if the monster is
 * target_able and the "interesting" mode is being used.
 *
 * The current grid is described using the "look" method above, and
 * a new command may be entered at any time, but note that if the
 * "TARGET_LOOK" bit flag is set (or if we are in "location" mode,
 * where "space" has no obvious meaning) then "space" will scan
 * through the description of the current grid until done, instead
 * of immediately jumping to the next "interesting" grid.  This
 * allows the "target" command to retain its old semantics.
 *
 * The "*", "+", and "-" keys may always be used to jump immediately
 * to the next (or previous) interesting grid, in the proper mode.
 *
 * The "return" key may always be used to scan through a complete
 * grid description (forever).
 *
 * This command will cancel any old target, even if used from
 * inside the "look" command.
 */
bool_ target_set(int mode)
{
	int i, d, m;
	int y = p_ptr->py;
	int x = p_ptr->px;

	bool_ done = FALSE;

	bool_ flag = TRUE;

	char query;

	char info[80];

	int screen_wid, screen_hgt;
	int panel_wid, panel_hgt;

	/* Get size */
	get_screen_size(&screen_wid, &screen_hgt);

	/* Calculate the amount of panel movement */
	panel_hgt = screen_hgt / 2;
	panel_wid = screen_wid / 2;

	/* Cancel target */
	target_who = 0;


	/* Cancel tracking */
	/* health_track(0); */


	/* Prepare the "temp" array */
	std::vector<point> points = target_set_prepare(mode);

	/* Start near the player */
	m = 0;

	/* Interact */
	while (!done)
	{
		/* Interesting grids */
		if (flag && points.size())
		{
			y = points[m].y();
			x = points[m].x();

			/* Access */
			cave_type *c_ptr = &cave[y][x];

			/* Allow target */
			if (target_able(c_ptr->m_idx))
			{
				strcpy(info, "q,t,p,o,+,-,'dir'");
			}

			/* Dis-allow target */
			else
			{
				strcpy(info, "q,p,o,+,-,'dir'");
			}

			/* Describe and Prompt */
			query = target_set_aux(y, x, mode, info);

			/* Cancel tracking */
			/* health_track(0); */

			/* Assume no "direction" */
			d = 0;

			/* Analyze */
			switch (query)
			{
			case ESCAPE:
			case 'q':
				{
					done = TRUE;
					break;
				}

			case 't':
			case '.':
			case '5':
			case '0':
				{
					if (target_able(c_ptr->m_idx))
					{
						health_track(c_ptr->m_idx);
						target_who = c_ptr->m_idx;
						target_row = y;
						target_col = x;
						done = TRUE;
					}
					else
					{
						bell();
					}
					break;
				}

			case ' ':
			case '*':
			case '+':
				{
					if (++m == points.size())
					{
						m = 0;
					}
					break;
				}

			case '-':
				{
					if (m-- == 0)
					{
						m = points.size() - 1;
					}
					break;
				}

			case 'p':
				{
					/* Recenter the map around the player */
					verify_panel();

					/* Update stuff */
					p_ptr->update |= (PU_MONSTERS);

					/* Redraw map */
					p_ptr->redraw |= (PR_MAP);

					/* Window stuff */
					p_ptr->window |= (PW_OVERHEAD);

					/* Handle stuff */
					handle_stuff();

					/* Recalculate interesting grids */
					target_set_prepare(mode);

					y = p_ptr->py;
					x = p_ptr->px;

					/* Fall through... */
				}

			case 'o':
				{
					flag = FALSE;
					break;
				}

			case 'm':
				{
					break;
				}

			default:
				{
					/* Extract the action (if any) */
					d = get_keymap_dir(query);

					if (!d) bell();
					break;
				}
			}

			/* Hack -- move around */
			if (d)
			{
				/* Find a new monster */
				i = target_pick(points[m], ddy[d], ddx[d], points);

				/* Scroll to find interesting grid */
				if (i < 0)
				{
					int dy;
					int dx;

					dy = ddy[d];
					dx = ddx[d];

					/* Note panel change */
					if (change_panel(dy, dx))
					{
						auto const t = points[m];

						/* Recalculate interesting grids */
						target_set_prepare(mode);

						/* Find a new monster */
						i = target_pick(t, dy, dx, points);

						/* Restore panel if needed */
						if (i < 0)
						{
							/* Restore panel */
							change_panel( -dy, -dx);

							/* Recalculate interesting grids */
							target_set_prepare(mode);
						}
					}
				}

				/* Use that grids */
				if (i >= 0) m = i;
			}
		}

		/* Arbitrary grids */
		else
		{
			/* Default prompt */
			strcpy(info, "q,t,p,m,+,-,'dir'");

			/* Describe and Prompt (enable "TARGET_LOOK") */
			query = target_set_aux(y, x, mode | TARGET_LOOK, info);

			/* Cancel tracking */
			/* health_track(0); */

			/* Assume no direction */
			d = 0;

			/* Analyze the keypress */
			switch (query)
			{
			case ESCAPE:
			case 'q':
				{
					done = TRUE;
					break;
				}

			case 't':
			case '.':
			case '5':
			case '0':
				{
					target_who = -1;
					target_row = y;
					target_col = x;
					done = TRUE;
					break;
				}

			case ' ':
			case '*':
			case '+':
			case '-':
				{
					break;
				}

			case 'p':
				{
					y = p_ptr->py;
					x = p_ptr->px;
				}

			case 'o':
				{
					break;
				}

			case 'm':
				{
					flag = TRUE;
					break;
				}

			default:
				{
					/* Extract the action (if any) */
					d = get_keymap_dir(query);

					if (!d) bell();
					break;
				}
			}

			/* Handle "direction" */
			if (d)
			{
				int dy = ddy[d];
				int dx = ddx[d];

				/* Move */
				y += dy;
				x += dx;

				/* Do not move horizontally if unnecessary */
				if (((x < panel_col_min + panel_wid) && (dx > 0)) ||
				                ((x > panel_col_min + panel_wid) && (dx < 0)))
				{
					dx = 0;
				}

				/* Do not move vertically if unnecessary */
				if (((y < panel_row_min + panel_hgt) && (dy > 0)) ||
				                ((y > panel_row_min + panel_hgt) && (dy < 0)))
				{
					dy = 0;
				}
				/* Apply the motion */
				if ((y >= panel_row_min + screen_hgt) ||
				                (y < panel_row_min) ||
				                (x > panel_col_min + screen_wid) ||
				                (x < panel_col_min))
				{
					/* Change panel and recalculate interesting grids */
					if (change_panel(dy, dx)) target_set_prepare(mode);
				}

				/* Boundary checks */
				if (!wizard)
				{
					/* Hack -- Verify y */
					if (y <= 0) y = 1;
					else if (y >= cur_hgt - 1) y = cur_hgt - 2;

					/* Hack -- Verify x */
					if (x <= 0) x = 1;
					else if (x >= cur_wid - 1) x = cur_wid - 2;
				}
				else
				{
					/* Hack -- Verify y */
					if (y < 0) y = 0;
					else if (y > cur_hgt - 1) y = cur_hgt - 1;

					/* Hack -- Verify x */
					if (x < 0) x = 0;
					else if (x > cur_wid - 1) x = cur_wid - 1;
				}
			}
		}
	}

	/* Clear the top line */
	prt("", 0, 0);

	/* Recenter the map around the player */
	verify_panel();

	/* Update stuff */
	p_ptr->update |= (PU_MONSTERS);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);

	/* Handle stuff */
	handle_stuff();

	/* Failure to set target */
	if (!target_who) return (FALSE);

	/* Success */
	return (TRUE);
}



/*
 * Get an "aiming direction" from the user.
 *
 * The "dir" is loaded with 1,2,3,4,6,7,8,9 for "actual direction", and
 * "0" for "current target", and "-1" for "entry aborted".
 *
 * Note that "Force Target", if set, will pre-empt user interaction,
 * if there is a usable target already set.
 *
 * Note that confusion over-rides any (explicit?) user choice.
 */
bool_ get_aim_dir(int *dp)
{
	int dir;

	char command;

	cptr p;

	if (repeat_pull(dp))
	{
		/* Confusion? */

		/* Verify */
		if (!(*dp == 5 && !target_okay()))
		{
			return (TRUE);
		}
	}

	/* Initialize */
	(*dp) = 0;

	/* Global direction */
	dir = command_dir;

	/* Hack -- auto-target if requested */
	if (options->use_old_target && target_okay())
	{
		dir = 5;
	}

	/* Ask until satisfied */
	while (!dir)
	{
		/* Choose a prompt */
		if (!target_okay())
		{
			p = "Direction ('*' to choose a target, Escape to cancel)? ";
		}
		else
		{
			p = "Direction ('5' for target, '*' to re-target, Escape to cancel)? ";
		}

		/* Get a command (or Cancel) */
		if (!get_com(p, &command)) break;

		/* Convert various keys to "standard" keys */
		switch (command)
		{
			/* Use current target */
		case 'T':
		case 't':
		case '.':
		case '5':
		case '0':
			{
				dir = 5;
				break;
			}

			/* Set new target */
		case '*':
			{
				if (target_set(TARGET_KILL)) dir = 5;
				break;
			}

		default:
			{
				/* Extract the action (if any) */
				dir = get_keymap_dir(command);

				break;
			}
		}

		/* Verify requested targets */
		if ((dir == 5) && !target_okay()) dir = 0;

		/* Error */
		if (!dir) bell();
	}

	/* No direction */
	if (!dir) return (FALSE);

	/* Save the direction */
	command_dir = dir;

	/* Check for confusion */
	if (p_ptr->confused)
	{
		/* XXX XXX XXX */
		/* Random direction */
		dir = ddd[rand_int(8)];
	}

	/* Notice confusion */
	if (command_dir != dir)
	{
		/* Warn the user */
		msg_print("You are confused.");
	}

	/* Save direction */
	(*dp) = dir;


	repeat_push(dir);

	/* A "valid" direction was entered */
	return (TRUE);
}



/*
 * Request a "movement" direction (1,2,3,4,6,7,8,9) from the user,
 * and place it into "command_dir", unless we already have one.
 *
 * This function should be used for all "repeatable" commands, such as
 * run, walk, open, close, bash, disarm, spike, tunnel, etc, as well
 * as all commands which must reference a grid adjacent to the player,
 * and which may not reference the grid under the player.  Note that,
 * for example, it is no longer possible to "disarm" or "open" chests
 * in the same grid as the player.
 *
 * Direction "5" is illegal and will (cleanly) abort the command.
 *
 * This function tracks and uses the "global direction", and uses
 * that as the "desired direction", to which "confusion" is applied.
 */
bool_ get_rep_dir(int *dp)
{
	int dir;

	if (repeat_pull(dp))
	{
		return (TRUE);
	}

	/* Initialize */
	(*dp) = 0;

	/* Global direction */
	dir = command_dir;

	/* Get a direction */
	while (!dir)
	{
		char ch;

		/* Get a command (or Cancel) */
		if (!get_com("Direction (Escape to cancel)? ", &ch)) break;

		/* Look up the direction */
		dir = get_keymap_dir(ch);

		/* Oops */
		if (!dir) bell();
	}

	/* Prevent weirdness */
	if (dir == 5) dir = 0;

	/* Aborted */
	if (!dir) return (FALSE);

	/* Save desired direction */
	command_dir = dir;

	/* Apply "confusion" */
	if (p_ptr->confused)
	{
		/* Standard confusion */
		if (rand_int(100) < 75)
		{
			/* Random direction */
			dir = ddd[rand_int(8)];
		}
	}

	/* Notice confusion */
	if (command_dir != dir)
	{
		/* Warn the user */
		msg_print("You are confused.");
	}

	/* Save direction */
	(*dp) = dir;


	repeat_push(dir);

	/* Success */
	return (TRUE);
}


/*
 * old -- from PsiAngband.
 */
bool_ tgt_pt(int *x, int *y)
{
	char ch = 0;
	int d, cu, cv;
	int screen_wid, screen_hgt;
	bool_ success = FALSE;

	*x = p_ptr->px;
	*y = p_ptr->py;

	/* Get size */
	get_screen_size(&screen_wid, &screen_hgt);

	cu = Term->scr->cu;
	cv = Term->scr->cv;
	Term->scr->cu = 0;
	Term->scr->cv = 1;
	msg_print("Select a point and press space.");

	while ((ch != 27) && (ch != ' '))
	{
		move_cursor_relative(*y, *x);
		ch = inkey();
		switch (ch)
		{
		case 27:
			break;
		case ' ':
			success = TRUE;
			break;
		default:
			/* Look up the direction */
			d = get_keymap_dir(ch);

			if (!d) break;

			*x += ddx[d];
			*y += ddy[d];

			/* Hack -- Verify x */
			if ((*x >= cur_wid - 1) || (*x >= panel_col_min + screen_wid)) (*x)--;
			else if ((*x <= 0) || (*x <= panel_col_min)) (*x)++;

			/* Hack -- Verify y */
			if ((*y >= cur_hgt - 1) || (*y >= panel_row_min + screen_hgt)) (*y)--;
			else if ((*y <= 0) || (*y <= panel_row_min)) (*y)++;

			break;
		}
	}

	Term->scr->cu = cu;
	Term->scr->cv = cv;
	Term_fresh();
	return success;
}

/*
 * Set "p_ptr->grace", notice observable changes
 */
void set_grace(s32b v)
{
	if (v < -300000) v = -300000;
	if (v > 300000) v = 300000;
	p_ptr->grace = v;
	p_ptr->update |= PU_BONUS;
	p_ptr->redraw |= (PR_FRAME);
	handle_stuff();
}

static bool_ test_object_wish(char *name, object_type *o_ptr, object_type *forge, const char *what)
{
	int i, j, jb, save_aware;
	char buf[200];

	/* try all objects, this *IS* a very ugly and slow method :( */
	for (i = 0; i < max_k_idx; i++)
	{
		object_kind *k_ptr = &k_info[i];

		o_ptr = forge;

		if (!k_ptr->name) continue;
		if (k_ptr->flags & TR_NORM_ART) continue;
		if (k_ptr->flags & TR_INSTA_ART) continue;
		if (k_ptr->tval == TV_GOLD) continue;

		object_prep(o_ptr, i);
		o_ptr->name1 = 0;
		o_ptr->name2 = 0;
		apply_magic(o_ptr, dun_level, FALSE, FALSE, FALSE);
		/* Hack : aware status must be restored after describing the item name */
		save_aware = k_ptr->aware;
		object_aware(o_ptr);
		object_known(o_ptr);
		object_desc(buf, o_ptr, FALSE, 0);
		strlower(buf);
		k_ptr->aware = save_aware;

		if (strstr(name, buf) ||
		   /* Hack hack hackery */
		   (o_ptr->tval == TV_ROD_MAIN && strstr(name, "rod of")))
		{
			/* try all ego */
			for (j = max_e_idx - 1; j >= 0; j--)
			{
				ego_item_type *e_ptr = &e_info[j];
				bool_ ok = FALSE;

				if (j && !e_ptr->name) continue;

				/* Must have the correct fields */
				if (j)
				{
					int z;

					for (z = 0; z < 6; z++)
					{
						if (e_ptr->tval[z] == k_ptr->tval)
						{
							if ((e_ptr->min_sval[z] <= k_ptr->sval) &&
							                (e_ptr->max_sval[z] >= k_ptr->sval)) ok = TRUE;
						}
						if (ok) break;
					}
					if (!ok)
					{
						continue;
					}
				}

				/* try all ego */
				for (jb = max_e_idx - 1; jb >= 0; jb--)
				{
					ego_item_type *eb_ptr = &e_info[jb];
					bool_ ok = FALSE;

					if (jb && !eb_ptr->name) continue;

					if (j && jb && (e_ptr->before == eb_ptr->before)) continue;

					/* Must have the correct fields */
					if (jb)
					{
						int z;

						for (z = 0; z < 6; z++)
						{
							if (eb_ptr->tval[z] == k_ptr->tval)
							{
								if ((eb_ptr->min_sval[z] <= k_ptr->sval) &&
								                (eb_ptr->max_sval[z] >= k_ptr->sval)) ok = TRUE;
							}
							if (ok) break;
						}
						if (!ok)
						{
							continue;
						}
					}

					object_prep(o_ptr, i);
					o_ptr->name1 = 0;
					o_ptr->name2 = j;
					o_ptr->name2b = jb;
					apply_magic(o_ptr, dun_level, FALSE, FALSE, FALSE);
					object_aware(o_ptr);
					object_known(o_ptr);
					object_desc(buf, o_ptr, FALSE, 0);
					strlower(buf);

					if (iequals(buf, name))
					{
						/* Don't search any more */
						return TRUE;
					}
					else
					{
						/* Restore again the aware status */
						k_ptr->aware = save_aware;
					}
				}
			}
		}
	}
	return FALSE;
}

static void clean_wish_name(char *buf, char *name)
{
	char *p;
	int i, j;

	/* Lowercase the wish */
	strlower(buf);

	/* Nuke uneccesary spaces */
	p = buf;
	while (*p == ' ') p++;
	i = 0;
	j = 0;
	while (p[i])
	{
		if ((p[i] == ' ') && (p[i + 1] == ' '))
		{
			i++;
			continue;
		}
		name[j++] = p[i++];
	}
	name[j++] = '\0';
	if (j)
	{
		j--;
		while (j && (name[j] == ' '))
		{
			name[j] = '\0';
			j--;
		}
	}
}

/*
 * Allow the player to make a wish
 */
void make_wish(void)
{
	char buf[200], name[200], *mname;
	int i, j, mstatus = MSTATUS_ENEMY;
	object_type forge, *o_ptr = &forge;

	/* Make an empty string */
	buf[0] = 0;

	/* Ask for the wish */
	if (!get_string("Wish for what? ", buf, 80)) return;

	clean_wish_name(buf, name);

	/* You can't wish for a wish! */
	if (strstr(name, "wish"))
	{
		msg_print("You can't wish for a wish!");
		return;
	}

	if (test_object_wish(name, o_ptr, &forge, "wish"))
	{
		msg_print("Your wish becomes truth!");

		/* Give it to the player */
		drop_near(o_ptr, -1, p_ptr->py, p_ptr->px);

		return;
	}

	/* try monsters */
	if (prefix(name, "enemy "))
	{
		mstatus = MSTATUS_ENEMY;
		mname = name + 6;
	}
	else if (prefix(name, "neutral "))
	{
		mstatus = MSTATUS_NEUTRAL;
		mname = name + 8;
	}
	else if (prefix(name, "friendly "))
	{
		mstatus = MSTATUS_FRIEND;
		mname = name + 9;
	}
	else if (prefix(name, "pet "))
	{
		mstatus = MSTATUS_PET;
		mname = name + 4;
	}
	else if (prefix(name, "companion "))
	{
		if (can_create_companion()) mstatus = MSTATUS_COMPANION;
		else mstatus = MSTATUS_PET;
		mname = name + 10;
	}
	else mname = name;
	for (i = 1; i < max_r_idx; i++)
	{
		monster_race *r_ptr = &r_info[i];

		if (!r_ptr->name) continue;

		if (r_ptr->flags & RF_SPECIAL_GENE) continue;
		if (r_ptr->flags & RF_NEVER_GENE) continue;
		if (r_ptr->flags & RF_UNIQUE) continue;

		sprintf(buf, "%s", r_ptr->name);
		strlower(buf);

		if (strstr(mname, buf))
		{
			/* try all ego */
			for (j = max_re_idx - 1; j >= 0; j--)
			{
				monster_ego *re_ptr = &re_info[j];

				if (j && !re_ptr->name) continue;

				if (!mego_ok(r_ptr, j)) continue;

				if (j)
				{
					if (re_ptr->before)
					{
						sprintf(buf, "%s %s", re_ptr->name, r_ptr->name);
					}
					else
					{
						sprintf(buf, "%s %s", r_ptr->name, re_ptr->name);
					}
				}
				else
				{
					sprintf(buf, "%s", r_ptr->name);
				}
				strlower(buf);

				if (iequals(mname, buf))
				{
					int wy = p_ptr->py, wx = p_ptr->px;
					int attempts = 100;

					do
					{
						scatter(&wy, &wx, p_ptr->py, p_ptr->px, 5);
					}
					while (!(in_bounds(wy, wx) && cave_floor_bold(wy, wx)) && --attempts);

					/* Create the monster */
					if (place_monster_one(wy, wx, i, j, FALSE, mstatus))
						msg_print("Your wish becomes truth!");

					/* Don't search any more */
					return;
				}
			}
		}
	}
}


/*
 * Corrupted have a 1/3 chance of losing a mutation each time this is called, 
 * assuming they have any in the first place
 */
static void corrupt_corrupted(void)
{
	if (magik(45))
	{
		lose_corruption();
	}
	else
	{
		gain_random_corruption();
	}

	/* We are done. */
	return;
}

/*
 * Change to an other subrace
 */
void switch_subrace(int racem, bool_ copy_old)
{
	if ((racem < 0) && (racem >= max_rmp_idx)) return;

	/* If we switch to the saved subrace, we copy over the old subrace data */
	if (copy_old && (racem == SUBRACE_SAVE))
	{
		// Keep old description
		auto old_desc = race_mod_info[SUBRACE_SAVE].description;
		// Copy everything
		race_mod_info[SUBRACE_SAVE] = race_mod_info[p_ptr->pracem];
		// Reinstate description
		race_mod_info[SUBRACE_SAVE].description = old_desc;
	}

	p_ptr->pracem = racem;
	rmp_ptr = &race_mod_info[p_ptr->pracem];
}

/*
 * Rebirth, recalc hp & exp/level
 */
void do_rebirth()
{
	/* Experience factor */
	p_ptr->expfact = rp_ptr->ps.exp + rmp_ptr->ps.exp + cp_ptr->ps.exp;

	/* Hitdice */
	p_ptr->hitdie = rp_ptr->ps.mhp + rmp_ptr->ps.mhp + cp_ptr->ps.mhp;

	/* Recalc HP */
	do_cmd_rerate();

	/* Change the level if needed */
	check_experience();
	p_ptr->max_plv = p_ptr->lev;

	/* Redraw/calc stuff */
	p_ptr->redraw |= (PR_FRAME);
	p_ptr->update |= (PU_BONUS);
	handle_stuff();

	lite_spot(p_ptr->py, p_ptr->px);
}
