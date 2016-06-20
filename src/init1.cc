#include "init1.hpp"

#include "ability_type.hpp"
#include "artifact_type.hpp"
#include "cave.hpp"
#include "cave_type.hpp"
#include "dungeon_info_type.hpp"
#include "dungeon_flag.hpp"
#include "ego_item_type.hpp"
#include "feature_type.hpp"
#include "files.hpp"
#include "gods.hpp"
#include "hist_type.hpp"
#include "init2.hpp"
#include "meta_class_type.hpp"
#include "monster2.hpp"
#include "monster_ego.hpp"
#include "monster_race.hpp"
#include "monster_spell.hpp"
#include "monster_type.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "object_kind.hpp"
#include "owner_type.hpp"
#include "player_class.hpp"
#include "player_race.hpp"
#include "player_race_mod.hpp"
#include "player_type.hpp"
#include "randart_gen_type.hpp"
#include "randart_part_type.hpp"
#include "set_type.hpp"
#include "skill_type.hpp"
#include "skills.hpp"
#include "spells5.hpp"
#include "store_action_type.hpp"
#include "store_info_type.hpp"
#include "store_type.hpp"
#include "tables.hpp"
#include "town_type.hpp"
#include "trap_type.hpp"
#include "traps.hpp"
#include "util.hpp"
#include "util.h"
#include "variable.h"
#include "variable.hpp"
#include "vault_type.hpp"
#include "wilderness_map.hpp"
#include "wilderness_type_info.hpp"
#include "z-rand.hpp"

#include <boost/algorithm/string/predicate.hpp>

using boost::algorithm::iequals;
using boost::algorithm::ends_with;


/*
 * This file is used to initialize various variables and arrays for the
 * Angband game.  Note the use of "fd_read()" and "fd_write()" to bypass
 * the common limitation of "read()" and "write()" to only 32767 bytes
 * at a time.
 *
 * Several of the arrays for Angband are built from "template" files in
 * the "lib/file" directory, from which quick-load binary "image" files
 * are constructed whenever they are not present in the "lib/data"
 * directory, or if those files become obsolete, if we are allowed.
 *
 * Warning -- the "ascii" file parsers use a minor hack to collect the
 * name and text information in a single pass.  Thus, the game will not
 * be able to load any template file with more than 20K of names or 60K
 * of text, even though technically, up to 64K should be legal.
 */


/*** Helper arrays for parsing ascii template files ***/

/*
 * Monster Blow Methods
 */
static cptr r_info_blow_method[] =
{
	"*",
	"HIT",
	"TOUCH",
	"PUNCH",
	"KICK",
	"CLAW",
	"BITE",
	"STING",
	"XXX1",
	"BUTT",
	"CRUSH",
	"ENGULF",
	"CHARGE",
	"CRAWL",
	"DROOL",
	"SPIT",
	"EXPLODE",
	"GAZE",
	"WAIL",
	"SPORE",
	"XXX4",
	"BEG",
	"INSULT",
	"MOAN",
	"SHOW",
	NULL
};


/*
 * Monster Blow Effects
 */
static cptr r_info_blow_effect[] =
{
	"*",
	"HURT",
	"POISON",
	"UN_BONUS",
	"UN_POWER",
	"EAT_GOLD",
	"EAT_ITEM",
	"EAT_FOOD",
	"EAT_LITE",
	"ACID",
	"ELEC",
	"FIRE",
	"COLD",
	"BLIND",
	"CONFUSE",
	"TERRIFY",
	"PARALYZE",
	"LOSE_STR",
	"LOSE_INT",
	"LOSE_WIS",
	"LOSE_DEX",
	"LOSE_CON",
	"LOSE_CHR",
	"LOSE_ALL",
	"SHATTER",
	"EXP_10",
	"EXP_20",
	"EXP_40",
	"EXP_80",
	"DISEASE",
	"TIME",
	"INSANITY",
	"HALLU",
	"PARASITE",
	"ABOMINATION",
	NULL
};


/*
 * Monster race flags
 */
static cptr r_info_flags1[] =
{
	"UNIQUE",
	"QUESTOR",
	"MALE",
	"FEMALE",
	"CHAR_CLEAR",
	"CHAR_MULTI",
	"ATTR_CLEAR",
	"ATTR_MULTI",
	"FORCE_DEPTH",
	"FORCE_MAXHP",
	"FORCE_SLEEP",
	"FORCE_EXTRA",
	"FRIEND",
	"FRIENDS",
	"ESCORT",
	"ESCORTS",
	"NEVER_BLOW",
	"NEVER_MOVE",
	"RAND_25",
	"RAND_50",
	"ONLY_GOLD",
	"ONLY_ITEM",
	"DROP_60",
	"DROP_90",
	"DROP_1D2",
	"DROP_2D2",
	"DROP_3D2",
	"DROP_4D2",
	"DROP_GOOD",
	"DROP_GREAT",
	"DROP_USEFUL",
	"DROP_CHOSEN"
};

/*
 * Monster race flags
 */
static cptr r_info_flags2[] =
{
	"STUPID",
	"SMART",
	"CAN_SPEAK",
	"REFLECTING",
	"INVISIBLE",
	"COLD_BLOOD",
	"EMPTY_MIND",
	"WEIRD_MIND",
	"DEATH_ORB",
	"REGENERATE",
	"SHAPECHANGER",
	"ATTR_ANY",
	"POWERFUL",
	"ELDRITCH_HORROR",
	"AURA_FIRE",
	"AURA_ELEC",
	"OPEN_DOOR",
	"BASH_DOOR",
	"PASS_WALL",
	"KILL_WALL",
	"MOVE_BODY",
	"KILL_BODY",
	"TAKE_ITEM",
	"KILL_ITEM",
	"BRAIN_1",
	"BRAIN_2",
	"BRAIN_3",
	"BRAIN_4",
	"BRAIN_5",
	"BRAIN_6",
	"BRAIN_7",
	"BRAIN_8"
};

/*
 * Monster race flags
 */
static cptr r_info_flags3[] =
{
	"ORC",
	"TROLL",
	"GIANT",
	"DRAGON",
	"DEMON",
	"UNDEAD",
	"EVIL",
	"ANIMAL",
	"THUNDERLORD",
	"GOOD",
	"AURA_COLD",  /* TODO: Implement aura_cold */
	"NONLIVING",
	"HURT_LITE",
	"HURT_ROCK",
	"SUSCEP_FIRE",
	"SUSCEP_COLD",
	"IM_ACID",
	"IM_ELEC",
	"IM_FIRE",
	"IM_COLD",
	"IM_POIS",
	"RES_TELE",
	"RES_NETH",
	"RES_WATE",
	"RES_PLAS",
	"RES_NEXU",
	"RES_DISE",
	"UNIQUE_4",
	"NO_FEAR",
	"NO_STUN",
	"NO_CONF",
	"NO_SLEEP"
};

/*
 * Monster race flags
 */
static cptr r_info_flags7[] =
{
	"AQUATIC",
	"CAN_SWIM",
	"CAN_FLY",
	"FRIENDLY",
	"PET",
	"MORTAL",
	"SPIDER",
	"NAZGUL",
	"DG_CURSE",
	"POSSESSOR",
	"NO_DEATH",
	"NO_TARGET",
	"AI_ANNOY",
	"AI_SPECIAL",
	"NEUTRAL",
	"DROP_ART",
	"DROP_RANDART",
	"AI_PLAYER",
	"NO_THEFT",
	"SPIRIT",
	"XXX7X20",
	"XXX7X21",
	"XXX7X22",
	"XXX7X23",
	"XXX7X24",
	"XXX7X25",
	"XXX7X26",
	"XXX7X27",
	"XXX7X28",
	"XXX7X29",
	"XXX7X30",
	"XXX7X31",
};

/*
 * Monster race flags
 */
static cptr r_info_flags8[] =
{
	"WILD_ONLY",
	"WILD_TOWN",
	"XXX8X02",
	"WILD_SHORE",
	"WILD_OCEAN",
	"WILD_WASTE",
	"WILD_WOOD",
	"WILD_VOLCANO",
	"XXX8X08",
	"WILD_MOUNTAIN",
	"WILD_GRASS",
	"NO_CUT",
	"CTHANGBAND",
	"XXX8X13",
	"ZANGBAND",
	"JOKEANGBAND",
	"BASEANGBAND",
	"XXX8X17",
	"XXX8X18",
	"XXX8X19",
	"XXX8X20",
	"XXX8X21",
	"XXX8X22",
	"XXX8X23",
	"XXX8X24",
	"XXX8X25",
	"XXX8X26",
	"XXX8X27",
	"XXX8X28",
	"XXX8X29",
	"WILD_SWAMP", 	/* ToDo: Implement Swamp */
	"WILD_TOO",
};


/*
 * Monster race flags - Drops
 */
static cptr r_info_flags9[] =
{
	"DROP_CORPSE",
	"DROP_SKELETON",
	"HAS_LITE",
	"MIMIC",
	"HAS_EGG",
	"IMPRESED",
	"SUSCEP_ACID",
	"SUSCEP_ELEC",
	"SUSCEP_POIS",
	"KILL_TREES",
	"WYRM_PROTECT",
	"DOPPLEGANGER",
	"ONLY_DEPTH",
	"SPECIAL_GENE",
	"NEVER_GENE",
	"XXX9X15",
	"XXX9X16",
	"XXX9X17",
	"XXX9X18",
	"XXX9X19",
	"XXX9X20",
	"XXX9X21",
	"XXX9X22",
	"XXX9X23",
	"XXX9X24",
	"XXX9X25",
	"XXX9X26",
	"XXX9X27",
	"XXX9X28",
	"XXX9X29",
	"XXX9X30",
	"XXX9X31",
};


/*
 * Object flags
 */
static cptr k_info_flags1[] =
{
	"STR",
	"INT",
	"WIS",
	"DEX",
	"CON",
	"CHR",
	"MANA",
	"SPELL",
	"STEALTH",
	"SEARCH",
	"INFRA",
	"TUNNEL",
	"SPEED",
	"BLOWS",
	"CHAOTIC",
	"VAMPIRIC",
	"SLAY_ANIMAL",
	"SLAY_EVIL",
	"SLAY_UNDEAD",
	"SLAY_DEMON",
	"SLAY_ORC",
	"SLAY_TROLL",
	"SLAY_GIANT",
	"SLAY_DRAGON",
	"KILL_DRAGON",
	"VORPAL",
	"IMPACT",
	"BRAND_POIS",
	"BRAND_ACID",
	"BRAND_ELEC",
	"BRAND_FIRE",
	"BRAND_COLD"
};

/*
 * Object flags
 */
static cptr k_info_flags2[] =
{
	"SUST_STR",
	"SUST_INT",
	"SUST_WIS",
	"SUST_DEX",
	"SUST_CON",
	"SUST_CHR",
	"INVIS",
	"LIFE",
	"IM_ACID",
	"IM_ELEC",
	"IM_FIRE",
	"IM_COLD",
	"SENS_FIRE",
	"REFLECT",
	"FREE_ACT",
	"HOLD_LIFE",
	"RES_ACID",
	"RES_ELEC",
	"RES_FIRE",
	"RES_COLD",
	"RES_POIS",
	"RES_FEAR",
	"RES_LITE",
	"RES_DARK",
	"RES_BLIND",
	"RES_CONF",
	"RES_SOUND",
	"RES_SHARDS",
	"RES_NETHER",
	"RES_NEXUS",
	"RES_CHAOS",
	"RES_DISEN"
};

/*
 * Trap flags
 */
static cptr k_info_flags2_trap[] =
{
	"AUTOMATIC_5",
	"AUTOMATIC_99",
	"KILL_GHOST",
	"TELEPORT_TO",
	"ONLY_DRAGON",
	"ONLY_DEMON",
	"XXX3",
	"XXX3",
	"ONLY_ANIMAL",
	"ONLY_UNDEAD",
	"ONLY_EVIL",
	"XXX3",
	"XXX3",
	"XXX3",
	"XXX3",
	"XXX3",
	"XXX3",
	"XXX3",
	"XXX3",
	"XXX3",
	"XXX3",
	"XXX3",
	"XXX3",
	"XXX3",
	"XXX3",
	"XXX3",
	"XXX3",
	"XXX3",
	"XXX3",
	"XXX3",
	"XXX3",
	"XXX3",
};


/*
 * Object flags
 */
static cptr k_info_flags3[] =
{
	"SH_FIRE",
	"SH_ELEC",
	"AUTO_CURSE",
	"DECAY",
	"NO_TELE",
	"NO_MAGIC",
	"WRAITH",
	"TY_CURSE",
	"EASY_KNOW",
	"HIDE_TYPE",
	"SHOW_MODS",
	"INSTA_ART",
	"FEATHER",
	"LITE1",
	"SEE_INVIS",
	"NORM_ART",
	"SLOW_DIGEST",
	"REGEN",
	"XTRA_MIGHT",
	"XTRA_SHOTS",
	"IGNORE_ACID",
	"IGNORE_ELEC",
	"IGNORE_FIRE",
	"IGNORE_COLD",
	"ACTIVATE",
	"DRAIN_EXP",
	"TELEPORT",
	"AGGRAVATE",
	"BLESSED",
	"CURSED",
	"HEAVY_CURSE",
	"PERMA_CURSE"
};

/*
 * Object flags
 */
static cptr k_info_flags4[] =
{
	"NEVER_BLOW",
	"PRECOGNITION",
	"BLACK_BREATH",
	"RECHARGE",
	"FLY",
	"DG_CURSE",
	"COULD2H",
	"MUST2H",
	"LEVELS",
	"CLONE",
	"SPECIAL_GENE",
	"CLIMB",
	"FAST_CAST",
	"CAPACITY",
	"CHARGING",
	"CHEAPNESS",
	"FOUNTAIN",
	"ANTIMAGIC_50",
	"XXX5",
	"XXX5",
	"XXX5",
	"EASY_USE",
	"IM_NETHER",
	"RECHARGED",
	"ULTIMATE",
	"AUTO_ID",
	"LITE2",
	"LITE3",
	"FUEL_LITE",
	"XXX5",
	"CURSE_NO_DROP",
	"NO_RECHARGE"
};

/*
 * Object flags
 */
static cptr k_info_flags5[] =
{
	"TEMPORARY",
	"DRAIN_MANA",
	"DRAIN_HP",
	"KILL_DEMON",
	"KILL_UNDEAD",
	"CRIT",
	"ATTR_MULTI",
	"WOUNDING",
	"FULL_NAME",
	"LUCK",
	"IMMOVABLE",
	"SPELL_CONTAIN",
	"RES_MORGUL",
	"ACTIVATE_NO_WIELD",
	"MAGIC_BREATH",
	"WATER_BREATH",
	"WIELD_CAST",
	"RANDOM_RESIST",
	"RANDOM_POWER",
	"RANDOM_RES_OR_POWER",
	"XXX8X20",
	"XXX8X21",
	"XXX8X22",
	"XXX8X23",
	"XXX8X24",
	"XXX8X25",
	"XXX8X26",
	"XXX8X27",
	"XXX8X28",
	"XXX8X29",
	"XXX8X02",
	"XXX8X22",
};

/*
 * ESP flags
 */
static cptr esp_flags[] =
{
	"ESP_ORC",
	"ESP_TROLL",
	"ESP_DRAGON",
	"ESP_GIANT",
	"ESP_DEMON",
	"ESP_UNDEAD",
	"ESP_EVIL",
	"ESP_ANIMAL",
	"ESP_THUNDERLORD",
	"ESP_GOOD",
	"ESP_NONLIVING",
	"ESP_UNIQUE",
	"ESP_SPIDER",
	"XXX8X02",
	"XXX8X02",
	"XXX8X02",
	"XXX8X02",
	"XXX8X17",
	"XXX8X18",
	"XXX8X19",
	"XXX8X20",
	"XXX8X21",
	"XXX8X22",
	"XXX8X23",
	"XXX8X24",
	"XXX8X25",
	"XXX8X26",
	"XXX8X27",
	"XXX8X28",
	"XXX8X29",
	"XXX8X02",
	"ESP_ALL",
};

/* Specially handled properties for ego-items */

static cptr ego_flags[] =
{
	"SUSTAIN",
	"OLD_RESIST",
	"ABILITY",
	"R_ELEM",
	"R_LOW",
	"R_HIGH",
	"R_ANY",
	"R_DRAGON",
	"SLAY_WEAP",
	"DAM_DIE",
	"DAM_SIZE",
	"PVAL_M1",
	"PVAL_M2",
	"PVAL_M3",
	"PVAL_M5",
	"AC_M1",
	"AC_M2",
	"AC_M3",
	"AC_M5",
	"TH_M1",
	"TH_M2",
	"TH_M3",
	"TH_M5",
	"TD_M1",
	"TD_M2",
	"TD_M3",
	"TD_M5",
	"R_P_ABILITY",
	"R_STAT",
	"R_STAT_SUST",
	"R_IMMUNITY",
	"LIMIT_BLOWS"
};

/*
 * Feature flags
 */
static cptr f_info_flags1[] =
{
	"NO_WALK",
	"NO_VISION",
	"CAN_LEVITATE",
	"CAN_PASS",
	"FLOOR",
	"WALL",
	"PERMANENT",
	"CAN_FLY",
	"REMEMBER",
	"NOTICE",
	"DONT_NOTICE_RUNNING",
	"CAN_RUN",
	"DOOR",
	"SUPPORT_LIGHT",
	"CAN_CLIMB",
	"TUNNELABLE",
	"WEB",
	"ATTR_MULTI",
	"SUPPORT_GROWTH",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1"
};

/*
 * Trap flags
 */
static cptr t_info_flags[] =
{
	"CHEST",
	"DOOR",
	"FLOOR",
	"XXX4",
	"XXX5",
	"XXX6",
	"XXX7",
	"XXX8",
	"XXX9",
	"XXX10",
	"XXX11",
	"XXX12",
	"XXX13",
	"XXX14",
	"XXX15",
	"XXX16",
	"LEVEL1",
	"LEVEL2",
	"LEVEL3",
	"LEVEL4",
	"XXX21",
	"XXX22",
	"XXX23",
	"XXX24",
	"XXX25",
	"XXX26",
	"XXX27",
	"XXX28",
	"XXX29",
	"XXX30",
	"XXX31",
	"XXX32"
};

/*
 * Stores flags
 */
static cptr st_info_flags1[] =
{
	"DEPEND_LEVEL",
	"SHALLOW_LEVEL",
	"MEDIUM_LEVEL",
	"DEEP_LEVEL",
	"RARE",
	"VERY_RARE",
	"COMMON",
	"ALL_ITEM",
	"RANDOM",
	"FORCE_LEVEL",
	"MUSEUM",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1"
};

/*
 * Race flags
 */
static cptr rp_info_flags1[] =
{
	"EXPERIMENTAL",
	"XXX",
	"RESIST_BLACK_BREATH",
	"NO_STUN",
	"XTRA_MIGHT_BOW",
	"XTRA_MIGHT_XBOW",
	"XTRA_MIGHT_SLING",
	"AC_LEVEL",
	"HURT_LITE",
	"VAMPIRE",
	"UNDEAD",
	"NO_CUT",
	"CORRUPT",
	"NO_FOOD",
	"NO_GOD",
	"XXX",
	"ELF",
	"SEMI_WRAITH",
	"NO_SUBRACE_CHANGE",
	"XXX",
	"XXX",
	"MOLD_FRIEND",
	"GOD_FRIEND",
	"XXX",
	"INNATE_SPELLS",
	"XXX",
	"XXX",
	"EASE_STEAL",
	"XXX",
	"XXX",
	"XXX",
	"XXX"
};

/*
 * Race flags
 */
static cptr rp_info_flags2[] =
{
	"XXX",
	"ASTRAL",
	"XXX",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1"
};

/* Skill flags */
static cptr s_info_flags1[] =
{
	"HIDDEN",
	"AUTO_HIDE",
	"RANDOM_GAIN",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1",
	"XXX1"
};


/*
 * Helpers for looking up flags in the above arrays
 * and extracting "bitmasks" from them.
 */

namespace { // anonymous

namespace detail {

/**
 * A "tie" (see e.g. std::tuple) between a "flags" value pointer and its
 * corresponding array of text strings. Implementation detail.
 */
template <size_t N> struct flag_tie_impl {
private:
	u32b *m_mask;
	cptr (&m_flags)[N];
public:
	flag_tie_impl(u32b *mask, cptr (&flags)[N]): m_mask(mask), m_flags(flags) {
		// Empty
	}

	bool match(cptr flag) {
		for (unsigned int i = 0; i < N; i++)
		{
			if (streq(flag, m_flags[i]))
			{
				*m_mask |= (1L << i);
				return true;
			}
		}
		return false;
	}
};

} // namespace detail

/**
 * Tie a flags value pointer and its corresponding array
 * of text strings.
 */
template<size_t N> detail::flag_tie_impl<N> flag_tie(u32b *mask, cptr (&flags)[N]) {
	static_assert(N <= 32, "Array too large to represent result");
	return detail::flag_tie_impl<N>(mask, flags);
}

/**
 * Look up flag in array of flags.
 */
template<size_t N> bool lookup_flags(cptr)
{
	// Base case: No match
	return false;
}

/**
 * Look up flag in array of flags.
 */
template<size_t N, typename... Pairs> bool lookup_flags(cptr flag, detail::flag_tie_impl<N> tie, Pairs&&...rest) {
	// Inductive case: Check against current "tie"
	if (tie.match(flag)) {
		// Match
		return true;
	} else {
		// No match; check against rest of the array of flags
		return lookup_flags<N>(flag, rest...);
	}
}

} // namespace anonymous



/*
 * Dungeon effect types (used in E:damage:frequency:type entry in d_info.txt)
 */
static struct
{
cptr name;
int feat;
}
d_info_dtypes[] =
{
	{"ELEC", GF_ELEC},
	{"POISON", GF_POIS},
	{"ACID", GF_ACID},
	{"COLD", GF_COLD},
	{"FIRE", GF_FIRE},
	{"MISSILE", GF_MISSILE},
	{"ARROW", GF_ARROW},
	{"PLASMA", GF_PLASMA},
	{"WATER", GF_WATER},
	{"LITE", GF_LITE},
	{"DARK", GF_DARK},
	{"LITE_WEAK", GF_LITE_WEAK},
	{"LITE_DARK", GF_DARK_WEAK},
	{"SHARDS", GF_SHARDS},
	{"SOUND", GF_SOUND},
	{"CONFUSION", GF_CONFUSION},
	{"FORCE", GF_FORCE},
	{"INERTIA", GF_INERTIA},
	{"MANA", GF_MANA},
	{"METEOR", GF_METEOR},
	{"ICE", GF_ICE},
	{"CHAOS", GF_CHAOS},
	{"NETHER", GF_NETHER},
	{"DISENCHANT", GF_DISENCHANT},
	{"NEXUS", GF_NEXUS},
	{"TIME", GF_TIME},
	{"GRAVITY", GF_GRAVITY},
	{"ROCKET", GF_ROCKET},
	{"NUKE", GF_NUKE},
	{"HOLY_FIRE", GF_HOLY_FIRE},
	{"HELL_FIRE", GF_HELL_FIRE},
	{"DISINTEGRATE", GF_DISINTEGRATE},
	{"DESTRUCTION", GF_DESTRUCTION},
	{"RAISE", GF_RAISE},
	{NULL, 0}
};

