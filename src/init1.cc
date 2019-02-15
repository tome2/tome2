#include "init1.hpp"

#include "ability_type.hpp"
#include "artifact_type.hpp"
#include "cave.hpp"
#include "cave_type.hpp"
#include "dungeon_info_type.hpp"
#include "dungeon_flag.hpp"
#include "ego_flag.hpp"
#include "ego_item_type.hpp"
#include "feature_flag.hpp"
#include "feature_type.hpp"
#include "files.hpp"
#include "game.hpp"
#include "gods.hpp"
#include "init2.hpp"
#include "monster2.hpp"
#include "monster_ego.hpp"
#include "monster_race.hpp"
#include "monster_race_flag.hpp"
#include "monster_spell.hpp"
#include "monster_type.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "object_flag.hpp"
#include "object_flag_meta.hpp"
#include "object_kind.hpp"
#include "player_class.hpp"
#include "player_race.hpp"
#include "player_race_flag.hpp"
#include "player_race_mod.hpp"
#include "player_type.hpp"
#include "set_type.hpp"
#include "skill_flag.hpp"
#include "skill_type.hpp"
#include "skills.hpp"
#include "spells5.hpp"
#include "store_flag.hpp"
#include "store_info_type.hpp"
#include "tables.hpp"
#include "town_type.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "wilderness_type_info.hpp"
#include "z-form.hpp"
#include "z-rand.hpp"
#include "z-util.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/optional.hpp>

using boost::algorithm::equals;
using boost::algorithm::iequals;
using boost::algorithm::ends_with;
using boost::algorithm::starts_with;


/**
 * Expand vector such that it has room for an item at index i.
 * If the vector is already large enough, nothing happens.
 */
template <class T> typename std::vector<T>::reference expand_to_fit_index(std::vector<T> &v, std::size_t i)
{
	if (v.size() < i + 1)
	{
		v.resize(i + 1);
	}
	return v[i];
}


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
static const char *r_info_blow_method[] =
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
static const char *r_info_blow_effect[] =
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
	const char *(&m_flags)[N];
