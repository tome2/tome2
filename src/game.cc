#include "game.hpp"

#include <stats.hpp>

Game *game;

/**
 * Default constructor
 */
Game::Game()
{
	// Initialize the available player powers
	powers = std::unordered_map<int, std::shared_ptr<power_type>> {
		{
			PWR_SPIT_ACID,
			std::make_shared<power_type>(
				"spit acid",
				"You can spit acid.",
				"You gain the ability to spit acid.",
				"You lose the ability to spit acid.",
				power_activation {
					9, 9, A_DEX, 15
				}
			),
		},
		{
			PWR_BR_FIRE,
			std::make_shared<power_type>(
				"fire breath",
				"You can breath fire.",
				"You gain the ability to breathe fire.",
				"You lose the ability to breathe fire.",
				power_activation {
					20, 10, A_CON, 18
				}
			),
		},
		{
			PWR_HYPN_GAZE,
			std::make_shared<power_type>(
				"hypnotic gaze",
				"Your gaze is hypnotic.",
				"Your eyes look mesmerising...",
				"Your eyes look uninteresting.",
				power_activation {
					12, 12, A_CHR, 18
				}
			),
		},
		{
			PWR_TELEKINES,
			std::make_shared<power_type>(
				"telekinesis",
				"You are telekinetic.",
				"You gain the ability to move objects telekinetically.",
				"You lose the ability to move objects telekinetically.",
				power_activation {
					9, 9, A_WIS, 14
				}
			),
		},
		{
			PWR_VTELEPORT,
			std::make_shared<power_type>(
				"teleport",
				"You can teleport at will.",
				"You gain the power of teleportation at will.",
				"You lose the power of teleportation at will.",
				power_activation {
					7, 7, A_WIS, 15
				}
			),
		},
		{
			PWR_MIND_BLST,
			std::make_shared<power_type>(
				"mind blast",
				"You can mind blast your enemies.",
				"You gain the power of Mind Blast.",
				"You lose the power of Mind Blast.",
				power_activation {
					5, 3, A_WIS, 15
				}
			),
		},
		{
			PWR_RADIATION,
			std::make_shared<power_type>(
				"emit radiation",
				"You can emit hard radiation at will.",
				"You start emitting hard radiation.",
				"You stop emitting hard radiation.",
				power_activation {
					15, 15, A_CON, 14
				}
			),
		},
		{
			PWR_VAMPIRISM,
			std::make_shared<power_type>(
				"vampiric drain",
				"You can drain life from a foe.",
				"You become vampiric.",
				"You are no longer vampiric.",
				power_activation {
					4, 5, A_CON, 9
				}
			),
		},
		{
			PWR_SMELL_MET,
			std::make_shared<power_type>(
				"smell metal",
				"You can smell nearby precious metal.",
				"You smell a metallic odour.",
				"You no longer smell a metallic odour.",
				power_activation {
					3, 2, A_INT, 12
				}
			),
		},
		{
			PWR_SMELL_MON,
			std::make_shared<power_type>(
				"smell monsters",
				"You can smell nearby monsters.",
				"You smell filthy monsters.",
				"You no longer smell filthy monsters.",
				power_activation {
					5, 4, A_INT, 15
				}
			),
		},
		{
			PWR_BLINK,
			std::make_shared<power_type>(
				"blink",
				"You can teleport yourself short distances.",
				"You gain the power of minor teleportation.",
				"You lose the power of minor teleportation.",
				power_activation {
					3, 3, A_WIS, 12
				}
			),
		},
		{
			PWR_EAT_ROCK,
			std::make_shared<power_type>(
				"eat rock",
				"You can consume solid rock.",
				"The walls look delicious.",
				"The walls look unappetising.",
				power_activation {
					8, 12, A_CON, 18
				}
			),
		},
		{
			PWR_SWAP_POS,
			std::make_shared<power_type>(
				"swap position",
				"You can switch locations with another being.",
				"You feel like walking a mile in someone else's shoes.",
				"You feel like staying in your own shoes.",
				power_activation {
					15, 12, A_DEX, 16
				}
			),
		},
		{
			PWR_SHRIEK,
			std::make_shared<power_type>(
				"shriek",
				"You can emit a horrible shriek.",
				"Your vocal cords get much tougher.",
				"Your vocal cords get much weaker.",
				power_activation {
					4, 4, A_CON, 6
				}
			),
		},
		{
			PWR_ILLUMINE,
			std::make_shared<power_type>(
				"illuminate",
				"You can emit bright light.",
				"You can light up rooms with your presence.",
				"You can no longer light up rooms with your presence.",
				power_activation {
					3, 2, A_INT, 10
				}
			),
		},
		{
			PWR_DET_CURSE,
			std::make_shared<power_type>(
				"detect curses",
				"You can feel the danger of evil magic.",
				"You can feel evil magic.",
				"You can no longer feel evil magic.",
				power_activation {
					7, 14, A_WIS, 14
				}
			),
		},
		{
			PWR_BERSERK,
			std::make_shared<power_type>(
				"berserk",
				"You can drive yourself into a berserk frenzy.",
				"You feel a controlled rage.",
				"You no longer feel a controlled rage.",
				power_activation {
					8, 8, A_STR, 14
				}
			),
		},
		{
			PWR_POLYMORPH,
			std::make_shared<power_type>(
				"polymorph",
				"You can polymorph yourself at will.",
				"Your body seems mutable.",
				"Your body seems stable.",
				power_activation {
					18, 20, A_CON, 18
				}
			),
		},
		{
			PWR_MIDAS_TCH,
			std::make_shared<power_type>(
				"Midas touch",
				"You can turn ordinary items to gold.",
				"You gain the Midas touch.",
				"You lose the Midas touch.",
				power_activation {
					10, 5, A_INT, 12
				}
			),
		},
		{
			PWR_GROW_MOLD,
			std::make_shared<power_type>(
				"grow mold",
				"You can cause mold to grow near you.",
				"You feel a sudden affinity for mold.",
				"You feel a sudden dislike for mold.",
				power_activation {
					1, 6, A_CON, 14
				}
			),
		},
		{
			PWR_RESIST,
			std::make_shared<power_type>(
				"resist elements",
				"You can harden yourself to the ravages of the elements.",
				"You feel like you can protect yourself.",
				"You feel like you might be vulnerable.",
				power_activation {
					10, 12, A_CON, 12
				}
			),
		},
		{
			PWR_EARTHQUAKE,
			std::make_shared<power_type>(
				"earthquake",
				"You can bring down the dungeon around your ears.",
				"You gain the ability to wreck the dungeon.",
				"You lose the ability to wreck the dungeon.",
				power_activation {
					12, 12, A_STR, 16
				}
			),
		},
		{
			PWR_EAT_MAGIC,
			std::make_shared<power_type>(
				"eat magic",
				"You can consume magic energy for your own use.",
				"Your magic items look delicious.",
				"Your magic items no longer look delicious.",
				power_activation {
					17, 1, A_WIS, 15
				}
			),
		},
		{
			PWR_WEIGH_MAG,
			std::make_shared<power_type>(
				"weigh magic",
				"You can feel the strength of the magics affecting you.",
				"You feel you can better understand the magic around you.",
				"You no longer sense magic.",
				power_activation {
					6, 6, A_INT, 10
				}
			),
		},
		{
			PWR_STERILITY,
			std::make_shared<power_type>(
				"sterilise",
				"You can cause mass impotence.",
				"You can give everything around you a headache.",
				"You hear a massed sigh of relief.",
				power_activation {
					20, 40, A_CHR, 18
				}
			),
		},
		{
			PWR_PANIC_HIT,
			std::make_shared<power_type>(
				"panic hit",
				"You can run for your life after hitting something.",
				"You suddenly understand how thieves feel.",
				"You no longer feel jumpy.",
				power_activation {
					10, 12, A_DEX, 14
				}
			),
		},
		{
			PWR_DAZZLE,
			std::make_shared<power_type>(
				"dazzle",
				"You can emit confusing, blinding radiation.",
				"You gain the ability to emit dazzling lights.",
				"You lose the ability to emit dazzling lights.",
				power_activation {
					7, 15, A_CHR, 8
				}
			),
		},
		{
			PWR_DARKRAY,
			std::make_shared<power_type>(
				"spear of darkness",
				"You can create a spear of darkness.",
				"An illusory spear of darkness appears in your hand.",
				"The spear of darkness disappear.",
				power_activation {
					7, 10, A_WIS, 9
				}
			),
		},
		{
			PWR_RECALL,
			std::make_shared<power_type>(
				"recall",
				"You can travel between towns and the depths.",
				"You feel briefly homesick, but it passes.",
				"You feel briefly homesick.",
				power_activation {
					17, 50, A_INT, 16
				}
			),
		},
		{
			PWR_BANISH,
			std::make_shared<power_type>(
				"banish evil",
				"You can send evil creatures directly to the Nether Realm.",
				"You feel a holy wrath fill you.",
				"You no longer feel a holy wrath.",
				power_activation {
					25, 25, A_WIS, 18
				}
			),
		},
		{
			PWR_COLD_TOUCH,
			std::make_shared<power_type>(
				"cold touch",
				"You can freeze things with a touch.",
				"Your hands get very cold.",
				"Your hands warm up.",
				power_activation {
					2, 2, A_CON, 11
				}
			),
		},
		{
			PWR_LAUNCHER,
			std::make_shared<power_type>(
				"throw object",
				"You can hurl objects with great force.",
				"Your throwing arm feels much stronger.",
				"Your throwing arm feels much weaker.",
				power_activation {
					1, 10, A_STR, 6
				}
			),
		},
		{
			PWR_PASSWALL,
			std::make_shared<power_type>(
				"find secret passages",
				"You can use secret passages.",
				"You suddenly notice lots of hidden ways.",
				"You no longer can use hidden ways.",
				power_activation {
					15, 15, A_DEX, 12
				}
			),
		},
		{
			PWR_DETECT_TD,
			std::make_shared<power_type>(
				"detect doors and traps",
				"You can detect hidden doors and traps.",
				"You develop an affinity for traps.",
				"You no longer can detect hidden doors and traps.",
				power_activation {
					5, 3, A_WIS, 10
				}
			),
		},
		{
			PWR_COOK_FOOD,
			std::make_shared<power_type>(
				"create food",
				"You can create food.",
				"Your cooking skills greatly improve.",
				"Your cooking skills return to a normal level.",
				power_activation {
					15, 10, A_INT, 10
				}
			),
		},
		{
			PWR_UNFEAR,
			std::make_shared<power_type>(
				"remove fear",
				"You can embolden yourself.",
				"You feel your fears lessening.",
				"You feel your fears growing again.",
				power_activation {
					3, 5, A_WIS, 8
				}
			),
		},
		{
			PWR_EXPL_RUNE,
			std::make_shared<power_type>(
				"set explosive rune",
				"You can set explosive runes.",
				"You suddenly understand how explosive runes work.",
				"You suddenly forget how explosive runes work.",
				power_activation {
					25, 35, A_INT, 15
				}
			),
		},
		{
			PWR_STM,
			std::make_shared<power_type>(
				"stone to mud",
				"You can destroy walls.",
				"You can destroy walls.",
				"You cannot destroy walls anymore.",
				power_activation {
					20, 10, A_STR, 12
				}
			),
		},
		{
			PWR_POIS_DART,
			std::make_shared<power_type>(
				"poison dart",
				"You can throw poisoned darts.",
				"You get an infinite supply of poisoned darts.",
				"You lose your infinite supply of poisoned darts.",
				power_activation {
					12, 8, A_DEX, 14
				}
			),
		},
		{
			PWR_MAGIC_MISSILE,
			std::make_shared<power_type>(
				"magic missile",
				"You can cast magic missiles.",
				"You suddenly understand the basics of magic.",
				"You forget the basics of magic.",
				power_activation {
					2, 2, A_INT, 9
				}
			),
		},
		{
			PWR_GROW_TREE,
			std::make_shared<power_type>(
				"grow trees",
				"You can grow trees.",
				"You feel an affinity for trees.",
				"You no longer feel an affinity for trees.",
				power_activation {
					2, 6, A_CHR, 3
				}
			),
		},
		{
			PWR_BR_COLD,
			std::make_shared<power_type>(
				"cold breath",
				"You can breath cold.",
				"You gain the ability to breathe cold.",
				"You lose the ability to breathe cold.",
				power_activation {
					20, 10, A_CON, 18
				}
			),
		},
		{
			PWR_BR_CHAOS,
			std::make_shared<power_type>(
				"chaos breath",
				"You can breath chaos.",
				"You gain the ability to breathe chaos.",
				"You lose the ability to breathe chaos.",
				power_activation {
					20, 10, A_CON, 18
				}
			),
		},
		{
			PWR_BR_ELEM,
			std::make_shared<power_type>(
				"elemental breath",
				"You can breath the elements.",
				"You gain the ability to breathe the elements.",
				"You lose the ability to breathe the elements.",
				power_activation {
					20, 10, A_CON, 18
				}
			),
		},
		{
			PWR_WRECK_WORLD,
			std::make_shared<power_type>(
				"change the world",
				"You can wreck the world around you.",
				"You gain the ability to wreck the world.",
				"You lose the ability to wreck the world.",
				power_activation {
					1, 30, A_CHR, 6
				}
			),
		},
		{
			PWR_SCARE,
			std::make_shared<power_type>(
				"scare monster",
				"You can scare monsters.",
				"You gain the ability to scare monsters.",
				"You lose the ability to scare monsters.",
				power_activation {
					4, 3, A_INT, 3
				}
			),
		},
		{
			PWR_REST_LIFE,
			std::make_shared<power_type>(
				"restore life",
				"You can restore lost life forces.",
				"You gain the ability to restore your life force.",
				"You lose the ability to restore your life force.",
				power_activation {
					30, 30, A_WIS, 18
				}
			),
		},
		{
			PWR_SUMMON_MONSTER,
			std::make_shared<power_type>(
				"summon monsters",
				"You can call upon monsters.",
				"You gain the ability to call upon monsters.",
				"You lose the ability to call upon monsters.",
				power_activation {
					0, 0, 0, 0
				}
			),
		},
		{
			PWR_NECRO,
			std::make_shared<power_type>(
				"necromantic powers",
				"You can use the foul necromantic magic.",
				"You gain the ability to use the foul necromantic magic.",
				"You lose the ability to use the foul necromantic magic.",
				power_activation {
					0, 0, 0, 0
				}
			),
		},
		{
			PWR_ROHAN,
			std::make_shared<power_type>(
				"Rohan Knight's Powers",
				"You can use rohir powers.",
				"You gain the ability to use rohir powers.",
				"You lose the ability to use rohir powers.",
				power_activation {
					0, 0, 0, 0
				}
			),
		},
		{
			PWR_THUNDER,
			std::make_shared<power_type>(
				"Thunderlord's Powers",
				"You can use thunderlords powers.",
				"You gain the ability to use thunderlords powers.",
				"You lose the ability to use thunderlords powers.",
				power_activation {
					0, 0, 0, 0
				}
			),
		},
		{
			PWR_DEATHMOLD,
			std::make_shared<power_type>(
				"Death Mold's Powers",
				"You can use the foul deathmold magic.",
				"You gain the ability to use the foul deathmold magic.",
				"You lose the ability to use the foul deathmold magic.",
				power_activation {
					0, 0, 0, 0
				}
			),
		},
		{
			PWR_HYPNO,
			std::make_shared<power_type>(
				"Hypnotise Pet",
				"You can mystify pets.",
				"You gain the ability to mystify pets.",
				"You lose the ability to mystify pets.",
				power_activation {
					0, 0, 0, 0
				}
			),
		},
		{
			PWR_UNHYPNO,
			std::make_shared<power_type>(
				"Awaken Hypnotised Pet",
				"You can wake up a pet.",
				"You gain the ability to wake up a pet.",
				"You lose the ability to wake up a pet.",
				power_activation {
					0, 0, 0, 0
				}
			),
		},
		{
			PWR_INCARNATE,
			std::make_shared<power_type>(
				"Incarnate",
				"You can incarnate into a body.",
				"You feel the need to get a body.",
				"You no longer feel the need for a new body.",
				power_activation {
					0, 0, 0, 0
				}
			),
		},
		{
			PWR_MAGIC_MAP,
			std::make_shared<power_type>(
				"magic map",
				"You can sense what is beyond walls.",
				"You feel you can sense what is beyond walls.",
				"You no longer can sense what is beyond walls.",
				power_activation {
					7, 10, A_WIS, 15
				}
			),
		},
		{
			PWR_COMPANION,
			std::make_shared<power_type>(
				"turn pet into companion",
				"You can turn a pet into a companion.",
				"You suddenly gain authority over your pets.",
				"You can no longer convert pets into companions.",
				power_activation {
					2, 10, A_CHR, 10
				}
			),
		},
		{
			PWR_BEAR,
			std::make_shared<power_type>(
				"turn into a bear",
				"You can turn into a bear.",
				"You suddenly gain beorning powers.",
				"You can no longer shapeshift into a bear.",
				power_activation {
					2, 5, A_CON, 5
				}
			),
		},
		{
			PWR_DODGE,
			std::make_shared<power_type>(
				"sense dodge success",
				"You can sense your dodging success chance.",
				"You suddenly can sense your dodging success chance.",
				"You can no longer sense your dodging success chance.",
				power_activation {
					0, 0, 0, 0
				}
			),
		},
		{
			PWR_BALROG,
			std::make_shared<power_type>(
				"turn into a Balrog",
				"You can turn into a Balrog at will.",
				"You feel the fire of Udun burning in you.",
				"You no longer feel the fire of Udun in you.",
				power_activation {
					35, 80, A_WIS, 25
				}
			),
		},
		{
			POWER_INVISIBILITY,
			std::make_shared<power_type>(
				"invisibility",
				"You are able melt into the shadows to become invisible.",
				"You suddenly become able to melt into the shadows.",
				"You lose your shadow-melting ability.",
				power_activation {
					30, 10, A_DEX, 20
				}
			),
		},
		{
			POWER_WEB,
			std::make_shared<power_type>(
				"web",
				"You are able throw a thick and very resistant spider web.",
				"You suddenly become able to weave webs.",
				"You lose your web-weaving capability.",
				power_activation {
					25, 30, A_DEX, 20
				}
			),
		},
		{
			POWER_COR_SPACE_TIME,
			std::make_shared<power_type>(
				"control space/time continuum",
				"You are able to control the space/time continuum.",
				"You become able to control the space/time continuum.",
				"You are no more able to control the space/time continuum.",
				power_activation {
					1, 10, A_WIS, 10
				}
			),
		},
	};
}