static const char *activation_names[] =
{
	"NO_ACTIVATION",        /*   0*/
	"SUNLIGHT",             /*   1*/
	"BO_MISS_1",            /*   2*/
	"BA_POIS_1",            /*   3*/
	"BO_ELEC_1",            /*   4*/
	"BO_ACID_1",            /*   5*/
	"BO_COLD_1",            /*   6*/
	"BO_FIRE_1",            /*   7*/
	"BA_COLD_1",            /*   8*/
	"BA_FIRE_1",            /*   9*/
	"DRAIN_1",              /*  10*/
	"BA_COLD_2",            /*  11*/
	"BA_ELEC_2",            /*  12*/
	"DRAIN_2",              /*  13*/
	"VAMPIRE_1",            /*  14*/
	"BO_MISS_2",            /*  15*/
	"BA_FIRE_2",            /*  16*/
	"BA_COLD_3",            /*  17*/
	"BA_ELEC_3",            /*  18*/
	"WHIRLWIND",            /*  19*/
	"VAMPIRE_2",            /*  20*/
	"CALL_CHAOS",           /*  21*/
	"ROCKET",               /*  22*/
	"DISP_EVIL",            /*  23*/
	"BA_MISS_3",            /*  24*/
	"DISP_GOOD",            /*  25*/
	"GILGALAD",             /*  26*/
	"CELEBRIMBOR",          /*  27*/
	"SKULLCLEAVER",         /*  28*/
	"HARADRIM",             /*  29*/
	"FUNDIN",               /*  30*/
	"EOL",                  /*  31*/
	"UMBAR",                /*  32*/
	"NUMENOR",              /*  33*/
	"KNOWLEDGE",            /*  34*/
	"UNDEATH",              /*  35*/
	"THRAIN",               /*  36*/
	"BARAHIR",              /*  37*/
	"TULKAS",               /*  38*/
	"NARYA",                /*  39*/
	"NENYA",                /*  40*/
	"VILYA",                /*  41*/
	"POWER",                /*  42*/
	"STONE_LORE",           /*  43*/
	"RAZORBACK",            /*  44*/
	"BLADETURNER",          /*  45*/
	"MEDIATOR",             /*  46*/
	"BELEGENNON",           /*  47*/
	"GORLIM",               /*  48*/
	"COLLUIN",              /*  49*/
	"BELANGIL",             /*  50*/
	"CONFUSE",              /*  51*/
	"SLEEP",                /*  52*/
	"QUAKE",                /*  53*/
	"TERROR",               /*  54*/
	"TELE_AWAY",            /*  55*/
	"BANISH_EVIL",          /*  56*/
	"GENOCIDE",             /*  57*/
	"MASS_GENO",            /*  58*/
	"ANGUIREL",             /*  59*/
	"ERU",                  /*  60*/
	"DAWN",                 /*  61*/
	"FIRESTAR",             /*  62*/
	"TURMIL",               /*  63*/
	"CUBRAGOL",             /*  64*/
	"CHARM_ANIMAL",         /*  65*/
	"CHARM_UNDEAD",         /*  66*/
	"CHARM_OTHER",          /*  67*/
	"CHARM_ANIMALS",        /*  68*/
	"CHARM_OTHERS",         /*  69*/
	"SUMMON_ANIMAL",        /*  70*/
	"SUMMON_PHANTOM",       /*  71*/
	"SUMMON_ELEMENTAL",     /*  72*/
	"SUMMON_DEMON",         /*  73*/
	"SUMMON_UNDEAD",        /*  74*/
	"ELESSAR",              /*  75*/
	"GANDALF",              /*  76*/
	"MARDA",                /*  77*/
	"PALANTIR",             /*  78*/
	"XXX79",
	"XXX80",
	"CURE_LW",              /*  81*/
	"CURE_MW",              /*  82*/
	"CURE_POISON",          /*  83*/
	"REST_LIFE",            /*  84*/
	"REST_ALL",             /*  85*/
	"CURE_700",             /*  86*/
	"CURE_1000",            /*  87*/
	"XXX88",
	"EREBOR",               /*  89*/
	"DRUEDAIN",             /*  90*/
	"ESP",                  /*  91*/
	"BERSERK",              /*  92*/
	"PROT_EVIL",            /*  93*/
	"RESIST_ALL",           /*  94*/
	"SPEED",                /*  95*/
	"XTRA_SPEED",           /*  96*/
	"WRAITH",               /*  97*/
	"INVULN",               /*  98*/
	"ROHAN",                /*  99*/
	"HELM",                 /*  100*/
	"BOROMIR",              /*  101*/
	"HURIN",                /*  102*/
	"AXE_GOTHMOG",          /*  103*/
	"MELKOR",               /*  104*/
	"GROND",                /*  105*/
	"NATUREBANE",           /*  106*/
	"NIGHT",                /*  107*/
	"ORCHAST",              /*  108*/
	"XXX109",
	"XXX110",
	"LIGHT",                /*  111*/
	"MAP_LIGHT",            /*  112*/
	"DETECT_ALL",           /*  113*/
	"DETECT_XTRA",          /*  114*/
	"ID_FULL",              /*  115*/
	"ID_PLAIN",             /*  116*/
	"RUNE_EXPLO",           /*  117*/
	"RUNE_PROT",            /*  118*/
	"SATIATE",              /*  119*/
	"DEST_DOOR",            /*  120*/
	"STONE_MUD",            /*  121*/
	"RECHARGE",             /*  122*/
	"ALCHEMY",              /*  123*/
	"DIM_DOOR",             /*  124*/
	"TELEPORT",             /*  125*/
	"RECALL",               /*  126*/
	"DEATH",                /*  127*/
	"RUINATION",            /*  128*/
	"DESTRUC",              /*  129*/
	"UNINT",                /*  130*/
	"UNSTR",                /*  131*/
	"UNCON",                /*  132*/
	"UNCHR",                /*  133*/
	"UNDEX",                /*  134*/
	"UNWIS",                /*  135*/
	"STATLOSS",             /*  136*/
	"HISTATLOSS",           /*  137*/
	"EXPLOSS",              /*  138*/
	"HIEXPLOSS",            /*  139*/
	"SUMMON_MONST",         /*  140*/
	"PARALYZE",             /*  141*/
	"HALLU",                /*  142*/
	"POISON",               /*  143*/
	"HUNGER",               /*  144*/
	"STUN",                 /*  145*/
	"CUTS",                 /*  146*/
	"PARANO",               /*  147*/
	"CONFUSION",            /*  148*/
	"BLIND",                /*  149*/
	"PET_SUMMON",           /*  150*/
	"CURE_PARA",            /*  151*/
	"CURE_HALLU",           /*  152*/
	"CURE_POIS",            /*  153*/
	"CURE_HUNGER",          /*  154*/
	"CURE_STUN",            /*  155*/
	"CURE_CUTS",            /*  156*/
	"CURE_FEAR",            /*  157*/
	"CURE_CONF",            /*  158*/
	"CURE_BLIND",           /*  159*/
	"CURING",               /*  160*/
	"DARKNESS",             /*  161*/
	"LEV_TELE",             /*  162*/
	"ACQUIREMENT",          /*  163*/
	"WEIRD",                /*  164*/
	"AGGRAVATE",            /*  165*/
	"MUT",                  /*  166*/
	"CURE_INSANITY",        /*  167*/
	"CURE_MUT",             /*  168*/
	"LIGHT_ABSORBTION",     /*  169*/
	"BA_FIRE_H",            /*  170*/
	"BA_COLD_H",            /*  171*/
	"BA_ELEC_H",            /*  172*/
	"BA_ACID_H",            /*  173*/
	"SPIN",                 /*  174*/
	"NOLDOR",               /*  175*/
	"SPECTRAL",             /*  176*/
	"JUMP",                 /*  177*/
	"DEST_TELE",            /*  178*/
	"BA_POIS_4",            /*  179*/
	"BA_COLD_4",            /*  180*/
	"BA_FIRE_4",            /*  181*/
	"BA_ACID_4",            /*  182*/
	"BA_ELEC_4",            /*  183*/
	"BR_ELEC",              /*  184*/
	"BR_COLD",              /*  185*/
	"BR_FIRE",              /*  186*/
	"BR_ACID",              /*  187*/
	"BR_POIS",              /*  188*/
	"BR_MANY",              /*  189*/
	"BR_CONF",              /*  190*/
	"BR_SOUND",             /*  191*/
	"BR_CHAOS",             /*  192*/
	"BR_SHARD",             /*  193*/
	"BR_BALANCE",           /*  194*/
	"BR_LIGHT",             /*  195*/
	"BR_POWER",             /*  196*/
	"GROW_MOLD",            /*  197*/
	"XXX198",
	"XXX199",
	"MUSIC",                /*  200*/
	"ETERNAL_FLAME",        /*  201 */
	"MAGGOT",               /*  202 */
	"LEBOHAUM",             /*  203 */
	"DURANDIL",             /*  204 */
	"RADAGAST",             /*  205, Theme */
	"VALAROMA",             /*  206, Theme */
	""
};

/*
 * Convert a "color letter" into an "actual" color
 * The colors are: dwsorgbuDWvyRGBU, as shown below
 */
int color_char_to_attr(char c)
{
	switch (c)
	{
	case 'd':
		return (TERM_DARK);
	case 'w':
		return (TERM_WHITE);
	case 's':
		return (TERM_SLATE);
	case 'o':
		return (TERM_ORANGE);
	case 'r':
		return (TERM_RED);
	case 'g':
		return (TERM_GREEN);
	case 'b':
		return (TERM_BLUE);
	case 'u':
		return (TERM_UMBER);

	case 'D':
		return (TERM_L_DARK);
	case 'W':
		return (TERM_L_WHITE);
	case 'v':
		return (TERM_VIOLET);
	case 'y':
		return (TERM_YELLOW);
	case 'R':
		return (TERM_L_RED);
	case 'G':
		return (TERM_L_GREEN);
	case 'B':
		return (TERM_L_BLUE);
	case 'U':
		return (TERM_L_UMBER);
	}

	return ( -1);
}

/*
 * Attr value-to-char convertion table
 */
byte conv_color[16] =
{
	'd',
	'w',
	's',
	'o',
	'r',
	'g',
	'b',
	'u',
	'D',
	'W',
	'v',
	'y',
	'R',
	'G',
	'B',
	'U',
};


/* Values in re_info can be fixed, added, substracted or percented */
static byte monster_ego_modify(char c)
{
	switch (c)
	{
	case '+':
		return MEGO_ADD;
	case '-':
		return MEGO_SUB;
	case '=':
		return MEGO_FIX;
	case '%':
		return MEGO_PRC;
	default:
		{
			msg_format("Unknown mego value modifier %c.", c);
			return MEGO_ADD;
		}
	}
}

/**
 * Version of strdup() which just aborts if an allocation
 * error occurs.
 */
static char *my_strdup(const char *s)
{
	char *p = strdup(s);
	if (!p)
	{
		abort();
	}
	return p;
}


/**
 * Append one string to the end of another, reallocating if
 * necessary.
 */
static void strappend(char **s, const char *t)
{
	// Do we need to initialize the destination string?
	if (*s == nullptr)
	{
		// Costs an extra allocation which could be avoided
		// but this leads to simpler code.
		*s = my_strdup("");
	}
	// We should really be preserving the original pointer and
	// do something else in case of failure to realloc(), but
	// instead we just do the lazy thing and call abort() if
	// reallocation fails. In practice it won't.
	*s = static_cast<char *>(realloc(*s, strlen(*s) + strlen(t) + 1));
	if (*s == nullptr)
	{
		abort(); // Cannot handle failure to reallocate
	}

	/* Append 't' to the destination string */
	strcat(*s, t);
}

/*** Initialize from ascii template files ***/

/*
 * Grab one race flag from a textual string
 */
static bool_ unknown_shut_up = FALSE;
static errr grab_one_class_flag(u32b *choice, cptr what)
{
	int i;
	cptr s;

	/* Scan classes flags */
	for (i = 0; i < max_c_idx && (s = class_info[i].title); i++)
	{
		if (streq(what, s))
		{
			(choice[i / 32]) |= (1L << i);
			return (0);
		}
	}

	/* Oops */
	if (!unknown_shut_up) msg_format("Unknown class flag '%s'.", what);

	/* Failure */
	return (1);
}
static errr grab_one_race_allow_flag(u32b *choice, cptr what)
{
	int i;
	cptr s;

	/* Scan classes flags */
	for (i = 0; i < max_rp_idx && (s = race_info[i].title); i++)
	{
		if (streq(what, s))
		{
			(choice[i / 32]) |= (1L << i);
			return (0);
		}
	}

	/* Oops */
	if (!unknown_shut_up) msg_format("(1)Unknown race flag '%s'.", what);

	/* Failure */
	return (1);
}

/*
 * Grab one flag from a textual string
 */
static errr grab_one_skill_flag(u32b *f1, cptr what)
{
	if (lookup_flags(what, flag_tie(f1, s_info_flags1)))
	{
		return 0;
	}

	/* Oops */
	msg_format("(2)Unknown skill flag '%s'.", what);

	/* Error */
	return (1);
}
/*
 * Grab one flag from a textual string
 */
static errr grab_one_player_race_flag(u32b *f1, u32b *f2, cptr what)
{
	if (lookup_flags(what,
		flag_tie(f1, rp_info_flags1),
		flag_tie(f2, rp_info_flags2)))
	{
		return 0;
	}

	/* Oops */
	msg_format("(2)Unknown race flag '%s'.", what);

	/* Error */
	return (1);
}

/* Get an activation number (good for artifacts, recipes, egos, and object kinds) */
static int get_activation(char *activation)
{
	int i;
	for ( i = 0 ; activation_names[i][0] ; i++)
	{
		if (!strncmp(activation_names[i], activation, 19))
		{
			return i;
		}
	}

	msg_format("Unknown activation '%s'.", activation);
	return -1;
}

/*
 * Grab one flag in an object_kind from a textual string
 */
static errr grab_one_race_kind_flag(u32b *f1, u32b *f2, u32b *f3, u32b *f4, u32b *f5, u32b *esp, cptr what)
{
	if (lookup_flags(what,
		flag_tie(f1, k_info_flags1),
		flag_tie(f2, k_info_flags2),
		flag_tie(f3, k_info_flags3),
		flag_tie(f4, k_info_flags4),
		flag_tie(f5, k_info_flags5),
		flag_tie(esp, esp_flags)))
	{
		return 0;
	}

	/* Oops */
	msg_format("Unknown object flag '%s'.", what);

	/* Error */
	return (1);
}

/*
 * Initialize the "player" arrays, by parsing an ascii "template" file
 */
