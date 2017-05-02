#include "h-basic.h"

#include <string>

void gain_random_corruption();
std::string dump_corruptions(bool color, bool header);
void lose_corruption();
bool_ player_has_corruption(int corruption_idx);
void player_gain_corruption(int corruption_idx);
s16b get_corruption_power(int corruption_idx);

/*
 * Corruptions
 */
#define CORRUPT_BALROG_AURA 0
#define CORRUPT_BALROG_WINGS 1
#define CORRUPT_BALROG_STRENGTH 2
#define CORRUPT_BALROG_FORM 3
#define CORRUPT_DEMON_SPIRIT 4
#define CORRUPT_DEMON_HIDE 5
#define CORRUPT_DEMON_BREATH 6
#define CORRUPT_DEMON_REALM 7
#define CORRUPT_RANDOM_TELEPORT 8
#define CORRUPT_ANTI_TELEPORT 9
#define CORRUPT_TROLL_BLOOD 10
#define CORRUPT_VAMPIRE_TEETH 11
#define CORRUPT_VAMPIRE_STRENGTH 12
#define CORRUPT_VAMPIRE_VAMPIRE 13
#define MUT1_SPIT_ACID 14
#define MUT1_BR_FIRE 15
#define MUT1_HYPN_GAZE 16
#define MUT1_TELEKINES 17
#define MUT1_VTELEPORT 18
#define MUT1_MIND_BLST 19
#define MUT1_VAMPIRISM 20
#define MUT1_SMELL_MET 21
#define MUT1_SMELL_MON 22
#define MUT1_BLINK 23
#define MUT1_EAT_ROCK 24
#define MUT1_SWAP_POS 25
#define MUT1_SHRIEK 26
#define MUT1_ILLUMINE 27
#define MUT1_DET_CURSE 28
#define MUT1_BERSERK 29
#define MUT1_MIDAS_TCH 30
#define MUT1_GROW_MOLD 31
#define MUT1_RESIST 32
#define MUT1_EARTHQUAKE 33
#define CORRUPTIONS_MAX 34