public:
	flag_tie_impl(u32b *mask, const char *(&flags)[N]): m_mask(mask), m_flags(flags) {
		// Empty
	}

	bool match(const char *flag) {
		for (unsigned int i = 0; i < N; i++)
		{
			if (equals(flag, m_flags[i]))
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
template<size_t N> detail::flag_tie_impl<N> flag_tie(u32b *mask, const char *(&flags)[N]) {
	static_assert(N <= 32, "Array too large to represent result");
	return detail::flag_tie_impl<N>(mask, flags);
}

/**
 * Look up flag in array of flags.
 */
template<size_t N> bool lookup_flags(const char *)
{
	// Base case: No match
	return false;
}

/**
 * Look up flag in array of flags.
 */
template<size_t N, typename... Pairs> bool lookup_flags(const char *flag, detail::flag_tie_impl<N> tie, Pairs&&...rest) {
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
const char *name;
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
	"XXX34",
	"UNDEATH",              /*  35*/
	"THRAIN",               /*  36*/
	"BARAHIR",              /*  37*/
	"TULKAS",               /*  38*/
	"NARYA",                /*  39*/
	"NENYA",                /*  40*/
	"VILYA",                /*  41*/
	"POWER",                /*  42*/
	"XXX43",
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
	"XXX115",               /*  115*/
	"XXX116",               /*  116*/
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
static bool unknown_shut_up = false;
static errr grab_one_class_flag(std::array<u32b, 2> &choice, const char *what)
{
	auto const &class_info = game->edit_data.class_info;

	/* Scan classes flags */
	for (std::size_t i = 0; i < class_info.size(); i++)
	{
		if (class_info[i].title == what)
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

static errr grab_one_race_allow_flag(std::array<u32b, 2> &choice, const char *what)
{
	auto const &race_info = game->edit_data.race_info;

	/* Scan classes flags */
	for (std::size_t i = 0; i < race_info.size(); i++)
	{
		if (race_info[i].title == what)
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
static errr grab_one_skill_flag(skill_flag_set *flags, const char *what)
{
#define SKF(tier, index, name) \
	if (equals(what, #name)) \
	{ \
	        *flags |= BOOST_PP_CAT(SKF_,name); \
	        return 0; \
        };
#include "skill_flag_list.hpp"
#undef SKF

	/* Oops */
	msg_format("(2)Unknown skill flag '%s'.", what);

	/* Error */
	return (1);
}
/*
 * Grab one flag from a textual string
 */
static errr grab_one_player_race_flag(player_race_flag_set *flags, const char *what)
{
#define PR(tier, index, name) \
	if (equals(what, #name)) \
	{ \
	        *flags |= BOOST_PP_CAT(PR_,name); \
	        return 0; \
        };
#include "player_race_flag_list.hpp"
#undef PR

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
 * Convert string to object_flag_set value
 */
static object_flag_set object_flag_set_from_string(const char *what)
{
	for (auto const flag_meta: object_flags_meta())
	{
		if (equals(what, flag_meta->e_name))
		{
			return flag_meta->flag_set;
		};
	}

	return object_flag_set();
}

/*
 * Grab one flag in an object_kind from a textual string
 */
static errr grab_object_flag(object_flag_set *flags, const char *what)
{
	if (object_flag_set f = object_flag_set_from_string(what))
	{
		*flags |= f;
		return 0;
	}

	/* Oops */
	msg_format("Unknown object flag '%s'.", what);

	/* Error */
	return (1);
}

/*
 * Read skill values
 */
static int read_skill_modifiers(skill_modifiers *skill_modifiers, const char *buf)
{
	long val, mod;
	char v, m;
	char name[200];

	if (5 != sscanf(buf, "%c%ld:%c%ld:%s", &v, &val, &m, &mod, name))
	{
		return 1;
	}

	long i;
	if ((i = find_skill(name)) == -1)
	{
		return 1;
	}

	auto s = &expand_to_fit_index(skill_modifiers->modifiers, i);

	s->basem = monster_ego_modify(v);
	s->base = val;
	s->modm = monster_ego_modify(m);
	s->mod = mod;

	return 0;
}


/*
 * Read prototype objects
 */
static int read_proto_object(std::vector<object_proto> *protos, const char *buf)
{
	int s0, s1, s2, s3, s4;

	if (5 != sscanf(buf, "%d:%d:%d:%dd%d", &s0, &s1, &s4, &s2, &s3))
	{
		s4 = 0;

		if (4 != sscanf(buf, "%d:%d:%dd%d", &s0, &s1, &s2, &s3))
		{
			return 1;
		}
	}

	object_proto proto;
	proto.pval = s4;
	proto.tval = s0;
	proto.sval = s1;
	proto.dd = s2;
	proto.ds = s3;

	protos->emplace_back(proto);

	return 0;
}


/*
 * Read an ability assignment
 */
static int read_ability(std::vector<player_race_ability_type> *abilities, char *buf)
{
	int level = 0;
	char *name = nullptr;

	// Find the ':' separator
	if (!(name = strchr(buf, ':')))
	{
		return 1;
	}

	// Split the buffer there and advance to point at the ability name
	name++;

	// Extract the level
	if (1 != sscanf(buf, "%d:", &level))
	{
		return 1;
	}

	// Try to find the ability by name
	int idx = find_ability(name);
	if (idx < 0)
	{
		return 1;
	}

	// Insert
	player_race_ability_type ability;
	ability.ability = idx;
	ability.level = level;
	abilities->emplace_back(ability);

	return 0;
}


/**
 * Find a power by its name
 */
static boost::optional<int> find_power_idx(const char *name)
{
	for (auto const &entry: game->powers)
	{
		auto power_ptr = entry.second;

		if (iequals(power_ptr->name, name))
		{
			return entry.first;
		}
	}

	return boost::none;
}


/*
 * Initialize the "player" arrays, by parsing an ascii "template" file
 */
errr init_player_info_txt(FILE *fp)
{
	auto &class_info = game->edit_data.class_info;
	auto &race_info = game->edit_data.race_info;
	auto &race_mod_info = game->edit_data.race_mod_info;
	auto &gen_skill = game->edit_data.gen_skill;

	int lev = 1;
	int tit_idx = 0;
	char buf[1024];

	/* Current entry */
	player_race *rp_ptr = NULL;
	player_race_mod *rmp_ptr = NULL;
	player_class *c_ptr = NULL;
	player_spec *s_ptr = NULL;


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

		/* Reinit error_idx */
		if (buf[0] == 'I')
		{
			error_idx = -1;
			continue;
		}

		/* Process 'G:k' for "General skills" */
		if ((buf[0] == 'G') && (buf[2] == 'k'))
		{
			if (read_skill_modifiers(&gen_skill, buf + 4))
			{
				return 1;
			}

			/* Next... */
			continue;
		}

		/* Process 'N' for "New/Number/Name" */
		if ((buf[0] == 'R') && (buf[2] == 'N'))
		{
			/* Find the colon before the name */
			char *s = strchr(buf + 4, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			int i = atoi(buf + 4);

			/* Verify information */
			if (i < error_idx) return (4);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			rp_ptr = &expand_to_fit_index(race_info, i);
			assert(rp_ptr->title.empty());

			/* Copy title */
			rp_ptr->title = s;

			/* Initialize */
			lev = 1;

			/* Next... */
			continue;
		}

		/* Process 'D' for "Description" */
		if ((buf[0] == 'R') && (buf[2] == 'D'))
		{
			// Need newline?
			if (!rp_ptr->desc.empty())
			{
				rp_ptr->desc += '\n';
			}

			// Append
			rp_ptr->desc += (buf + 4);

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
			rp_ptr->lflags[lev].pval = s[1];

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
			{
				rp_ptr->ps.adj[z] = s[z];
			}

			/* Next... */
			continue;
		}

		/* Process 'Z' for "powers" */
		if ((buf[0] == 'R') && (buf[2] == 'Z'))
		{
			/* Acquire the text */
			char const *s = buf + 4;

			/* Find it in the list */
			if (auto power_idx = find_power_idx(s))
			{
				rp_ptr->ps.powers.push_back(*power_idx);
			}
			else
			{
				return 6;
			}

			/* Next... */
			continue;
		}

		/* Process 'k' for "skills" */
		if ((buf[0] == 'R') && (buf[2] == 'k'))
		{
			if (read_skill_modifiers(&rp_ptr->skill_modifiers, buf + 4))
			{
				return 1;
			}

			/* Next... */
			continue;
		}

		/* Process 'b' for "abilities" */
		if ((buf[0] == 'R') && (buf[2] == 'b'))
		{
			if (read_ability(&rp_ptr->abilities, buf + 4))
			{
				return 1;
			}

			/* Next... */
			continue;
		}

		/* Process 'P' for "xtra" */
		if ((buf[0] == 'R') && (buf[2] == 'P'))
		{
			int s[3];

			/* Scan for the values */
			if (3 != sscanf(buf + 4, "%d:%d:%d",
					&s[0], &s[1], &s[2])) return (1);

			rp_ptr->ps.mhp = s[0];
			rp_ptr->ps.exp = s[1];
			rp_ptr->infra = s[2];

			/* Next... */
			continue;
		}

		/* Process 'G' for "Player flags" (multiple lines) */
		if ((buf[0] == 'R') && (buf[2] == 'G'))
		{
			if (0 != grab_one_player_race_flag(&rp_ptr->flags, buf + 4))
			{
				return (5);
			}

			/* Next... */
			continue;
		}

		/* Process 'F' for "level Flags" (multiple lines) */
		if ((buf[0] == 'R') && (buf[2] == 'F'))
		{
			if (grab_object_flag(&rp_ptr->lflags[lev].oflags, buf + 4))
			{
				return (5);
			}

			/* Next... */
			continue;
		}

		/* Process 'O' for "Object birth" */
		if ((buf[0] == 'R') && (buf[2] == 'O'))
		{
			if (read_proto_object(&rp_ptr->object_protos, buf + 4))
			{
				return 1;
			}

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
			char *s = strchr(buf + 4, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			int i = atoi(buf + 4);

			/* Verify information */
			if (i < error_idx) return (4);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			rmp_ptr = &expand_to_fit_index(race_mod_info, i);
			assert(rmp_ptr->title.empty());

			/* Copy title */
			rmp_ptr->title = s;

			/* Initialize */
			lev = 1;

			/* Next... */
			continue;
		}

		/* Process 'D' for "Description" */
		if ((buf[0] == 'S') && (buf[2] == 'D'))
		{
			/* Acquire the text */
			char const *s = buf + 6;

			/* Place */
			rmp_ptr->place = (buf[4] == 'A');

			/* Description */
			if (!rmp_ptr->description.empty())
			{
				rmp_ptr->description += '\n';
			}
			rmp_ptr->description += s;

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
			rmp_ptr->lflags[lev].pval = s[1];

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
			{
				rmp_ptr->ps.adj[z] = s[z];
			}

			/* Next... */
			continue;
		}

		/* Process 'Z' for "powers" */
		if ((buf[0] == 'S') && (buf[2] == 'Z'))
		{
			/* Acquire the text */
			char const *s = buf + 4;

			/* Find it in the list */
			if (auto power_idx = find_power_idx(s))
			{
				rmp_ptr->ps.powers.push_back(*power_idx);
			}
			else
			{
				return 6;
			}

			/* Next... */
			continue;
		}

		/* Process 'k' for "skills" */
		if ((buf[0] == 'S') && (buf[2] == 'k'))
		{
			if (read_skill_modifiers(&rmp_ptr->skill_modifiers, buf + 4))
			{
				return 1;
			}

			/* Next... */
			continue;
		}

		/* Process 'b' for "abilities" */
		if ((buf[0] == 'S') && (buf[2] == 'b'))
		{
			if (read_ability(&rmp_ptr->abilities, buf + 4))
			{
				return (1);
			}

			continue;
		}

		/* Process 'P' for "xtra" */
		if ((buf[0] == 'S') && (buf[2] == 'P'))
		{
			int s[3];

			/* Scan for the values */
			if (3 != sscanf(buf + 4, "%d:%d:%d",
			                &s[0], &s[1], &s[2])) return (1);

			rmp_ptr->ps.mhp = s[0];
			rmp_ptr->ps.exp = s[1];
			rmp_ptr->infra = s[2];

			/* Next... */
			continue;
		}

		/* Process 'G' for "Player flags" (multiple lines) */
		if ((buf[0] == 'S') && (buf[2] == 'G'))
		{
			if (0 != grab_one_player_race_flag(&rmp_ptr->flags, buf + 4))
			{
				return (5);
			}

			/* Next... */
			continue;
		}

		/* Process 'F' for "level Flags" (multiple lines) */
		if ((buf[0] == 'S') && (buf[2] == 'F'))
		{
			if (0 != grab_object_flag(&rmp_ptr->lflags[lev].oflags, buf + 4))
			{
				return (5);
			}

			/* Next... */
			continue;
		}

		/* Process 'O' for "Object birth" */
		if ((buf[0] == 'S') && (buf[2] == 'O'))
		{
			if (read_proto_object(&rmp_ptr->object_protos, buf + 4))
			{
				return 1;
			}

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
			std::array<u32b, 2> choice { };
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
			/* Advance beyond prefix */
			char *s = strchr(buf + 4, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Extract the suffix */
			std::string suffix(s + 1);
			if (suffix.empty()) return (1);

			/* Split suffix into fields */
			std::vector<std::string> fields;
			boost::algorithm::split(fields, suffix, boost::is_any_of(":"));

			/* Make sure we have two fields */
			if (fields.size() < 2) return (1);

			/* Get the entry index */
			int i = atoi(buf + 4);
			if (i < error_idx) return (4);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			c_ptr = &expand_to_fit_index(class_info, i);
			assert(c_ptr->title.empty());

			/* Initialize */
			c_ptr->display_order_idx = std::stoi(fields[0]);
			c_ptr->title = fields[1];

			/* Initialize */
			lev = 1;
			tit_idx = 0;

			/* Next... */
			continue;
		}

		/* Process 'D' for "Description" */
		if ((buf[0] == 'C') && (buf[2] == 'D'))
		{
			/* Acquire the text */
			char const *s = buf + 6;

			switch (buf[4])
			{
			case '0': /* Class description */
				// Need newline?
				if (!c_ptr->desc.empty())
				{
					c_ptr->desc += '\n';
				}
				// Append
				c_ptr->desc += s;
				break;

			case '1': /* Class title */
				/* Copy */
				assert(c_ptr->titles[tit_idx].empty());
				c_ptr->titles[tit_idx] = s;

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
			if (read_proto_object(&c_ptr->object_protos, buf + 4))
			{
				return 1;
			}

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
			c_ptr->lflags[lev].pval = s[1];

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
			{
				c_ptr->ps.adj[z] = s[z];
			}

			/* Next... */
			continue;
		}

		/* Process 'k' for "skills" */
		if ((buf[0] == 'C') && (buf[2] == 'k'))
		{
			if (read_skill_modifiers(&c_ptr->skill_modifiers, buf + 4))
			{
				return 1;
			}

			/* Next... */
			continue;
		}

		/* Process 'b' for "abilities" */
		if ((buf[0] == 'C') && (buf[2] == 'b'))
		{
			if (read_ability(&c_ptr->abilities, buf + 4))
			{
				return 1;
			}

			/* Next... */
			continue;
		}

		/* Process 'g' for "gods" */
		if ((buf[0] == 'C') && (buf[2] == 'g'))
		{
			int i;

			if (equals(buf + 4, "All Gods"))
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
			/* Acquire the text */
			char const *s = buf + 4;

			/* Find it in the list */
			if (auto power_idx = find_power_idx(s))
			{
				c_ptr->ps.powers.push_back(*power_idx);
			}
			else
			{
				return 6;
			}

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

			c_ptr->ps.mhp = s[0];
			c_ptr->ps.exp = s[1];

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
			if (0 != grab_one_player_race_flag(&c_ptr->flags, buf + 4))
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
			for (char *s = buf + 4; *s; )
			{
				char *t;

				/* Find the end of this entry */
				for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) /* loop */;

				/* Nuke and skip any dividers */
				if (*t)
				{
					*t++ = '\0';
					while (*t == ' ' || *t == '|') t++;
				}

				/* Parse this entry */
				if (0 != grab_object_flag(&c_ptr->lflags[lev].oflags, s))
				{
					return (5);
				}

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
				char const *s = buf + 6;

				/* Paranoia -- require a name */
				if (!*s) return (1);

				/* Create the spec entry */
				c_ptr->spec.emplace_back(player_spec());

				/* Fill in initial values */
				s_ptr = &c_ptr->spec.back();
				s_ptr->title = my_strdup(s);

				/* Next... */
				continue;
			}

			/* Process 'D' for "Description" */
			if (buf[4] == 'D')
			{
				/* Acquire the text */
				char const *s = buf + 6;

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
				if (read_proto_object(&s_ptr->object_protos, buf + 6))
				{
					return 1;
				}

				/* Next... */
				continue;
			}

			/* Process 'g' for "gods" */
			if (buf[4] == 'g')
			{
				int i;

				if (equals(buf + 6, "All Gods"))
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
				if (read_skill_modifiers(&s_ptr->skill_modifiers, buf + 6))
				{
					return 1;
				}

				/* Next... */
				continue;
			}

			/* Process 'b' for "abilities" */
			if (buf[4] == 'b')
			{
				if (read_ability(&s_ptr->abilities, buf + 6))
				{
					return 1;
				}

				/* Next... */
				continue;
			}

			/* Process 'G' for "Player flags" (multiple lines) */
			if (buf[4] == 'G')
			{
				/* Parse every entry */
				for (char *s = buf + 6; *s; )
				{
					char *t;

					/* Find the end of this entry */
					for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) /* loop */;

					/* Nuke and skip any dividers */
					if (*t)
					{
						*t++ = '\0';
						while (*t == ' ' || *t == '|') t++;
					}

					/* Parse this entry */
					if (0 != grab_one_player_race_flag(&s_ptr->flags, s))
					{
						return (5);
					}

					/* Start the next entry */
					s = t;
				}

				/* Next... */
				continue;
			}
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
	char buf[1024];

	auto &v_info = game->edit_data.v_info;

	/* Current entry */
	vault_type *v_ptr = nullptr;

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
			char *s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			int i = atoi(buf + 2);

			/* Verify information */
			if (i <= error_idx) return (4);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			v_ptr = &expand_to_fit_index(v_info, i);

			/* Next... */
			continue;
		}

		/* There better be a current v_ptr */
		if (!v_ptr) return (3);

		/* Process 'D' for "Description" */
		if (buf[0] == 'D')
		{
			/* Acquire the text */
			const char *s = buf + 2;

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
static int grab_one_feature_flag(const char *what, feature_flag_set *flags)
{
#define FF(tier, index, name) \
	if (equals(what, #name)) \
	{ \
	        *flags |= BOOST_PP_CAT(FF_,name); \
	        return 0; \
        };
#include "feature_flag_list.hpp"
#undef FF

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
	auto &f_info = game->edit_data.f_info;

	char buf[1024];

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
			char *s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			int i = atoi(buf + 2);

			/* Verify information */
			if (i <= error_idx) return (4);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			f_ptr = &expand_to_fit_index(f_info, i);

			/* Copy name */
			assert(f_ptr->name.empty());
			f_ptr->name = s;

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
			const char *s = buf + 4;

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
			const char *tmp;
			int i;

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
				{
					if (equals(d_info_dtypes[j].name, tmp))
					{
						f_ptr->d_type[i] = d_info_dtypes[j].feat;
						break;
					}
					else
					{
						j++;
					}
				}

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
			if (0 != grab_one_feature_flag(buf + 2, &f_ptr->flags))
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
 * Initialize the "k_info" array, by parsing an ascii "template" file
 */
errr init_k_info_txt(FILE *fp)
{
	auto &k_info = game->edit_data.k_info;

	int i;
	char buf[1024];
	char *s, *t;

	/* Current entry */
	std::shared_ptr<object_kind> k_ptr;

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

			/* Save the index */
			error_idx = i;

			/* Point at the "info"; automatically creates an entry */
			k_info.emplace(std::make_pair(i, std::make_shared<object_kind>(i)));
			k_ptr = k_info[i];
			k_ptr->name = s;

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

			if (!k_ptr->text.empty())
			{
				k_ptr->text += '\n';
			}

			k_ptr->text += s;

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

					if (auto spell_idx = find_spell(spl))
					{
						pval2 = *spell_idx;
					}
					else
					{
						msg_format("No spell '%s'.", spl);
						return 1;
					}
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
			/* Acquire the text */
			s = buf + 2;

			/* Find it in the list */
			if (auto power_idx = find_power_idx(s))
			{
				k_ptr->power = *power_idx;
			}
			else
			{
				return 6;
			}

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
			if (0 != grab_object_flag(&k_ptr->flags, buf + 2))
			{
				return (5);
			}

			/* Next... */
			continue;
		}

		/* Hack -- Process 'f' for obvious flags */
		if (buf[0] == 'f')
		{
			if (0 != grab_object_flag(&k_ptr->oflags, buf + 2))
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
 * Initialize the "a_info" array, by parsing an ascii "template" file
 */
errr init_a_info_txt(FILE *fp)
{
	auto &a_info = game->edit_data.a_info;

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

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			a_ptr = &expand_to_fit_index(a_info, i);

			/* Copy name */
			assert(!a_ptr->name);
			a_ptr->name = my_strdup(s);

			/* Ensure empty description */
			a_ptr->text = my_strdup("");

			/* Ignore everything */
			a_ptr->flags |= TR_IGNORE_ACID |
					TR_IGNORE_ELEC |
					TR_IGNORE_FIRE |
					TR_IGNORE_COLD;

			/*Require activating artifacts to have a activation type */
			if (a_ptr && (a_ptr->flags & TR_ACTIVATE) && !a_ptr->activate)
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
			/* Acquire the text */
			s = buf + 2;

			/* Find it in the list */
			if (auto power_idx = find_power_idx(s))
			{
				a_ptr->power = *power_idx;
			}
			else
			{
				return 6;
			}

			/* Next... */
			continue;
		}

		/* Hack -- Process 'F' for flags */
		if (buf[0] == 'F')
		{
			if (grab_object_flag(&a_ptr->flags, buf+2))
			{
				return (5);
			}

			/* Next... */
			continue;
		}

		/* Hack -- Process 'f' for obvious flags */
		if (buf[0] == 'f')
		{
			if (grab_object_flag(&a_ptr->oflags, buf+2))
			{
				return (5);
			}

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
	auto &set_info = game->edit_data.set_info;

	int cur_art = 0, cur_num = 0;
	char buf[1024];

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
			/* Find the colon before the name */
			char *s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			int i = atoi(buf + 2);

			/* Verify information */
			if (i < error_idx) return (4);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			set_ptr = &expand_to_fit_index(set_info, i);
			assert(set_ptr->name.empty());

			/* Copy name */
			set_ptr->name = s;

			/* Next... */
			continue;
		}

		/* There better be a current set_ptr */
		if (!set_ptr) return (3);

		/* Process 'D' for "Description" */
		if (buf[0] == 'D')
		{
			/* Need newline? */
			if (!set_ptr->desc.empty())
			{
				set_ptr->desc += '\n';
			}

			/* Append */
			set_ptr->desc += (buf + 2);

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
			if (grab_object_flag(&set_ptr->arts[cur_art].flags[cur_num], buf + 2))
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
	auto &s_descriptors = game->edit_data.s_descriptors;
	auto &s_info = game->s_info;

	int order = 1;
	char buf[1024];

	/* Current entry */
	skill_descriptor *s_ptr = NULL;


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

			s_descriptors[s2].father = s1;
			s_descriptors[s2].order = order++;

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

			// The "exclusive" relation is symmetric, so
			// add summetrically so we don't have to specify
			// twice in data files.
			s_descriptors[s1].excludes.push_back(s2);
			s_descriptors[s2].excludes.push_back(s1);

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

			s_descriptors[s1].increases.emplace_back(
				std::make_tuple(s2, atoi(cval)));

			/* Next... */
			continue;
		}

		/* Process 'N' for "New/Number/Name" */
		if (buf[0] == 'N')
		{
			/* Find the colon before the name */
			char *s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			int i = atoi(buf + 2);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			s_ptr = &expand_to_fit_index(s_descriptors, i);
			assert(s_ptr->name.empty());

			/* Make sure s_info also expands appropriately */
			expand_to_fit_index(s_info, i);

			/* Copy name */
			s_ptr->name = s;

			/* Next... */
			continue;
		}

		/* There better be a current s_ptr */
		if (!s_ptr) return (3);

		/* Process 'D' for "Description" */
		if (buf[0] == 'D')
		{
			/* Need newline? */
			if (!s_ptr->desc.empty())
			{
				s_ptr->desc += '\n';
			}

			/* Append */
			s_ptr->desc += (buf + 2);

			/* Next... */
			continue;
		}

		/* Process 'A' for "Activation Description" (one line only) */
		if (buf[0] == 'A')
		{
			char *txt;

			/* Acquire the text */
			char *s = buf + 2;

			if (NULL == (txt = strchr(s, ':'))) return (1);
			*txt = '\0';
			txt++;

			/* Copy action description */
			assert(s_ptr->action_desc.empty());
			s_ptr->action_desc = txt;

			/* Copy mkey index */
			s_ptr->action_mkey = atoi(s);

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
			if (0 != grab_one_skill_flag(&s_ptr->flags, buf + 2))
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
	auto &ab_info = game->edit_data.ab_info;

	char buf[1024];

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
			char *s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			int i = atoi(buf + 2);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			ab_ptr = &expand_to_fit_index(ab_info, i);
			assert(ab_ptr->name.empty());

			/* Copy name */
			ab_ptr->name = s;

			/* Next... */
			continue;
		}

		/* There better be a current ab_ptr */
		if (!ab_ptr) return (3);

		/* Process 'D' for "Description" */
		if (buf[0] == 'D')
		{
			/* Need newline? */
			if (!ab_ptr->desc.empty())
			{
				ab_ptr->desc += '\n';
			}

			/* Append */
			ab_ptr->desc += (buf + 2);

			/* Next... */
			continue;
		}

		/* Process 'A' for "Activation Description" */
		if (buf[0] == 'A')
		{
			char *txt;

			/* Acquire the text */
			char *s = buf + 2;

			if (NULL == (txt = strchr(s, ':'))) return (1);
			*txt = '\0';
			txt++;

			/* Copy name */
			assert(ab_ptr->action_desc.empty());
			ab_ptr->action_desc = txt;

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

			/* Scan for the values */
			if (NULL == (sec = strchr(buf + 2, ':')))
			{
				return (1);
			}
			*sec = '\0';
			sec++;
			if (!*sec) return (1);

			s16b level = atoi(buf + 2);
			s16b skill = find_skill(sec);
			if (skill == -1) return (1);

			ability_type::skill_requirement req;
			req.skill_idx = skill;
			req.level = level;

			ab_ptr->need_skills.emplace_back(req);

			/* Next... */
			continue;
		}

		/* Process 'a' for "needed ability" */
		if (buf[0] == 'a')
		{
			s16b ab = find_ability(buf + 2);
			if (ab == -1)
			{
				return (1);
			}

			ab_ptr->need_abilities.push_back(ab);

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
				if (equals(stat_names[stat], sec))
					break;
			}

			if (stat == 6) return (1);

			ab_ptr->stat[stat] = atoi(buf + 2);

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
 * Look up ego flag
 */
static ego_flag_set lookup_ego_flag(const char *what)
{
#define ETR(tier, index, name) \
	if (equals(what, #name)) \
	{ \
	        return BOOST_PP_CAT(ETR_,name); \
        };
#include "ego_flag_list.hpp"
#undef ETR
	return ego_flag_set();
}


/*
 * Grab one flag in a ego-item_type from a textual string.
 *
 * We explicitly allow nullptr for the "ego" parameter.
 */
static bool grab_one_ego_item_flag(object_flag_set *flags, ego_flag_set *ego, const char *what)
{
	/* Lookup as an object_flag */
	if (auto f = object_flag_set_from_string(what))
	{
		*flags |= f;
		return 0;
	}

	/* Lookup as ego flag */
	if (ego)
	{
		if (auto f = lookup_ego_flag(what))
		{
			*ego |= f;
			return (0);
		}
	}

	/* Oops */
	msg_format("Unknown ego-item flag '%s'.", what);

	/* Error */
	return (1);
}



/*
 * Initialize the "e_info" array, by parsing an ascii "template" file
 */
errr init_e_info_txt(FILE *fp)
{
	auto &e_info = game->edit_data.e_info;

	int i, cur_r = -1, cur_t = 0;
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

			/* Save the index */
			error_idx = i;

			/* Reset cur_* variables */
			cur_r = -1;
			cur_t = 0;

			/* Point at the "info" */
			e_ptr = &expand_to_fit_index(e_info, i);
			e_ptr->name = s;

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
			e_ptr->before = (pos == 'B');

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
			/* Acquire the text */
			s = buf + 2;

			/* Find it in the list */
			if (auto power_idx = find_power_idx(s))
			{
				e_ptr->power = *power_idx;
			}
			else
			{
				return 6;
			}

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
				if (grab_object_flag(&e_ptr->need_flags, s))
				{
					return (5);
				}

				/* Start the next entry */
				s = t;
			}

			/* Next... */
			continue;
		}

		/* Hack -- Process 'r:F' for forbidden flags */
		if ((buf[0] == 'r') && (buf[2] == 'F'))
		{
			if (grab_object_flag(&e_ptr->forbid_flags, buf + 4))
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
				assert(cur_r < FLAG_RARITY_MAX);
				if (0 != grab_one_ego_item_flag(
				                        &e_ptr->flags[cur_r],
				                        &e_ptr->fego[cur_r],
				                        s))
				{
					return (5);
				}

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
				assert(cur_r < FLAG_RARITY_MAX);
				if (0 != grab_one_ego_item_flag(
				                &e_ptr->oflags[cur_r],
				                nullptr,
				                s))
				{
					return (5);
				}

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
 * Initialize the "ra_info" array, by parsing an ascii "template" file
 */
errr init_ra_info_txt(FILE *fp)
{
	auto &ra_gen = game->edit_data.ra_gen;
	auto &ra_info = game->edit_data.ra_info;

	char buf[1024];

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
			randart_gen_type gen;
			gen.chance = chance;
			gen.dd = dd;
			gen.ds = ds;
			gen.plus = plus;

			/* Add to data */
			ra_gen.emplace_back(gen);

			/* Next... */
			continue;
		}

		/* Process 'N' for "New/Number" */
		if (buf[0] == 'N')
		{
			/* Get the index */
			int i = atoi(buf + 2);

			/* Verify information */
			if (i < error_idx) return (4);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			ra_ptr = &expand_to_fit_index(ra_info, i);

			/* Next... */
			continue;
		}

		/* There better be a current ra_ptr */
		if (!ra_ptr) return (3);

		/* Process 'T' for "Tval/Sval" (up to 5 lines) */
		if (buf[0] == 'T')
		{
			/* Scan for the values */
			int tv, minsv, maxsv;
			if (3 != sscanf(buf + 2, "%d:%d:%d",
			                &tv, &minsv, &maxsv)) return (1);

			/* Set up filter */
			randart_part_type::kind_filter_t filter;
			filter.tval = tv;
			filter.min_sval = minsv;
			filter.max_sval = maxsv;

			/* Add filter */
			ra_ptr->kind_filter.emplace_back(filter);

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
			/* Acquire the text */
			char const *s = buf + 2;

			/* Find it in the list */
			if (auto power_idx = find_power_idx(s))
			{
				ra_ptr->power = *power_idx;
			}
			else
			{
				return 6;
			}

			/* Next... */
			continue;
		}

		/* Process 'F' for flags */
		if (buf[0] == 'F')
		{
			if (0 != grab_one_ego_item_flag(
			                &ra_ptr->flags,
			                &ra_ptr->fego,
			                buf + 2))
			{
				return (5);
			}

			/* Next... */
			continue;
		}

		/* Process 'A' for antagonic flags */
		if (buf[0] == 'A')
		{
			if (0 != grab_one_ego_item_flag(
			                &ra_ptr->aflags,
			                nullptr,
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


static errr grab_monster_race_flag(monster_race_flag_set *flags, const char *what)
{
#define RF(tier, index, name) \
	if (equals(what, #name)) \
	{ \
		*flags |= BOOST_PP_CAT(RF_,name); \
		return 0; \
	};
#include "monster_race_flag_list.hpp"
#undef RF

	/* Oops */
	msg_format("Unknown monster flag '%s'.", what);

	/* Failure */
	return (1);
}


/*
 * Grab one (spell) flag in a monster_race from a textual string
 */
static errr grab_one_monster_spell_flag(monster_spell_flag_set *flags, const char *what)
{
	for (auto const &monster_spell: monster_spells())
	{
		if (equals(what, monster_spell->name))
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
	auto &r_info = game->edit_data.r_info;

	char buf[1024];

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
			char *s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			int i = atoi(buf + 2);

			/* Verify information */
			if (i < error_idx) return (4);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			r_ptr = &expand_to_fit_index(r_info, i);

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
			/* Append to description */
			strappend(&r_ptr->text, buf + 2);

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
			int i, n1, n2;
			char *s;
			char *t;

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
				if (equals(s, r_info_blow_method[n1])) break;
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
				if (equals(s, r_info_blow_effect[n2])) break;
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
			if (0 != grab_monster_race_flag(&r_ptr->flags, buf + 2))
			{
				return (5);
			}

			/* Next... */
			continue;
		}

		/* Process 'S' for "Spell Flags" (multiple lines) */
		if (buf[0] == 'S')
		{
			char const *s = buf + 2;
			int i;

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

	/* Success */
	return (0);
}


/*
 * Initialize the "re_info" array, by parsing an ascii "template" file
 */
errr init_re_info_txt(FILE *fp)
{
	auto &re_info = game->edit_data.re_info;

	char buf[1024];
	byte blow_num = 0;
	int r_char_number = 0;
	int nr_char_number = 0;

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
			char *s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			int i = atoi(buf + 2);

			/* Verify information */
			if (i < error_idx) return (4);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			re_ptr = &expand_to_fit_index(re_info, i);

			/* Copy name */
			assert(!re_ptr->name);
			re_ptr->name = my_strdup(s);

			/* Some inits */
			blow_num = 0;
			r_char_number = 0;
			nr_char_number = 0;

			for (std::size_t j = 0; j < 4; j++)
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
			re_ptr->before = (pos == 'B') ? true : false;

			/* Next... */
			continue;
		}

		/* Process 'B' for "Blows" (up to four lines) */
		if (buf[0] == 'B')
		{
			int n1, n2, dice, side;
			char mdice, mside;
			char *s;
			char *t;

			/* Oops, no more slots */
			if (blow_num == 4) return (1);

			/* Analyze the first field */
			for (s = t = buf + 2; *t && (*t != ':'); t++) /* loop */;

			/* Terminate the field (if necessary) */
			if (*t == ':') *t++ = '\0';

			/* Analyze the method */
			for (n1 = 0; r_info_blow_method[n1]; n1++)
			{
				if (equals(s, r_info_blow_method[n1])) break;
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
				if (equals(s, r_info_blow_effect[n2])) break;
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
			char const *s = buf + 2;

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
				if (0 != grab_monster_race_flag(&re_ptr->flags, s))
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
			char const *s = buf + 2;

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
				if (0 != grab_monster_race_flag(&re_ptr->hflags, s))
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
			if (0 != grab_monster_race_flag(&re_ptr->mflags, buf + 2))
			{
				return (5);
			}

			/* Next... */
			continue;
		}

		/* Process 'O' for "Basic Monster -Flags" (multiple lines) */
		if (buf[0] == 'O')
		{
			char const *s = buf + 2;

			/* XXX XXX XXX Hack -- Read no flags */
			if (equals(s, "MF_ALL"))
			{
				re_ptr->nflags = ~monster_race_flag_set();
			}

			/* Parse this entry */
			else {
				if (0 != grab_monster_race_flag(&re_ptr->nflags, s))
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
			char const *s = buf + 2;
			int i;

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
			for (char *s = buf + 2; *s; )
			{
				char *t;

				/* Find the end of this entry */
				for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) /* loop */;

				/* Nuke and skip any dividers */
				if (*t)
				{
					*t++ = '\0';
					while ((*t == ' ') || (*t == '|')) t++;
				}

				/* XXX XXX XXX Hack -- Read no flags */
				if (equals(s, "MF_ALL"))
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
 * Grab one flag for a dungeon type from a textual string
 */
errr grab_one_dungeon_flag(dungeon_flag_set *flags, const char *str)
{
#define DF(tier, index, name) \
	if (equals(str, #name)) { *flags |= DF_##name; return 0; }
#include "dungeon_flag_list.hpp"
#undef DF

	/* Oops */
	msg_format("Unknown dungeon type flag '%s'.", str);

	/* Failure */
	return (1);
}


/*
 * Post-process "d_info" array.
 */
static void post_d_info()
{
	auto &d_info = game->edit_data.d_info;

	for (std::size_t parent_d_idx = 0; parent_d_idx < d_info.size(); parent_d_idx++)
	{
		auto parent_d_ptr = &d_info[parent_d_idx];

		if (parent_d_ptr->name.empty())
		{
			continue;
		}

		// Go through all the references to side dungeons
		// and fill in back-references to "parent" dungeons.
		for (auto &parent_depth_and_level_data: parent_d_ptr->level_data_by_depth)
		{
			auto parent_depth = std::get<0>(parent_depth_and_level_data);
			auto &parent_level_data = std::get<1>(parent_depth_and_level_data);

			// Do we have a branch-off point at this level?
			if (parent_level_data.branch > 0)
			{
				// Look up target dungeon.
				auto child_d_ptr = &d_info[parent_level_data.branch];
				auto &child_level_data = child_d_ptr->level_data_by_depth[child_d_ptr->mindepth];

				// Set up back-reference.
				child_level_data.fbranch = parent_d_idx;
				child_level_data.flevel = parent_depth - parent_d_ptr->mindepth;
			}
		}
	}
}

/*
 * Initialize the "d_info" array, by parsing an ascii "template" file
 */
errr init_d_info_txt(FILE *fp)
{
	auto &d_info = game->edit_data.d_info;

	char buf[1024];

	s16b rule_num = 0;

	byte r_char_number = 0;

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
			char *s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			int i = atoi(buf + 2);

			/* Verify information */
			if (i < error_idx) return (4);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			d_ptr = &expand_to_fit_index(d_info, i);
			assert(d_ptr->name.empty());

			/* Copy name */
			d_ptr->name = s;

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
			for (std::size_t j = 0; j < 5; j++)
			{
				d_ptr->rules[j].mode = DUNGEON_MODE_NONE;
				d_ptr->rules[j].percent = 0;

				for (std::size_t k = 0; k < 5; k++)
				{
					d_ptr->rules[j].r_char[k] = 0;
				}
			}

			/* Set default drop theme */
			d_ptr->objs = obj_theme::defaults();

			/* The default generator */
			d_ptr->generator = "dungeon";

			/* Next... */
			continue;
		}

		/* There better be a current d_ptr */
		if (!d_ptr) return (3);

		/* Process 'D' for "Description */
		if (buf[0] == 'D')
		{
			/* Acquire short name */
			d_ptr->short_name += buf[2];
			d_ptr->short_name += buf[3];
			d_ptr->short_name += buf[4];

			/* Append to description */
			d_ptr->text += (buf + 6);

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
			d_ptr->generator = (buf + 2);

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
			const char *tmp;

			/* Find the next empty blow slot (if any) */
			std::size_t i;
			for (i = 0; i < 4; i++)
			{
				if ((!d_ptr->d_side[i]) &&
					(!d_ptr->d_dice[i]))
				{
					break;
				}
			}

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
					if (equals(d_info_dtypes[j].name, tmp))
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

			char const *s = buf + 2;

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
			char const *s = buf + 2;

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
				if (0 != grab_monster_race_flag(&d_ptr->rules[rule_num].mflags, s))
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
			char const *s = buf + 2;

			/* Parse this entry */
			if (0 != grab_one_monster_spell_flag(&d_ptr->rules[rule_num].mspells, s))
			{
				return (5);
			}

			/* Next... */
			continue;
		}

		/* Process '@' for level-dependent information */
		if (buf[0] == '@')
		{
			/* Split into fields */
			std::string buf_s(buf + 2);
			std::vector<std::string> fields;
			boost::algorithm::split(fields, buf_s, boost::is_any_of(":"));

			// Get the depth; depths are relative to the first floor
			// of the dungeon. This is consistent with the handling of
			// the 'A' flag.
			int const depth = std::stoi(fields[0]);
			auto &level_data = d_ptr->level_data_by_depth[depth + d_ptr->mindepth];

			// Parse into appropriate field.
			if (fields[1] == "B")
			{
				level_data.branch = std::stoi(fields[2]);
			}
			else if (fields[1] == "F")
			{
				if (0 != grab_one_dungeon_flag(&level_data.flags, fields[2].c_str()))
				{
					return 5;
				}
			}
			else if (fields[1] == "S")
			{
				level_data.save_extension = fields[2];
			}
			else if (fields[1] == "U")
			{
				level_data.map_name = fields[2];
			}
			else if (fields[1] == "N")
			{
				level_data.name = fields[2];
			}
			else if (fields[1] == "D")
			{
				level_data.description = fields[2];
			}
			else
			{
				return 1;
			}

			/* Next... */
			continue;
		}

		/* Oops */
		return (6);
	}

	/* Post-process */
	post_d_info();

	/* Success */
	return (0);
}

/*
 * Grab one race flag from a textual string
 */
static errr grab_one_race_flag(owner_type *ow_ptr, int state, const char *what)
{
	/* Scan race flags */
	unknown_shut_up = true;
	if (!grab_one_race_allow_flag(ow_ptr->races[state], what))
	{
		unknown_shut_up = false;
		return (0);
	}

	/* Scan classes flags */
	if (!grab_one_class_flag(ow_ptr->classes[state], what))
	{
		unknown_shut_up = false;
		return (0);
	}

	/* Oops */
	unknown_shut_up = false;
	msg_format("Unknown race/class flag '%s'.", what);

	/* Failure */
	return (1);
}

/*
 * Grab one store flag from a textual string
 */
static errr grab_one_store_flag(store_flag_set *flags, const char *what)
{
#define STF(tier, index, name) \
	if (equals(what, #name)) { \
	        *flags |= BOOST_PP_CAT(STF_,name); \
	        return 0; \
        }
#include "store_flag_list.hpp"
#undef STF

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
	auto &st_info = game->edit_data.st_info;

	char buf[1024];

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
			char *s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			int i = atoi(buf + 2);

			/* Verify information */
			if (i < error_idx) return (4);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			st_ptr = &expand_to_fit_index(st_info, i);
			assert(st_ptr->name.empty());

			/* Copy name */
			st_ptr->name = s;

			/* Next... */
			continue;
		}

		/* There better be a current st_ptr */
		if (!st_ptr) return (3);

		/* Process 'I' for "Items" (multiple lines) */
		if (buf[0] == 'I')
		{
			/* Find the colon before the name */
			char *s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Add to items array */
			auto chance = atoi(buf + 2);
			int k_idx = test_item_name(s);

			if (k_idx < 0)
			{
				msg_format("Unknown k_info entry: [%s].", s);
				return 1;
			}

			st_ptr->items.emplace_back(
				store_item::k_idx(k_idx, chance));

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

			/* Add to the items array */
			store_item item = (sv1 < 256)
				? store_item::k_idx(lookup_kind(tv1, sv1), rar1)
				: store_item::tval(tv1, rar1);    /* An SVAL of 256 means all possible items. */

			st_ptr->items.emplace_back(item);

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
			st_ptr->actions.push_back(a1);
			st_ptr->actions.push_back(a2);
			st_ptr->actions.push_back(a3);
			st_ptr->actions.push_back(a4);
			st_ptr->actions.push_back(a5);
			st_ptr->actions.push_back(a6);

			/* Remove zero entries since they have no effect */
			st_ptr->actions.erase(
			        std::remove(st_ptr->actions.begin(), st_ptr->actions.end(), 0),
			        st_ptr->actions.end()
			);

			/* Next... */
			continue;
		}

		/* Process 'F' for "store Flags" (multiple lines) */
		if (buf[0] == 'F')
		{
			if (0 != grab_one_store_flag(&st_ptr->flags, buf + 2))
			{
				return (5);
			}

			/* Next... */
			continue;
		}

		/* Process 'O' for "Owners" (one line only) */
		if (buf[0] == 'O')
		{
			int a1, a2, a3, a4;

			/* Scan for the values */
			if (4 != sscanf(buf + 2, "%d:%d:%d:%d",
			                &a1, &a2, &a3, &a4))
			{
				return 1;
			}

			/* Get a reference to the owners */
			auto owners = &st_ptr->owners;

			/* Save the values */
			owners->push_back(a1);
			owners->push_back(a2);
			owners->push_back(a3);
			owners->push_back(a4);

			/* Sort and remove duplicates */
			std::sort(owners->begin(), owners->end());
			owners->erase(std::unique(owners->begin(), owners->end()), owners->end());

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
	auto &ba_info = game->edit_data.ba_info;

	char buf[1024];

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
			char *s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			int i = atoi(buf + 2);

			/* Verify information */
			if (i < error_idx) return (4);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			ba_ptr = &expand_to_fit_index(ba_info, i);

			/* Copy name */
			ba_ptr->name = s;

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
	auto &ow_info = game->edit_data.ow_info;

	char buf[1024];

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
			char *s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			int i = atoi(buf + 2);

			/* Verify information */
			if (i < error_idx) return (4);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			ow_ptr = &expand_to_fit_index(ow_info, i);

			/* Copy name */
			ow_ptr->name = s;

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
	auto &wf_info = game->edit_data.wf_info;

	char buf[1024];

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
			char *s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return (1);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return (1);

			/* Get the index */
			int i = atoi(buf + 2);

			/* Verify information */
			if (i < error_idx) return (4);

			/* Save the index */
			error_idx = i;

			/* Point at the "info" */
			wf_ptr = &expand_to_fit_index(wf_info, i);

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
			char *s = buf + 2;

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


typedef struct dungeon_grid dungeon_grid;

struct dungeon_grid
{
	int	feature; 		/* Terrain feature */
	int	monster; 		/* Monster */
	int	object; 			/* Object */
	int	ego; 			/* Ego-Item */
	int	artifact; 		/* Artifact */
	int	cave_info; 		/* Flags for CAVE_MARK, CAVE_GLOW, CAVE_ICKY, CAVE_ROOM */
	int	special; 		/* Reserved for special terrain info */
	int	random; 			/* Number of the random effect */
	int bx, by;                  /* For between gates */
	int mimic;                   /* Mimiced features */
	s32b mflag;			/* monster's mflag */
	bool ok;
	bool defined;
};
static bool meta_sleep = true;

static dungeon_grid letter[255];

/*
 * Parse a sub-file of the "extra info"
 */
static errr process_dungeon_file_aux(char *buf, int *yval, int *xval, int xvalstart, int ymax, int xmax, bool full)
{
	auto &wilderness = game->wilderness;
	auto &wf_info = game->edit_data.wf_info;
	auto &a_info = game->edit_data.a_info;
	auto &k_info = game->edit_data.k_info;
	auto &dungeon_flags = game->dungeon_flags;

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
		return (process_dungeon_file(buf + 2, yval, xval, ymax, xmax, false, full));
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

	/* Process "F:<letter>:<terrain>:<cave_info>:<monster>:<object>:<ego>:<artifact>:<special>:<mimic>:<mflag>" -- info for dungeon grid */
	if (buf[0] == 'F')
	{
		int num;

		if ((num = tokenize(buf + 2, 10, zz, ':', '/')) > 1)
		{
			int index = zz[0][0];

			/* Reset the feature */
			letter[index].feature = 0;
			letter[index].monster = 0;
			letter[index].object = 0;
			letter[index].ego = 0;
			letter[index].artifact = 0;
			letter[index].cave_info = 0;
			letter[index].special = 0;
			letter[index].random = 0;
			letter[index].mimic = 0;
			letter[index].mflag = 0;
			letter[index].ok = true;
			letter[index].defined = true;

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
				char *field = zz[7];
				/* Quests can be defined by name only */
				if (field[0] == '"')
				{
					/* Hunt & shoot the ending " */
					int i = strlen(field) - 1;
					if (field[i] == '"') field[i] = '\0';
					letter[index].special = 0;
					for (i = 0; i < MAX_Q_IDX; i++)
					{
						if (equals(&field[1], quest[i].name))
						{
							letter[index].special = i;
							break;
						}
					}
				}
				else
					letter[index].special = atoi(field);
			}

			if (num > 8)
			{
				letter[index].mimic = atoi(zz[8]);
			}

			if (num > 9)
			{
				letter[index].mflag = atoi(zz[9]);
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

				m_idx = place_monster(y, x, meta_sleep, false);

				monster_level = level;
			}
			else if (monster_index)
			{
				/* Place it */
				m_allow_special[monster_index] = true;
				m_idx = place_monster_aux(y, x, monster_index, meta_sleep, false, MSTATUS_ENEMY);
				m_allow_special[monster_index] = false;
			}

			/* Set the mflag of the monster */
			if (m_idx) m_list[m_idx].mflag |= letter[idx].mflag;

			/* Object (and possible trap) */
			if (random & RANDOM_OBJECT)
			{
				/* Create an out of deep object */
				if (object_index)
				{
					int level = object_level;

					object_level = quest[p_ptr->inside_quest].level + object_index;
					if (rand_int(100) < 75)
						place_object(y, x, false, false, OBJ_FOUND_SPECIAL);
					else if (rand_int(100) < 80)
						place_object(y, x, true, false, OBJ_FOUND_SPECIAL);
					else
						place_object(y, x, true, true, OBJ_FOUND_SPECIAL);

					object_level = level;
				}
				else if (rand_int(100) < 75)
				{
					place_object(y, x, false, false, OBJ_FOUND_SPECIAL);
				}
				else if (rand_int(100) < 80)
				{
					place_object(y, x, true, false, OBJ_FOUND_SPECIAL);
				}
				else
				{
					place_object(y, x, true, true, OBJ_FOUND_SPECIAL);
				}
			}
			else if (object_index)
			{
				/* Get local object */
				object_type *o_ptr = &object_type_body;

				k_info[object_index]->allow_special = true;

				/* Create the item */
				object_prep(o_ptr, object_index);

				/* Apply magic (no messages, no artifacts) */
				apply_magic(o_ptr, dun_level, false, true, false);

				o_ptr->found = OBJ_FOUND_SPECIAL;

				k_info[object_index]->allow_special = false;

				drop_near(o_ptr, -1, y, x);
			}

			/* Artifact */
			if (artifact_index)
			{
				int I_kind = 0;

				auto a_ptr = &a_info[artifact_index];

				/* Get local object */
				object_type forge;
				object_type *q_ptr = &forge;

				a_allow_special[artifact_index] = true;

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

				a_allow_special[artifact_index] = false;

				/* It's amazing that this "creating objects anywhere"
				   junk ever worked.
				   Let's just HACK around one observed bug: Shadow Cloak
				   of Luthien [Globe of Light] */
				{
					auto const flags = object_flags(q_ptr);
					if (flags & TR_SPELL_CONTAIN)
					{
						q_ptr->pval2 = -1;
					}
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
			/* Acquire the text */
			char *s = buf + 4;

			int y = *yval;

			for (std::size_t x = 0; x < wilderness.width(); x++)
			{
				char i;
				if (1 != sscanf(s + x, "%c", &i))
				{
					return (1);
				}

				auto const wi = wildc2i[(int)i];

				wilderness(x, y).feat = wi;

				/*
				 * If this is a town/dungeon entrance, note
				 * its coordinates.  (Have to check for
				 * duplicate Morias...)
				 */
				if (wf_info[wi].entrance && wf_info[wi].wild_x == 0)
				{
					wf_info[wi].wild_x = x;
					wf_info[wi].wild_y = y;
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
				int y = atoi(zz[1]);
				int x = atoi(zz[2]);
				wilderness(x, y).entrance = 1000 + atoi(zz[0]);
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

			/* Maximum o_idx */
			else if (zz[0][0] == 'O')
			{
				max_o_idx = atoi(zz[1]);
			}

			/* Maximum m_idx */
			else if (zz[0][0] == 'M')
			{
				max_m_idx = atoi(zz[1]);
			}

			/* Maximum wilderness x size */
			else if (zz[0][0] == 'X')
			{
				wilderness.width(atoi(zz[1]));
			}

			/* Maximum wilderness y size */
			else if (zz[0][0] == 'Y')
			{
				wilderness.height(atoi(zz[1]));
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
static const char *process_dungeon_file_expr(char **sp, char *fp)
{
	static char pref_tmp_value[8];
	const char *v;

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
		else if (equals(t, "IOR"))
		{
			v = "0";
			while (*s && (f != b2))
			{
				t = process_dungeon_file_expr(&s, &f);
				if (*t && !equals(t, "0")) v = "1";
			}
		}

		/* Function: AND */
		else if (equals(t, "AND"))
		{
			v = "1";
			while (*s && (f != b2))
			{
				t = process_dungeon_file_expr(&s, &f);
				if (*t && equals(t, "0")) v = "0";
			}
		}

		/* Function: NOT */
		else if (equals(t, "NOT"))
		{
			v = "1";
			while (*s && (f != b2))
			{
				t = process_dungeon_file_expr(&s, &f);
				if (*t && equals(t, "1")) v = "0";
			}
		}

		/* Function: EQU */
		else if (equals(t, "EQU"))
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
				if (*t && !equals(p, t)) v = "0";
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
		bool text_mode = false;

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
			/* Town */
			if (equals(b + 1, "TOWN"))
			{
				strnfmt(pref_tmp_value, 8, "%d", p_ptr->town_num);
				v = pref_tmp_value;
			}

			/* Town destroyed */
			else if (starts_with(b + 1, "TOWN_DESTROY"))
			{
				strnfmt(pref_tmp_value, 8, "%d", town_info[atoi(b + 13)].destroyed);
				v = pref_tmp_value;
			}

			/* Number of last quest */
			else if (equals(b + 1, "LEAVING_QUEST"))
			{
				strnfmt(pref_tmp_value, 8, "%d", leaving_quest);
				v = pref_tmp_value;
			}

			/* DAYTIME status */
			else if (starts_with(b + 1, "DAYTIME"))
			{
				if ((bst(HOUR, turn) >= 6) && (bst(HOUR, turn) < 18))
					v = "1";
				else
					v = "0";
			}

			/* Quest status */
			else if (starts_with(b + 1, "QUEST"))
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
						if (equals(c, quest[i].name))
						{
							strnfmt(pref_tmp_value, 8, "%d", quest[i].status);
							break;
						}
					}
				}
				v = pref_tmp_value;
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


errr process_dungeon_file(const char *name, int *yval, int *xval, int ymax, int xmax, bool init, bool full)
{
	FILE *fp = 0;

	char buf[1024];

	int num = -1, i;

	errr err = 0;

	bool bypass = false;

	/* Save the start since it ought to be modified */
	int xmin = *xval;

	if (init)
	{
		meta_sleep = true;
		for (i = 0; i < 255; i++)
		{
			letter[i].defined = false;
			if (i == ' ') letter[i].ok = true;
			else letter[i].ok = false;
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
			const char *v;
			char *s;

			/* Start */
			s = buf + 2;

			/* Parse the expr */
			v = process_dungeon_file_expr(&s, &f);

			/* Set flag */
			bypass = (equals(v, "0") ? true : false);

			/* Continue */
			continue;
		}

		/* Apply conditionals */
		if (bypass) continue;


		/* Process "%:<file>" */
		if (buf[0] == '%')
		{
			/* Process that file if allowed */
			process_dungeon_file(buf + 2, yval, xval, ymax, xmax, false, full);

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