errr init_player_info_txt(FILE *fp)
{
	int i = 0, z;
	int powers = 0;
	int lev = 1;
	int tit_idx = 0;
	int spec_idx = 0;
	int cur_ab = -1;
	char buf[1024];
	char *s, *t;

	/* Current entry */
	player_race *rp_ptr = NULL;
	player_race_mod *rmp_ptr = NULL;
	player_class *c_ptr = NULL;
	player_spec *s_ptr = NULL;
	meta_class_type *mc_ptr = NULL;


	/* Just before the first record */
	error_idx = -1;

	/* Just before the first line */
	error_line = -1;

	/* Init general skills */
	for (z = 0; z < MAX_SKILLS; z++)
	{
		gen_skill_basem[z] = 0;
		gen_skill_base[z] = 0;
		gen_skill_modm[z] = 0;
		gen_skill_mod[z] = 0;
	}

	/* Parse */
	while (0 == my_fgets(fp, buf, 1024))
	{
		/* Advance the line number */
		error_line++;

		/* Skip comments and blank lines */
		if (!buf[0] || (buf[0] == '#')) continue;

		/* Verify correct "colon" format */
		if (buf[1] != ':') return (1);

		/* Reinit error_idx */
		if (buf[0] == 'I')
		{
			error_idx = -1;
			continue;
		}

		/* Process 'H' for "History" */
		if (buf[0] == 'H')
		{
			int idx;
			char *zz[6];

			/* Scan for the values */
			if (tokenize(buf + 2, 6, zz, ':', ':') != 6) return (1);

			idx = atoi(zz[0]);
			bg[idx].roll = atoi(zz[1]);
			bg[idx].chart = atoi(zz[2]);
			bg[idx].next = atoi(zz[3]);
			bg[idx].bonus = atoi(zz[4]);

			/* Copy text */
			assert(!bg[idx].info);
			bg[idx].info = my_strdup(zz[5]);

			/* Next... */
			continue;
		}

		/* Process 'G:k' for "General skills" */
		if ((buf[0] == 'G') && (buf[2] == 'k'))
		{
			long val, mod, i;
			char name[200], v, m;

			/* Scan for the values */
			if (5 != sscanf(buf + 4, "%c%ld:%c%ld:%s",
			                &v, &val, &m, &mod, name)) return (1);

			if ((i = find_skill(name)) == -1) return (1);
			gen_skill_basem[i] = monster_ego_modify(v);
			gen_skill_base[i] = val;
			gen_skill_modm[i] = monster_ego_modify(m);
			gen_skill_mod[i] = mod;

			/* Next... */
			continue;
		}

		/* Process 'N' for "New/Number/Name" */
		if ((buf[0] == 'R') && (buf[2] == 'N'))
		{
			/* Find the colon before the name */
			s = strchr(buf + 4, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			i = atoi(buf + 4);

			/* Verify information */
			if (i < error_idx) return (4);

			/* Verify information */
			if (i >= max_rp_idx) return (2);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			rp_ptr = &race_info[i];

			/* Copy title */
			assert(!rp_ptr->title);
			rp_ptr->title = my_strdup(s);

			/* Initialize */
			rp_ptr->powers[0] = rp_ptr->powers[1] = rp_ptr->powers[2] = rp_ptr->powers[3] = -1;
			powers = 0;
			lev = 1;
			cur_ab = 0;
			for (z = 0; z < 10; z++)
				rp_ptr->abilities[z].level = -1;

			/* Next... */
			continue;
		}

		/* Process 'D' for "Description" */
		if ((buf[0] == 'R') && (buf[2] == 'D'))
		{
			/* Acquire the text */
			s = buf + 4;

			if (!rp_ptr->desc)
			{
				rp_ptr->desc = my_strdup(s);
			}
			else
			{
				strappend(&rp_ptr->desc, format("\n%s", s));
			}

			/* Next... */
			continue;
		}

		/* Process 'E' for "body parts" */
		if ((buf[0] == 'R') && (buf[2] == 'E'))
		{
			int s[BODY_MAX], z;

			/* Scan for the values */
			if (BODY_MAX != sscanf(buf + 4, "%d:%d:%d:%d:%d:%d",
			                       &s[0], &s[1], &s[2], &s[3], &s[4], &s[5])) return (1);

			for (z = 0; z < BODY_MAX; z++)
				rp_ptr->body_parts[z] = s[z];

			/* Next... */
			continue;
		}

		/* Process 'R' for "flag level" */
		if ((buf[0] == 'R') && (buf[2] == 'R'))
		{
			int s[2];

			/* Scan for the values */
			if (2 != sscanf(buf + 4, "%d:%d",
			                &s[0], &s[1])) return (1);

			lev = s[0];
			rp_ptr->opval[lev] = s[1];

			/* Next... */
			continue;
		}

		/* Process 'S' for "Stats" */
		if ((buf[0] == 'R') && (buf[2] == 'S'))
		{
			int s[7], z;

			/* Scan for the values */
			if (7 != sscanf(buf + 4, "%d:%d:%d:%d:%d:%d:%d",
			                &s[0], &s[1], &s[2], &s[3], &s[4], &s[5], &s[6])) return (1);

			rp_ptr->luck = s[6];
			for (z = 0; z < 6; z++)
				rp_ptr->r_adj[z] = s[z];

			/* Next... */
			continue;
		}

		/* Process 'Z' for "powers" */
		if ((buf[0] == 'R') && (buf[2] == 'Z'))
		{
			int i;

			/* Acquire the text */
			s = buf + 4;

			/* Find it in the list */
			for (i = 0; i < POWER_MAX; i++)
			{
				if (iequals(s, powers_type[i].name)) break;
			}

			if (i == POWER_MAX) return (6);

			rp_ptr->powers[powers++] = i;

			/* Next... */
			continue;
		}

		/* Process 'K' for "sKills" */
		if ((buf[0] == 'R') && (buf[2] == 'K'))
		{
			int s[8];

			/* Scan for the values */
			if (8 != sscanf(buf + 4, "%d:%d:%d:%d:%d:%d:%d:%d",
			                &s[0], &s[1], &s[2], &s[3], &s[4], &s[5], &s[6], &s[7])) return (1);

			rp_ptr->r_dis = s[0];
			rp_ptr->r_dev = s[1];
			rp_ptr->r_sav = s[2];
			rp_ptr->r_stl = s[3];
			rp_ptr->r_srh = s[4];
			rp_ptr->r_fos = s[5];
			rp_ptr->r_thn = s[6];
			rp_ptr->r_thb = s[7];

			/* Next... */
			continue;
		}

		/* Process 'k' for "skills" */
		if ((buf[0] == 'R') && (buf[2] == 'k'))
		{
			long val, mod, i;
			char name[200], v, m;

			/* Scan for the values */
			if (5 != sscanf(buf + 4, "%c%ld:%c%ld:%s",
			                &v, &val, &m, &mod, name)) return (1);

			if ((i = find_skill(name)) == -1) return (1);
			rp_ptr->skill_basem[i] = monster_ego_modify(v);
			rp_ptr->skill_base[i] = val;
			rp_ptr->skill_modm[i] = monster_ego_modify(m);
			rp_ptr->skill_mod[i] = mod;

			/* Next... */
			continue;
		}

		/* Process 'b' for "abilities" */
		if ((buf[0] == 'R') && (buf[2] == 'b'))
		{
			char *sec;

			/* Scan for the values */
			if (NULL == (sec = strchr(buf + 4, ':')))
			{
				return (1);
			}
			*sec = '\0';
			sec++;
			if (!*sec) return (1);

			if ((i = find_ability(sec)) == -1) return (1);

			rp_ptr->abilities[cur_ab].ability = i;
			rp_ptr->abilities[cur_ab].level = atoi(buf + 4);
			cur_ab++;

			/* Next... */
			continue;
		}

		/* Process 'P' for "xtra" */
		if ((buf[0] == 'R') && (buf[2] == 'P'))
		{
			int s[4];

			/* Scan for the values */
			if (4 != sscanf(buf + 4, "%d:%d:%d:%d",
			                &s[0], &s[1], &s[2], &s[3])) return (1);

			rp_ptr->r_mhp = s[0];
			rp_ptr->r_exp = s[1];
			rp_ptr->infra = s[2];
			rp_ptr->chart = s[3];

			/* Next... */
			continue;
		}

		/* Process 'G' for "Player flags" (multiple lines) */
		if ((buf[0] == 'R') && (buf[2] == 'G'))
		{
			if (0 != grab_one_player_race_flag(&rp_ptr->flags1, &rp_ptr->flags2, buf + 4))
			{
				return (5);
			}

			/* Next... */
			continue;
		}

		/* Process 'F' for "level Flags" (multiple lines) */
		if ((buf[0] == 'R') && (buf[2] == 'F'))
		{
			if (0 != grab_one_race_kind_flag(&rp_ptr->oflags1[lev], &rp_ptr->oflags2[lev], &rp_ptr->oflags3[lev], &rp_ptr->oflags4[lev], &rp_ptr->oflags5[lev], &rp_ptr->oesp[lev], buf + 4))
			{
				return (5);
			}

			/* Next... */
			continue;
		}

		/* Process 'O' for "Object birth" */
		if ((buf[0] == 'R') && (buf[2] == 'O'))
		{
			int s[5];

			/* Scan for the values */
			if (5 != sscanf(buf + 4, "%d:%d:%d:%dd%d",
			                &s[0], &s[1], &s[4], &s[2], &s[3]))
			{
				s[4] = 0;

				if (4 != sscanf(buf + 4, "%d:%d:%dd%d",
				                &s[0], &s[1], &s[2], &s[3]))
				{
					return (1);
				}
			}

			rp_ptr->obj_pval[rp_ptr->obj_num] = s[4];
			rp_ptr->obj_tval[rp_ptr->obj_num] = s[0];
			rp_ptr->obj_sval[rp_ptr->obj_num] = s[1];
			rp_ptr->obj_dd[rp_ptr->obj_num] = s[2];
			rp_ptr->obj_ds[rp_ptr->obj_num++] = s[3];

			/* Next... */
			continue;
		}

		/* Process 'C' for "Class choice flags" (multiple lines) */
		if ((buf[0] == 'R') && (buf[2] == 'C'))
		{
			if (0 != grab_one_class_flag(rp_ptr->choice, buf + 4))
			{
				return (5);
			}

			/* Next... */
			continue;
		}

		/* Process 'N' for "New/Number/Name" */
		if ((buf[0] == 'S') && (buf[2] == 'N'))
		{
			/* Find the colon before the name */
			s = strchr(buf + 4, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			i = atoi(buf + 4);

			/* Verify information */
			if (i < error_idx) return (4);

			/* Verify information */
			if (i >= max_rmp_idx) return (2);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			rmp_ptr = &race_mod_info[i];

			/* Copy title */
			assert(!rmp_ptr->title);
			rmp_ptr->title = my_strdup(s);

			/* Initialize */
			rmp_ptr->powers[0] = rmp_ptr->powers[1] = rmp_ptr->powers[2] = rmp_ptr->powers[3] = -1;
			powers = 0;
			lev = 1;
			cur_ab = 0;
			for (z = 0; z < 10; z++)
				rmp_ptr->abilities[z].level = -1;

			/* Next... */
			continue;
		}

		/* Process 'D' for "Description" */
		if ((buf[0] == 'S') && (buf[2] == 'D'))
		{
			/* Acquire the text */
			s = buf + 6;

			/* Place */
			if (buf[4] == 'A')
			{
				rmp_ptr->place = TRUE;
			}
			else
			{
				rmp_ptr->place = FALSE;
			}

			/* Description */
			if (!rmp_ptr->desc)
			{
				rmp_ptr->desc = my_strdup(s);
			}
			else
			{
				/* Append chars to the name */
				strappend(&rmp_ptr->desc, format("\n%s", s));
			}

			/* Next... */
			continue;
		}

		/* Process 'E' for "body parts" */
		if ((buf[0] == 'S') && (buf[2] == 'E'))
		{
			int s[BODY_MAX], z;

			/* Scan for the values */
			if (BODY_MAX != sscanf(buf + 4, "%d:%d:%d:%d:%d:%d",
			                       &s[0], &s[1], &s[2], &s[3], &s[4], &s[5])) return (1);

			for (z = 0; z < BODY_MAX; z++)
				rmp_ptr->body_parts[z] = s[z];

			/* Next... */
			continue;
		}

		/* Process 'R' for "flag level" */
		if ((buf[0] == 'S') && (buf[2] == 'R'))
		{
			int s[2];

			/* Scan for the values */
			if (2 != sscanf(buf + 4, "%d:%d",
			                &s[0], &s[1])) return (1);

			lev = s[0];
			rmp_ptr->opval[lev] = s[1];

			/* Next... */
			continue;
		}

		/* Process 'S' for "Stats" */
		if ((buf[0] == 'S') && (buf[2] == 'S'))
		{
			int s[8], z;

			/* Scan for the values */
			if (8 != sscanf(buf + 4, "%d:%d:%d:%d:%d:%d:%d:%d",
			                &s[0], &s[1], &s[2], &s[3], &s[4], &s[5], &s[6], &s[7])) return (1);

			rmp_ptr->mana = s[7];
			rmp_ptr->luck = s[6];
			for (z = 0; z < 6; z++)
				rmp_ptr->r_adj[z] = s[z];

			/* Next... */
			continue;
		}

		/* Process 'Z' for "powers" */
		if ((buf[0] == 'S') && (buf[2] == 'Z'))
		{
			int i;

			/* Acquire the text */
			s = buf + 4;

			/* Find it in the list */
			for (i = 0; i < POWER_MAX; i++)
			{
				if (iequals(s, powers_type[i].name)) break;
			}

			if (i == POWER_MAX) return (6);

			rmp_ptr->powers[powers++] = i;

			/* Next... */
			continue;
		}

		/* Process 'k' for "skills" */
		if ((buf[0] == 'S') && (buf[2] == 'k'))
		{
			long val, mod, i;
			char name[200], v, m;

			/* Scan for the values */
			if (5 != sscanf(buf + 4, "%c%ld:%c%ld:%s",
			                &v, &val, &m, &mod, name)) return (1);

			if ((i = find_skill(name)) == -1) return (1);
			rmp_ptr->skill_basem[i] = monster_ego_modify(v);
			rmp_ptr->skill_base[i] = val;
			rmp_ptr->skill_modm[i] = monster_ego_modify(m);
			rmp_ptr->skill_mod[i] = mod;

			/* Next... */
			continue;
		}

		/* Process 'b' for "abilities" */
		if ((buf[0] == 'S') && (buf[2] == 'b'))
		{
			char *sec;

			/* Scan for the values */
			if (NULL == (sec = strchr(buf + 4, ':')))
			{
				return (1);
			}
			*sec = '\0';
			sec++;
			if (!*sec) return (1);

			if ((i = find_ability(sec)) == -1) return (1);

			rmp_ptr->abilities[cur_ab].ability = i;
			rmp_ptr->abilities[cur_ab].level = atoi(buf + 4);
			cur_ab++;

			/* Next... */
			continue;
		}

		/* Process 'K' for "sKills" */
		if ((buf[0] == 'S') && (buf[2] == 'K'))
		{
			int s[8];

			/* Scan for the values */
			if (8 != sscanf(buf + 4, "%d:%d:%d:%d:%d:%d:%d:%d",
			                &s[0], &s[1], &s[2], &s[3], &s[4], &s[5], &s[6], &s[7])) return (1);

			rmp_ptr->r_dis = s[0];
			rmp_ptr->r_dev = s[1];
			rmp_ptr->r_sav = s[2];
			rmp_ptr->r_stl = s[3];
			rmp_ptr->r_srh = s[4];
			rmp_ptr->r_fos = s[5];
			rmp_ptr->r_thn = s[6];
			rmp_ptr->r_thb = s[7];

			/* Next... */
			continue;
		}

		/* Process 'P' for "xtra" */
		if ((buf[0] == 'S') && (buf[2] == 'P'))
		{
			int s[3];

			/* Scan for the values */
			if (3 != sscanf(buf + 4, "%d:%d:%d",
			                &s[0], &s[1], &s[2])) return (1);

			rmp_ptr->r_mhp = s[0];
			rmp_ptr->r_exp = s[1];
			rmp_ptr->infra = s[2];

			/* Next... */
			continue;
		}

		/* Process 'G' for "Player flags" (multiple lines) */
		if ((buf[0] == 'S') && (buf[2] == 'G'))
		{
			if (0 != grab_one_player_race_flag(&rmp_ptr->flags1, &rmp_ptr->flags2, buf + 4))
			{
				return (5);
			}

			/* Next... */
			continue;
		}

		/* Process 'F' for "level Flags" (multiple lines) */
		if ((buf[0] == 'S') && (buf[2] == 'F'))
		{
			if (0 != grab_one_race_kind_flag(&rmp_ptr->oflags1[lev], &rmp_ptr->oflags2[lev], &rmp_ptr->oflags3[lev], &rmp_ptr->oflags4[lev], &rmp_ptr->oflags5[lev], &rmp_ptr->oesp[lev], buf + 4))
			{
				return (5);
			}

			/* Next... */
			continue;
		}

		/* Process 'O' for "Object birth" */
		if ((buf[0] == 'S') && (buf[2] == 'O'))
		{
			int s[5];

			/* Scan for the values */
			if (5 != sscanf(buf + 4, "%d:%d:%d:%dd%d",
			                &s[0], &s[1], &s[4], &s[2], &s[3]))
			{
				s[4] = 0;

				if (4 != sscanf(buf + 4, "%d:%d:%dd%d",
				                &s[0], &s[1], &s[2], &s[3]))
				{
					return (1);
				}
			}

			rmp_ptr->obj_pval[rmp_ptr->obj_num] = s[4];
			rmp_ptr->obj_tval[rmp_ptr->obj_num] = s[0];
			rmp_ptr->obj_sval[rmp_ptr->obj_num] = s[1];
			rmp_ptr->obj_dd[rmp_ptr->obj_num] = s[2];
			rmp_ptr->obj_ds[rmp_ptr->obj_num++] = s[3];

			/* Next... */
			continue;
		}

		/* Process 'A' for "Allowed races" (multiple lines) */
		if ((buf[0] == 'S') && (buf[2] == 'A'))
		{
			if (0 != grab_one_race_allow_flag(rmp_ptr->choice, buf + 4))
			{
			       return (5);
			}

			/* Next... */
			continue;
		}

		/* Process 'C' for "Class choice flags" (multiple lines) */
		if ((buf[0] == 'S') && (buf[2] == 'C'))
		{
			u32b choice[2] = {0, 0};
			if (0 != grab_one_class_flag(choice, buf + 6))
			{
				return (5);
			}

			/* Combine into the class flags */
			for (int z = 0; z < 2; z++)
			{
				if (buf[4] == 'A')
				{
					rmp_ptr->pclass[z] |= choice[z];
				}
				else
				{
					rmp_ptr->mclass[z] |= choice[z];
				}
			}

			/* Next... */
			continue;
		}

		/* Process 'N' for "New/Number/Name" */
		if ((buf[0] == 'C') && (buf[2] == 'N'))
		{
			int z;

			/* Find the colon before the name */
			s = strchr(buf + 4, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			i = atoi(buf + 4);

			/* Verify information */
			if (i < error_idx) return (4);

			/* Verify information */
			if (i >= max_c_idx) return (2);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			c_ptr = &class_info[i];

			/* Copy name */
			assert(!c_ptr->title);
			c_ptr->title = my_strdup(s);

			/* Initialize */
			c_ptr->powers[0] = c_ptr->powers[1] = c_ptr->powers[2] = c_ptr->powers[3] = -1;
			powers = 0;
			lev = 1;
			for (z = 0; z < 10; z++)
				c_ptr->abilities[z].level = -1;
			cur_ab = 0;
			c_ptr->obj_num = 0;
			tit_idx = 0;
			spec_idx = -1;
			for (z = 0; z < MAX_SPEC; z++)
				c_ptr->spec[z].title = 0;

			/* Next... */
			continue;
		}

		/* Process 'D' for "Description" */
		if ((buf[0] == 'C') && (buf[2] == 'D'))
		{
			/* Acquire the text */
			s = buf + 6;

			switch (buf[4])
			{
			case '0': /* Class description */
				if (!c_ptr->desc)
				{

					c_ptr->desc = my_strdup(s);
				}
				else
				{
					strappend(&c_ptr->desc, format("\n%s", s));
				}
				break;

			case '1': /* Class title */
				/* Copy */
				assert(!c_ptr->titles[tit_idx]);
				c_ptr->titles[tit_idx] = my_strdup(s);

				/* Go to next title in array */
				tit_idx++;

				break;

			default: /* Unknown */
				return (6);
				break;
			}

			/* Next... */
			continue;
		}

		/* Process 'O' for "Object birth" */
		if ((buf[0] == 'C') && (buf[2] == 'O'))
		{
			int s[5];

			/* Scan for the values */
			if (5 != sscanf(buf + 4, "%d:%d:%d:%dd%d",
			                &s[0], &s[1], &s[4], &s[2], &s[3]))
			{
				s[4] = 0;

				if (4 != sscanf(buf + 4, "%d:%d:%dd%d",
				                &s[0], &s[1], &s[2], &s[3]))
				{
					return (1);
				}
			}

			c_ptr->obj_pval[c_ptr->obj_num] = s[4];
			c_ptr->obj_tval[c_ptr->obj_num] = s[0];
			c_ptr->obj_sval[c_ptr->obj_num] = s[1];
			c_ptr->obj_dd[c_ptr->obj_num] = s[2];
			c_ptr->obj_ds[c_ptr->obj_num++] = s[3];

			/* Next... */
			continue;
		}

		/* Process 'E' for "body parts" */
		if ((buf[0] == 'C') && (buf[2] == 'E'))
		{
			int s[BODY_MAX], z;

			/* Scan for the values */
			if (BODY_MAX != sscanf(buf + 4, "%d:%d:%d:%d:%d:%d",
			                       &s[0], &s[1], &s[2], &s[3], &s[4], &s[5])) return (1);

			for (z = 0; z < BODY_MAX; z++)
				c_ptr->body_parts[z] = s[z];

			/* Next... */
			continue;
		}

		/* Process 'R' for "flag level" */
		if ((buf[0] == 'C') && (buf[2] == 'R'))
		{
			int s[2];

			/* Scan for the values */
			if (2 != sscanf(buf + 4, "%d:%d",
			                &s[0], &s[1])) return (1);

			lev = s[0];
			c_ptr->opval[lev] = s[1];

			/* Next... */
			continue;
		}

		/* Process 'C' for "Stats" */
		if ((buf[0] == 'C') && (buf[2] == 'S'))
		{
			int s[8], z;

			/* Scan for the values */
			if (8 != sscanf(buf + 4, "%d:%d:%d:%d:%d:%d:%d:%d",
			                &s[0], &s[1], &s[2], &s[3], &s[4], &s[5], &s[6], &s[7])) return (1);

			c_ptr->mana = s[6];
			c_ptr->extra_blows = s[7];
			for (z = 0; z < 6; z++)
				c_ptr->c_adj[z] = s[z];

			/* Next... */
			continue;
		}

		/* Process 'k' for "skills" */
		if ((buf[0] == 'C') && (buf[2] == 'k'))
		{
			long val, mod, i;
			char name[200], v, m;

			/* Scan for the values */
			if (5 != sscanf(buf + 4, "%c%ld:%c%ld:%s",
			                &v, &val, &m, &mod, name)) return (1);

			if ((i = find_skill(name)) == -1) return (1);
			c_ptr->skill_basem[i] = monster_ego_modify(v);
			c_ptr->skill_base[i] = val;
			c_ptr->skill_modm[i] = monster_ego_modify(m);
			c_ptr->skill_mod[i] = mod;

			/* Next... */
			continue;
		}

		/* Process 'b' for "abilities" */
		if ((buf[0] == 'C') && (buf[2] == 'b'))
		{
			char *sec;

			/* Scan for the values */
			if (NULL == (sec = strchr(buf + 4, ':')))
			{
				return (1);
			}
			*sec = '\0';
			sec++;
			if (!*sec) return (1);

			if ((i = find_ability(sec)) == -1) return (1);

			c_ptr->abilities[cur_ab].ability = i;
			c_ptr->abilities[cur_ab].level = atoi(buf + 4);
			cur_ab++;

			/* Next... */
			continue;
		}

		/* Process 'g' for "gods" */
		if ((buf[0] == 'C') && (buf[2] == 'g'))
		{
			int i;

			if (streq(buf + 4, "All Gods"))
				c_ptr->gods = 0xFFFFFFFF;
			else
			{
				if ((i = find_god(buf + 4)) == -1) return (1);
				c_ptr->gods |= BIT(i);
			}

			/* Next... */
			continue;
		}

		/* Process 'Z' for "powers" */
		if ((buf[0] == 'C') && (buf[2] == 'Z'))
		{
			int i;

			/* Acquire the text */
			s = buf + 4;

			/* Find it in the list */
			for (i = 0; i < POWER_MAX; i++)
			{
				if (iequals(s, powers_type[i].name)) break;
			}

			if (i == POWER_MAX) return (6);

			c_ptr->powers[powers++] = i;

			/* Next... */
			continue;
		}

		/* Process 'K' for "sKills" */
		if ((buf[0] == 'C') && (buf[2] == 'K'))
		{
			int s[8];

			/* Scan for the values */
			if (8 != sscanf(buf + 4, "%d:%d:%d:%d:%d:%d:%d:%d",
			                &s[0], &s[1], &s[2], &s[3], &s[4], &s[5], &s[6], &s[7])) return (1);

			c_ptr->c_dis = s[0];
			c_ptr->c_dev = s[1];
			c_ptr->c_sav = s[2];
			c_ptr->c_stl = s[3];
			c_ptr->c_srh = s[4];
			c_ptr->c_fos = s[5];
			c_ptr->c_thn = s[6];
			c_ptr->c_thb = s[7];

			/* Next... */
			continue;
		}

		/* Process 'x' for "Xtra skills" */
		if ((buf[0] == 'C') && (buf[2] == 'X'))
		{
			int s[8];

			/* Scan for the values */
			if (8 != sscanf(buf + 4, "%d:%d:%d:%d:%d:%d:%d:%d",
			                &s[0], &s[1], &s[2], &s[3], &s[4], &s[5], &s[6], &s[7])) return (1);

			c_ptr->x_dis = s[0];
			c_ptr->x_dev = s[1];
			c_ptr->x_sav = s[2];
			c_ptr->x_stl = s[3];
			c_ptr->x_srh = s[4];
			c_ptr->x_fos = s[5];
			c_ptr->x_thn = s[6];
			c_ptr->x_thb = s[7];

			/* Next... */
			continue;
		}

		/* Process 'P' for "xtra" */
		if ((buf[0] == 'C') && (buf[2] == 'P'))
		{
			int s[2];

			/* Scan for the values */
			if (2 != sscanf(buf + 4, "%d:%d",
			                &s[0], &s[1])) return (1);

			c_ptr->c_mhp = s[0];
			c_ptr->c_exp = s[1];

			/* Next... */
			continue;
		}

		/* Process 'C' for "sensing" */
		if ((buf[0] == 'C') && (buf[2] == 'C'))
		{
			long int s[3];
			char h, m;

			/* Scan for the values */
			if (5 != sscanf(buf + 4, "%c:%c:%ld:%ld:%ld",
			                &h, &m, &s[0], &s[1], &s[2])) return (1);

			c_ptr->sense_heavy = (h == 'H') ? TRUE : FALSE;
			c_ptr->sense_heavy_magic = (m == 'H') ? TRUE : FALSE;
			c_ptr->sense_base = s[0];
			c_ptr->sense_pl = s[1];
			c_ptr->sense_plus = s[2];

			/* Next... */
			continue;
		}

		/* Process 'B' for "blows" */
		if ((buf[0] == 'C') && (buf[2] == 'B'))
		{
			int s[3];

			/* Scan for the values */
			if (3 != sscanf(buf + 4, "%d:%d:%d",
			                &s[0], &s[1], &s[2])) return (1);

			c_ptr->blow_num = s[0];
			c_ptr->blow_wgt = s[1];
			c_ptr->blow_mul = s[2];

			/* Next... */
			continue;
		}

		/* Process 'G' for "Player flags" (multiple lines) */
		if ((buf[0] == 'C') && (buf[2] == 'G'))
		{
			if (0 != grab_one_player_race_flag(&c_ptr->flags1, &c_ptr->flags2, buf + 4))
			{
				return (5);
			}

			/* Next... */
			continue;
		}

		/* Process 'F' for "level Flags" (multiple lines) */
		if ((buf[0] == 'C') && (buf[2] == 'F'))
		{
			/* Parse every entry */
			for (s = buf + 4; *s; )
			{
				/* Find the end of this entry */
				for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) /* loop */;

				/* Nuke and skip any dividers */
				if (*t)
				{
					*t++ = '\0';
					while (*t == ' ' || *t == '|') t++;
				}

				/* Parse this entry */
				if (0 != grab_one_race_kind_flag(&c_ptr->oflags1[lev], &c_ptr->oflags2[lev], &c_ptr->oflags3[lev], &c_ptr->oflags4[lev], &c_ptr->oflags5[lev], &c_ptr->oesp[lev], s)) return (5);

				/* Start the next entry */
				s = t;
			}

			/* Next... */
			continue;
		}

		/* Specialities  */
		if ((buf[0] == 'C') && (buf[2] == 'a'))
		{
			/* Process 'N' for "New/Number/Name" */
			if (buf[4] == 'N')
			{
				/* Find the colon before the name */
				s = buf + 6;

				/* Paranoia -- require a name */
				if (!*s) return (1);
				/* Get the index */
				spec_idx++;

				/* Verify information */
				if (spec_idx >= MAX_SPEC) return (2);

				/* Point at the "info" */
				s_ptr = &c_ptr->spec[spec_idx];

				/* Copy title */
				assert(!s_ptr->title);
				s_ptr->title = my_strdup(s);

				/* Initialize */
				s_ptr->obj_num = 0;
				cur_ab = 0;
				for (z = 0; z < 10; z++)
					s_ptr->abilities[z].level = -1;

				/* Next... */
				continue;
			}

			/* Process 'D' for "Description" */
			if (buf[4] == 'D')
			{
				/* Acquire the text */
				s = buf + 6;

				if (!s_ptr->desc)
				{
					s_ptr->desc = my_strdup(s);
				}
				else
				{
					strappend(&s_ptr->desc, format("\n%s", s));
				}

				/* Next... */
				continue;
			}

			/* Process 'O' for "Object birth" */
			if (buf[4] == 'O')
			{
				int s[5];

				/* Scan for the values */
				if (5 != sscanf(buf + 6, "%d:%d:%d:%dd%d",
				                &s[0], &s[1], &s[4], &s[2], &s[3]))
				{
					s[4] = 0;

					if (4 != sscanf(buf + 6, "%d:%d:%dd%d",
					                &s[0], &s[1], &s[2], &s[3]))
					{
						return (1);
					}
				}

				s_ptr->obj_pval[s_ptr->obj_num] = s[4];
				s_ptr->obj_tval[s_ptr->obj_num] = s[0];
				s_ptr->obj_sval[s_ptr->obj_num] = s[1];
				s_ptr->obj_dd[s_ptr->obj_num] = s[2];
				s_ptr->obj_ds[s_ptr->obj_num++] = s[3];

				/* Next... */
				continue;
			}

			/* Process 'g' for "gods" */
			if (buf[4] == 'g')
			{
				int i;

				if (streq(buf + 6, "All Gods"))
					s_ptr->gods = 0xFFFFFFFF;
				else
				{
					if ((i = find_god(buf + 6)) == -1) return (1);
					s_ptr->gods |= BIT(i);
				}

				/* Next... */
				continue;
			}

			/* Process 'k' for "skills" */
			if (buf[4] == 'k')
			{
				long val, mod, i;
				char name[200], v, m;

				/* Scan for the values */
				if (5 != sscanf(buf + 6, "%c%ld:%c%ld:%s",
				                &v, &val, &m, &mod, name)) return (1);

				if ((i = find_skill(name)) == -1) return (1);
				s_ptr->skill_basem[i] = monster_ego_modify(v);
				s_ptr->skill_base[i] = val;
				s_ptr->skill_modm[i] = monster_ego_modify(m);
				s_ptr->skill_mod[i] = mod;

				/* Next... */
				continue;
			}

			/* Process 'b' for "abilities" */
			if (buf[4] == 'b')
			{
				char *sec;

				/* Scan for the values */
				if (NULL == (sec = strchr(buf + 6, ':')))
				{
					return (1);
				}
				*sec = '\0';
				sec++;
				if (!*sec) return (1);

				if ((i = find_ability(sec)) == -1) return (1);

				s_ptr->abilities[cur_ab].ability = i;
				s_ptr->abilities[cur_ab].level = atoi(buf + 6);
				cur_ab++;

				/* Next... */
				continue;
			}

			/* Process 'G' for "Player flags" (multiple lines) */
			if (buf[4] == 'G')
			{
				/* Parse every entry */
				for (s = buf + 6; *s; )
				{
					/* Find the end of this entry */
					for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) /* loop */;

					/* Nuke and skip any dividers */
					if (*t)
					{
						*t++ = '\0';
						while (*t == ' ' || *t == '|') t++;
					}

					/* Parse this entry */
					if (0 != grab_one_player_race_flag(&s_ptr->flags1, &s_ptr->flags2, s)) return (5);

					/* Start the next entry */
					s = t;
				}

				/* Next... */
				continue;
			}


			/* Process 'K' for "desired skills" */
			if (buf[4] == 'K')
			{
				long val;
				char name[200];

				/* Scan for the values */
				if (2 != sscanf(buf + 6, "%ld:%s",
				                &val, name)) return (1);

				if ((i = find_skill(name)) == -1) return (1);
				s_ptr->skill_ideal[i] = val;

				/* Next... */
				continue;
			}
		}

		/* Process 'N' for "New/Number/Name" */
		if ((buf[0] == 'M') && (buf[2] == 'N'))
		{
			/* Find the colon before the name */
			s = strchr(buf + 4, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			i = atoi(buf + 4);

			/* Verify information */
			if (i < error_idx) return (4);

			/* Verify information */
			if (i >= max_mc_idx) return (2);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			mc_ptr = &meta_class_info[i];

			/* Append chars to the name */
			strcpy(mc_ptr->name, s + 2);
			mc_ptr->color = color_char_to_attr(s[0]);
			for (powers = 0; powers < max_c_idx; powers++)
				mc_ptr->classes[powers] = -1;
			powers = 0;

			/* Next... */
			continue;
		}

		/* Process 'C' for "Classes" */
		if ((buf[0] == 'M') && (buf[2] == 'C'))
		{
			int i;

			/* Acquire the text */
			s = buf + 4;

			/* Find it in the list */
			for (i = 0; i < max_c_idx; i++)
			{
				if (class_info[i].title && iequals(s, class_info[i].title))
				{
					break;
				}
			}

			if (i == max_c_idx) return (6);

			mc_ptr->classes[powers++] = i;

			/* Next... */
			continue;
		}

		/* Oops */
		return (6);
	}

	/* Success */
	return (0);
}


/*
 * Initialize the "v_info" array, by parsing an ascii "template" file
 */
errr init_v_info_txt(FILE *fp)
{
	int i;
	char buf[1024];
	char *s;

	/* Current entry */
	vault_type *v_ptr = NULL;

	/* Just before the first record */
	error_idx = -1;

	/* Just before the first line */
	error_line = -1;

	/* Parse */
	while (0 == my_fgets(fp, buf, 1024))
	{
		/* Advance the line number */
		error_line++;

		/* Skip comments and blank lines */
		if (!buf[0] || (buf[0] == '#')) continue;
		if ((buf[0] == 'Q') || (buf[0] == 'T')) continue;

		/* Verify correct "colon" format */
		if (buf[1] != ':') return (1);

		/* Process 'N' for "New/Number/Name" */
		if (buf[0] == 'N')
		{
			/* Find the colon before the name */
			s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			i = atoi(buf + 2);

			/* Verify information */
			if (i <= error_idx) return (4);

			/* Verify information */
			if (i >= max_v_idx) return (2);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			v_ptr = &v_info[i];

			/* Initialize data -- we ignore the name, it's not
			 * used for anything */
			v_ptr->data = my_strdup("");

			/* Next... */
			continue;
		}

		/* There better be a current v_ptr */
		if (!v_ptr) return (3);

		/* Process 'D' for "Description" */
		if (buf[0] == 'D')
		{
			/* Acquire the text */
			s = buf + 2;

			/* Append data */
			v_ptr->data += s;

			/* Next... */
			continue;
		}


		/* Process 'X' for "Extra info" (one line only) */
		if (buf[0] == 'X')
		{
			int typ, rat, hgt, wid;

			/* Scan for the values */
			if (4 != sscanf(buf + 2, "%d:%d:%d:%d",
			                &typ, &rat, &hgt, &wid)) return (1);

			/* Save the values */
			v_ptr->typ = typ;
			v_ptr->rat = rat;
			v_ptr->hgt = hgt;
			v_ptr->wid = wid;

			/* Next... */
			continue;
		}

		/* There better be a current v_ptr */
		if (!v_ptr) return (3);

		/* Process monster, item and level info for special levels */
		if (buf[0] == 'Y')
		{

			int mon1, mon2, mon3, mon4, mon5, mon6, mon7, mon8, mon9;
			int mon10, item1, item2, item3, lvl, dun_type;

			/* Scan for the values */
			if (15 != sscanf(buf + 2, "%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d",
			                 &mon1, &mon2, &mon3, &mon4, &mon5, &mon6, &mon7, &mon8, &mon9, &mon10, &item1, &item2, &item3, &lvl, &dun_type)) return (1);

			/* Save the values */
			v_ptr->mon[0] = mon1;
			v_ptr->mon[1] = mon2;
			v_ptr->mon[2] = mon3;
			v_ptr->mon[3] = mon4;
			v_ptr->mon[4] = mon5;
			v_ptr->mon[5] = mon6;
			v_ptr->mon[6] = mon7;
			v_ptr->mon[7] = mon8;
			v_ptr->mon[8] = mon9;
			v_ptr->mon[9] = mon10;
			v_ptr->item[0] = item1;
			v_ptr->item[1] = item2;
			v_ptr->item[2] = item3;
			v_ptr->lvl = lvl;
			v_ptr->dun_type = dun_type;

			/* Next... */
			continue;
		}


		/* Oops */
		return (6);
	}


	/* Success */
	return (0);
}


/*
 * Grab one flag in an feature_type from a textual string
 */
static errr grab_one_feature_flag(u32b *f1, cptr what)
{
	if (lookup_flags(what, flag_tie(f1, f_info_flags1)))
	{
		return (0);
	}

	/* Oops */
	msg_format("Unknown feature flag '%s'.", what);

	/* Error */
	return (1);
}


/*
 * Initialize the "f_info" array, by parsing an ascii "template" file
 */
errr init_f_info_txt(FILE *fp)
{
	int i;
	char buf[1024];
	char *s;

	/* Current entry */
	feature_type *f_ptr = NULL;

	/* Just before the first record */
	error_idx = -1;

	/* Just before the first line */
	error_line = -1;

	/* Parse */
	while (0 == my_fgets(fp, buf, 1024))
	{
		/* Advance the line number */
		error_line++;

		/* Skip comments and blank lines */
		if (!buf[0] || (buf[0] == '#')) continue;

		/* Verify correct "colon" format */
		if (buf[1] != ':') return (1);

		/* Process 'N' for "New/Number/Name" */
		if (buf[0] == 'N')
		{
			/* Find the colon before the name */
			s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			i = atoi(buf + 2);

			/* Verify information */
			if (i <= error_idx) return (4);

			/* Verify information */
			if (i >= max_f_idx) return (2);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			f_ptr = &f_info[i];

			/* Copy name */
			assert(!f_ptr->name);
			f_ptr->name = my_strdup(s);

			/* Initialize */
			f_ptr->mimic = i;
			f_ptr->text = DEFAULT_FEAT_TEXT;
			f_ptr->tunnel = DEFAULT_FEAT_TUNNEL;
			f_ptr->block = DEFAULT_FEAT_BLOCK;

			/* Next... */
			continue;
		}

		/* There better be a current f_ptr */
		if (!f_ptr) return (3);


		/* Process 'D' for "Descriptions" */
		if (buf[0] == 'D')
		{
			/* Acquire the text */
			s = buf + 4;

			switch (buf[2])
			{
			case '0':
				assert(f_ptr->text == DEFAULT_FEAT_TEXT);
				f_ptr->text = my_strdup(s);
				break;
			case '1':
				assert(f_ptr->tunnel == DEFAULT_FEAT_TUNNEL);
				f_ptr->tunnel = my_strdup(s);
				break;
			case '2':
				assert(f_ptr->block == DEFAULT_FEAT_BLOCK);
				f_ptr->block = my_strdup(s);
				break;
			default:
				return (6);
			}

			/* Next... */
			continue;
		}


		/* Process 'M' for "Mimic" (one line only) */
		if (buf[0] == 'M')
		{
			int mimic;

			/* Scan for the values */
			if (1 != sscanf(buf + 2, "%d",
			                &mimic)) return (1);

			/* Save the values */
			f_ptr->mimic = mimic;

			/* Next... */
			continue;
		}

		/* Process 'S' for "Shimmer" (one line only) */
		if (buf[0] == 'S')
		{
			char s0, s1, s2, s3, s4, s5, s6;

			/* Scan for the values */
			if (7 != sscanf(buf + 2, "%c:%c:%c:%c:%c:%c:%c",
			                &s0, &s1, &s2, &s3, &s4, &s5, &s6)) return (1);

			/* Save the values */
			f_ptr->shimmer[0] = color_char_to_attr(s0);
			f_ptr->shimmer[1] = color_char_to_attr(s1);
			f_ptr->shimmer[2] = color_char_to_attr(s2);
			f_ptr->shimmer[3] = color_char_to_attr(s3);
			f_ptr->shimmer[4] = color_char_to_attr(s4);
			f_ptr->shimmer[5] = color_char_to_attr(s5);
			f_ptr->shimmer[6] = color_char_to_attr(s6);

			/* Next... */
			continue;
		}


		/* Process 'G' for "Graphics" (one line only) */
		if (buf[0] == 'G')
		{
			int tmp;

			/* Paranoia */
			if (!buf[2]) return (1);
			if (!buf[3]) return (1);
			if (!buf[4]) return (1);

			/* Extract the color */
			tmp = color_char_to_attr(buf[4]);

			/* Paranoia */
			if (tmp < 0) return (1);

			/* Save the values */
			f_ptr->d_attr = tmp;
			f_ptr->d_char = buf[2];

			/* Next... */
			continue;
		}

		/* Process 'E' for "Effects" (up to four lines) -SC- */
		if (buf[0] == 'E')
		{
			int side, dice, freq, type;
			cptr tmp;

			/* Find the next empty blow slot (if any) */
			for (i = 0; i < 4; i++) if ((!f_ptr->d_side[i]) &&
				                            (!f_ptr->d_dice[i])) break;

			/* Oops, no more slots */
			if (i == 4) return (1);

			/* Scan for the values */
			if (4 != sscanf(buf + 2, "%dd%d:%d:%d",
			                &dice, &side, &freq, &type))
			{
				int j;

				if (3 != sscanf(buf + 2, "%dd%d:%d",
				                &dice, &side, &freq)) return (1);

				tmp = buf + 2;
				for (j = 0; j < 2; j++)
				{
					tmp = strchr(tmp, ':');
					if (tmp == NULL) return (1);
					tmp++;
				}

				j = 0;

				while (d_info_dtypes[j].name != NULL)
					if (strcmp(d_info_dtypes[j].name, tmp) == 0)
					{
						f_ptr->d_type[i] = d_info_dtypes[j].feat;
						break;
					}
					else j++;

				if (d_info_dtypes[j].name == NULL) return (1);
			}
			else
				f_ptr->d_type[i] = type;

			freq *= 10;
			/* Save the values */
			f_ptr->d_side[i] = side;
			f_ptr->d_dice[i] = dice;
			f_ptr->d_frequency[i] = freq;

			/* Next... */
			continue;
		}

		/* Hack -- Process 'F' for flags */
		if (buf[0] == 'F')
		{
			if (0 != grab_one_feature_flag(&f_ptr->flags1, buf + 2))
			{
				return (5);
			}

			/* Next... */
			continue;
		}

		/* Oops */
		return (6);
	}

	/* Success */
	return (0);
}


/*
 * Grab one flag in an object_kind from a textual string
 */
static errr grab_one_kind_flag(object_kind *k_ptr, cptr what, bool_ obvious)
{
	/* Dispatch to correct set of flags */
	u32b *f1  = obvious ? &k_ptr->oflags1 : &k_ptr->flags1;
	u32b *f2  = obvious ? &k_ptr->oflags2 : &k_ptr->flags2;
	u32b *f3  = obvious ? &k_ptr->oflags3 : &k_ptr->flags3;
	u32b *f4  = obvious ? &k_ptr->oflags4 : &k_ptr->flags4;
	u32b *f5  = obvious ? &k_ptr->oflags5 : &k_ptr->flags5;
	u32b *esp = obvious ? &k_ptr->oesp    : &k_ptr->esp;

	/* Lookup */
	if (lookup_flags(what,
		flag_tie(f1, k_info_flags1),
		flag_tie(f2, k_info_flags2),
		flag_tie(f2, k_info_flags2_trap),
		flag_tie(f3, k_info_flags3),
		flag_tie(f4, k_info_flags4),
		flag_tie(f5, k_info_flags5),
		flag_tie(esp, esp_flags)))
	{
		return 0;
	}

	/* Oops */
	msg_format("Unknown object flag '%s'.", what);

	/* Error */
	return (1);
}

/*
 * Initialize the "k_info" array, by parsing an ascii "template" file
 */
errr init_k_info_txt(FILE *fp)
{
	int i;
	char buf[1024];
	char *s, *t;

	/* Current entry */
	object_kind *k_ptr = NULL;


	/* Just before the first record */
	error_idx = -1;

	/* Just before the first line */
	error_line = -1;


	/* Parse */
	while (0 == my_fgets(fp, buf, 1024))
	{
		/* Advance the line number */
		error_line++;

		/* Skip comments and blank lines */
		if (!buf[0] || (buf[0] == '#')) continue;

		/* Verify correct "colon" format */
		if (buf[1] != ':') return (1);


		/* Process 'N' for "New/Number/Name" */
		if (buf[0] == 'N')
		{
			/* Find the colon before the name */
			s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			i = atoi(buf + 2);

			/* Verify information */
			if (i <= error_idx) return (4);

			/* Verify information */
			if (i >= max_k_idx) return (2);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			k_ptr = &k_info[i];

			/* Advance and Save the name index */
			assert(!k_ptr->name);
			k_ptr->name = my_strdup(s);

			/* Ensure empty description */
			k_ptr->text = my_strdup("");

			/* Needed hack */
			k_ptr->esp = 0;
			k_ptr->power = -1;

			/* Next... */
			continue;
		}

		/* There better be a current k_ptr */
		if (!k_ptr) return (3);

		/* Process 'D' for "Description" */
		if (buf[0] == 'D')
		{
			/* Acquire the text */
			s = buf + 2;

			if (!k_ptr->text)
			{
				k_ptr->text = my_strdup(s);
			}
			else
			{
				strappend(&k_ptr->text, format("\n%s", s));
			}

			/* Next... */
			continue;
		}

		/* Process 'G' for "Graphics" (one line only) */
		if (buf[0] == 'G')
		{
			char sym;
			int tmp;

			/* Paranoia */
			if (!buf[2]) return (1);
			if (!buf[3]) return (1);
			if (!buf[4]) return (1);

			/* Extract the char */
			sym = buf[2];

			/* Extract the attr */
			tmp = color_char_to_attr(buf[4]);

			/* Paranoia */
			if (tmp < 0) return (1);

			/* Save the values */
			k_ptr->d_attr = tmp;
			k_ptr->d_char = sym;

			/* Next... */
			continue;
		}

		/* Process 'I' for "Info" (one line only) */
		if (buf[0] == 'I')
		{
			int tval, sval, pval, pval2 = 0;

			/* Scan for the values */
			if (4 != sscanf(buf + 2, "%d:%d:%d:%d",
			                &tval, &sval, &pval, &pval2))
			{
				char spl[70];

				if (4 != sscanf(buf + 2, "%d:%d:%d:SPELL=%s",
				                &tval, &sval, &pval, spl))
				{
					if (3 != sscanf(buf + 2, "%d:%d:%d",
					                &tval, &sval, &pval))
						return (1);
				}
				else
				{
					char *spl = strchr(buf + 2, '=') + 1;

					pval2 = find_spell(spl);
				}
			}

			/* Save the values */
			k_ptr->tval = tval;
			k_ptr->sval = sval;
			k_ptr->pval = pval;
			k_ptr->pval2 = pval2;

			/* Next... */
			continue;
		}

		/* Process 'W' for "More Info" (one line only) */
		if (buf[0] == 'W')
		{
			int level, unused, wgt;
			long cost;

			/* Scan for the values */
			if (4 != sscanf(buf + 2, "%d:%d:%d:%ld",
					&level, &unused, &wgt, &cost)) return (1);

			/* Save the values */
			k_ptr->level = level;
			k_ptr->weight = wgt;
			k_ptr->cost = cost;

			/* Next... */
			continue;
		}

		/* Process 'T' for "arTifact Info" (one line only) */
		if (buf[0] == 'T')
		{
			int btval, bsval;

			/* Scan for the values */
			if (2 != sscanf(buf + 2, "%d:%d",
			                &btval, &bsval)) return (1);

			/* Save the values */
			k_ptr->btval = btval;
			k_ptr->bsval = bsval;

			/* Next... */
			continue;
		}

		/* Process 'Z' for "Granted power" */
		if (buf[0] == 'Z')
		{
			int i;

			/* Acquire the text */
			s = buf + 2;

			/* Find it in the list */
			for (i = 0; i < POWER_MAX; i++)
			{
				if (iequals(s, powers_type[i].name)) break;
			}

			if (i == POWER_MAX) return (6);

			k_ptr->power = i;

			/* Next... */
			continue;
		}

		/* Process 'a' for Activation */
		if ( buf[0] == 'a')
		{
			k_ptr->activate = get_activation(buf + 2);
			if (k_ptr->activate == -1)
			{
				return 1;
			}

			/* Next... */
			continue;
		}

		/* Process 'A' for "Allocation" (one line only) */
		if (buf[0] == 'A')
		{
			int i;

			/* XXX XXX XXX Simply read each number following a colon */
			for (i = 0, s = buf + 1; s && (s[0] == ':') && s[1]; ++i)
			{
				if (i >= ALLOCATION_MAX) {
					msg_print("Too many allocation entries.");
					return 1;
				}

				/* Default chance */
				k_ptr->chance[i] = 1;

				/* Store the level */
				k_ptr->locale[i] = atoi(s + 1);

				/* Find the slash */
				t = strchr(s + 1, '/');

				/* Find the next colon */
				s = strchr(s + 1, ':');

				/* If the slash is "nearby", use it */
				if (t && (!s || t < s))
				{
					int chance = atoi(t + 1);
					if (chance > 0) {
						k_ptr->chance[i] = chance;
					}
				}
			}

			/* Next... */
			continue;
		}

		/* Hack -- Process 'P' for "power" and such */
		if (buf[0] == 'P')
		{
			int ac, hd1, hd2, th, td, ta;

			/* Scan for the values */
			if (6 != sscanf(buf + 2, "%d:%dd%d:%d:%d:%d",
			                &ac, &hd1, &hd2, &th, &td, &ta)) return (1);

			k_ptr->ac = ac;
			k_ptr->dd = hd1;
			k_ptr->ds = hd2;
			k_ptr->to_h = th;
			k_ptr->to_d = td;
			k_ptr->to_a = ta;

			/* Next... */
			continue;
		}

		/* Hack -- Process 'F' for flags */
		if (buf[0] == 'F')
		{
			if (0 != grab_one_kind_flag(k_ptr, buf + 2, FALSE))
			{
				return (5);
			}

			/* Next... */
			continue;
		}

		/* Hack -- Process 'f' for obvious flags */
		if (buf[0] == 'f')
		{
			if (0 != grab_one_kind_flag(k_ptr, buf + 2, TRUE))
			{
				return (5);
			}

			/* Next... */
			continue;
		}


		/* Oops */
		return (6);
	}

	/* Success */
	return (0);
}

/*
 * Grab one flag in an artifact_type from a textual string
 */
static errr grab_one_artifact_flag(artifact_type *a_ptr, cptr what, bool_ obvious)
{
	/* Dispatch to correct set of flags */
	u32b *f1  = obvious ? &a_ptr->oflags1 : &a_ptr->flags1;
	u32b *f2  = obvious ? &a_ptr->oflags2 : &a_ptr->flags2;
	u32b *f3  = obvious ? &a_ptr->oflags3 : &a_ptr->flags3;
	u32b *f4  = obvious ? &a_ptr->oflags4 : &a_ptr->flags4;
	u32b *f5  = obvious ? &a_ptr->oflags5 : &a_ptr->flags5;
	u32b *esp = obvious ? &a_ptr->oesp    : &a_ptr->esp;

	/* Lookup */
	if (lookup_flags(what,
		flag_tie(f1, k_info_flags1),
		flag_tie(f2, k_info_flags2),
		flag_tie(f2, k_info_flags2_trap),
		flag_tie(f3, k_info_flags3),
		flag_tie(f4, k_info_flags4),
		flag_tie(f5, k_info_flags5),
		flag_tie(esp, esp_flags)))
	{
		return 0;
	}

	/* Oops */
	msg_format("Unknown artifact flag '%s'.", what);

	/* Error */
	return (1);
}




/*
 * Initialize the "a_info" array, by parsing an ascii "template" file
 */
errr init_a_info_txt(FILE *fp)
{
	int i;
	char buf[1024];
	char *s;

	/* Current entry */
	artifact_type *a_ptr = NULL;


	/* Just before the first record */
	error_idx = -1;

	/* Just before the first line */
	error_line = -1;


	/* Parse */
	while (0 == my_fgets(fp, buf, 1024))
	{
		/* Advance the line number */
		error_line++;

		/* Skip comments and blank lines */
		if (!buf[0] || (buf[0] == '#')) continue;

		/* Verify correct "colon" format */
		if (buf[1] != ':') return (1);

		/* Process 'N' for "New/Number/Name" */
		if (buf[0] == 'N')
		{
			/* Find the colon before the name */
			s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			i = atoi(buf + 2);

			/* Verify information */
			if (i < error_idx) return (4);

			/* Verify information */
			if (i >= max_a_idx) return (2);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			a_ptr = &a_info[i];

			/* Copy name */
			assert(!a_ptr->name);
			a_ptr->name = my_strdup(s);

			/* Ensure empty description */
			a_ptr->text = my_strdup("");

			/* Ignore everything */
			a_ptr->flags3 |= (TR3_IGNORE_ACID);
			a_ptr->flags3 |= (TR3_IGNORE_ELEC);
			a_ptr->flags3 |= (TR3_IGNORE_FIRE);
			a_ptr->flags3 |= (TR3_IGNORE_COLD);

			/* Needed hack */
			a_ptr->esp = 0;
			a_ptr->power = -1;

			/*Require activating artifacts to have a activation type */
			if (a_ptr && a_ptr->flags3 & TR3_ACTIVATE && !a_ptr->activate)
			{
				msg_print("Activate flag without activate type");
				return 1;
			}

			/* Next... */
			continue;
		}

		/* There better be a current a_ptr */
		if (!a_ptr) return (3);

		/* Process 'D' for "Description" */
		if (buf[0] == 'D')
		{
			/* Acquire the text */
			s = buf + 2;

			/* Add separator if necessary */
			if (*a_ptr->text != '\0' && !ends_with(a_ptr->text, " ")) {
				strappend(&a_ptr->text, " ");
			}

			/* Append to description */
			strappend(&a_ptr->text, s);

			/* Next... */
			continue;
		}

		/* Process 'I' for "Info" (one line only) */
		if (buf[0] == 'I')
		{
			int tval, sval, pval;

			/* Scan for the values */
			if (3 != sscanf(buf + 2, "%d:%d:%d",
			                &tval, &sval, &pval))
			{
				return (1);
			}

			/* Save the values */
			a_ptr->tval = tval;
			a_ptr->sval = sval;
			a_ptr->pval = pval;

			/* Verify */
			if (!lookup_kind(tval, sval)) return (6);

			/* Next... */
			continue;
		}

		/* Process 'W' for "More Info" (one line only) */
		if (buf[0] == 'W')
		{
			int level, rarity, wgt;
			long cost;

			/* Scan for the values */
			if (4 != sscanf(buf + 2, "%d:%d:%d:%ld",
			                &level, &rarity, &wgt, &cost)) return (1);

			/* Save the values */
			a_ptr->level = level;
			a_ptr->rarity = rarity;
			a_ptr->weight = wgt;
			a_ptr->cost = cost;

			/* Next... */
			continue;
		}

		/* Hack -- Process 'P' for "power" and such */
		if (buf[0] == 'P')
		{
			int ac, hd1, hd2, th, td, ta;

			/* Scan for the values */
			if (6 != sscanf(buf + 2, "%d:%dd%d:%d:%d:%d",
			                &ac, &hd1, &hd2, &th, &td, &ta)) return (1);

			a_ptr->ac = ac;
			a_ptr->dd = hd1;
			a_ptr->ds = hd2;
			a_ptr->to_h = th;
			a_ptr->to_d = td;
			a_ptr->to_a = ta;

			/* Next... */
			continue;
		}

		/* Process 'Z' for "Granted power" */
		if (buf[0] == 'Z')
		{
			int i;

			/* Acquire the text */
			s = buf + 2;

			/* Find it in the list */
			for (i = 0; i < POWER_MAX; i++)
			{
				if (iequals(s, powers_type[i].name)) break;
			}

			if (i == POWER_MAX) return (6);

			a_ptr->power = i;

			/* Next... */
			continue;
		}

		/* Hack -- Process 'F' for flags */
		if (buf[0] == 'F')
		{
			if (0 != grab_one_artifact_flag(a_ptr, buf+2, FALSE)) return (5);

			/* Next... */
			continue;
		}

		/* Hack -- Process 'f' for obvious flags */
		if (buf[0] == 'f')
		{
			if (0 != grab_one_artifact_flag(a_ptr, buf+2, TRUE)) return (5);

			/* Next... */
			continue;
		}

		/* Read activation type. */
		if (buf[0] == 'a')
		{
			a_ptr->activate = get_activation(buf + 2);
			if (a_ptr->activate == -1)
			{
				return 1;
			}

			/* Next... */
			continue;
		}


		/* Oops */
		return (6);
	}

	/* Success */
	return (0);
}

/*
* Initialize the "set_info" array, by parsing an ascii "template" file
*/
errr init_set_info_txt(FILE *fp)
{
	int i;
	int cur_art = 0, cur_num = 0;
	char buf[1024];

	char *s;

	/* Current entry */
	set_type *set_ptr = NULL;


	/* Just before the first record */
	error_idx = -1;

	/* Just before the first line */
	error_line = -1;


	/* Parse */
	while (0 == my_fgets(fp, buf, 1024))
	{
		/* Advance the line number */
		error_line++;

		/* Skip comments and blank lines */
		if (!buf[0] || (buf[0] == '#')) continue;

		/* Verify correct "colon" format */
		if (buf[1] != ':') return (1);

		/* Process 'N' for "New/Number/Name" */
		if (buf[0] == 'N')
		{
			int z, y;

			/* Find the colon before the name */
			s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			i = atoi(buf + 2);

			/* Verify information */
			if (i < error_idx) return (4);

			/* Verify information */
			if (i >= max_set_idx) return (2);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			set_ptr = &set_info[i];

			/* Copy name */
			assert(!set_ptr->name);
			set_ptr->name = my_strdup(s);

			/* Initialize */
			set_ptr->num = 0;
			set_ptr->num_use = 0;
			for (z = 0; z < 6; z++)
			{
				set_ptr->arts[z].a_idx = 0;
				set_ptr->arts[z].present = FALSE;
				for (y = 0; y < 6; y++)
				{
					set_ptr->arts[z].flags1[y] = 0;
					set_ptr->arts[z].flags2[y] = 0;
					set_ptr->arts[z].flags3[y] = 0;
					set_ptr->arts[z].flags4[y] = 0;
					set_ptr->arts[z].flags5[y] = 0;
					set_ptr->arts[z].esp[y] = 0;
					set_ptr->arts[z].pval[y] = 0;
				}
			}

			/* Next... */
			continue;
		}

		/* There better be a current set_ptr */
		if (!set_ptr) return (3);

		/* Process 'D' for "Description" */
		if (buf[0] == 'D')
		{
			/* Acquire the text */
			s = buf + 2;

			/* Append chars to the description */
			strappend(&set_ptr->desc, s);

			/* Next... */
			continue;
		}

		/* Process 'P' for "Power" (up to 6) */
		if (buf[0] == 'P')
		{
			int a_idx, num, pval;
			int z;

			/* Scan for the values */
			if (3 != sscanf(buf + 2, "%d:%d:%d",
			                &a_idx, &num, &pval))
			{
				return (1);
			}

			for (z = 0; z < set_ptr->num; z++)
				if (set_ptr->arts[z].a_idx == a_idx) break;
			if (z == set_ptr->num)
			{
				set_ptr->num++;
				set_ptr->arts[z].a_idx = a_idx;
			}

			/* Save the values */
			set_ptr->arts[z].pval[num - 1] = pval;
			cur_art = z;
			cur_num = num - 1;

			/* Next... */
			continue;
		}

		/* Process 'F' for flags */
		if (buf[0] == 'F')
		{
			/* Parse this entry */
			if (0 != grab_one_race_kind_flag(&set_ptr->arts[cur_art].flags1[cur_num],
							 &set_ptr->arts[cur_art].flags2[cur_num],
							 &set_ptr->arts[cur_art].flags3[cur_num],
							 &set_ptr->arts[cur_art].flags4[cur_num],
							 &set_ptr->arts[cur_art].flags5[cur_num],
							 &set_ptr->arts[cur_art].esp[cur_num],
							 buf + 2))
			{
				return (5);
			}

			/* Next... */
			continue;
		}


		/* Oops */
		return (6);
	}

	/* Success */
	return (0);
}


/*
 * Initialize the "s_info" array, by parsing an ascii "template" file
 */
errr init_s_info_txt(FILE *fp)
{
	int i, z, order = 1;
	char buf[1024];
	char *s;

	/* Current entry */
	skill_type *s_ptr = NULL;


	/* Just before the first record */
	error_idx = -1;

	/* Just before the first line */
	error_line = -1;


	/* Parse */
	while (0 == my_fgets(fp, buf, 1024))
	{
		/* Advance the line number */
		error_line++;

		/* Skip comments and blank lines */
		if (!buf[0] || (buf[0] == '#')) continue;

		/* Verify correct "colon" format */
		if (buf[1] != ':') return (1);

		/* Process 'T' for "skill Tree" */
		if (buf[0] == 'T')
		{
			char *sec;
			s16b s1, s2;

			/* Scan for the values */
			if (NULL == (sec = strchr(buf + 2, ':')))
			{
				return (1);
			}
			*sec = '\0';
			sec++;
			if (!*sec) return (1);

			s1 = find_skill(buf + 2);
			s2 = find_skill(sec);
			if (s2 == -1) return (1);

			s_info[s2].father = s1;
			s_info[s2].order = order++;

			/* Next... */
			continue;
		}

		/* Process 'E' for "Exclusive" */
		if (buf[0] == 'E')
		{
			char *sec;
			s16b s1, s2;

			/* Scan for the values */
			if (NULL == (sec = strchr(buf + 2, ':')))
			{
				return (1);
			}
			*sec = '\0';
			sec++;
			if (!*sec) return (1);

			s1 = find_skill(buf + 2);
			s2 = find_skill(sec);
			if ((s1 == -1) || (s2 == -1)) return (1);

			s_info[s1].action[s2] = SKILL_EXCLUSIVE;
			s_info[s2].action[s1] = SKILL_EXCLUSIVE;

			/* Next... */
			continue;
		}


		/* Process 'O' for "Opposite" */
		if (buf[0] == 'O')
		{
			char *sec, *cval;
			s16b s1, s2;

			/* Scan for the values */
			if (NULL == (sec = strchr(buf + 2, ':')))
			{
				return (1);
			}
			*sec = '\0';
			sec++;
			if (!*sec) return (1);
			if (NULL == (cval = strchr(sec, '%')))
			{
				return (1);
			}
			*cval = '\0';
			cval++;
			if (!*cval) return (1);

			s1 = find_skill(buf + 2);
			s2 = find_skill(sec);
			if ((s1 == -1) || (s2 == -1)) return (1);

			s_info[s1].action[s2] = -atoi(cval);

			/* Next... */
			continue;
		}

		/* Process 'A' for "Amical/friendly" */
		if (buf[0] == 'f')
		{
			char *sec, *cval;
			s16b s1, s2;

			/* Scan for the values */
			if (NULL == (sec = strchr(buf + 2, ':')))
			{
				return (1);
			}
			*sec = '\0';
			sec++;
			if (!*sec) return (1);
			if (NULL == (cval = strchr(sec, '%')))
			{
				return (1);
			}
			*cval = '\0';
			cval++;
			if (!*cval) return (1);

			s1 = find_skill(buf + 2);
			s2 = find_skill(sec);
			if ((s1 == -1) || (s2 == -1)) return (1);

			s_info[s1].action[s2] = atoi(cval);

			/* Next... */
			continue;
		}

		/* Process 'N' for "New/Number/Name" */
		if (buf[0] == 'N')
		{
			/* Find the colon before the name */
			s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			i = atoi(buf + 2);

			/* Verify information */
			if (i >= max_s_idx) return (2);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			s_ptr = &s_info[i];

			/* Copy name */
			assert(!s_ptr->name);
			s_ptr->name = my_strdup(s);

			/* Init */
			s_ptr->action_mkey = 0;
			s_ptr->dev = FALSE;
			s_ptr->random_gain_chance = 100;
			for (z = 0; z < max_s_idx; z++)
			{
				s_ptr->action[z] = 0;
			}

			/* Next... */
			continue;
		}

		/* There better be a current s_ptr */
		if (!s_ptr) return (3);

		/* Process 'D' for "Description" */
		if (buf[0] == 'D')
		{
			/* Acquire the text */
			s = buf + 2;

			/* Description */
			if (!s_ptr->desc)
			{
				s_ptr->desc = my_strdup(s);
			}
			else
			{
				strappend(&s_ptr->desc, format("\n%s", s));
			}

			/* Next... */
			continue;
		}

		/* Process 'A' for "Activation Description" (one line only) */
		if (buf[0] == 'A')
		{
			char *txt;

			/* Acquire the text */
			s = buf + 2;

			if (NULL == (txt = strchr(s, ':'))) return (1);
			*txt = '\0';
			txt++;

			/* Copy action description */
			assert(!s_ptr->action_desc);
			s_ptr->action_desc = my_strdup(txt);

			/* Copy mkey index */
			s_ptr->action_mkey = atoi(s);

			/* Next... */
			continue;
		}

		/* Process 'I' for "Info" (one line only) */
		if (buf[0] == 'I')
		{
			int rate;

			/* Scan for the values */
			if (1 != sscanf(buf + 2, "%d", &rate))
			{
				return (1);
			}

			/* Save the values */
			s_ptr->rate = rate;

			/* Next... */
			continue;
		}

		/* Process 'G' for "random Gain" (one line only) */
		if (buf[0] == 'G')
		{
			int chance;

			/* Scan for the values */
			if (1 != sscanf(buf + 2, "%d", &chance))
			{
				return (1);
			}

			/* Save the values */
			s_ptr->random_gain_chance = chance;

			/* Next... */
			continue;
		}

		/* Process 'F' for flags */
		if (buf[0] == 'F')
		{
			if (0 != grab_one_skill_flag(&s_ptr->flags1, buf + 2))
			{
				return (5);
			}

			/* Next... */
			continue;
		}

		/* Oops */
		return (6);
	}

	/* Success */
	return (0);
}

/*
 * Initialize the "ab_info" array, by parsing an ascii "template" file
 */
errr init_ab_info_txt(FILE *fp)
{
	int i, z;
	char buf[1024];
	char *s;

	/* Current entry */
	ability_type *ab_ptr = NULL;


	/* Just before the first record */
	error_idx = -1;

	/* Just before the first line */
	error_line = -1;


	/* Parse */
	while (0 == my_fgets(fp, buf, 1024))
	{
		/* Advance the line number */
		error_line++;

		/* Skip comments and blank lines */
		if (!buf[0] || (buf[0] == '#')) continue;

		/* Verify correct "colon" format */
		if (buf[1] != ':') return (1);

		/* Process 'N' for "New/Number/Name" */
		if (buf[0] == 'N')
		{
			/* Find the colon before the name */
			s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			i = atoi(buf + 2);

			/* Verify information */
			if (i >= max_ab_idx) return (2);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			ab_ptr = &ab_info[i];

			/* Copy name */
			assert(!ab_ptr->name);
			ab_ptr->name = my_strdup(s);

			/* Init */
			ab_ptr->action_mkey = 0;
			ab_ptr->acquired = FALSE;
			for (z = 0; z < 10; z++)
			{
				ab_ptr->skills[z] = -1;
				ab_ptr->need_abilities[z] = -1;
				ab_ptr->forbid_abilities[z] = -1;
			}
			for (z = 0; z < 6; z++)
			{
				ab_ptr->stat[z] = -1;
			}

			/* Next... */
			continue;
		}

		/* There better be a current ab_ptr */
		if (!ab_ptr) return (3);

		/* Process 'D' for "Description" */
		if (buf[0] == 'D')
		{
			/* Acquire the text */
			s = buf + 2;

			/* Append description */
			if (!ab_ptr->desc)
			{
				ab_ptr->desc = my_strdup(s);
			}
			else
			{
				strappend(&ab_ptr->desc, format("\n%s", s));
			}

			/* Next... */
			continue;
		}

		/* Process 'A' for "Activation Description" */
		if (buf[0] == 'A')
		{
			char *txt;

			/* Acquire the text */
			s = buf + 2;

			if (NULL == (txt = strchr(s, ':'))) return (1);
			*txt = '\0';
			txt++;

			/* Copy name */
			assert(!ab_ptr->action_desc);
			ab_ptr->action_desc = my_strdup(txt);

			/* Set mkey */
			ab_ptr->action_mkey = atoi(s);

			/* Next... */
			continue;
		}

		/* Process 'I' for "Info" (one line only) */
		if (buf[0] == 'I')
		{
			int cost;

			/* Scan for the values */
			if (1 != sscanf(buf + 2, "%d", &cost))
			{
				return (1);
			}

			/* Save the values */
			ab_ptr->cost = cost;

			/* Next... */
			continue;
		}

		/* Process 'k' for "Skill" */
		if (buf[0] == 'k')
		{
			char *sec;
			s16b level, skill;

			/* Scan for the values */
			if (NULL == (sec = strchr(buf + 2, ':')))
			{
				return (1);
			}
			*sec = '\0';
			sec++;
			if (!*sec) return (1);

			level = atoi(buf + 2);
			skill = find_skill(sec);

			if (skill == -1) return (1);

			for (z = 0; z < 10; z++)
				if (ab_ptr->skills[z] == -1) break;

			if (z < 10)
			{
				ab_ptr->skills[z] = skill;
				ab_ptr->skill_levels[z] = level;
			}

			/* Next... */
			continue;
		}

		/* Process 'a' for "needed ability" */
		if (buf[0] == 'a')
		{
			s16b ab;

			ab = find_ability(buf + 2);

			if (ab == -1) return (1);

			for (z = 0; z < 10; z++)
				if (ab_ptr->need_abilities[z] == -1) break;

			if (z < 10)
			{
				ab_ptr->need_abilities[z] = ab;
			}

			/* Next... */
			continue;
		}

		/* Process 'S' for "Stat" */
		if (buf[0] == 'S')
		{
			char *sec;
			s16b stat;

			/* Scan for the values */
			if (NULL == (sec = strchr(buf + 2, ':')))
			{
				return (1);
			}
			*sec = '\0';
			sec++;
			if (!*sec) return (1);

			for (stat = 0; stat < 6; stat++)
			{
				if (!strcmp(stat_names[stat], sec))
					break;
			}

			if (stat == 6) return (1);

			ab_ptr->stat[stat] = atoi(buf + 2);

			/* Next... */
			continue;
		}

		/* Process 'E' for "Excluding ability" */
		if (buf[0] == 'E')
		{
			char *sec;
			s16b ab1, ab2;

			/* Scan for the values */
			if (NULL == (sec = strchr(buf + 2, ':')))
			{
				return (1);
			}
			*sec = '\0';
			sec++;
			if (!*sec) return (1);

			ab1 = find_ability(buf + 2);
			ab2 = find_ability(sec);

			if ((ab1 == -1) || (ab2 == -1)) return (1);

			for (z = 0; z < 10; z++)
				if (ab_info[ab1].forbid_abilities[z] == -1) break;
			if (z < 10)
			{
				ab_info[ab1].forbid_abilities[z] = ab2;
			}

			for (z = 0; z < 10; z++)
				if (ab_info[ab2].forbid_abilities[z] == -1) break;
			if (z < 10)
			{
				ab_info[ab2].forbid_abilities[z] = ab1;
			}

			/* Next... */
			continue;
		}

		/* Oops */
		return (6);
	}

	/* Success */
	return (0);
}


/*
 * Grab one flag in a ego-item_type from a textual string
 */
static bool_ grab_one_ego_item_flag(ego_item_type *e_ptr, cptr what, int n, bool_ obvious)
{
	assert(n < FLAG_RARITY_MAX);

	/* Dispatch to correct set of flags */
	u32b *f1  = obvious ? &e_ptr->oflags1[n] : &e_ptr->flags1[n];
	u32b *f2  = obvious ? &e_ptr->oflags2[n] : &e_ptr->flags2[n];
	u32b *f3  = obvious ? &e_ptr->oflags3[n] : &e_ptr->flags3[n];
	u32b *f4  = obvious ? &e_ptr->oflags4[n] : &e_ptr->flags4[n];
	u32b *f5  = obvious ? &e_ptr->oflags5[n] : &e_ptr->flags5[n];
	u32b *esp = obvious ? &e_ptr->oesp[n]    : &e_ptr->esp[n];
	u32b *ego = obvious ? &e_ptr->fego[n]    : &e_ptr->fego[n];

	/* Lookup */
	if (lookup_flags(what,
		flag_tie(f1,  k_info_flags1),
		flag_tie(f2,  k_info_flags2),
		flag_tie(f2,  k_info_flags2_trap),
		flag_tie(f3,  k_info_flags3),
		flag_tie(f4,  k_info_flags4),
		flag_tie(f5,  k_info_flags5),
		flag_tie(esp, esp_flags),
		flag_tie(ego, ego_flags)))
	{
		return (0);
	}

	/* Oops */
	msg_format("Unknown ego-item flag '%s'.", what);

	/* Error */
	return (1);
}

static bool_ grab_one_ego_item_flag_restrict(ego_item_type *e_ptr, cptr what, bool_ need)
{
	/* Dispatch to correct set of flags */
	u32b *f1  = need ? &e_ptr->need_flags1 : &e_ptr->forbid_flags1;
	u32b *f2  = need ? &e_ptr->need_flags2 : &e_ptr->forbid_flags2;
	u32b *f3  = need ? &e_ptr->need_flags3 : &e_ptr->forbid_flags3;
	u32b *f4  = need ? &e_ptr->need_flags4 : &e_ptr->forbid_flags4;
	u32b *f5  = need ? &e_ptr->need_flags5 : &e_ptr->forbid_flags5;
	u32b *esp = need ? &e_ptr->need_esp    : &e_ptr->forbid_esp;

	/* Lookup */
	if (lookup_flags(what,
		flag_tie(f1, k_info_flags1),
		flag_tie(f2, k_info_flags2),
		flag_tie(f2, k_info_flags2_trap),
		flag_tie(f3, k_info_flags3),
		flag_tie(f4, k_info_flags4),
		flag_tie(f5, k_info_flags5),
		flag_tie(esp, esp_flags)))
	{
		return 0;
	}

	/* Oops */
	msg_format("Unknown ego-item restrict flag '%s'.", what);

	/* Error */
	return (1);
}




/*
 * Initialize the "e_info" array, by parsing an ascii "template" file
 */
errr init_e_info_txt(FILE *fp)
{
	int i, cur_r = -1, cur_t = 0, j;
	char buf[1024];
	char *s, *t;

	/* Current entry */
	ego_item_type *e_ptr = NULL;


	/* Just before the first record */
	error_idx = -1;

	/* Just before the first line */
	error_line = -1;


	/* Parse */
	while (0 == my_fgets(fp, buf, 1024))
	{
		/* Advance the line number */
		error_line++;

		/* Skip comments and blank lines */
		if (!buf[0] || (buf[0] == '#')) continue;

		/* Verify correct "colon" format */
		if (buf[1] != ':') return (1);

		/* Process 'N' for "New/Number/Name" */
		if (buf[0] == 'N')
		{
			/* Find the colon before the name */
			s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			i = atoi(buf + 2);

			/* Verify information */
			if (i < error_idx) return (4);

			/* Verify information */
			if (i >= max_e_idx) return (2);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			e_ptr = &e_info[i];

			/* Copy name */
			assert(!e_ptr->name);
			e_ptr->name = my_strdup(s);

			/* Needed hack */
			e_ptr->power = -1;
			cur_r = -1;
			cur_t = 0;

			for (j = 0; j < 10; j++)
			{
				e_ptr->tval[j] = 255;
			}
			for (j = 0; j < FLAG_RARITY_MAX; j++)
			{
				e_ptr->rar[j] = 0;
				e_ptr->flags1[j] = 0;
				e_ptr->flags2[j] = 0;
				e_ptr->flags3[j] = 0;
				e_ptr->flags4[j] = 0;
				e_ptr->flags5[j] = 0;
				e_ptr->esp[j] = 0;
				e_ptr->oflags1[j] = 0;
				e_ptr->oflags2[j] = 0;
				e_ptr->oflags3[j] = 0;
				e_ptr->oflags4[j] = 0;
				e_ptr->oflags5[j] = 0;
				e_ptr->oesp[j] = 0;
				e_ptr->fego[j] = 0;
			}

			/* Next... */
			continue;
		}

		/* There better be a current e_ptr */
		if (!e_ptr) return (3);


		/* Process 'T' for "Tval/Sval" (up to 5 lines) */
		if (buf[0] == 'T')
		{
			int tv, minsv, maxsv;

			if (cur_t == 10) return 1;

			/* Scan for the values */
			if (3 != sscanf(buf + 2, "%d:%d:%d",
			                &tv, &minsv, &maxsv)) return (1);

			/* Save the values */
			e_ptr->tval[cur_t] = tv;
			e_ptr->min_sval[cur_t] = minsv;
			e_ptr->max_sval[cur_t] = maxsv;

			cur_t++;

			/* Next... */
			continue;
		}

		/* Process 'R' for "flags rarity" (up to 5 lines) */
		if (buf[0] == 'R')
		{
			int rar;

			cur_r++;

			if (cur_r >= FLAG_RARITY_MAX) {
				return 1;
			}

			/* Scan for the values */
			if (1 != sscanf(buf + 2, "%d",
			                &rar)) return (1);

			/* Save the values */
			e_ptr->rar[cur_r] = rar;

			/* Next... */
			continue;
		}

		/* Process 'X' for "Xtra" (one line only) */
		if (buf[0] == 'X')
		{
			int slot, rating;
			char pos;

			/* Scan for the values */
			if (3 != sscanf(buf + 2, "%c:%d:%d",
			                &pos, &slot, &rating)) return (1);

			/* Save the values */
			/* e_ptr->slot = slot; */
			e_ptr->rating = rating;
			e_ptr->before = (pos == 'B') ? TRUE : FALSE;

			/* Next... */
			continue;
		}

		/* Process 'W' for "More Info" (one line only) */
		if (buf[0] == 'W')
		{
			int level, rarity, rarity2;
			long cost;

			/* Scan for the values */
			if (4 != sscanf(buf + 2, "%d:%d:%d:%ld",
			                &level, &rarity, &rarity2, &cost)) return (1);

			/* Save the values */
			e_ptr->level = level;
			e_ptr->rarity = rarity;
			e_ptr->mrarity = rarity2;
			e_ptr->cost = cost;

			/* Next... */
			continue;
		}

		/* Hack -- Process 'C' for "creation" */
		if (buf[0] == 'C')
		{
			int th, td, ta, pv;

			/* Scan for the values */
			if (4 != sscanf(buf + 2, "%d:%d:%d:%d",
			                &th, &td, &ta, &pv)) return (1);

			e_ptr->max_to_h = th;
			e_ptr->max_to_d = td;
			e_ptr->max_to_a = ta;
			e_ptr->max_pval = pv;

			/* Next... */
			continue;
		}

		/* Process 'Z' for "Granted power" */
		if (buf[0] == 'Z')
		{
			int i;

			/* Acquire the text */
			s = buf + 2;

			/* Find it in the list */
			for (i = 0; i < POWER_MAX; i++)
			{
				if (iequals(s, powers_type[i].name)) break;
			}

			if (i == POWER_MAX) return (6);

			e_ptr->power = i;

			/* Next... */
			continue;
		}

		if (buf[0] == 'a')
		{
			e_ptr->activate = get_activation(buf + 2);
			if (e_ptr->activate == -1)
			{
					return 1;
			}

			/* Next... */
			continue;
		}

		/* Hack -- Process 'r:N' for needed flags */
		if ((buf[0] == 'r') && (buf[2] == 'N'))
		{
			/* Parse every entry textually */
			for (s = buf + 4; *s; )
			{
				/* Find the end of this entry */
				for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) /* loop */;

				/* Nuke and skip any dividers */
				if (*t)
				{
					*t++ = '\0';
					while ((*t == ' ') || (*t == '|')) t++;
				}

				/* Parse this entry */
				if (0 != grab_one_ego_item_flag_restrict(e_ptr, s, TRUE)) return (5);

				/* Start the next entry */
				s = t;
			}

			/* Next... */
			continue;
		}

		/* Hack -- Process 'r:F' for forbidden flags */
		if ((buf[0] == 'r') && (buf[2] == 'F'))
		{
			if (0 != grab_one_ego_item_flag_restrict(e_ptr, buf + 4, FALSE))
			{
				return (5);
			}

			/* Next... */
			continue;
		}

		/* Hack -- Process 'F' for flags */
		if (buf[0] == 'F')
		{
			if (cur_r == -1) return (6);

			/* Parse every entry textually */
			for (s = buf + 2; *s; )
			{
				/* Find the end of this entry */
				for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) /* loop */;

				/* Nuke and skip any dividers */
				if (*t)
				{
					*t++ = '\0';
					while ((*t == ' ') || (*t == '|')) t++;
				}

				/* Parse this entry */
				if (0 != grab_one_ego_item_flag(e_ptr, s, cur_r, FALSE)) return (5);

				/* Start the next entry */
				s = t;
			}

			/* Next... */
			continue;
		}

		/* Hack -- Process 'f' for obvious flags */
		if (buf[0] == 'f')
		{
			if (cur_r == -1) return (6);

			/* Parse every entry textually */
			for (s = buf + 2; *s; )
			{
				/* Find the end of this entry */
				for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) /* loop */;

				/* Nuke and skip any dividers */
				if (*t)
				{
					*t++ = '\0';
					while ((*t == ' ') || (*t == '|')) t++;
				}

				/* Parse this entry */
				if (0 != grab_one_ego_item_flag(e_ptr, s, cur_r, TRUE)) return (5);

				/* Start the next entry */
				s = t;
			}

			/* Next... */
			continue;
		}

		/* Oops */
		return (6);
	}

	/* Success */
	return (0);
}

/*
 * Grab one flag in a randart_part_type from a textual string
 */
static bool_ grab_one_randart_item_flag(randart_part_type *ra_ptr, cptr what, char c)
{
	bool regular = (c == 'F');

	/* Dispatch to correct set of flags */
	u32b *f1  = regular ? &ra_ptr->flags1 : &ra_ptr->aflags1;
	u32b *f2  = regular ? &ra_ptr->flags2 : &ra_ptr->aflags2;
	u32b *f3  = regular ? &ra_ptr->flags3 : &ra_ptr->aflags3;
	u32b *f4  = regular ? &ra_ptr->flags4 : &ra_ptr->aflags4;
	u32b *f5  = regular ? &ra_ptr->flags5 : &ra_ptr->aflags5;
	u32b *esp = regular ? &ra_ptr->esp    : &ra_ptr->aesp;

	/* Check flags */
	if (lookup_flags(what,
		flag_tie(f1, k_info_flags1),
		flag_tie(f2, k_info_flags2),
		flag_tie(f2, k_info_flags2_trap),
		flag_tie(f3, k_info_flags3),
		flag_tie(f4, k_info_flags4),
		flag_tie(f5, k_info_flags5),
		flag_tie(esp, esp_flags)))
	{
		return 0;
	}

	/* Check ego_flags */
	if (regular)
	{
		if (lookup_flags(what, flag_tie(&ra_ptr->fego, ego_flags)))
		{
			return 0;
		}
	}

	/* Oops */
	msg_format("Unknown ego-item flag '%s'.", what);

	/* Error */
	return (1);
}




/*
 * Initialize the "ra_info" array, by parsing an ascii "template" file
 */
errr init_ra_info_txt(FILE *fp)
{
	int i, cur_t = 0, j, cur_g = 0;
	char buf[1024];
	char *s;

	/* Current entry */
	randart_part_type *ra_ptr = NULL;


	/* Just before the first record */
	error_idx = -1;

	/* Just before the first line */
	error_line = -1;


	/* Parse */
	while (0 == my_fgets(fp, buf, 1024))
	{
		/* Advance the line number */
		error_line++;

		/* Skip comments and blank lines */
		if (!buf[0] || (buf[0] == '#')) continue;

		/* Verify correct "colon" format */
		if (buf[1] != ':') return (1);

		/* Process 'G' for "General" (up to 30 lines) */
		if (buf[0] == 'G')
		{
			int chance, dd, ds, plus;

			/* Scan for the values */
			if (4 != sscanf(buf + 2, "%d:%dd%d:%d",
			                &chance, &dd, &ds, &plus)) return (1);

			/* Save the values */
			ra_gen[cur_g].chance = chance;
			ra_gen[cur_g].dd = dd;
			ra_gen[cur_g].ds = ds;
			ra_gen[cur_g].plus = plus;
			cur_g++;

			/* Next... */
			continue;
		}

		/* Process 'N' for "New/Number" */
		if (buf[0] == 'N')
		{
			/* Get the index */
			i = atoi(buf + 2);

			/* Verify information */
			if (i < error_idx) return (4);

			/* Verify information */
			if (i >= max_ra_idx) return (2);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			ra_ptr = &ra_info[i];

			/* Needed hack */
			ra_ptr->power = -1;
			cur_t = 0;

			for (j = 0; j < 20; j++)
			{
				ra_ptr->tval[j] = 255;
			}
			ra_ptr->flags1 = 0;
			ra_ptr->flags2 = 0;
			ra_ptr->flags3 = 0;
			ra_ptr->flags4 = 0;
			ra_ptr->flags5 = 0;
			ra_ptr->esp = 0;
			ra_ptr->fego = 0;

			/* Next... */
			continue;
		}

		/* There better be a current ra_ptr */
		if (!ra_ptr) return (3);

		/* Process 'T' for "Tval/Sval" (up to 5 lines) */
		if (buf[0] == 'T')
		{
			int tv, minsv, maxsv;

			if (cur_t == 20) return 1;

			/* Scan for the values */
			if (3 != sscanf(buf + 2, "%d:%d:%d",
			                &tv, &minsv, &maxsv)) return (1);

			/* Save the values */
			ra_ptr->tval[cur_t] = tv;
			ra_ptr->min_sval[cur_t] = minsv;
			ra_ptr->max_sval[cur_t] = maxsv;

			cur_t++;

			/* Next... */
			continue;
		}

		/* Process 'X' for "Xtra" (one line only) */
		if (buf[0] == 'X')
		{
			int power, max;

			/* Scan for the values */
			if (2 != sscanf(buf + 2, "%d:%d",
			                &power, &max)) return (1);

			/* Save the values */
			ra_ptr->value = power;
			ra_ptr->max = max;

			/* Next... */
			continue;
		}

		/* Process 'W' for "More Info" (one line only) */
		if (buf[0] == 'W')
		{
			int level, rarity, rarity2;

			/* Scan for the values */
			if (3 != sscanf(buf + 2, "%d:%d:%d",
			                &level, &rarity, &rarity2)) return (1);

			/* Save the values */
			ra_ptr->level = level;
			ra_ptr->rarity = rarity;
			ra_ptr->mrarity = rarity2;

			/* Next... */
			continue;
		}

		/* Hack -- Process 'C' for "creation" */
		if (buf[0] == 'C')
		{
			int th, td, ta, pv;

			/* Scan for the values */
			if (4 != sscanf(buf + 2, "%d:%d:%d:%d",
			                &th, &td, &ta, &pv)) return (1);

			ra_ptr->max_to_h = th;
			ra_ptr->max_to_d = td;
			ra_ptr->max_to_a = ta;
			ra_ptr->max_pval = pv;

			/* Next... */
			continue;
		}

		/* Process 'Z' for "Granted power" */
		if (buf[0] == 'Z')
		{
			int i;

			/* Acquire the text */
			s = buf + 2;

			/* Find it in the list */
			for (i = 0; i < POWER_MAX; i++)
			{
				if (iequals(s, powers_type[i].name)) break;
			}

			if (i == POWER_MAX) return (6);

			ra_ptr->power = i;

			/* Next... */
			continue;
		}

		/* Hack -- Process 'F' for flags */
		if (buf[0] == 'F')
		{
			if (0 != grab_one_randart_item_flag(ra_ptr, buf + 2, 'F'))
			{
				return (5);
			}

			/* Next... */
			continue;
		}

		/* Hack -- Process 'A' for antagonic flags */
		if (buf[0] == 'A')
		{
			if (0 != grab_one_randart_item_flag(ra_ptr, buf + 2, 'A'))
			{
				return (5);
			}

			/* Next... */
			continue;
		}

		/* Oops */
		return (6);
	}

	/* Success */
	return (0);
}

/*
 * Grab one (basic) flag in a monster_race from a textual string
 */
static errr grab_one_basic_flag(monster_race *r_ptr, cptr what)
{
	if (lookup_flags(what,
		flag_tie(&r_ptr->flags1, r_info_flags1),
		flag_tie(&r_ptr->flags2, r_info_flags2),
		flag_tie(&r_ptr->flags3, r_info_flags3),
		flag_tie(&r_ptr->flags7, r_info_flags7),
		flag_tie(&r_ptr->flags8, r_info_flags8),
		flag_tie(&r_ptr->flags9, r_info_flags9)))
	{
		return 0;
	}

	/* Oops */
	msg_format("Unknown monster flag '%s'.", what);

	/* Failure */
	return (1);
}


/*
 * Grab one (spell) flag in a monster_race from a textual string
 */
static errr grab_one_monster_spell_flag(monster_spell_flag_set *flags, cptr what)
{
	for (auto const &monster_spell: monster_spells())
	{
		if (streq(what, monster_spell->name))
		{
			*flags |= monster_spell->flag_set;
			return 0;
		}
	}

	/* Oops */
	msg_format("Unknown monster flag '%s'.", what);

	/* Failure */
	return (1);
}


/*
 * Initialize the "r_info" array, by parsing an ascii "template" file
 */
errr init_r_info_txt(FILE *fp)
{
	int i;
	char buf[1024];
	char *s, *t;

	/* Current entry */
	monster_race *r_ptr = NULL;


	/* Just before the first record */
	error_idx = -1;

	/* Just before the first line */
	error_line = -1;


	/* Parse */
	while (0 == my_fgets(fp, buf, 1024))
	{
		/* Advance the line number */
		error_line++;

		/* Skip comments and blank lines */
		if (!buf[0] || (buf[0] == '#')) continue;

		/* Verify correct "colon" format */
		if (buf[1] != ':') return (1);

		/* Process 'N' for "New/Number/Name" */
		if (buf[0] == 'N')
		{
			/* Find the colon before the name */
			s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			i = atoi(buf + 2);

			/* Verify information */
			if (i < error_idx) return (4);

			/* Verify information */
			if (i >= max_r_idx) return (2);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			r_ptr = &r_info[i];

			/* Allocate name string. */
			assert(!r_ptr->name); // Sanity check that we aren't overwriting anything
			r_ptr->name = my_strdup(s);

			/* Ensure empty description */
			r_ptr->text = my_strdup("");

			/* Set default drop theme */
			r_ptr->drops = obj_theme::defaults();

			r_ptr->freq_inate = 0;
			r_ptr->freq_spell = 0;

			/* Next... */
			continue;
		}

		/* There better be a current r_ptr */
		if (!r_ptr) return (3);


		/* Process 'D' for "Description" */
		if (buf[0] == 'D')
		{
			/* Acquire the text */
			s = buf + 2;

			/* Append to description */
			strappend(&r_ptr->text, s);

			/* Next... */
			continue;
		}

		/* Process 'G' for "Graphics" (one line only) */
		if (buf[0] == 'G')
		{
			char sym;
			int tmp;

			/* Paranoia */
			if (!buf[2]) return (1);
			if (!buf[3]) return (1);
			if (!buf[4]) return (1);

			/* Extract the char */
			sym = buf[2];

			/* Extract the attr */
			tmp = color_char_to_attr(buf[4]);

			/* Paranoia */
			if (tmp < 0) return (1);

			/* Save the values */
			r_ptr->d_char = sym;
			r_ptr->d_attr = tmp;

			/* Next... */
			continue;
		}

		/* Process 'I' for "Info" (one line only) */
		if (buf[0] == 'I')
		{
			int spd, hp1, hp2, aaf, ac, slp;

			/* Scan for the other values */
			if (6 != sscanf(buf + 2, "%d:%dd%d:%d:%d:%d",
			                &spd, &hp1, &hp2, &aaf, &ac, &slp)) return (1);

			/* Save the values */
			r_ptr->speed = spd;
			r_ptr->hdice = hp1;
			r_ptr->hside = hp2;
			r_ptr->aaf = aaf;
			r_ptr->ac = ac;
			r_ptr->sleep = slp;

			/* Next... */
			continue;
		}

		/* Process 'E' for "Body Parts" (one line only) */
		if (buf[0] == 'E')
		{
			int weap, tors, fing, head, arms, legs;

			/* Scan for the other values */
			if (BODY_MAX != sscanf(buf + 2, "%d:%d:%d:%d:%d:%d",
			                       &weap, &tors, &arms, &fing, &head, &legs)) return (1);

			/* Save the values */
			r_ptr->body_parts[BODY_WEAPON] = weap;
			r_ptr->body_parts[BODY_TORSO] = tors;
			r_ptr->body_parts[BODY_ARMS] = arms;
			r_ptr->body_parts[BODY_FINGER] = fing;
			r_ptr->body_parts[BODY_HEAD] = head;
			r_ptr->body_parts[BODY_LEGS] = legs;

			/* Mega debugging hack */
			if (weap > arms) quit(format("monster %d, %d weapon(s), %d arm(s) !", error_idx, weap, arms));

			/* Next... */
			continue;
		}

		/* Process 'O' for "Object type" (one line only) */
		if (buf[0] == 'O')
		{
			int treasure, combat, magic, tools;

			/* Scan for the values */
			if (4 != sscanf(buf + 2, "%d:%d:%d:%d",
			                &treasure, &combat, &magic, &tools)) return (1);

			/* Save the values */
			r_ptr->drops.treasure = treasure;
			r_ptr->drops.combat = combat;
			r_ptr->drops.magic = magic;
			r_ptr->drops.tools = tools;

			/* Next... */
			continue;
		}

		/* Process 'A' for standard artifact drop (one line only) */
		if (buf[0] == 'A')
		{
			int artifact_idx;
			int artifact_chance;

			/* Scan for values */
			if (2 != sscanf(buf + 2, "%d:%d",
					&artifact_idx,
					&artifact_chance)) return (1);

			/* Save the values */
			r_ptr->artifact_idx = artifact_idx;
			r_ptr->artifact_chance = artifact_chance;

			/* Next... */
			continue;
		}

		/* Process 'W' for "More Info" (one line only) */
		if (buf[0] == 'W')
		{
			int lev, rar, wt;
			long exp;

			/* Scan for the values */
			if (4 != sscanf(buf + 2, "%d:%d:%d:%ld",
			                &lev, &rar, &wt, &exp)) return (1);

			/* Save the values */
			r_ptr->level = lev;
			r_ptr->rarity = rar;
			/* MEGA HACK */
			if (!wt) wt = 100;
			r_ptr->weight = wt;
			r_ptr->mexp = exp;

			/* Next... */
			continue;
		}

		/* Process 'B' for "Blows" (up to four lines) */
		if (buf[0] == 'B')
		{
			int n1, n2;

			/* Find the next empty blow slot (if any) */
			for (i = 0; i < 4; i++) if (!r_ptr->blow[i].method) break;

			/* Oops, no more slots */
			if (i == 4) return (1);

			/* Analyze the first field */
			for (s = t = buf + 2; *t && (*t != ':'); t++) /* loop */;

			/* Terminate the field (if necessary) */
			if (*t == ':') *t++ = '\0';

			/* Analyze the method */
			for (n1 = 0; r_info_blow_method[n1]; n1++)
			{
				if (streq(s, r_info_blow_method[n1])) break;
			}

			/* Invalid method */
			if (!r_info_blow_method[n1]) return (1);

			/* Analyze the second field */
			for (s = t; *t && (*t != ':'); t++) /* loop */;

			/* Terminate the field (if necessary) */
			if (*t == ':') *t++ = '\0';

			/* Analyze effect */
			for (n2 = 0; r_info_blow_effect[n2]; n2++)
			{
				if (streq(s, r_info_blow_effect[n2])) break;
			}

			/* Invalid effect */
			if (!r_info_blow_effect[n2]) return (1);

			/* Analyze the third field */
			for (s = t; *t && (*t != 'd'); t++) /* loop */;

			/* Terminate the field (if necessary) */
			if (*t == 'd') *t++ = '\0';

			/* Save the method */
			r_ptr->blow[i].method = n1;

			/* Save the effect */
			r_ptr->blow[i].effect = n2;

			/* Extract the damage dice and sides */
			r_ptr->blow[i].d_dice = atoi(s);
			r_ptr->blow[i].d_side = atoi(t);

			/* Next... */
			continue;
		}

		/* Process 'F' for "Basic Flags" (multiple lines) */
		if (buf[0] == 'F')
		{
			if (0 != grab_one_basic_flag(r_ptr, buf + 2))
			{
				return (5);
			}

			/* Next... */
			continue;
		}

		/* Process 'S' for "Spell Flags" (multiple lines) */
		if (buf[0] == 'S')
		{
			s = buf + 2;

			/* XXX XXX XXX Hack -- Read spell frequency */
			if (1 == sscanf(s, "1_IN_%d", &i))
			{
				/* Extract a "frequency" */
				r_ptr->freq_spell = r_ptr->freq_inate = 100 / i;
			}

			/* Parse this entry */
			else
			{
				if (0 != grab_one_monster_spell_flag(&r_ptr->spells, s))
				{
					return (5);
				}
			}

			/* Next... */
			continue;
		}

		/* Oops */
		return (6);
	}

	/* Postprocessing */
	for (i = 1; i < max_r_idx; i++)
	{
		/* Invert flag WILD_ONLY <-> RF8_DUNGEON */
		r_info[i].flags8 ^= 1L;

		/* WILD_TOO without any other wilderness flags enables all flags */
		if ((r_info[i].flags8 & RF8_WILD_TOO) && !(r_info[i].flags8 & 0x7FFFFFFE))
			r_info[i].flags8 = 0x0463;
	}

	/* Success */
	return (0);
}


/*
 * Grab one (basic) flag in a monster_race from a textual string
 */
static errr grab_one_basic_ego_flag(monster_ego *re_ptr, cptr what, bool_ add)
{
	/* Dispatch to correct set of flags */
	u32b *f1 = add ? &re_ptr->mflags1 : &re_ptr->nflags1;
	u32b *f2 = add ? &re_ptr->mflags2 : &re_ptr->nflags2;
	u32b *f3 = add ? &re_ptr->mflags3 : &re_ptr->nflags3;
	u32b *f7 = add ? &re_ptr->mflags7 : &re_ptr->nflags7;
	u32b *f8 = add ? &re_ptr->mflags8 : &re_ptr->nflags8;
	u32b *f9 = add ? &re_ptr->mflags9 : &re_ptr->nflags9;

	/* Lookup */
	if (lookup_flags(what,
		flag_tie(f1, r_info_flags1),
		flag_tie(f2, r_info_flags2),
		flag_tie(f3, r_info_flags3),
		flag_tie(f7, r_info_flags7),
		flag_tie(f8, r_info_flags8),
		flag_tie(f9, r_info_flags9)))
	{
		return 0;
	}

	/* Oops */
	msg_format("Unknown monster flag '%s'.", what);

	/* Failure */
	return (1);
}


/*
 * Grab one (basic) flag in a monster_race from a textual string
 */
static errr grab_one_ego_flag(monster_ego *re_ptr, cptr what, bool_ must)
{
	/* Dispatch to correct set of flags */
	u32b *f1 = must ? &re_ptr->flags1 : &re_ptr->hflags1;
	u32b *f2 = must ? &re_ptr->flags2 : &re_ptr->hflags2;
	u32b *f3 = must ? &re_ptr->flags3 : &re_ptr->hflags3;
	u32b *f7 = must ? &re_ptr->flags7 : &re_ptr->hflags7;
	u32b *f8 = must ? &re_ptr->flags8 : &re_ptr->hflags8;
	u32b *f9 = must ? &re_ptr->flags9 : &re_ptr->hflags9;

	/* Lookup */
	if (lookup_flags(what,
		flag_tie(f1, r_info_flags1),
		flag_tie(f2, r_info_flags2),
		flag_tie(f3, r_info_flags3),
		flag_tie(f7, r_info_flags7),
		flag_tie(f8, r_info_flags8),
		flag_tie(f9, r_info_flags9)))
	{
		return (0);
	}

	/* Oops */
	msg_format("Unknown monster flag '%s'.", what);

	/* Failure */
	return (1);
}

/*
 * Initialize the "re_info" array, by parsing an ascii "template" file
 */
errr init_re_info_txt(FILE *fp)
{
	int i, j;
	char buf[1024];
	byte blow_num = 0;
	int r_char_number = 0, nr_char_number = 0;

	char *s, *t;

	/* Current entry */
	monster_ego *re_ptr = NULL;


	/* Just before the first record */
	error_idx = -1;

	/* Just before the first line */
	error_line = -1;

	/* Parse */
	while (0 == my_fgets(fp, buf, 1024))
	{
		/* Advance the line number */
		error_line++;

		/* Skip comments and blank lines */
		if (!buf[0] || (buf[0] == '#')) continue;

		/* Verify correct "colon" format */
		if (buf[1] != ':') return (1);

		/* Process 'N' for "New/Number/Name" */
		if (buf[0] == 'N')
		{
			/* Find the colon before the name */
			s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			i = atoi(buf + 2);

			/* Verify information */
			if (i < error_idx) return (4);

			/* Verify information */
			if (i >= max_re_idx) return (2);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			re_ptr = &re_info[i];

			/* Copy name */
			assert(!re_ptr->name);
			re_ptr->name = my_strdup(s);

			/* Some inits */
			blow_num = 0;
			r_char_number = 0;
			nr_char_number = 0;
			for (j = 0; j < 5; j++) re_ptr->r_char[j] = 0;
			for (j = 0; j < 5; j++) re_ptr->nr_char[j] = 0;
			for (j = 0; j < 4; j++)
			{
				re_ptr->blow[j].method = 0;
				re_ptr->blow[j].effect = 0;
				re_ptr->blow[j].d_dice = 0;
				re_ptr->blow[j].d_side = 0;
				re_ptr->blowm[j][0] = MEGO_ADD;
				re_ptr->blowm[j][1] = MEGO_ADD;
			}

			/* Next... */
			continue;
		}

		/* There better be a current re_ptr */
		if (!re_ptr) return (3);

		/* Process 'G' for "Graphics" (one line only) */
		if (buf[0] == 'G')
		{
			char sym;
			int tmp;

			/* Paranoia */
			if (!buf[2]) return (1);
			if (!buf[3]) return (1);
			if (!buf[4]) return (1);

			/* Extract the char */
			if (buf[2] != '*') sym = buf[2];
			else sym = MEGO_CHAR_ANY;

			/* Extract the attr */
			if (buf[4] != '*') tmp = color_char_to_attr(buf[4]);
			else tmp = MEGO_CHAR_ANY;

			/* Paranoia */
			if (tmp < 0) return (1);

			/* Save the values */
			re_ptr->d_char = sym;
			re_ptr->d_attr = tmp;

			/* Next... */
			continue;
		}

		/* Process 'I' for "Info" (one line only) */
		if (buf[0] == 'I')
		{
			int spd, hp1, hp2, aaf, ac, slp;
			char mspd, mhp1, mhp2, maaf, mac, mslp;

			/* Scan for the other values */
			if (12 != sscanf(buf + 2, "%c%d:%c%dd%c%d:%c%d:%c%d:%c%d",
			                 &mspd, &spd, &mhp1, &hp1, &mhp2, &hp2, &maaf, &aaf, &mac, &ac, &mslp, &slp)) return (1);

			/* Save the values */
			re_ptr->speed = (spd << 2) + monster_ego_modify(mspd);
			re_ptr->hdice = (hp1 << 2) + monster_ego_modify(mhp1);
			re_ptr->hside = (hp2 << 2) + monster_ego_modify(mhp2);
			re_ptr->aaf = (aaf << 2) + monster_ego_modify(maaf);
			re_ptr->ac = (ac << 2) + monster_ego_modify(mac);
			re_ptr->sleep = (slp << 2) + monster_ego_modify(mslp);

			/* Next... */
			continue;
		}

		/* Process 'W' for "More Info" (one line only) */
		if (buf[0] == 'W')
		{
			int lev, rar, wt;
			char mlev, mwt, mexp, pos;
			long exp;

			/* Scan for the values */
			if (8 != sscanf(buf + 2, "%c%d:%d:%c%d:%c%ld:%c",
			                &mlev, &lev, &rar, &mwt, &wt, &mexp, &exp, &pos)) return (1);

			/* Save the values */
			re_ptr->level = (lev << 2) + monster_ego_modify(mlev);
			re_ptr->rarity = rar;
			re_ptr->weight = (wt << 2) + monster_ego_modify(mwt);
			re_ptr->mexp = (exp << 2) + monster_ego_modify(mexp);
			re_ptr->before = (pos == 'B') ? TRUE : FALSE;

			/* Next... */
			continue;
		}

		/* Process 'B' for "Blows" (up to four lines) */
		if (buf[0] == 'B')
		{
			int n1, n2, dice, side;
			char mdice, mside;

			/* Oops, no more slots */
			if (blow_num == 4) return (1);

			/* Analyze the first field */
			for (s = t = buf + 2; *t && (*t != ':'); t++) /* loop */;

			/* Terminate the field (if necessary) */
			if (*t == ':') *t++ = '\0';

			/* Analyze the method */
			for (n1 = 0; r_info_blow_method[n1]; n1++)
			{
				if (streq(s, r_info_blow_method[n1])) break;
			}

			/* Invalid method */
			if (!r_info_blow_method[n1]) return (1);

			/* Analyze the second field */
			for (s = t; *t && (*t != ':'); t++) /* loop */;

			/* Terminate the field (if necessary) */
			if (*t == ':') *t++ = '\0';

			/* Analyze effect */
			for (n2 = 0; r_info_blow_effect[n2]; n2++)
			{
				if (streq(s, r_info_blow_effect[n2])) break;
			}

			/* Invalid effect */
			if (!r_info_blow_effect[n2]) return (1);

			/* Save the method */
			re_ptr->blow[blow_num].method = n1;

			/* Save the effect */
			re_ptr->blow[blow_num].effect = n2;

			/* Extract the damage dice and sides */
			if (4 != sscanf(t, "%c%dd%c%d",
			                &mdice, &dice, &mside, &side)) return (1);

			re_ptr->blow[blow_num].d_dice = dice;
			re_ptr->blow[blow_num].d_side = side;
			re_ptr->blowm[blow_num][0] = monster_ego_modify(mdice);
			re_ptr->blowm[blow_num][1] = monster_ego_modify(mside);
			blow_num++;

			/* Next... */
			continue;
		}

		/* Process 'F' for "Flags monster must have" (multiple lines) */
		if (buf[0] == 'F')
		{
			char r_char;

			/* Parse every entry */
			s = buf + 2;

			/* XXX XXX XXX Hack -- Read monster symbols */
			if (1 == sscanf(s, "R_CHAR_%c", &r_char))
			{
				/* Limited to 5 races */
				if (r_char_number >= 5) continue;

				/* Extract a "frequency" */
				re_ptr->r_char[r_char_number++] = r_char;
			}

			/* Parse this entry */
			else {
				if (0 != grab_one_ego_flag(re_ptr, s, TRUE))
				{
					return (5);
				}
			}

			/* Next... */
			continue;
		}

		/* Process 'H' for "Flags monster must not have" (multiple lines) */
		if (buf[0] == 'H')
		{
			char r_char;

			/* Parse every entry */
			s = buf + 2;

			/* XXX XXX XXX Hack -- Read monster symbols */
			if (1 == sscanf(s, "R_CHAR_%c", &r_char))
			{
				/* Limited to 5 races */
				if (nr_char_number >= 5) continue;

				/* Extract a "frequency" */
				re_ptr->nr_char[nr_char_number++] = r_char;
			}

			/* Parse this entry */
			else {
				if (0 != grab_one_ego_flag(re_ptr, s, FALSE))
				{
					return (5);
				}
			}

			/* Next... */
			continue;
		}

		/* Process 'M' for "Basic Monster Flags" (multiple lines) */
		if (buf[0] == 'M')
		{
			if (0 != grab_one_basic_ego_flag(re_ptr, buf + 2, TRUE))
			{
				return (5);
			}

			/* Next... */
			continue;
		}

		/* Process 'O' for "Basic Monster -Flags" (multiple lines) */
		if (buf[0] == 'O')
		{
			s = buf + 2;

			/* XXX XXX XXX Hack -- Read no flags */
			if (!strcmp(s, "MF_ALL"))
			{
				/* No flags */
				re_ptr->nflags1 = re_ptr->nflags2 = re_ptr->nflags3 = re_ptr->nflags7 = re_ptr->nflags8 = re_ptr->nflags9 = 0xFFFFFFFF;
			}

			/* Parse this entry */
			else {
				if (0 != grab_one_basic_ego_flag(re_ptr, s, FALSE))
				{
					return (5);
				}
			}

			/* Next... */
			continue;
		}

		/* Process 'S' for "Spell Flags" (multiple lines) */
		if (buf[0] == 'S')
		{
			s = buf + 2;

			/* XXX XXX XXX Hack -- Read spell frequency */
			if (1 == sscanf(s, "1_IN_%d", &i))
			{
				/* Extract a "frequency" */
				re_ptr->freq_spell = re_ptr->freq_inate = 100 / i;
			}

			/* Parse this entry */
			else {
				if (0 != grab_one_monster_spell_flag(&re_ptr->mspells, s))
				{
					return (5);
				}
			}

			/* Next... */
			continue;
		}

		/* Process 'T' for "Spell -Flags" (multiple lines) */
		if (buf[0] == 'T')
		{
			/* Parse every entry */
			for (s = buf + 2; *s; )
			{
				/* Find the end of this entry */
				for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) /* loop */;

				/* Nuke and skip any dividers */
				if (*t)
				{
					*t++ = '\0';
					while ((*t == ' ') || (*t == '|')) t++;
				}

				/* XXX XXX XXX Hack -- Read no flags */
				if (!strcmp(s, "MF_ALL"))
				{
					/* No flags */
					re_ptr->nspells = ~monster_spell_flag_set();

					/* Start at next entry */
					s = t;

					/* Continue */
					continue;
				}

				/* Parse this entry */
				if (0 != grab_one_monster_spell_flag(&re_ptr->nspells, s)) return (5);

				/* Start the next entry */
				s = t;
			}

			/* Next... */
			continue;
		}

		/* Oops */
		return (6);
	}

	/* Success */
	return (0);
}


/*
 * Grab one flag in an trap_type from a textual string
 */
static errr grab_one_trap_type_flag(trap_type *t_ptr, cptr what)
{
	if (lookup_flags(what, flag_tie(&t_ptr->flags, t_info_flags)))
	{
		return (0);
	}

	/* Oops */
	msg_format("Unknown trap_type flag '%s'.", what);

	/* Error */
	return (1);
}


/*
 * Initialize the "tr_info" array, by parsing an ascii "template" file
 */
errr init_t_info_txt(FILE *fp)
{
	int i;
	char buf[1024];
	char *s, *t;

	/* Current entry */
	trap_type *t_ptr = NULL;

	/* Just before the first record */
	error_idx = -1;

	/* Just before the first line */
	error_line = -1;

	/* Parse */
	while (0 == my_fgets(fp, buf, 1024))
	{
		/* Advance the line number */
		error_line++;

		/* Skip comments and blank lines */
		if (!buf[0] || (buf[0] == '#')) continue;

		/* Verify correct "colon" format */
		if (buf[1] != ':') return (1);

		/* Process 'N' for "New/Number/Name" */
		if (buf[0] == 'N')
		{
			/* Find the colon before the name */
			s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			i = atoi(buf + 2);

			/* Verify information */
			if (i <= error_idx) return (4);

			/* Verify information */
			if (i >= max_t_idx) return (2);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			t_ptr = &t_info[i];

			/* Copy name */
			t_ptr->name = my_strdup(s);

			/* Initialize */
			t_ptr->text = my_strdup("");

			/* Next... */
			continue;
		}

		/* There better be a current t_ptr */
		if (!t_ptr) return (3);


		/* Process 'I' for "Information" */
		if (buf[0] == 'I')
		{
			int probability, another, p1valinc, difficulty;
			int minlevel;
			int dd, ds;
			char color;

			/* Scan for the values */
			if (8 != sscanf(buf + 2, "%d:%d:%d:%d:%d:%dd%d:%c",
			                &difficulty, &probability, &another,
			                &p1valinc, &minlevel, &dd, &ds,
			                &color)) return (1);

			t_ptr->difficulty = (byte)difficulty;
			t_ptr->probability = (s16b)probability;
			t_ptr->another = (s16b)another;
			t_ptr->p1valinc = (s16b)p1valinc;
			t_ptr->minlevel = (byte)minlevel;
			t_ptr->dd = (s16b)dd;
			t_ptr->ds = (s16b)ds;
			t_ptr->color = color_char_to_attr(color);

			/* Next... */
			continue;
		}


		/* Process 'D' for "Description" */
		if (buf[0] == 'D')
		{
			/* Acquire the text */
			s = buf + 2;

			/* Append chars to the name */
			strappend(&t_ptr->text, s);

			/* Next... */
			continue;
		}


		/* Hack -- Process 'F' for flags */
		if (buf[0] == 'F')
		{

			t_ptr->flags = 0;

			/* Parse every entry textually */
			for (s = buf + 2; *s; )
			{
				/* Find the end of this entry */
				for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) /* loop */;

				/* Nuke and skip any dividers */
				if (*t)
				{
					*t++ = '\0';
					while (*t == ' ' || *t == '|') t++;
				}

				/* Parse this entry */
				if (0 != grab_one_trap_type_flag(t_ptr, s)) return (5);

				/* Start the next entry */
				s = t;
			}

			/* Next... */
			continue;
		}


		/* Oops */
		return (6);
	}

	/* Success */
	return (0);
}

errr grab_one_dungeon_flag(dungeon_flag_set *flags, const char *str)
{
#define DF(tier, index, name) \
	if (streq(str, #name)) { *flags |= DF_##name; return 0; }
#include "dungeon_flag_list.hpp"
#undef DF

	/* Oops */
	msg_format("Unknown dungeon type flag '%s'.", str);

	/* Failure */
	return (1);
}

/*
 * Grab one (basic) flag in a monster_race from a textual string
 */
static errr grab_one_basic_monster_flag(dungeon_info_type *d_ptr, cptr what, byte rule)
{
	if (lookup_flags(what,
		flag_tie(&d_ptr->rules[rule].mflags1, r_info_flags1),
		flag_tie(&d_ptr->rules[rule].mflags2, r_info_flags2),
		flag_tie(&d_ptr->rules[rule].mflags3, r_info_flags3),
		flag_tie(&d_ptr->rules[rule].mflags7, r_info_flags7),
		flag_tie(&d_ptr->rules[rule].mflags8, r_info_flags8),
		flag_tie(&d_ptr->rules[rule].mflags9, r_info_flags9)))
	{
		return 0;
	}

	/* Oops */
	msg_format("Unknown monster flag '%s'.", what);

	/* Failure */
	return (1);
}


/*
 * Initialize the "d_info" array, by parsing an ascii "template" file
 */
errr init_d_info_txt(FILE *fp)
{
	int i, j;
	char buf[1024];

	s16b rule_num = 0;

	byte r_char_number = 0;

	char *s;

	/* Current entry */
	dungeon_info_type *d_ptr = NULL;


	/* Just before the first record */
	error_idx = -1;

	/* Just before the first line */
	error_line = -1;

	/* Parse */
	while (0 == my_fgets(fp, buf, 1024))
	{
		/* Advance the line number */
		error_line++;

		/* Skip comments and blank lines */
		if (!buf[0] || (buf[0] == '#')) continue;

		/* Verify correct "colon" format */
		if (buf[1] != ':') return (1);

		/* Process 'N' for "New/Number/Name" */
		if (buf[0] == 'N')
		{
			/* Find the colon before the name */
			s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			i = atoi(buf + 2);

			/* Verify information */
			if (i < error_idx) return (4);

			/* Verify information */
			if (i >= max_d_idx) return (2);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			d_ptr = &d_info[i];

			/* Copy name */
			assert(!d_ptr->name);
			d_ptr->name = my_strdup(s);

			/* Initialize description */
			d_ptr->text = my_strdup("");

			/* HACK -- Those ones HAVE to have a set default value */
			d_ptr->size_x = -1;
			d_ptr->size_y = -1;
			d_ptr->ix = -1;
			d_ptr->iy = -1;
			d_ptr->ox = -1;
			d_ptr->oy = -1;
			d_ptr->fill_method = 1;
			rule_num = -1;
			r_char_number = 0;
			for (j = 0; j < 5; j++)
			{
				int k;

				d_ptr->rules[j].mode = DUNGEON_MODE_NONE;
				d_ptr->rules[j].percent = 0;

				for (k = 0; k < 5; k++) d_ptr->rules[j].r_char[k] = 0;
			}

			/* Set default drop theme */
			d_ptr->objs = obj_theme::defaults();

			/* The default generator */
			strcpy(d_ptr->generator, "dungeon");

			/* Next... */
			continue;
		}

		/* There better be a current d_ptr */
		if (!d_ptr) return (3);

		/* Process 'D' for "Description */
		if (buf[0] == 'D')
		{
			/* Acquire short name */
			d_ptr->short_name[0] = buf[2];
			d_ptr->short_name[1] = buf[3];
			d_ptr->short_name[2] = buf[4];

			/* Acquire the text */
			s = buf + 6;

			/* Append to description */
			strappend(&d_ptr->text, s);

			/* Next... */
			continue;
		}

		/* Process 'W' for "More Info" (one line only) */
		if (buf[0] == 'W')
		{
			int min_lev, max_lev;
			int min_plev;
			int min_alloc, max_chance;

			/* Scan for the values */
			if (5 != sscanf(buf + 2, "%d:%d:%d:%d:%d",
					&min_lev, &max_lev, &min_plev, &min_alloc, &max_chance)) return (1);

			/* Save the values */
			d_ptr->mindepth = min_lev;
			d_ptr->maxdepth = max_lev;
			d_ptr->min_plev = min_plev;
			d_ptr->min_m_alloc_level = min_alloc;
			d_ptr->max_m_alloc_chance = max_chance;

			/* Next... */
			continue;
		}

		/* Process 'L' for "fLoor type" (one line only) */
		if (buf[0] == 'L')
		{
			int f1, f2, f3;
			int p1, p2, p3;
			int i;

			/* Scan for the values */
			if (6 != sscanf(buf + 2, "%d:%d:%d:%d:%d:%d",
			                &f1, &p1, &f2, &p2, &f3, &p3))
			{
				/* Scan for the values - part ii*/
				if (3 != sscanf(buf + 2, "%d:%d:%d", &p1, &p2,
				                &p3)) return (1);

				/* Save the values */
				d_ptr->floor_percent1[1] = p1;
				d_ptr->floor_percent2[1] = p2;
				d_ptr->floor_percent3[1] = p3;

				continue;
			}

			/* Save the values */
			d_ptr->floor1 = f1;
			d_ptr->floor2 = f2;
			d_ptr->floor3 = f3;

			for (i = 0; i < 2; i++)
			{
				d_ptr->floor_percent1[i] = p1;
				d_ptr->floor_percent2[i] = p2;
				d_ptr->floor_percent3[i] = p3;
			}

			/* Next... */
			continue;
		}

		/* Process 'O' for "Object type" (one line only) */
		if (buf[0] == 'O')
		{
			int treasure, combat, magic, tools;

			/* Scan for the values */
			if (4 != sscanf(buf + 2, "%d:%d:%d:%d",
			                &treasure, &combat, &magic, &tools)) return (1);

			/* Save the values */
			d_ptr->objs.treasure = treasure;
			d_ptr->objs.combat = combat;
			d_ptr->objs.magic = magic;
			d_ptr->objs.tools = tools;

			/* Next... */
			continue;
		}

		/* Process 'G' for "Generator" (one line only) */
		if (buf[0] == 'G')
		{
			strnfmt(d_ptr->generator, 30, "%s", buf + 2);

			/* Next... */
			continue;
		}

		/* Process 'A' for "wAll type" (one line only) */
		if (buf[0] == 'A')
		{
			int w1, w2, w3, outer, inner;
			int p1, p2, p3;
			int i;

			/* Scan for the values */
			if (8 != sscanf(buf + 2, "%d:%d:%d:%d:%d:%d:%d:%d",
			                &w1, &p1, &w2, &p2, &w3, &p3, &outer, &inner))
			{
				/* Scan for the values - part ii*/
				if (3 != sscanf(buf + 2, "%d:%d:%d", &p1, &p2,
				                &p3)) return (1);

				/* Save the values */
				d_ptr->fill_percent1[1] = p1;
				d_ptr->fill_percent2[1] = p2;
				d_ptr->fill_percent3[1] = p3;
				continue;
			}

			/* Save the values */
			d_ptr->fill_type1 = w1;
			d_ptr->fill_type2 = w2;
			d_ptr->fill_type3 = w3;

			for (i = 0; i < 2; i++)
			{
				d_ptr->fill_percent1[i] = p1;
				d_ptr->fill_percent2[i] = p2;
				d_ptr->fill_percent3[i] = p3;
			}

			d_ptr->outer_wall = outer;
			d_ptr->inner_wall = inner;

			/* Next... */
			continue;
		}

		/* Process 'E' for "Effects" (up to four lines) -SC- */
		if (buf[0] == 'E')
		{
			int side, dice, freq, type;
			cptr tmp;

			/* Find the next empty blow slot (if any) */
			for (i = 0; i < 4; i++) if ((!d_ptr->d_side[i]) &&
				                            (!d_ptr->d_dice[i])) break;

			/* Oops, no more slots */
			if (i == 4) return (1);

			/* Scan for the values */
			if (4 != sscanf(buf + 2, "%dd%d:%d:%d",
			                &dice, &side, &freq, &type))
			{
				int j;

				if (3 != sscanf(buf + 2, "%dd%d:%d",
				                &dice, &side, &freq)) return (1);

				tmp = buf + 2;
				for (j = 0; j < 2; j++)
				{
					tmp = strchr(tmp, ':');
					if (tmp == NULL) return (1);
					tmp++;
				}

				j = 0;

				while (d_info_dtypes[j].name != NULL)
					if (strcmp(d_info_dtypes[j].name, tmp) == 0)
					{
						d_ptr->d_type[i] = d_info_dtypes[j].feat;
						break;
					}
					else j++;

				if (d_info_dtypes[j].name == NULL) return (1);
			}
			else
				d_ptr->d_type[i] = type;

			freq *= 10;
			/* Save the values */
			d_ptr->d_side[i] = side;
			d_ptr->d_dice[i] = dice;
			d_ptr->d_frequency[i] = freq;

			/* Next... */
			continue;
		}

		/* Process 'F' for "Dungeon Flags" (multiple lines) */
		if (buf[0] == 'F')
		{
			int artif = 0, monst = 0, obj = 0;
			int ix = -1, iy = -1, ox = -1, oy = -1;
			int fill_method;
			s = buf + 2;

			/* Read dungeon in/out coords */
			if (4 == sscanf(s, "WILD_%d_%d__%d_%d", &ix, &iy, &ox, &oy))
			{
				d_ptr->ix = ix;
				d_ptr->iy = iy;
				d_ptr->ox = ox;
				d_ptr->oy = oy;
			}

			/* Read dungeon size */
			else if (2 == sscanf(s, "SIZE_%d_%d", &ix, &iy))
			{
				d_ptr->size_x = ix;
				d_ptr->size_y = iy;
			}

			/* Read dungeon fill method */
			else if (1 == sscanf(s, "FILL_METHOD_%d", &fill_method))
			{
				d_ptr->fill_method = fill_method;
			}

			/* Read Final Object */
			else if (1 == sscanf(s, "FINAL_OBJECT_%d", &obj))
			{
				/* Extract a "Final Artifact" */
				d_ptr->final_object = obj;
			}

			/* Read Final Artifact */
			else if (1 == sscanf(s, "FINAL_ARTIFACT_%d", &artif ))
			{
				/* Extract a "Final Artifact" */
				d_ptr->final_artifact = artif ;
			}

			/* Read Artifact Guardian */
			else if (1 == sscanf(s, "FINAL_GUARDIAN_%d", &monst))
			{
				/* Extract a "Artifact Guardian" */
				d_ptr->final_guardian = monst;
			}

			/* Parse this entry */
			else {
				if (0 != grab_one_dungeon_flag(&d_ptr->flags, s))
				{
					return (5);
				}
			}

			/* Next... */
			continue;
		}

		/* Process 'R' for "monster generation Rule" (up to 5 lines) */
		if (buf[0] == 'R')
		{
			int percent, mode;
			int z, y, lims[5];

			/* Scan for the values */
			if (2 != sscanf(buf + 2, "%d:%d",
			                &percent, &mode)) return (1);

			/* Save the values */
			r_char_number = 0;
			rule_num++;

			d_ptr->rules[rule_num].percent = percent;
			d_ptr->rules[rule_num].mode = mode;

			/* Lets remap the flat percents */
			lims[0] = d_ptr->rules[0].percent;
			for (y = 1; y <= rule_num; y++)
			{
				lims[y] = lims[y - 1] + d_ptr->rules[y].percent;
			}
			for (z = 0; z < 100; z++)
			{
				for (y = rule_num; y >= 0; y--)
				{
					if (z < lims[y]) d_ptr->rule_percents[z] = y;
				}
			}

			/* Next... */
			continue;
		}

		/* Process 'M' for "Basic Flags" (multiple lines) */
		if (buf[0] == 'M')
		{
			byte r_char;
			s = buf + 2;

			/* Read monster symbols */
			if (1 == sscanf(s, "R_CHAR_%c", &r_char))
			{
				/* Limited to 5 races */
				if (r_char_number >= 5) continue;

				/* Extract a "frequency" */
				d_ptr->rules[rule_num].r_char[r_char_number++] = r_char;
			}

			/* Parse this entry */
			else {
				if (0 != grab_one_basic_monster_flag(d_ptr, s, rule_num))
				{
					return (5);
				}
			}

			/* Next... */
			continue;
		}

		/* Process 'S' for "Spell Flags" (multiple lines) */
		if (buf[0] == 'S')
		{
			s = buf + 2;

			/* Parse this entry */
			if (0 != grab_one_monster_spell_flag(&d_ptr->rules[rule_num].mspells, s))
			{
				return (5);
			}

			/* Next... */
			continue;
		}

		/* Oops */
		return (6);
	}

	/* Success */
	return (0);
}

/*
 * Grab one race flag from a textual string
 */
static errr grab_one_race_flag(owner_type *ow_ptr, int state, cptr what)
{
	/* Scan race flags */
	unknown_shut_up = TRUE;
	if (!grab_one_race_allow_flag(ow_ptr->races[state], what))
	{
		unknown_shut_up = FALSE;
		return (0);
	}

	/* Scan classes flags */
	if (!grab_one_class_flag(ow_ptr->classes[state], what))
	{
		unknown_shut_up = FALSE;
		return (0);
	}

	/* Oops */
	unknown_shut_up = FALSE;
	msg_format("Unknown race/class flag '%s'.", what);

	/* Failure */
	return (1);
}

/*
 * Grab one store flag from a textual string
 */
static errr grab_one_store_flag(store_info_type *st_ptr, cptr what)
{
	/* Scan store flags */
	if (lookup_flags(what, flag_tie(&st_ptr->flags1, st_info_flags1)))
	{
		return 0;
	}

	/* Oops */
	msg_format("Unknown store flag '%s'.", what);

	/* Failure */
	return (1);
}

/*
 * Initialize the "st_info" array, by parsing an ascii "template" file
 */
errr init_st_info_txt(FILE *fp)
{
	int i = 0, item_idx = 0;
	char buf[1024];
	char *s;

	/* Current entry */
	store_info_type *st_ptr = NULL;


	/* Just before the first record */
	error_idx = -1;

	/* Just before the first line */
	error_line = -1;

	/* Parse */
	while (0 == my_fgets(fp, buf, 1024))
	{
		/* Advance the line number */
		error_line++;

		/* Skip comments and blank lines */
		if (!buf[0] || (buf[0] == '#')) continue;

		/* Verify correct "colon" format */
		if (buf[1] != ':') return (1);

		/* Process 'N' for "New/Number/Name" */
		if (buf[0] == 'N')
		{
			/* Find the colon before the name */
			s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			i = atoi(buf + 2);

			/* Verify information */
			if (i < error_idx) return (4);

			/* Verify information */
			if (i >= max_st_idx) return (2);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			st_ptr = &st_info[i];

			/* Copy name */
			assert(!st_ptr->name);
			st_ptr->name = my_strdup(s);

			/* We are ready for a new set of objects */
			item_idx = 0;

			/* Next... */
			continue;
		}

		/* There better be a current st_ptr */
		if (!st_ptr) return (3);

		/* Process 'I' for "Items" (multiple lines) */
		if (buf[0] == 'I')
		{
			/* Find the colon before the name */
			s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			st_ptr->table[item_idx][1] = atoi(buf + 2);

			/* Append chars to the name */
			st_ptr->table[item_idx++][0] = test_item_name(s);

			st_ptr->table_num = item_idx;
			assert(st_ptr->table_num <= STORE_CHOICES);

			/* Next... */
			continue;
		}

		/* Process 'T' for "Tval/sval" */
		if (buf[0] == 'T')
		{
			int tv1, sv1, rar1;

			/* Scan for the values */
			if (3 != sscanf(buf + 2, "%d:%d:%d",
			                &rar1, &tv1, &sv1)) return (1);

			/* Get the index */
			st_ptr->table[item_idx][1] = rar1;
			/* Hack -- 256 as a sval means all possible items */
			st_ptr->table[item_idx++][0] = (sv1 < 256) ? lookup_kind(tv1, sv1) : tv1 + 10000;

			st_ptr->table_num = item_idx;

			/* Next... */
			continue;
		}

		/* Process 'G' for "Graphics" one line only) */
		if (buf[0] == 'G')
		{
			char c, a;
			int attr;

			/* Scan for the values */
			if (2 != sscanf(buf + 2, "%c:%c",
			                &c, &a)) return (1);

			/* Extract the color */
			attr = color_char_to_attr(a);

			/* Paranoia */
			if (attr < 0) return (1);

			/* Save the values */
			st_ptr->d_char = c;
			st_ptr->d_attr = attr;

			/* Next... */
			continue;
		}

		/* Process 'A' for "Actions" (one line only) */
		if (buf[0] == 'A')
		{
			int a1, a2, a3, a4, a5, a6;

			/* Scan for the values */
			if (6 != sscanf(buf + 2, "%d:%d:%d:%d:%d:%d",
			                &a1, &a2, &a3, &a4, &a5, &a6)) return (1);

			/* Save the values */
			st_ptr->actions[0] = a1;
			st_ptr->actions[1] = a2;
			st_ptr->actions[2] = a3;
			st_ptr->actions[3] = a4;
			st_ptr->actions[4] = a5;
			st_ptr->actions[5] = a6;

			/* Next... */
			continue;
		}

		/* Process 'F' for "store Flags" (multiple lines) */
		if (buf[0] == 'F')
		{
			if (0 != grab_one_store_flag(st_ptr, buf + 2)) return (5);

			/* Next... */
			continue;
		}

		/* Process 'O' for "Owners" (one line only) */
		if (buf[0] == 'O')
		{
			int a1, a2, a3, a4;

			/* Scan for the values */
			if (4 != sscanf(buf + 2, "%d:%d:%d:%d",
			                &a1, &a2, &a3, &a4)) return (1);

			/* Save the values */
			st_ptr->owners[0] = a1;
			st_ptr->owners[1] = a2;
			st_ptr->owners[2] = a3;
			st_ptr->owners[3] = a4;

			/* Next... */
			continue;
		}

		/* Process 'W' for "Extra info" (one line only) */
		if (buf[0] == 'W')
		{
			int max_obj;

			/* Scan for the values */
			if (1 != sscanf(buf + 2, "%d",
			                &max_obj)) return (1);

			/* Save the values */
			if (max_obj > STORE_INVEN_MAX) max_obj = STORE_INVEN_MAX;
			st_ptr->max_obj = max_obj;

			/* Next... */
			continue;
		}

		/* Oops */
		return (6);
	}

	/* Success */
	return (0);
}

/*
 * Initialize the "ba_info" array, by parsing an ascii "template" file
 */
errr init_ba_info_txt(FILE *fp)
{
	int i = 0;
	char buf[1024];
	char *s;

	/* Current entry */
	store_action_type *ba_ptr = NULL;


	/* Just before the first record */
	error_idx = -1;

	/* Just before the first line */
	error_line = -1;

	/* Parse */
	while (0 == my_fgets(fp, buf, 1024))
	{
		/* Advance the line number */
		error_line++;

		/* Skip comments and blank lines */
		if (!buf[0] || (buf[0] == '#')) continue;

		/* Verify correct "colon" format */
		if (buf[1] != ':') return (1);

		/* Process 'N' for "New/Number/Name" */
		if (buf[0] == 'N')
		{
			/* Find the colon before the name */
			s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			i = atoi(buf + 2);

			/* Verify information */
			if (i < error_idx) return (4);

			/* Verify information */
			if (i >= max_ba_idx) return (2);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			ba_ptr = &ba_info[i];

			/* Copy name */
			assert(!ba_ptr->name);
			ba_ptr->name = my_strdup(s);

			/* Next... */
			continue;
		}

		/* There better be a current ba_ptr */
		if (!ba_ptr) return (3);

		/* Process 'C' for "Costs" (one line only) */
		if (buf[0] == 'C')
		{
			int ch, cn, cl;

			/* Scan for the values */
			if (3 != sscanf(buf + 2, "%d:%d:%d",
			                &ch, &cn, &cl)) return (1);

			/* Save the values */
			ba_ptr->costs[STORE_HATED] = ch;
			ba_ptr->costs[STORE_NORMAL] = cn;
			ba_ptr->costs[STORE_LIKED] = cl;

			/* Next... */
			continue;
		}

		/* Process 'I' for "Infos" (one line only) */
		if (buf[0] == 'I')
		{
			int act, act_res;
			char letter, letter_aux = 0;

			/* Scan for the values */
			if (4 != sscanf(buf + 2, "%d:%d:%c:%c", &act, &act_res, &letter, &letter_aux))
				if (3 != sscanf(buf + 2, "%d:%d:%c", &act, &act_res, &letter))
					return (1);

			/* Save the values */
			ba_ptr->action = act;
			ba_ptr->action_restr = act_res;
			ba_ptr->letter = letter;
			ba_ptr->letter_aux = letter_aux;

			/* Next... */
			continue;
		}

		/* Oops */
		return (6);
	}

	/* Success */
	return (0);
}

/*
 * Initialize the "ow_info" array, by parsing an ascii "template" file
 */
errr init_ow_info_txt(FILE *fp)
{
	int i;
	char buf[1024];
	char *s;

	/* Current entry */
	owner_type *ow_ptr = NULL;

	/* Just before the first record */
	error_idx = -1;

	/* Just before the first line */
	error_line = -1;

	/* Parse */
	while (0 == my_fgets(fp, buf, 1024))
	{
		/* Advance the line number */
		error_line++;

		/* Skip comments and blank lines */
		if (!buf[0] || (buf[0] == '#')) continue;

		/* Verify correct "colon" format */
		if (buf[1] != ':') return (1);

		/* Process 'N' for "New/Number/Name" */
		if (buf[0] == 'N')
		{
			/* Find the colon before the name */
			s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			i = atoi(buf + 2);

			/* Verify information */
			if (i < error_idx) return (4);

			/* Verify information */
			if (i >= max_ow_idx) return (2);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			ow_ptr = &ow_info[i];

			/* Copy name */
			assert(!ow_ptr->name);
			ow_ptr->name = my_strdup(s);

			/* Next... */
			continue;
		}

		/* There better be a current ow_ptr */
		if (!ow_ptr) return (3);


		/* Process 'C' for "Costs" (one line only) */
		if (buf[0] == 'C')
		{
			int ch, cn, cl;

			/* Scan for the values */
			if (3 != sscanf(buf + 2, "%d:%d:%d",
			                &ch, &cn, &cl)) return (1);

			/* Save the values */
			ow_ptr->costs[STORE_HATED] = ch;
			ow_ptr->costs[STORE_NORMAL] = cn;
			ow_ptr->costs[STORE_LIKED] = cl;

			/* Next... */
			continue;
		}

		/* Process 'I' for "Info" (multiple lines line only) */
		if (buf[0] == 'I')
		{
			int cost, inf;

			/* Scan for the values */
			if (2 != sscanf(buf + 2, "%d:%d",
			                &cost, &inf)) return (1);

			/* Save the values */
			ow_ptr->max_cost = cost;
			ow_ptr->inflation = inf;

			/* Next... */
			continue;
		}

		/* Process 'L' for "Liked races/classes" (multiple lines) */
		if (buf[0] == 'L')
		{
			if (0 != grab_one_race_flag(ow_ptr, STORE_LIKED, buf + 2))
			{
				return (5);
			}

			/* Next... */
			continue;
		}
		/* Process 'H' for "Hated races/classes" (multiple lines) */
		if (buf[0] == 'H')
		{
			if (0 != grab_one_race_flag(ow_ptr, STORE_HATED, buf + 2))
			{
				return (5);
			}

			/* Next... */
			continue;
		}

		/* Oops */
		return (6);
	}

	/* Success */
	return (0);
}

/*
 * Initialize the "wf_info" array, by parsing an ascii "template" file
 */
errr init_wf_info_txt(FILE *fp)
{
	int i;
	char buf[1024];
	char *s;

	/* Current entry */
	wilderness_type_info *wf_ptr = NULL;

	/* Just before the first record */
	error_idx = -1;

	/* Just before the first line */
	error_line = -1;

	/* Parse */
	while (0 == my_fgets(fp, buf, 1024))
	{
		/* Advance the line number */
		error_line++;

		/* Skip comments and blank lines */
		if (!buf[0] || (buf[0] == '#')) continue;

		/* Verify correct "colon" format */
		if (buf[1] != ':') return (1);

		/* Process 'N' for "New/Number/Name" */
		if (buf[0] == 'N')
		{
			/* Find the colon before the name */
			s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			i = atoi(buf + 2);

			/* Verify information */
			if (i < error_idx) return (4);

			/* Verify information */
			if (i >= max_wf_idx) return (2);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			wf_ptr = &wf_info[i];

			/* Copy the name */
			assert(!wf_ptr->name);
			wf_ptr->name = my_strdup(s);

			/* Next... */
			continue;
		}

		/* There better be a current wf_ptr */
		if (!wf_ptr) return (3);

		/* Process 'D' for "Description (one line only) */
		if (buf[0] == 'D')
		{
			/* Acquire the text */
			s = buf + 2;

			/* Copy description */
			assert(!wf_ptr->text);
			wf_ptr->text = my_strdup(s);

			/* Next... */
			continue;
		}

		/* Process 'W' for "More Info" (one line only) */
		if (buf[0] == 'W')
		{
			int entrance, level;
			int road, feat, ter_idx;
			char car;

			/* Scan for the values */
			if (6 != sscanf(buf + 2, "%d:%d:%d:%d:%d:%c",
			                &level, &entrance, &road, &feat, &ter_idx, &car)) return (1);

			/* Save the values */
			wf_ptr->level = level;
			wf_ptr->entrance = entrance;
			wf_ptr->road = road;
			wf_ptr->feat = feat;
			wf_ptr->terrain_idx = ter_idx;

			/* To acces it easily from the map structure */
			wildc2i[(int)car] = error_idx;

			/* Next... */
			continue;
		}

		/* Process 'X' for "More Info" (one line only) */
		if (buf[0] == 'X')
		{
			int terrain[MAX_WILD_TERRAIN], i;

			/* Scan for the values */
			if (MAX_WILD_TERRAIN != sscanf(buf + 2, "%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d",
			                               &terrain[0], &terrain[1], &terrain[2],
			                               &terrain[3], &terrain[4], &terrain[5],
			                               &terrain[6], &terrain[7], &terrain[8],
			                               &terrain[9], &terrain[10], &terrain[11],
			                               &terrain[12], &terrain[13], &terrain[14],
			                               &terrain[15], &terrain[16], &terrain[17])) return (1);

			/* Save the values */
			for (i = 0; i < MAX_WILD_TERRAIN; i++)
			{
				wf_ptr->terrain[i] = terrain[i];
			}

			/* Next... */
			continue;
		}

		/* Oops */
		return (6);
	}

	/* Success */
	return (0);
}


/* Random dungeon grid effects */
#define RANDOM_NONE         0x00
#define RANDOM_FEATURE      0x01
#define RANDOM_MONSTER      0x02
#define RANDOM_OBJECT       0x04
#define RANDOM_EGO          0x08
#define RANDOM_ARTIFACT     0x10
#define RANDOM_TRAP         0x20


typedef struct dungeon_grid dungeon_grid;

struct dungeon_grid
{
	int	feature; 		/* Terrain feature */
	int	monster; 		/* Monster */
	int	object; 			/* Object */
	int	ego; 			/* Ego-Item */
	int	artifact; 		/* Artifact */
	int	trap; 			/* Trap */
	int	cave_info; 		/* Flags for CAVE_MARK, CAVE_GLOW, CAVE_ICKY, CAVE_ROOM */
	int	special; 		/* Reserved for special terrain info */
	int	random; 			/* Number of the random effect */
	int bx, by;                  /* For between gates */
	int mimic;                   /* Mimiced features */
	s32b mflag;			/* monster's mflag */
	bool_ ok;
	bool_ defined;
};
static bool_ meta_sleep = TRUE;

static dungeon_grid letter[255];

/*
 * Parse a sub-file of the "extra info"
 */
static errr process_dungeon_file_aux(char *buf, int *yval, int *xval, int xvalstart, int ymax, int xmax, bool_ full)
{
	int i;

	char *zz[33];


	/* Skip "empty" lines */
	if (!buf[0]) return (0);

	/* Skip "blank" lines */
	if (isspace(buf[0])) return (0);

	/* Skip comments */
	if (buf[0] == '#') return (0);

	/* Require "?:*" format */
	if (buf[1] != ':') return (1);


	/* Process "%:<fname>" */
	if (buf[0] == '%')
	{
		/* Attempt to Process the given file */
		return (process_dungeon_file(buf + 2, yval, xval, ymax, xmax, FALSE, full));
	}

	/* Process "N:<sleep>" */
	if (buf[0] == 'N')
	{
		int num;

		if ((num = tokenize(buf + 2, 1, zz, ':', '/')) > 0)
		{
			meta_sleep = atoi(zz[0]);
		}

		return (0);
	}

	/* Process "F:<letter>:<terrain>:<cave_info>:<monster>:<object>:<ego>:<artifact>:<trap>:<special>:<mimic>:<mflag>" -- info for dungeon grid */
	if (buf[0] == 'F')
	{
		int num;

		if ((num = tokenize(buf + 2, 11, zz, ':', '/')) > 1)
		{
			int index = zz[0][0];

			/* Reset the feature */
			letter[index].feature = 0;
			letter[index].monster = 0;
			letter[index].object = 0;
			letter[index].ego = 0;
			letter[index].artifact = 0;
			letter[index].trap = 0;
			letter[index].cave_info = 0;
			letter[index].special = 0;
			letter[index].random = 0;
			letter[index].mimic = 0;
			letter[index].mflag = 0;
			letter[index].ok = TRUE;
			letter[index].defined = TRUE;

			if (num > 1)
			{
				if (zz[1][0] == '*')
				{
					letter[index].random |= RANDOM_FEATURE;
					if (zz[1][1])
					{
						zz[1]++;
						letter[index].feature = atoi(zz[1]);
					}
				}
				else
				{
					letter[index].feature = atoi(zz[1]);
				}
			}

			if (num > 2)
				letter[index].cave_info = atoi(zz[2]);

			/* Monster */
			if (num > 3)
			{
				if (zz[3][0] == '*')
				{
					letter[index].random |= RANDOM_MONSTER;
					if (zz[3][1])
					{
						zz[3]++;
						letter[index].monster = atoi(zz[3]);
					}
				}
				else
				{
					letter[index].monster = atoi(zz[3]);
				}
			}

			/* Object */
			if (num > 4)
			{
				if (zz[4][0] == '*')
				{
					letter[index].random |= RANDOM_OBJECT;

					if (zz[4][1])
					{
						zz[4]++;
						letter[index].object = atoi(zz[4]);
					}
				}
				else
				{
					letter[index].object = atoi(zz[4]);
				}
			}

			/* Ego-Item */
			if (num > 5)
			{
				if (zz[5][0] == '*')
				{
					letter[index].random |= RANDOM_EGO;

					if (zz[5][1])
					{
						zz[5]++;
						letter[index].ego = atoi(zz[5]);
					}
				}
				else
				{
					letter[index].ego = atoi(zz[5]);
				}
			}

			/* Artifact */
			if (num > 6)
			{
				if (zz[6][0] == '*')
				{
					letter[index].random |= RANDOM_ARTIFACT;

					if (zz[6][1])
					{
						zz[6]++;
						letter[index].artifact = atoi(zz[6]);
					}
				}
				else
				{
					letter[index].artifact = atoi(zz[6]);
				}
			}

			if (num > 7)
			{
				if (zz[7][0] == '*')
				{
					letter[index].random |= RANDOM_TRAP;

					if (zz[7][1])
					{
						zz[7]++;
						letter[index].trap = atoi(zz[7]);
					}
				}
				else
					letter[index].trap = atoi(zz[7]);
			}

			if (num > 8)
			{
				/* Quests can be defined by name only */
				if (zz[8][0] == '"')
				{
					int i;

					/* Hunt & shoot the ending " */
					i = strlen(zz[8]) - 1;
					if (zz[8][i] == '"') zz[8][i] = '\0';
					letter[index].special = 0;
					for (i = 0; i < MAX_Q_IDX; i++)
					{
						if (!strcmp(&zz[8][1], quest[i].name))
						{
							letter[index].special = i;
							break;
						}
					}
				}
				else
					letter[index].special = atoi(zz[8]);
			}

			if (num > 9)
			{
				letter[index].mimic = atoi(zz[9]);
			}

			if (num > 10)
			{
				letter[index].mflag = atoi(zz[10]);
			}

			return (0);
		}
	}

	/* Process "f:flags" -- level flags */
	else if (buf[0] == 'f')
	{
		char *s, *t;

		/* Parse every entry textually */
		for (s = buf + 2; *s; )
		{
			/* Find the end of this entry */
			for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) /* loop */;

			/* Nuke and skip any dividers */
			if (*t)
			{
				*t++ = '\0';
				while (*t == ' ' || *t == '|') t++;
			}

			/* Parse this entry */
			if (0 != grab_one_dungeon_flag(&dungeon_flags, s)) return 1;

			/* Start the next entry */
			s = t;
		}

		return 0;
	}

	/* Process "D:<dungeon>" -- info for the cave grids */
	else if (buf[0] == 'D')
	{
		int x, m_idx = 0;

		object_type object_type_body;

		/* Acquire the text */
		char *s = buf + 2;

		/* Length of the text */
		int len = strlen(s);

		int y = *yval;
		*xval = xvalstart;
		for (x = *xval, i = 0; ((x < xmax) && (i < len)); x++, s++, i++)
		{
			/* Access the grid */
			cave_type *c_ptr = &cave[y][x];

			int idx = s[0];

			int object_index = letter[idx].object;
			int monster_index = letter[idx].monster;
			int random = letter[idx].random;
			int artifact_index = letter[idx].artifact;

			if (!letter[idx].ok) msg_format("Warning '%c' not defined but used.", idx);

			if (init_flags & INIT_GET_SIZE) continue;

			/* use the plasma generator wilderness */
			if (((!dun_level) || (!letter[idx].defined)) && (idx == ' ')) continue;

			/* Clear some info */
			c_ptr->info = 0;

			/* Lay down a floor */
			c_ptr->mimic = letter[idx].mimic;
			cave_set_feat(y, x, letter[idx].feature);

			/* Cave info */
			c_ptr->info |= letter[idx].cave_info;

			/* Create a monster */
			if (random & RANDOM_MONSTER)
			{
				int level = monster_level;

				monster_level = quest[p_ptr->inside_quest].level + monster_index;

				m_idx = place_monster(y, x, meta_sleep, FALSE);

				monster_level = level;
			}
			else if (monster_index)
			{
				/* Place it */
				m_allow_special[monster_index] = TRUE;
				m_idx = place_monster_aux(y, x, monster_index, meta_sleep, FALSE, MSTATUS_ENEMY);
				m_allow_special[monster_index] = FALSE;
			}

			/* Set the mflag of the monster */
			if (m_idx) m_list[m_idx].mflag |= letter[idx].mflag;

			/* Object (and possible trap) */
			if ((random & RANDOM_OBJECT) && (random & RANDOM_TRAP))
			{
				int level = object_level;

				object_level = quest[p_ptr->inside_quest].level;

				/*
				 * Random trap and random treasure defined
				 * 25% chance for trap and 75% chance for object
				 */
				if (rand_int(100) < 75)
				{
					place_object(y, x, FALSE, FALSE, OBJ_FOUND_SPECIAL);
				}
				else
				{
					place_trap(y, x);
				}

				object_level = level;
			}
			else if (random & RANDOM_OBJECT)
			{
				/* Create an out of deep object */
				if (object_index)
				{
					int level = object_level;

					object_level = quest[p_ptr->inside_quest].level + object_index;
					if (rand_int(100) < 75)
						place_object(y, x, FALSE, FALSE, OBJ_FOUND_SPECIAL);
					else if (rand_int(100) < 80)
						place_object(y, x, TRUE, FALSE, OBJ_FOUND_SPECIAL);
					else
						place_object(y, x, TRUE, TRUE, OBJ_FOUND_SPECIAL);

					object_level = level;
				}
				else if (rand_int(100) < 75)
				{
					place_object(y, x, FALSE, FALSE, OBJ_FOUND_SPECIAL);
				}
				else if (rand_int(100) < 80)
				{
					place_object(y, x, TRUE, FALSE, OBJ_FOUND_SPECIAL);
				}
				else
				{
					place_object(y, x, TRUE, TRUE, OBJ_FOUND_SPECIAL);
				}
			}
			/* Random trap */
			else if (random & RANDOM_TRAP)
			{
				place_trap(y, x);
			}
			else if (object_index)
			{
				/* Get local object */
				object_type *o_ptr = &object_type_body;

				k_allow_special[object_index] = TRUE;

				/* Create the item */
				object_prep(o_ptr, object_index);

				/* Apply magic (no messages, no artifacts) */
				apply_magic(o_ptr, dun_level, FALSE, TRUE, FALSE);

				o_ptr->found = OBJ_FOUND_SPECIAL;

				k_allow_special[object_index] = FALSE;

				drop_near(o_ptr, -1, y, x);
			}

			/* Artifact */
			if (artifact_index)
			{
				int I_kind = 0;

				artifact_type *a_ptr = &a_info[artifact_index];

				object_type forge;

				/* Get local object */
				object_type *q_ptr = &forge;

				a_allow_special[artifact_index] = TRUE;

				/* Wipe the object */
				object_wipe(q_ptr);

				/* Acquire the "kind" index */
				I_kind = lookup_kind(a_ptr->tval, a_ptr->sval);

				/* Create the artifact */
				object_prep(q_ptr, I_kind);

				/* Save the name */
				q_ptr->name1 = artifact_index;

				/* Extract the fields */
				q_ptr->pval = a_ptr->pval;
				q_ptr->ac = a_ptr->ac;
				q_ptr->dd = a_ptr->dd;
				q_ptr->ds = a_ptr->ds;
				q_ptr->to_a = a_ptr->to_a;
				q_ptr->to_h = a_ptr->to_h;
				q_ptr->to_d = a_ptr->to_d;
				q_ptr->weight = a_ptr->weight;
				q_ptr->found = OBJ_FOUND_SPECIAL;

				random_artifact_resistance(q_ptr);

				a_info[artifact_index].cur_num = 1;

				a_allow_special[artifact_index] = FALSE;

				/* It's amazing that this "creating objects anywhere"
				   junk ever worked.
				   Let's just HACK around one observed bug: Shadow Cloak
				   of Luthien [Globe of Light] */
				{
					u32b f1, f2, f3, f4, f5, esp;
					object_flags(q_ptr, &f1, &f2, &f3, &f4, &f5, &esp);
					if (f5 & TR5_SPELL_CONTAIN)
						q_ptr->pval2 = -1;
				}

				/* Drop the artifact */
				drop_near(q_ptr, -1, y, x);

			}

			/* Terrain special */
			if (letter[idx].special == -1)
			{
				if (!letter[idx].bx)
				{
					letter[idx].bx = x;
					letter[idx].by = y;
				}
				else
				{
					c_ptr->special = (letter[idx].by << 8) + letter[idx].bx;
					cave[letter[idx].by][letter[idx].bx].special = (y << 8) + x;
				}
			}
			else
			{
				c_ptr->special = letter[idx].special;
			}
		}
		if (full && (*xval < x)) *xval = x;
		(*yval)++;

		return (0);
	}

	/* Process "W:<command>: ..." -- info for the wilderness */
	else if (buf[0] == 'W')
	{
		/* Process "W:D:<layout> */
		/* Layout of the wilderness */
		if (buf[2] == 'D')
		{
			int x;
			char i;

			/* Acquire the text */
			char *s = buf + 4;

			int y = *yval;

			for (x = 0; x < max_wild_x; x++)
			{
				if (1 != sscanf(s + x, "%c", &i)) return (1);
				wild_map[y][x].feat = wildc2i[(int)i];

				/*
				 * If this is a town/dungeon entrance, note
				 * its coordinates.  (Have to check for
				 * duplicate Morias...)
				 */
				if (wf_info[wildc2i[(int)i]].entrance &&
				                wf_info[wildc2i[(int)i]].wild_x == 0)
				{
					wf_info[wildc2i[(int)i]].wild_x = x;
					wf_info[wildc2i[(int)i]].wild_y = y;
				}
			}

			(*yval)++;

			return (0);
		}
		/* Process "M:<plus>:<line>" -- move line lines */
		else if (buf[2] == 'M')
		{
			if (tokenize(buf + 4, 2, zz, ':', '/') == 2)
			{
				if (atoi(zz[0]))
				{
					(*yval) += atoi(zz[1]);
				}
				else
				{
					(*yval) -= atoi(zz[1]);
				}
			}
			else
			{
				return (1);
			}
			return (0);
		}
		/* Process "W:P:<x>:<y> - starting position in the wilderness */
		else if (buf[2] == 'P')
		{
			if ((p_ptr->wilderness_x == 0) &&
			                (p_ptr->wilderness_y == 0))
			{
				if (tokenize(buf + 4, 2, zz, ':', '/') == 2)
				{
					p_ptr->wilderness_x = atoi(zz[0]);
					p_ptr->wilderness_y = atoi(zz[1]);
				}
				else
				{
					return (1);
				}
			}

			return (0);
		}
		/* Process "W:E:<dungeon>:<y>:<x> - entrance to the dungeon <dungeon> */
		else if (buf[2] == 'E')
		{
			if (tokenize(buf + 4, 3, zz, ':', '/') == 3)
			{
				wild_map[atoi(zz[1])][atoi(zz[2])].entrance = 1000 + atoi(zz[0]);
			}
			else
			{
				return (1);
			}

			return (0);
		}
	}

	/* Process "P:<y>:<x>" -- player position */
	else if (buf[0] == 'P')
	{
		if (init_flags & INIT_CREATE_DUNGEON)
		{
			if (tokenize(buf + 2, 2, zz, ':', '/') == 2)
			{
				/* Place player in a quest level */
				if (p_ptr->inside_quest || (init_flags & INIT_POSITION))
				{
					p_ptr->py = atoi(zz[0]);
					p_ptr->px = atoi(zz[1]);
				}
				/* Place player in the town */
				else if ((p_ptr->oldpx == 0) && (p_ptr->oldpy == 0))
				{
					p_ptr->oldpy = atoi(zz[0]);
					p_ptr->oldpx = atoi(zz[1]);
				}
			}
		}

		return (0);
	}

	/* Process "M:<type>:<maximum>" -- set maximum values */
	else if (buf[0] == 'M')
	{
		if (tokenize(buf + 2, 3, zz, ':', '/') >= 2)
		{
			/* Maximum towns */
			if (zz[0][0] == 'T')
			{
				max_towns = atoi(zz[1]);
			}

			/* Maximum real towns */
			if (zz[0][0] == 't')
			{
				max_real_towns = atoi(zz[1]);
			}

			/* Maximum r_idx */
			else if (zz[0][0] == 'R')
			{
				max_r_idx = atoi(zz[1]);
			}

			/* Maximum re_idx */
			else if (zz[0][0] == 'r')
			{
				max_re_idx = atoi(zz[1]);
			}

			/* Maximum s_idx */
			else if (zz[0][0] == 'k')
			{
				max_s_idx = atoi(zz[1]);
				if (max_s_idx > MAX_SKILLS) return (1);
			}

			/* Maximum ab_idx */
			else if (zz[0][0] == 'b')
			{
				max_ab_idx = atoi(zz[1]);
			}

			/* Maximum k_idx */
			else if (zz[0][0] == 'K')
			{
				max_k_idx = atoi(zz[1]);
			}

			/* Maximum v_idx */
			else if (zz[0][0] == 'V')
			{
				max_v_idx = atoi(zz[1]);
			}

			/* Maximum f_idx */
			else if (zz[0][0] == 'F')
			{
				max_f_idx = atoi(zz[1]);
			}

			/* Maximum a_idx */
			else if (zz[0][0] == 'A')
			{
				max_a_idx = atoi(zz[1]);
			}

			/* Maximum e_idx */
			else if (zz[0][0] == 'E')
			{
				max_e_idx = atoi(zz[1]);
			}

			/* Maximum ra_idx */
			else if (zz[0][0] == 'Z')
			{
				max_ra_idx = atoi(zz[1]);
			}

			/* Maximum o_idx */
			else if (zz[0][0] == 'O')
			{
				max_o_idx = atoi(zz[1]);
			}

			/* Maximum player types */
			else if (zz[0][0] == 'P')
			{
				if (zz[1][0] == 'R')
				{
					max_rp_idx = atoi(zz[2]);
				}
				else if (zz[1][0] == 'S')
				{
					max_rmp_idx = atoi(zz[2]);
				}
				else if (zz[1][0] == 'C')
				{
					max_c_idx = atoi(zz[2]);
				}
				else if (zz[1][0] == 'M')
				{
					max_mc_idx = atoi(zz[2]);
				}
				else if (zz[1][0] == 'H')
				{
					max_bg_idx = atoi(zz[2]);
				}
			}

			/* Maximum m_idx */
			else if (zz[0][0] == 'M')
			{
				max_m_idx = atoi(zz[1]);
			}

			/* Maximum tr_idx */
			else if (zz[0][0] == 'U')
			{
				max_t_idx = atoi(zz[1]);
			}

			/* Maximum wf_idx */
			else if (zz[0][0] == 'W')
			{
				max_wf_idx = atoi(zz[1]);
			}

			/* Maximum ba_idx */
			else if (zz[0][0] == 'B')
			{
				max_ba_idx = atoi(zz[1]);
			}

			/* Maximum st_idx */
			else if (zz[0][0] == 'S')
			{
				max_st_idx = atoi(zz[1]);
			}

			/* Maximum set_idx */
			else if (zz[0][0] == 's')
			{
				max_set_idx = atoi(zz[1]);
			}

			/* Maximum ow_idx */
			else if (zz[0][0] == 'N')
			{
				max_ow_idx = atoi(zz[1]);
			}

			/* Maximum wilderness x size */
			else if (zz[0][0] == 'X')
			{
				max_wild_x = atoi(zz[1]);
			}

			/* Maximum wilderness y size */
			else if (zz[0][0] == 'Y')
			{
				max_wild_y = atoi(zz[1]);
			}

			/* Maximum d_idx */
			else if (zz[0][0] == 'D')
			{
				max_d_idx = atoi(zz[1]);
			}

			return (0);
		}
	}

	/* Failure */
	return (1);
}




/*
 * Helper function for "process_dungeon_file()"
 */
static cptr process_dungeon_file_expr(char **sp, char *fp)
{
	static char pref_tmp_value[8];
	cptr v;

	char *b;
	char *s;

	char b1 = '[';
	char b2 = ']';

	char f = ' ';

	/* Initial */
	s = (*sp);

	/* Skip spaces */
	while (isspace(*s)) s++;

	/* Save start */
	b = s;

	/* Default */
	v = "?o?o?";

	/* Analyze */
	if (*s == b1)
	{
		const char *p;
		const char *t;

		/* Skip b1 */
		s++;

		/* First */
		t = process_dungeon_file_expr(&s, &f);

		/* Oops */
		if (!*t)
		{
			/* Nothing */
		}

		/* Function: IOR */
		else if (streq(t, "IOR"))
		{
			v = "0";
			while (*s && (f != b2))
			{
				t = process_dungeon_file_expr(&s, &f);
				if (*t && !streq(t, "0")) v = "1";
			}
		}

		/* Function: AND */
		else if (streq(t, "AND"))
		{
			v = "1";
			while (*s && (f != b2))
			{
				t = process_dungeon_file_expr(&s, &f);
				if (*t && streq(t, "0")) v = "0";
			}
		}

		/* Function: NOT */
		else if (streq(t, "NOT"))
		{
			v = "1";
			while (*s && (f != b2))
			{
				t = process_dungeon_file_expr(&s, &f);
				if (*t && streq(t, "1")) v = "0";
			}
		}

		/* Function: EQU */
		else if (streq(t, "EQU"))
		{
			v = "1";
			if (*s && (f != b2))
			{
				t = process_dungeon_file_expr(&s, &f);
			}
			while (*s && (f != b2))
			{
				p = t;
				t = process_dungeon_file_expr(&s, &f);
				if (*t && !streq(p, t)) v = "0";
			}
		}

		/* Function: LEQ */
		else if (streq(t, "LEQ"))
		{
			v = "1";
			if (*s && (f != b2))
			{
				t = process_dungeon_file_expr(&s, &f);
			}
			while (*s && (f != b2))
			{
				p = t;
				t = process_dungeon_file_expr(&s, &f);
				if (*t && (strcmp(p, t) > 0)) v = "0";
			}
		}

		/* Function: GEQ */
		else if (streq(t, "GEQ"))
		{
			v = "1";
			if (*s && (f != b2))
			{
				t = process_dungeon_file_expr(&s, &f);
			}
			while (*s && (f != b2))
			{
				p = t;
				t = process_dungeon_file_expr(&s, &f);
				if (*t && (strcmp(p, t) < 0)) v = "0";
			}
		}

		/* Oops */
		else
		{
			while (*s && (f != b2))
			{
				t = process_dungeon_file_expr(&s, &f);
			}
		}

		/* Verify ending */
		if (f != b2) v = "?x?x?";

		/* Extract final and Terminate */
		if ((f = *s) != '\0') * s++ = '\0';
	}

	/* Other */
	else
	{
		bool_ text_mode = FALSE;

		/* Accept all printables except spaces and brackets */
		while (isprint(*s))
		{
			if (*s == '"') text_mode = !text_mode;
			if (!text_mode)
			{
				if (strchr(" []", *s))
					break;
			}
			else
			{
				if (strchr("[]", *s))
					break;
			}

			++s;
		}

		/* Extract final and Terminate */
		if ((f = *s) != '\0') * s++ = '\0';

		/* Variable */
		if (*b == '$')
		{
			/* System */
			if (streq(b + 1, "SYS"))
			{
				v = ANGBAND_SYS;
			}

			/* Race */
			else if (streq(b + 1, "RACE"))
			{
				v = rp_ptr->title;
			}

			/* Race Mod */
			else if (streq(b + 1, "RACEMOD"))
			{
				v = rmp_ptr->title;
			}

			/* Class */
			else if (streq(b + 1, "CLASS"))
			{
				v = cp_ptr->title;
			}

			/* Player */
			else if (streq(b + 1, "PLAYER"))
			{
				v = player_base;
			}

			/* Town */
			else if (streq(b + 1, "TOWN"))
			{
				strnfmt(pref_tmp_value, 8, "%d", p_ptr->town_num);
				v = pref_tmp_value;
			}

			/* Town destroyed */
			else if (prefix(b + 1, "TOWN_DESTROY"))
			{
				strnfmt(pref_tmp_value, 8, "%d", town_info[atoi(b + 13)].destroyed);
				v = pref_tmp_value;
			}

			/* Current quest number */
			else if (streq(b + 1, "QUEST_NUMBER"))
			{
				strnfmt(pref_tmp_value, 8, "%d", p_ptr->inside_quest);
				v = pref_tmp_value;
			}

			/* Number of last quest */
			else if (streq(b + 1, "LEAVING_QUEST"))
			{
				strnfmt(pref_tmp_value, 8, "%d", leaving_quest);
				v = pref_tmp_value;
			}

			/* DAYTIME status */
			else if (prefix(b + 1, "DAYTIME"))
			{
				if ((bst(HOUR, turn) >= 6) && (bst(HOUR, turn) < 18))
					v = "1";
				else
					v = "0";
			}

			/* Quest status */
			else if (prefix(b + 1, "QUEST"))
			{
				/* "QUEST" uses a special parameter to determine the number of the quest */
				if (*(b + 6) != '"')
					strnfmt(pref_tmp_value, 8, "%d", quest[atoi(b + 6)].status);
				else
				{
					char c[80];
					int i;

					/* Copy it to temp array, so that we can modify it safely */
					strcpy(c, b + 7);

					/* Hunt & shoot the ending " */
					for (i = 0; (c[i] != '"') && (c[i] != '\0'); i++);
					if (c[i] == '"') c[i] = '\0';
					strcpy(pref_tmp_value, "-1");
					for (i = 0; i < MAX_Q_IDX; i++)
					{
						if (streq(c, quest[i].name))
						{
							strnfmt(pref_tmp_value, 8, "%d", quest[i].status);
							break;
						}
					}
				}
				v = pref_tmp_value;
			}

			/* Variant name */
			else if (streq(b + 1, "VARIANT"))
			{
				v = "ToME";
			}

			/* Wilderness */
			else if (streq(b + 1, "WILDERNESS"))
			{
				v = "NORMAL";
			}
		}

		/* Constant */
		else
		{
			v = b;
		}
	}

	/* Save */
	(*fp) = f;

	/* Save */
	(*sp) = s;

	/* Result */
	return (v);
}


errr process_dungeon_file(cptr name, int *yval, int *xval, int ymax, int xmax, bool_ init, bool_ full)
{
	FILE *fp = 0;

	char buf[1024];

	int num = -1, i;

	errr err = 0;

	bool_ bypass = FALSE;

	/* Save the start since it ought to be modified */
	int xmin = *xval;

	if (init)
	{
		meta_sleep = TRUE;
		for (i = 0; i < 255; i++)
		{
			letter[i].defined = FALSE;
			if (i == ' ') letter[i].ok = TRUE;
			else letter[i].ok = FALSE;
			letter[i].bx = 0;
			letter[i].by = 0;
		}
	}

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, name);

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* No such file */
	if (!fp)
	{
		msg_format("Cannot find file %s at %s", name, buf);
		return ( -1);
	}

	/* Process the file */
	while (0 == my_fgets(fp, buf, 1024))
	{
		/* Count lines */
		num++;


		/* Skip "empty" lines */
		if (!buf[0]) continue;

		/* Skip "blank" lines */
		if (isspace(buf[0])) continue;

		/* Skip comments */
		if (buf[0] == '#') continue;


		/* Process "?:<expr>" */
		if ((buf[0] == '?') && (buf[1] == ':'))
		{
			char f;
			cptr v;
			char *s;

			/* Start */
			s = buf + 2;

			/* Parse the expr */
			v = process_dungeon_file_expr(&s, &f);

			/* Set flag */
			bypass = (streq(v, "0") ? TRUE : FALSE);

			/* Continue */
			continue;
		}

		/* Apply conditionals */
		if (bypass) continue;


		/* Process "%:<file>" */
		if (buf[0] == '%')
		{
			/* Process that file if allowed */
			(void)process_dungeon_file(buf + 2, yval, xval, ymax, xmax, FALSE, full);

			/* Continue */
			continue;
		}


		/* Process the line */
		err = process_dungeon_file_aux(buf, yval, xval, xmin, ymax, xmax, full);

		/* Oops */
		if (err) break;
	}


	/* Error */
	if (err)
	{
		/* Useful error message */
		msg_format("Error %d in line %d of file '%s'.", err, num, name);
		msg_format("Parsing '%s'", buf);
	}

	/* Close the file */
	my_fclose(fp);

	/* Result */
	return (err);
}
