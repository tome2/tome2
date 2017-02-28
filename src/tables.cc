/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "tables.hpp"
#include "tables.h"

#include "modules.hpp"
#include "monster_race_flag.hpp"
#include "monster_spell_flag.hpp"
#include "object_flag.hpp"
#include "options.hpp"
#include "q_library.hpp"
#include "q_fireprof.hpp"
#include "q_bounty.hpp"
#include "q_thrain.hpp"
#include "q_narsil.hpp"
#include "q_evil.hpp"
#include "q_betwen.hpp"
#include "q_haunted.hpp"
#include "q_invas.hpp"
#include "q_nirna.hpp"
#include "q_eol.hpp"
#include "q_god.hpp"
#include "q_dragons.hpp"
#include "q_poison.hpp"
#include "q_spider.hpp"
#include "q_wolves.hpp"
#include "q_shroom.hpp"
#include "q_nazgul.hpp"
#include "q_wight.hpp"
#include "q_troll.hpp"
#include "q_hobbit.hpp"
#include "q_thief.hpp"
#include "q_ultrae.hpp"
#include "q_ultrag.hpp"
#include "q_one.hpp"
#include "q_main.hpp"
#include "q_rand.hpp"
#include "stats.hpp"
#include "variable.hpp"



/*
 * Global array for looping through the "keypad directions"
 */
s16b ddd[9] =
	{ 2, 8, 6, 4, 3, 1, 9, 7, 5 };

/*
 * Global arrays for converting "keypad direction" into offsets
 */
s16b ddx[10] =
	{ 0, -1, 0, 1, -1, 0, 1, -1, 0, 1 };

s16b ddy[10] =
	{ 0, 1, 1, 1, 0, 0, 0, -1, -1, -1 };

/*
 * Global arrays for optimizing "ddx[ddd[i]]" and "ddy[ddd[i]]"
 */
s16b ddx_ddd[9] =
	{ 0, 0, 1, -1, 1, -1, 1, -1, 0 };

s16b ddy_ddd[9] =
	{ 1, -1, 0, 0, 1, 1, -1, -1, 0 };



/*
* Global array for converting numbers to uppercase hexadecimal digit
 * This array can also be used to convert a number to an octal digit
 */
char hexsym[16] =
	{
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
	};


/*
 * Stat Table (INT/WIS) -- extra half-mana-points per level
 */
byte adj_mag_mana[] =
{
	0       /* 3 */,
	0       /* 4 */,
	0       /* 5 */,
	0       /* 6 */,
	0       /* 7 */,
	1       /* 8 */,
	2       /* 9 */,
	2       /* 10 */,
	2       /* 11 */,
	2       /* 12 */,
	2       /* 13 */,
	2       /* 14 */,
	2       /* 15 */,
	2       /* 16 */,
	2       /* 17 */,
	3       /* 18/00-18/09 */,
	3       /* 18/10-18/19 */,
	3       /* 18/20-18/29 */,
	3       /* 18/30-18/39 */,
	3       /* 18/40-18/49 */,
	4       /* 18/50-18/59 */,
	4       /* 18/60-18/69 */,
	5       /* 18/70-18/79 */,
	6       /* 18/80-18/89 */,
	7       /* 18/90-18/99 */,
	8       /* 18/100-18/109 */,
	9       /* 18/110-18/119 */,
	10      /* 18/120-18/129 */,
	11      /* 18/130-18/139 */,
	12      /* 18/140-18/149 */,
	13      /* 18/150-18/159 */,
	14      /* 18/160-18/169 */,
	15      /* 18/170-18/179 */,
	16      /* 18/180-18/189 */,
	16      /* 18/190-18/199 */,
	17      /* 18/200-18/209 */,
	17      /* 18/210-18/219 */,
	18      /* 18/220+ */
};


/*
 * Stat Table (INT/WIS) -- Minimum failure rate (percentage)
 */
byte adj_mag_fail[] =
{
	99      /* 3 */,
	99      /* 4 */,
	99      /* 5 */,
	99      /* 6 */,
	99      /* 7 */,
	50      /* 8 */,
	30      /* 9 */,
	20      /* 10 */,
	15      /* 11 */,
	12      /* 12 */,
	11      /* 13 */,
	10      /* 14 */,
	9       /* 15 */,
	8       /* 16 */,
	7       /* 17 */,
	6       /* 18/00-18/09 */,
	6       /* 18/10-18/19 */,
	5       /* 18/20-18/29 */,
	5       /* 18/30-18/39 */,
	5       /* 18/40-18/49 */,
	4       /* 18/50-18/59 */,
	4       /* 18/60-18/69 */,
	4       /* 18/70-18/79 */,
	4       /* 18/80-18/89 */,
	3       /* 18/90-18/99 */,
	3       /* 18/100-18/109 */,
	2       /* 18/110-18/119 */,
	2       /* 18/120-18/129 */,
	2       /* 18/130-18/139 */,
	2       /* 18/140-18/149 */,
	1       /* 18/150-18/159 */,
	1       /* 18/160-18/169 */,
	1       /* 18/170-18/179 */,
	1       /* 18/180-18/189 */,
	1       /* 18/190-18/199 */,
	0       /* 18/200-18/209 */,
	0       /* 18/210-18/219 */,
	0       /* 18/220+ */
};


/*
 * Stat Table (INT/WIS) -- Various things
 */
byte adj_mag_stat[] =
{
	0       /* 3 */,
	0       /* 4 */,
	0       /* 5 */,
	0       /* 6 */,
	0       /* 7 */,
	1       /* 8 */,
	1       /* 9 */,
	1       /* 10 */,
	1       /* 11 */,
	1       /* 12 */,
	1       /* 13 */,
	1       /* 14 */,
	2       /* 15 */,
	2       /* 16 */,
	2       /* 17 */,
	3       /* 18/00-18/09 */,
	3       /* 18/10-18/19 */,
	3       /* 18/20-18/29 */,
	3       /* 18/30-18/39 */,
	3       /* 18/40-18/49 */,
	4       /* 18/50-18/59 */,
	4       /* 18/60-18/69 */,
	5       /* 18/70-18/79 */,
	6       /* 18/80-18/89 */,
	7       /* 18/90-18/99 */,
	8       /* 18/100-18/109 */,
	9       /* 18/110-18/119 */,
	10      /* 18/120-18/129 */,
	11      /* 18/130-18/139 */,
	12      /* 18/140-18/149 */,
	13      /* 18/150-18/159 */,
	14      /* 18/160-18/169 */,
	15      /* 18/170-18/179 */,
	16      /* 18/180-18/189 */,
	17      /* 18/190-18/199 */,
	18      /* 18/200-18/209 */,
	19      /* 18/210-18/219 */,
	20      /* 18/220+ */
};


/*
 * Stat Table (CHR) -- payment percentages
 */
byte adj_chr_gold[] =
{
	130     /* 3 */,
	125     /* 4 */,
	122     /* 5 */,
	120     /* 6 */,
	118     /* 7 */,
	116     /* 8 */,
	114     /* 9 */,
	112     /* 10 */,
	110     /* 11 */,
	108     /* 12 */,
	106     /* 13 */,
	104     /* 14 */,
	103     /* 15 */,
	102     /* 16 */,
	101     /* 17 */,
	100     /* 18/00-18/09 */,
	99      /* 18/10-18/19 */,
	98      /* 18/20-18/29 */,
	97      /* 18/30-18/39 */,
	96      /* 18/40-18/49 */,
	95      /* 18/50-18/59 */,
	94      /* 18/60-18/69 */,
	93      /* 18/70-18/79 */,
	92      /* 18/80-18/89 */,
	91      /* 18/90-18/99 */,
	90      /* 18/100-18/109 */,
	89      /* 18/110-18/119 */,
	88      /* 18/120-18/129 */,
	87      /* 18/130-18/139 */,
	86      /* 18/140-18/149 */,
	85      /* 18/150-18/159 */,
	84      /* 18/160-18/169 */,
	83      /* 18/170-18/179 */,
	82      /* 18/180-18/189 */,
	81      /* 18/190-18/199 */,
	80      /* 18/200-18/209 */,
	79      /* 18/210-18/219 */,
	78      /* 18/220+ */
};


/*
 * Stat Table (WIS) -- Saving throw
 */
byte adj_wis_sav[] =
{
	0       /* 3 */,
	0       /* 4 */,
	0       /* 5 */,
	0       /* 6 */,
	0       /* 7 */,
	1       /* 8 */,
	1       /* 9 */,
	1       /* 10 */,
	1       /* 11 */,
	1       /* 12 */,
	1       /* 13 */,
	1       /* 14 */,
	2       /* 15 */,
	2       /* 16 */,
	2       /* 17 */,
	3       /* 18/00-18/09 */,
	3       /* 18/10-18/19 */,
	3       /* 18/20-18/29 */,
	3       /* 18/30-18/39 */,
	3       /* 18/40-18/49 */,
	4       /* 18/50-18/59 */,
	4       /* 18/60-18/69 */,
	5       /* 18/70-18/79 */,
	5       /* 18/80-18/89 */,
	6       /* 18/90-18/99 */,
	7       /* 18/100-18/109 */,
	8       /* 18/110-18/119 */,
	9       /* 18/120-18/129 */,
	10      /* 18/130-18/139 */,
	11      /* 18/140-18/149 */,
	12      /* 18/150-18/159 */,
	13      /* 18/160-18/169 */,
	14      /* 18/170-18/179 */,
	15      /* 18/180-18/189 */,
	16      /* 18/190-18/199 */,
	17      /* 18/200-18/209 */,
	18      /* 18/210-18/219 */,
	19      /* 18/220+ */
};


/*
 * Stat Table (DEX) -- bonus to ac (plus 128)
 */
byte adj_dex_ta[] =
{
	128 + -4        /* 3 */,
	128 + -3        /* 4 */,
	128 + -2        /* 5 */,
	128 + -1        /* 6 */,
	128 + 0 /* 7 */,
	128 + 0 /* 8 */,
	128 + 0 /* 9 */,
	128 + 0 /* 10 */,
	128 + 0 /* 11 */,
	128 + 0 /* 12 */,
	128 + 0 /* 13 */,
	128 + 0 /* 14 */,
	128 + 1 /* 15 */,
	128 + 1 /* 16 */,
	128 + 1 /* 17 */,
	128 + 2 /* 18/00-18/09 */,
	128 + 2 /* 18/10-18/19 */,
	128 + 2 /* 18/20-18/29 */,
	128 + 2 /* 18/30-18/39 */,
	128 + 2 /* 18/40-18/49 */,
	128 + 3 /* 18/50-18/59 */,
	128 + 3 /* 18/60-18/69 */,
	128 + 3 /* 18/70-18/79 */,
	128 + 4 /* 18/80-18/89 */,
	128 + 5 /* 18/90-18/99 */,
	128 + 6 /* 18/100-18/109 */,
	128 + 7 /* 18/110-18/119 */,
	128 + 8 /* 18/120-18/129 */,
	128 + 9 /* 18/130-18/139 */,
	128 + 9 /* 18/140-18/149 */,
	128 + 10        /* 18/150-18/159 */,
	128 + 11        /* 18/160-18/169 */,
	128 + 12        /* 18/170-18/179 */,
	128 + 13        /* 18/180-18/189 */,
	128 + 14        /* 18/190-18/199 */,
	128 + 15        /* 18/200-18/209 */,
	128 + 15        /* 18/210-18/219 */,
	128 + 16        /* 18/220+ */
};


/*
 * Stat Table (STR) -- bonus to dam (plus 128)
 */
byte adj_str_td[] =
{
	128 + -2        /* 3 */,
	128 + -2        /* 4 */,
	128 + -1        /* 5 */,
	128 + -1        /* 6 */,
	128 + 0 /* 7 */,
	128 + 0 /* 8 */,
	128 + 0 /* 9 */,
	128 + 0 /* 10 */,
	128 + 0 /* 11 */,
	128 + 0 /* 12 */,
	128 + 0 /* 13 */,
	128 + 0 /* 14 */,
	128 + 0 /* 15 */,
	128 + 1 /* 16 */,
	128 + 2 /* 17 */,
	128 + 2 /* 18/00-18/09 */,
	128 + 2 /* 18/10-18/19 */,
	128 + 3 /* 18/20-18/29 */,
	128 + 3 /* 18/30-18/39 */,
	128 + 3 /* 18/40-18/49 */,
	128 + 3 /* 18/50-18/59 */,
	128 + 3 /* 18/60-18/69 */,
	128 + 4 /* 18/70-18/79 */,
	128 + 5 /* 18/80-18/89 */,
	128 + 5 /* 18/90-18/99 */,
	128 + 6 /* 18/100-18/109 */,
	128 + 7 /* 18/110-18/119 */,
	128 + 8 /* 18/120-18/129 */,
	128 + 9 /* 18/130-18/139 */,
	128 + 10        /* 18/140-18/149 */,
	128 + 11        /* 18/150-18/159 */,
	128 + 12        /* 18/160-18/169 */,
	128 + 13        /* 18/170-18/179 */,
	128 + 14        /* 18/180-18/189 */,
	128 + 15        /* 18/190-18/199 */,
	128 + 16        /* 18/200-18/209 */,
	128 + 18        /* 18/210-18/219 */,
	128 + 20        /* 18/220+ */
};


/*
 * Stat Table (DEX) -- bonus to hit (plus 128)
 */
byte adj_dex_th[] =
{
	128 + -3        /* 3 */,
	128 + -2        /* 4 */,
	128 + -2        /* 5 */,
	128 + -1        /* 6 */,
	128 + -1        /* 7 */,
	128 + 0 /* 8 */,
	128 + 0 /* 9 */,
	128 + 0 /* 10 */,
	128 + 0 /* 11 */,
	128 + 0 /* 12 */,
	128 + 0 /* 13 */,
	128 + 0 /* 14 */,
	128 + 0 /* 15 */,
	128 + 1 /* 16 */,
	128 + 2 /* 17 */,
	128 + 3 /* 18/00-18/09 */,
	128 + 3 /* 18/10-18/19 */,
	128 + 3 /* 18/20-18/29 */,
	128 + 3 /* 18/30-18/39 */,
	128 + 3 /* 18/40-18/49 */,
	128 + 4 /* 18/50-18/59 */,
	128 + 4 /* 18/60-18/69 */,
	128 + 4 /* 18/70-18/79 */,
	128 + 4 /* 18/80-18/89 */,
	128 + 5 /* 18/90-18/99 */,
	128 + 6 /* 18/100-18/109 */,
	128 + 7 /* 18/110-18/119 */,
	128 + 8 /* 18/120-18/129 */,
	128 + 9 /* 18/130-18/139 */,
	128 + 9 /* 18/140-18/149 */,
	128 + 10        /* 18/150-18/159 */,
	128 + 11        /* 18/160-18/169 */,
	128 + 12        /* 18/170-18/179 */,
	128 + 13        /* 18/180-18/189 */,
	128 + 14        /* 18/190-18/199 */,
	128 + 15        /* 18/200-18/209 */,
	128 + 15        /* 18/210-18/219 */,
	128 + 16        /* 18/220+ */
};


/*
 * Stat Table (STR) -- bonus to hit (plus 128)
 */
byte adj_str_th[] =
{
	128 + -3        /* 3 */,
	128 + -2        /* 4 */,
	128 + -1        /* 5 */,
	128 + -1        /* 6 */,
	128 + 0 /* 7 */,
	128 + 0 /* 8 */,
	128 + 0 /* 9 */,
	128 + 0 /* 10 */,
	128 + 0 /* 11 */,
	128 + 0 /* 12 */,
	128 + 0 /* 13 */,
	128 + 0 /* 14 */,
	128 + 0 /* 15 */,
	128 + 0 /* 16 */,
	128 + 0 /* 17 */,
	128 + 1 /* 18/00-18/09 */,
	128 + 1 /* 18/10-18/19 */,
	128 + 1 /* 18/20-18/29 */,
	128 + 1 /* 18/30-18/39 */,
	128 + 1 /* 18/40-18/49 */,
	128 + 1 /* 18/50-18/59 */,
	128 + 1 /* 18/60-18/69 */,
	128 + 2 /* 18/70-18/79 */,
	128 + 3 /* 18/80-18/89 */,
	128 + 4 /* 18/90-18/99 */,
	128 + 5 /* 18/100-18/109 */,
	128 + 6 /* 18/110-18/119 */,
	128 + 7 /* 18/120-18/129 */,
	128 + 8 /* 18/130-18/139 */,
	128 + 9 /* 18/140-18/149 */,
	128 + 10        /* 18/150-18/159 */,
	128 + 11        /* 18/160-18/169 */,
	128 + 12        /* 18/170-18/179 */,
	128 + 13        /* 18/180-18/189 */,
	128 + 14        /* 18/190-18/199 */,
	128 + 15        /* 18/200-18/209 */,
	128 + 15        /* 18/210-18/219 */,
	128 + 16        /* 18/220+ */
};


/*
 * Stat Table (STR) -- weight limit in deca-pounds
 */
byte adj_str_wgt[] =
{
	5       /* 3 */,
	6       /* 4 */,
	7       /* 5 */,
	8       /* 6 */,
	9       /* 7 */,
	10      /* 8 */,
	11      /* 9 */,
	12      /* 10 */,
	13      /* 11 */,
	14      /* 12 */,
	15      /* 13 */,
	16      /* 14 */,
	17      /* 15 */,
	18      /* 16 */,
	19      /* 17 */,
	20      /* 18/00-18/09 */,
	22      /* 18/10-18/19 */,
	24      /* 18/20-18/29 */,
	26      /* 18/30-18/39 */,
	28      /* 18/40-18/49 */,
	30      /* 18/50-18/59 */,
	31      /* 18/60-18/69 */,
	31      /* 18/70-18/79 */,
	32      /* 18/80-18/89 */,
	32      /* 18/90-18/99 */,
	33      /* 18/100-18/109 */,
	33      /* 18/110-18/119 */,
	34      /* 18/120-18/129 */,
	34      /* 18/130-18/139 */,
	35      /* 18/140-18/149 */,
	35      /* 18/150-18/159 */,
	36      /* 18/160-18/169 */,
	36      /* 18/170-18/179 */,
	37      /* 18/180-18/189 */,
	37      /* 18/190-18/199 */,
	38      /* 18/200-18/209 */,
	38      /* 18/210-18/219 */,
	39      /* 18/220+ */
};


/*
 * Stat Table (STR) -- weapon weight limit in pounds
 */
byte adj_str_hold[] =
{
	4       /* 3 */,
	5       /* 4 */,
	6       /* 5 */,
	7       /* 6 */,
	8       /* 7 */,
	10      /* 8 */,
	12      /* 9 */,
	14      /* 10 */,
	16      /* 11 */,
	18      /* 12 */,
	20      /* 13 */,
	22      /* 14 */,
	24      /* 15 */,
	26      /* 16 */,
	28      /* 17 */,
	30      /* 18/00-18/09 */,
	30      /* 18/10-18/19 */,
	35      /* 18/20-18/29 */,
	40      /* 18/30-18/39 */,
	45      /* 18/40-18/49 */,
	50      /* 18/50-18/59 */,
	55      /* 18/60-18/69 */,
	60      /* 18/70-18/79 */,
	65      /* 18/80-18/89 */,
	70      /* 18/90-18/99 */,
	80      /* 18/100-18/109 */,
	80      /* 18/110-18/119 */,
	80      /* 18/120-18/129 */,
	80      /* 18/130-18/139 */,
	80      /* 18/140-18/149 */,
	90      /* 18/150-18/159 */,
	90      /* 18/160-18/169 */,
	90      /* 18/170-18/179 */,
	90      /* 18/180-18/189 */,
	90      /* 18/190-18/199 */,
	100     /* 18/200-18/209 */,
	100     /* 18/210-18/219 */,
	100     /* 18/220+ */
};


/*
 * Stat Table (STR) -- digging value
 */
byte adj_str_dig[] =
{
	0       /* 3 */,
	0       /* 4 */,
	1       /* 5 */,
	2       /* 6 */,
	3       /* 7 */,
	4       /* 8 */,
	4       /* 9 */,
	5       /* 10 */,
	5       /* 11 */,
	6       /* 12 */,
	6       /* 13 */,
	7       /* 14 */,
	7       /* 15 */,
	8       /* 16 */,
	8       /* 17 */,
	9       /* 18/00-18/09 */,
	10      /* 18/10-18/19 */,
	12      /* 18/20-18/29 */,
	15      /* 18/30-18/39 */,
	20      /* 18/40-18/49 */,
	25      /* 18/50-18/59 */,
	30      /* 18/60-18/69 */,
	35      /* 18/70-18/79 */,
	40      /* 18/80-18/89 */,
	45      /* 18/90-18/99 */,
	50      /* 18/100-18/109 */,
	55      /* 18/110-18/119 */,
	60      /* 18/120-18/129 */,
	65      /* 18/130-18/139 */,
	70      /* 18/140-18/149 */,
	75      /* 18/150-18/159 */,
	80      /* 18/160-18/169 */,
	85      /* 18/170-18/179 */,
	90      /* 18/180-18/189 */,
	95      /* 18/190-18/199 */,
	100     /* 18/200-18/209 */,
	100     /* 18/210-18/219 */,
	100     /* 18/220+ */
};


/*
 * Stat Table (STR) -- help index into the "blow" table
 */
byte adj_str_blow[] =
{
	3       /* 3 */,
	4       /* 4 */,
	5       /* 5 */,
	6       /* 6 */,
	7       /* 7 */,
	8       /* 8 */,
	9       /* 9 */,
	10      /* 10 */,
	11      /* 11 */,
	12      /* 12 */,
	13      /* 13 */,
	14      /* 14 */,
	15      /* 15 */,
	16      /* 16 */,
	17      /* 17 */,
	20 /* 18/00-18/09 */,
	30 /* 18/10-18/19 */,
	40 /* 18/20-18/29 */,
	50 /* 18/30-18/39 */,
	60 /* 18/40-18/49 */,
	70 /* 18/50-18/59 */,
	80 /* 18/60-18/69 */,
	90 /* 18/70-18/79 */,
	100 /* 18/80-18/89 */,
	110 /* 18/90-18/99 */,
	120 /* 18/100-18/109 */,
	130 /* 18/110-18/119 */,
	140 /* 18/120-18/129 */,
	150 /* 18/130-18/139 */,
	160 /* 18/140-18/149 */,
	170 /* 18/150-18/159 */,
	180 /* 18/160-18/169 */,
	190 /* 18/170-18/179 */,
	200 /* 18/180-18/189 */,
	210 /* 18/190-18/199 */,
	220 /* 18/200-18/209 */,
	230 /* 18/210-18/219 */,
	240 /* 18/220+ */
};


/*
 * Stat Table (DEX) -- index into the "blow" table
 */
byte adj_dex_blow[] =
{
	0       /* 3 */,
	0       /* 4 */,
	0       /* 5 */,
	0       /* 6 */,
	0       /* 7 */,
	0       /* 8 */,
	0       /* 9 */,
	1       /* 10 */,
	1       /* 11 */,
	1       /* 12 */,
	1       /* 13 */,
	1       /* 14 */,
	1       /* 15 */,
	1       /* 16 */,
	1       /* 17 */,
	1       /* 18/00-18/09 */,
	2       /* 18/10-18/19 */,
	2       /* 18/20-18/29 */,
	2       /* 18/30-18/39 */,
	2       /* 18/40-18/49 */,
	3       /* 18/50-18/59 */,
	3       /* 18/60-18/69 */,
	4       /* 18/70-18/79 */,
	4       /* 18/80-18/89 */,
	5       /* 18/90-18/99 */,
	6       /* 18/100-18/109 */,
	7       /* 18/110-18/119 */,
	8       /* 18/120-18/129 */,
	9       /* 18/130-18/139 */,
	10      /* 18/140-18/149 */,
	11      /* 18/150-18/159 */,
	12      /* 18/160-18/169 */,
	14      /* 18/170-18/179 */,
	16      /* 18/180-18/189 */,
	18      /* 18/190-18/199 */,
	20      /* 18/200-18/209 */,
	20      /* 18/210-18/219 */,
	20      /* 18/220+ */
};


/*
 * Stat Table (DEX) -- chance of avoiding "theft" and "falling"
 */
byte adj_dex_safe[] =
{
	0       /* 3 */,
	1       /* 4 */,
	2       /* 5 */,
	3       /* 6 */,
	4       /* 7 */,
	5       /* 8 */,
	5       /* 9 */,
	6       /* 10 */,
	6       /* 11 */,
	7       /* 12 */,
	7       /* 13 */,
	8       /* 14 */,
	8       /* 15 */,
	9       /* 16 */,
	9       /* 17 */,
	10      /* 18/00-18/09 */,
	10      /* 18/10-18/19 */,
	15      /* 18/20-18/29 */,
	15      /* 18/30-18/39 */,
	20      /* 18/40-18/49 */,
	25      /* 18/50-18/59 */,
	30      /* 18/60-18/69 */,
	35      /* 18/70-18/79 */,
	40      /* 18/80-18/89 */,
	45      /* 18/90-18/99 */,
	50      /* 18/100-18/109 */,
	60      /* 18/110-18/119 */,
	70      /* 18/120-18/129 */,
	80      /* 18/130-18/139 */,
	90      /* 18/140-18/149 */,
	100     /* 18/150-18/159 */,
	100     /* 18/160-18/169 */,
	100     /* 18/170-18/179 */,
	100     /* 18/180-18/189 */,
	100     /* 18/190-18/199 */,
	100     /* 18/200-18/209 */,
	100     /* 18/210-18/219 */,
	100     /* 18/220+ */
};


/*
 * Stat Table (CON) -- base regeneration rate
 */
byte adj_con_fix[] =
{
	0       /* 3 */,
	0       /* 4 */,
	0       /* 5 */,
	0       /* 6 */,
	0       /* 7 */,
	0       /* 8 */,
	0       /* 9 */,
	0       /* 10 */,
	0       /* 11 */,
	0       /* 12 */,
	0       /* 13 */,
	1       /* 14 */,
	1       /* 15 */,
	1       /* 16 */,
	1       /* 17 */,
	2       /* 18/00-18/09 */,
	2       /* 18/10-18/19 */,
	2       /* 18/20-18/29 */,
	2       /* 18/30-18/39 */,
	2       /* 18/40-18/49 */,
	3       /* 18/50-18/59 */,
	3       /* 18/60-18/69 */,
	3       /* 18/70-18/79 */,
	3       /* 18/80-18/89 */,
	3       /* 18/90-18/99 */,
	4       /* 18/100-18/109 */,
	4       /* 18/110-18/119 */,
	5       /* 18/120-18/129 */,
	6       /* 18/130-18/139 */,
	6       /* 18/140-18/149 */,
	7       /* 18/150-18/159 */,
	7       /* 18/160-18/169 */,
	8       /* 18/170-18/179 */,
	8       /* 18/180-18/189 */,
	8       /* 18/190-18/199 */,
	9       /* 18/200-18/209 */,
	9       /* 18/210-18/219 */,
	9       /* 18/220+ */
};


/*
 * Stat Table (CON) -- extra half-hitpoints per level (plus 128)
 */
byte adj_con_mhp[] =
{
	128 + -5        /* 3 */,
	128 + -3        /* 4 */,
	128 + -2        /* 5 */,
	128 + -1        /* 6 */,
	128 + 0 /* 7 */,
	128 + 0 /* 8 */,
	128 + 0 /* 9 */,
	128 + 0 /* 10 */,
	128 + 0 /* 11 */,
	128 + 0 /* 12 */,
	128 + 0 /* 13 */,
	128 + 0 /* 14 */,
	128 + 1 /* 15 */,
	128 + 1 /* 16 */,
	128 + 2 /* 17 */,
	128 + 3 /* 18/00-18/09 */,
	128 + 4 /* 18/10-18/19 */,
	128 + 4 /* 18/20-18/29 */,
	128 + 4 /* 18/30-18/39 */,
	128 + 4 /* 18/40-18/49 */,
	128 + 5 /* 18/50-18/59 */,
	128 + 6 /* 18/60-18/69 */,
	128 + 7 /* 18/70-18/79 */,
	128 + 8 /* 18/80-18/89 */,
	128 + 9 /* 18/90-18/99 */,
	128 + 10        /* 18/100-18/109 */,
	128 + 11        /* 18/110-18/119 */,
	128 + 12        /* 18/120-18/129 */,
	128 + 13        /* 18/130-18/139 */,
	128 + 14        /* 18/140-18/149 */,
	128 + 15        /* 18/150-18/159 */,
	128 + 16        /* 18/160-18/169 */,
	128 + 18        /* 18/170-18/179 */,
	128 + 20        /* 18/180-18/189 */,
	128 + 22        /* 18/190-18/199 */,
	128 + 25        /* 18/200-18/209 */,
	128 + 26        /* 18/210-18/219 */,
	128 + 27        /* 18/220+ */
};


/*
 * This table is used to help calculate the number of blows the player can
 * make in a single round of attacks (one player turn) with a normal weapon.
 *
 * This number ranges from a single blow/round for weak players to up to six
 * blows/round for powerful warriors.
 *
 * Note that certain artifacts and ego-items give "bonus" blows/round.
 *
 * First, from the player class, we extract some values:
 *
 *    Warrior --> num = 6; mul = 5; div = MAX(30, weapon_weight);
 *    Mage    --> num = 4; mul = 2; div = MAX(40, weapon_weight);
 *    Priest  --> num = 5; mul = 3; div = MAX(35, weapon_weight);
 *    Rogue   --> num = 5; mul = 3; div = MAX(30, weapon_weight);
 *    Ranger  --> num = 5; mul = 4; div = MAX(35, weapon_weight);
 *    Paladin --> num = 5; mul = 4; div = MAX(30, weapon_weight);
 *
 * To get "P", we look up the relevant "adj_str_blow[]" (see above),
 * multiply it by "mul", and then divide it by "div", rounding down.
 *
 * To get "D", we look up the relevant "adj_dex_blow[]" (see above),
 * note especially column 6 (DEX 18/101) and 11 (DEX 18/150).
 *
 * The player gets "blows_table[P][D]" blows/round, as shown below,
 * up to a maximum of "num" blows/round, plus any "bonus" blows/round.
 */
byte blows_table[12][12] =
{
	/* P/D */
	/* 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11+ */

	/* 0  */
	{ 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3 },

	/* 1  */
	{ 1, 1, 1, 1, 2, 2, 3, 3, 3, 4, 4, 4 },

	/* 2  */
	{ 1, 1, 2, 2, 3, 3, 4, 4, 4, 5, 5, 5 },

	/* 3  */
	{ 1, 2, 2, 3, 3, 4, 4, 4, 5, 5, 5, 5 },

	/* 4  */
	{ 1, 2, 2, 3, 3, 4, 4, 5, 5, 5, 5, 5 },

	/* 5  */
	{ 2, 2, 3, 3, 4, 4, 5, 5, 5, 5, 5, 6 },

	/* 6  */
	{ 2, 2, 3, 3, 4, 4, 5, 5, 5, 5, 5, 6 },

	/* 7  */
	{ 2, 3, 3, 4, 4, 4, 5, 5, 5, 5, 5, 6 },

	/* 8  */
	{ 3, 3, 3, 4, 4, 4, 5, 5, 5, 5, 6, 6 },

	/* 9  */
	{ 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6 },

	/* 10 */
	{ 3, 3, 4, 4, 4, 4, 5, 5, 5, 6, 6, 6 },

	/* 11+ */
	{ 3, 3, 4, 4, 4, 4, 5, 5, 6, 6, 6, 6 },
};



/*
 * This table allows quick conversion from "speed" to "energy"
 * The basic function WAS ((S>=110) ? (S-110) : (100 / (120-S)))
 * Note that table access is *much* quicker than computation.
 *
 * Note that the table has been changed at high speeds.  From
 * "Slow (-40)" to "Fast (+30)" is pretty much unchanged, but
 * at speeds above "Fast (+30)", one approaches an asymptotic
 * effective limit of 50 energy per turn.  This means that it
 * is relatively easy to reach "Fast (+30)" and get about 40
 * energy per turn, but then speed becomes very "expensive",
 * and you must get all the way to "Fast (+50)" to reach the
 * point of getting 45 energy per turn.  After that point,
 * further increases in speed are more or less pointless,
 * except to balance out heavy inventory.
 *
 * Note that currently the fastest monster is "Fast (+30)".
 *
 * It should be possible to lower the energy threshold from
 * 100 units to 50 units, though this may interact badly with
 * the (compiled out) small random energy boost code.  It may
 * also tend to cause more "clumping" at high speeds.
 */
byte extract_energy[300] =
{
	/* Slow */     1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
	/* Slow */     1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
	/* Slow */     1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
	/* Slow */     1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
	/* Slow */     1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
	/* Slow */     1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
	/* S-50 */     1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
	/* S-40 */     2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
	/* S-30 */     2,    2,    2,    2,    2,    2,    2,    3,    3,    3,
	/* S-20 */     3,    3,    3,    3,    3,    4,    4,    4,    4,    4,
	/* S-10 */     5,    5,    5,    5,    6,    6,    7,    7,    8,    9,
	/* Norm */    10,   11,   12,   13,   14,   15,   16,   17,   18,   19,
	/* F+10 */    20,   21,   22,   23,   24,   25,   26,   27,   28,   29,
	/* F+20 */    30,   31,   32,   33,   34,   35,   36,   36,   37,   37,
	/* F+30 */    38,   38,   39,   39,   40,   40,   40,   41,   41,   41,
	/* F+40 */    42,   42,   42,   43,   43,   43,   44,   44,   44,   44,
	/* F+50 */    45,   45,   45,   45,   45,   46,   46,   46,   46,   46,
	/* F+60 */    47,   47,   47,   47,   47,   48,   48,   48,   48,   48,
	/* F+70 */    49,   49,   49,   49,   49,   49,   49,   49,   49,   49,
	/* Fast */    49,   49,   49,   49,   49,   49,   49,   49,   49,   49,
	/* Virtual */  49,   49,   49,   49,   49,   49,   49,   49,   49,   49,
	/* Virtual */  49,   49,   49,   49,   49,   49,   49,   49,   49,   49,
	/* Virtual */  49,   49,   49,   49,   49,   49,   49,   49,   49,   49,
	/* Virtual */  49,   49,   49,   49,   49,   49,   49,   49,   49,   49,
	/* Virtual */  49,   49,   49,   49,   49,   49,   49,   49,   49,   49,
	/* Virtual */  49,   49,   49,   49,   49,   49,   49,   49,   49,   49,
	/* Virtual */  49,   49,   49,   49,   49,   49,   49,   49,   49,   49,
	/* Virtual */  49,   49,   49,   49,   49,   49,   49,   49,   49,   49,
	/* Virtual */  49,   49,   49,   49,   49,   49,   49,   49,   49,   49,
	/* Virtual */  49,   49,   49,   49,   49,   49,   49,   49,   49,   49,
};




/*
 * Base experience levels, may be adjusted up for race and/or class
 */
s32b player_exp[PY_MAX_LEVEL] =
{
	10,
	25,
	45,
	70,
	100,
	140,
	200,
	280,
	380,
	500,
	650,
	850,
	1100,
	1400,
	1800,
	2300,
	2900,
	3600,
	4400,
	5400,
	6800,
	8400,
	10200,
	12500,
	17500,
	25000,
	35000L,
	50000L,
	75000L,
	100000L,
	150000L,
	200000L,
	275000L,
	350000L,
	450000L,
	550000L,
	700000L,
	850000L,
	1000000L,
	1250000L,
	1500000L,
	1800000L,
	2100000L,
	2400000L,
	2700000L,
	3000000L,
	3500000L,
	4000000L,
	4500000L,
	5000000L
};


/*
 * Hack -- the "basic" color names (see "TERM_xxx")
 */
cptr color_names[16] =
{
	"Dark",
	"White",
	"Slate",
	"Orange",
	"Red",
	"Green",
	"Blue",
	"Umber",
	"Light Dark",
	"Light Slate",
	"Violet",
	"Yellow",
	"Light Red",
	"Light Green",
	"Light Blue",
	"Light Umber",
};


/*
 * Abbreviations of healthy stats
 */
cptr stat_names[6] =
{
	"STR", "INT", "WIS", "DEX", "CON", "CHR"
};

/*
 * Abbreviations of damaged stats
 */
cptr stat_names_reduced[6] =
{
	"Str", "Int", "Wis", "Dex", "Con", "Chr"
};


/*
 * Certain "screens" always use the main screen, including News, Birth,
 * Dungeon, Tomb-stone, High-scores, Macros, Colors, Visuals, Options.
 *
 * Later, special flags may allow sub-windows to "steal" stuff from the
 * main window, including File dump (help), File dump (artifacts, uniques),
 * Character screen, Small scale map, Previous Messages, Store screen, etc.
 *
 * The "ctrl-i" (tab) command flips the "Display inven/equip" and "Display
 * equip/inven" flags for all windows.
 *
 * The "ctrl-g" command (or pseudo-command) should perhaps grab a snapshot
 * of the main screen into any interested windows.
 */
cptr window_flag_desc[32] =
{
	"Display inven/equip",
	"Display equip/inven",
	NULL,
	"Display character",
	"Show visible monsters",
	NULL,
	"Display messages",
	"Display overhead view",
	"Display monster recall",
	"Display object recall",
	NULL,
	"Display snap-shot",
	NULL,
	NULL,
	"Display borg messages",
	"Display borg status",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};


/* Names used for random artifact name generation */
cptr artifact_names_list =
	"adanedhel\n"
	"adurant\n"
	"aeglos\n"
	"aegnor\n"
	"aelin\n"
	"aeluin\n"
	"aerandir\n"
	"aerin\n"
	"agarwaen\n"
	"aglareb\n"
	"aglarond\n"
	"aglon\n"
	"ainulindale\n"
	"ainur\n"
	"alcarinque\n"
	"aldaron\n"
	"aldudenie\n"
	"almaren\n"
	"alqualonde\n"
	"aman\n"
	"amandil\n"
	"amarie\n"
	"amarth\n"
	"amlach\n"
	"amon\n"
	"amras\n"
	"amrod\n"
	"anach\n"
	"anar\n"
	"anarion\n"
	"ancalagon\n"
	"ancalimon\n"
	"anarrima\n"
	"andor\n"
	"andram\n"
	"androth\n"
	"anduin\n"
	"andunie\n"
	"anfauglir\n"
	"anfauglith\n"
	"angainor\n"
	"angband\n"
	"anghabar\n"
	"anglachel\n"
	"angrenost\n"
	"angrim\n"
	"angrist\n"
	"angrod\n"
	"anguirel\n"
	"annael\n"
	"annatar\n"
	"annon\n"
	"annuminas\n"
	"apanonar\n"
	"aradan\n"
	"aragorn\n"
	"araman\n"
	"aranel\n"
	"aranruth\n"
	"aranwe\n"
	"aras\n"
	"aratan\n"
	"aratar\n"
	"arathorn\n"
	"arda\n"
	"ard-galen\n"
	"aredhel\n"
	"ar-feiniel\n"
	"argonath\n"
	"arien\n"
	"armenelos\n"
	"arminas\n"
	"arnor\n"
	"aros\n"
	"arossiach\n"
	"arthad\n"
	"arvernien\n"
	"arwen\n"
	"ascar\n"
	"astaldo\n"
	"atalante\n"
	"atanamir\n"
	"atanatari\n"
	"atani\n"
	"aule\n"
	"avallone\n"
	"avari\n"
	"avathar\n"
	"balan\n"
	"balar\n"
	"balrog\n"
	"barad\n"
	"baragund\n"
	"barahir\n"
	"baran\n"
	"baranduin\n"
	"bar\n"
	"bauglir\n"
	"beleg\n"
	"belegaer\n"
	"belegost\n"
	"belegund\n"
	"beleriand\n"
	"belfalas\n"
	"belthil\n"
	"belthronding\n"
	"beor\n"
	"beraid\n"
	"bereg\n"
	"beren\n"
	"boromir\n"
	"boron\n"
	"bragollach\n"
	"brandir\n"
	"bregolas\n"
	"bregor\n"
	"brethil\n"
	"brilthor\n"
	"brithiach\n"
	"brithombar\n"
	"brithon\n"
	"cabed\n"
	"calacirya\n"
	"calaquendi\n"
	"calenardhon\n"
	"calion\n"
	"camlost\n"
	"caragdur\n"
	"caranthir\n"
	"carcharoth\n"
	"cardolan\n"
	"carnil\n"
	"celeborn\n"
	"celebrant\n"
	"celebrimbor\n"
	"celebrindal\n"
	"celebros\n"
	"celegorm\n"
	"celon\n"
	"cirdan\n"
	"cirith\n"
	"cirth\n"
	"ciryatan\n"
	"ciryon\n"
	"coimas\n"
	"corollaire\n"
	"crissaegrim\n"
	"cuarthal\n"
	"cuivienen\n"
	"culurien\n"
	"curufin\n"
	"curufinwe\n"
	"curunir\n"
	"cuthalion\n"
	"daedeloth\n"
	"daeron\n"
	"dagnir\n"
	"dagor\n"
	"dagorlad\n"
	"dairuin\n"
	"danwedh\n"
	"delduwath\n"
	"denethor\n"
	"dimbar\n"
	"dimrost\n"
	"dinen\n"
	"dior\n"
	"dirnen\n"
	"dolmed\n"
	"doriath\n"
	"dorlas\n"
	"dorthonion\n"
	"draugluin\n"
	"drengist\n"
	"duath\n"
	"duinath\n"
	"duilwen\n"
	"dunedain\n"
	"dungortheb\n"
	"earendil\n"
	"earendur\n"
	"earnil\n"
	"earnur\n"
	"earrame\n"
	"earwen\n"
	"echor\n"
	"echoriath\n"
	"ecthelion\n"
	"edain\n"
	"edrahil\n"
	"eglador\n"
	"eglarest\n"
	"eglath\n"
	"eilinel\n"
	"eithel\n"
	"ekkaia\n"
	"elbereth\n"
	"eldalie\n"
	"eldalieva\n"
	"eldamar\n"
	"eldar\n"
	"eledhwen\n"
	"elemmire\n"
	"elende\n"
	"elendil\n"
	"elendur\n"
	"elenna\n"
	"elentari\n"
	"elenwe\n"
	"elerrina\n"
	"elleth\n"
	"elmoth\n"
	"elostirion\n"
	"elrond\n"
	"elros\n"
	"elu\n"
	"eluchil\n"
	"elured\n"
	"elurin\n"
	"elwe\n"
	"elwing\n"
	"emeldir\n"
	"endor\n"
	"engrin\n"
	"engwar\n"
	"eol\n"
	"eonwe\n"
	"ephel\n"
	"erchamion\n"
	"ereb\n"
	"ered\n"
	"erech\n"
	"eregion\n"
	"ereinion\n"
	"erellont\n"
	"eressea\n"
	"eriador\n"
	"eru\n"
	"esgalduin\n"
	"este\n"
	"estel\n"
	"estolad\n"
	"ethir\n"
	"ezellohar\n"
	"faelivrin\n"
	"falas\n"
	"falathar\n"
	"falathrim\n"
	"falmari\n"
	"faroth\n"
	"fauglith\n"
	"feanor\n"
	"feanturi\n"
	"felagund\n"
	"finarfin\n"
	"finduilas\n"
	"fingolfin\n"
	"fingon\n"
	"finwe\n"
	"firimar\n"
	"formenos\n"
	"fornost\n"
	"frodo\n"
	"fuin\n"
	"fuinur\n"
	"gabilgathol\n"
	"galad\n"
	"galadriel\n"
	"galathilion\n"
	"galdor\n"
	"galen\n"
	"galvorn\n"
	"gandalf\n"
	"gaurhoth\n"
	"gelion\n"
	"gelmir\n"
	"gelydh\n"
	"gil\n"
	"gildor\n"
	"giliath\n"
	"ginglith\n"
	"girith\n"
	"glaurung\n"
	"glingal\n"
	"glirhuin\n"
	"gloredhel\n"
	"glorfindel\n"
	"golodhrim\n"
	"gondolin\n"
	"gondor\n"
	"gonnhirrim\n"
	"gorgoroth\n"
	"gorlim\n"
	"gorthaur\n"
	"gorthol\n"
	"gothmog\n"
	"guilin\n"
	"guinar\n"
	"guldur\n"
	"gundor\n"
	"gurthang\n"
	"gwaith\n"
	"gwareth\n"
	"gwindor\n"
	"hadhodrond\n"
	"hador\n"
	"haladin\n"
	"haldad\n"
	"haldan\n"
	"haldar\n"
	"haldir\n"
	"haleth\n"
	"halmir\n"
	"handir\n"
	"harad\n"
	"hareth\n"
	"hathaldir\n"
	"hathol\n"
	"haudh\n"
	"helcar\n"
	"helcaraxe\n"
	"helevorn\n"
	"helluin\n"
	"herumor\n"
	"herunumen\n"
	"hildorien\n"
	"himlad\n"
	"himring\n"
	"hirilorn\n"
	"hisilome\n"
	"hithaeglir\n"
	"hithlum\n"
	"hollin\n"
	"huan\n"
	"hunthor\n"
	"huor\n"
	"hurin\n"
	"hyarmendacil\n"
	"hyarmentir\n"
	"iant\n"
	"iaur\n"
	"ibun\n"
	"idril\n"
	"illuin\n"
	"ilmare\n"
	"ilmen\n"
	"iluvatar\n"
	"imlach\n"
	"imladris\n"
	"indis\n"
	"ingwe\n"
	"irmo\n"
	"isil\n"
	"isildur\n"
	"istari\n"
	"ithil\n"
	"ivrin\n"
	"kelvar\n"
	"kementari\n"
	"ladros\n"
	"laiquendi\n"
	"lalaith\n"
	"lamath\n"
	"lammoth\n"
	"lanthir\n"
	"laurelin\n"
	"leithian\n"
	"legolin\n"
	"lembas\n"
	"lenwe\n"
	"linaewen\n"
	"lindon\n"
	"lindorie\n"
	"loeg\n"
	"lomelindi\n"
	"lomin\n"
	"lomion\n"
	"lorellin\n"
	"lorien\n"
	"lorindol\n"
	"losgar\n"
	"lothlann\n"
	"lothlorien\n"
	"luin\n"
	"luinil\n"
	"lumbar\n"
	"luthien\n"
	"mablung\n"
	"maedhros\n"
	"maeglin\n"
	"maglor\n"
	"magor\n"
	"mahanaxar\n"
	"mahtan\n"
	"maiar\n"
	"malduin\n"
	"malinalda\n"
	"mandos\n"
	"manwe\n"
	"mardil\n"
	"melian\n"
	"melkor\n"
	"menegroth\n"
	"meneldil\n"
	"menelmacar\n"
	"meneltarma\n"
	"minas\n"
	"minastir\n"
	"mindeb\n"
	"mindolluin\n"
	"mindon\n"
	"minyatur\n"
	"mirdain\n"
	"miriel\n"
	"mithlond\n"
	"mithrandir\n"
	"mithrim\n"
	"mordor\n"
	"morgoth\n"
	"morgul\n"
	"moria\n"
	"moriquendi\n"
	"mormegil\n"
	"morwen\n"
	"nahar\n"
	"naeramarth\n"
	"namo\n"
	"nandor\n"
	"nargothrond\n"
	"narog\n"
	"narsil\n"
	"narsilion\n"
	"narya\n"
	"nauglamir\n"
	"naugrim\n"
	"ndengin\n"
	"neithan\n"
	"neldoreth\n"
	"nenar\n"
	"nenning\n"
	"nenuial\n"
	"nenya\n"
	"nerdanel\n"
	"nessa\n"
	"nevrast\n"
	"nibin\n"
	"nienna\n"
	"nienor\n"
	"nimbrethil\n"
	"nimloth\n"
	"nimphelos\n"
	"nimrais\n"
	"nimras\n"
	"ningloron\n"
	"niniel\n"
	"ninniach\n"
	"ninquelote\n"
	"niphredil\n"
	"nirnaeth\n"
	"nivrim\n"
	"noegyth\n"
	"nogrod\n"
	"noldolante\n"
	"noldor\n"
	"numenor\n"
	"nurtale\n"
	"obel\n"
	"ohtar\n"
	"oiolosse\n"
	"oiomure\n"
	"olorin\n"
	"olvar\n"
	"olwe\n"
	"ondolinde\n"
	"orfalch\n"
	"ormal\n"
	"orocarni\n"
	"orodreth\n"
	"orodruin\n"
	"orome\n"
	"oromet\n"
	"orthanc\n"
	"osgiliath\n"
	"osse\n"
	"ossiriand\n"
	"palantir\n"
	"pelargir\n"
	"pelori\n"
	"periannath\n"
	"quendi\n"
	"quenta\n"
	"quenya\n"
	"radagast\n"
	"radhruin\n"
	"ragnor\n"
	"ramdal\n"
	"rana\n"
	"rathloriel\n"
	"rauros\n"
	"region\n"
	"rerir\n"
	"rhovanion\n"
	"rhudaur\n"
	"rhun\n"
	"rhunen\n"
	"rian\n"
	"ringil\n"
	"ringwil\n"
	"romenna\n"
	"rudh\n"
	"rumil\n"
	"saeros\n"
	"salmar\n"
	"saruman\n"
	"sauron\n"
	"serech\n"
	"seregon\n"
	"serinde\n"
	"shelob\n"
	"silmarien\n"
	"silmaril\n"
	"silpion\n"
	"sindar\n"
	"singollo\n"
	"sirion\n"
	"soronume\n"
	"sul\n"
	"sulimo\n"
	"talath\n"
	"taniquetil\n"
	"tar\n"
	"taras\n"
	"tarn\n"
	"tathren\n"
	"taur\n"
	"tauron\n"
	"teiglin\n"
	"telchar\n"
	"telemnar\n"
	"teleri\n"
	"telperion\n"
	"telumendil\n"
	"thalion\n"
	"thalos\n"
	"thangorodrim\n"
	"thargelion\n"
	"thingol\n"
	"thoronath\n"
	"thorondor\n"
	"thranduil\n"
	"thuringwethil\n"
	"tilion\n"
	"tintalle\n"
	"tinuviel\n"
	"tirion\n"
	"tirith\n"
	"tol\n"
	"tulkas\n"
	"tumhalad\n"
	"tumladen\n"
	"tuna\n"
	"tuor\n"
	"turambar\n"
	"turgon\n"
	"turin\n"
	"uial\n"
	"uilos\n"
	"uinen\n"
	"ulairi\n"
	"ulmo\n"
	"ulumuri\n"
	"umanyar\n"
	"umarth\n"
	"umbar\n"
	"ungoliant\n"
	"urthel\n"
	"uruloki\n"
	"utumno\n"
	"vaire\n"
	"valacirca\n"
	"valandil\n"
	"valaquenta\n"
	"valar\n"
	"valaraukar\n"
	"valaroma\n"
	"valier\n"
	"valimar\n"
	"valinor\n"
	"valinoreva\n"
	"valmar\n"
	"vana\n"
	"vanyar\n"
	"varda\n"
	"vasa\n"
	"vilya\n"
	"vingilot\n"
	"vinyamar\n"
	"voronwe\n"
	"wethrin\n"
	"wilwarin\n"
	"yavanna\n"
	;


martial_arts ma_blows[MAX_MA] =
{
	{ "You punch %s.", 1, 0, 2, 4, 0, 0 },
	{ "You kick %s.", 2, 0, 2, 6, 0, 0 },
	{ "You strike %s.", 3, 0, 2, 7, 0, 0 },
	{ "You hit %s with your knee.", 5, 5, 4, 3, MA_KNEE, 0 },
	{ "You hit %s with your elbow.", 7, 5, 2, 8, 0, 0 },
	{ "You butt %s.", 9, 10, 4, 5, 0, 0 },
	{ "You kick %s.", 11, 10, 6, 4, MA_SLOW, 0 },
	{ "You uppercut %s.", 13, 12, 8, 4, MA_STUN, 6 },
	{ "You double-kick %s.", 16, 15, 10, 4, MA_STUN, 8 },
	{ "You hit %s with a Cat's Claw.", 20, 20, 10, 5, 0, 0 },
	{ "You hit %s with a jump kick.", 25, 25, 10, 6, MA_STUN, 10 },
	{ "You hit %s with an Eagle's Claw.", 29, 25, 12, 6, 0, 0 },
	{ "You hit %s with a circle kick.", 33, 30, 12, 8, MA_STUN, 10 },
	{ "You hit %s with an Iron Fist.", 37, 35, 16, 8, MA_STUN, 10 },
	{ "You hit %s with a flying kick.", 41, 35, 16, 10, MA_STUN, 12 },
	{ "You hit %s with a Dragon Fist.", 45, 35, 20, 10, MA_STUN, 16 },
	{ "You hit %s with a Crushing Blow.", 48, 35, 20, 12, MA_STUN, 18 },
};

/*
 *   cptr    desc;      A verbose attack description
 *   int     min_level; Minimum level to use
 *   int     chance;    Chance of 'success
 *   int     dd;        Damage dice
 *   int     ds;        Damage sides
 *   s16b    effect;    Special effects
 *   s16b    power;     Special effects power
 */
martial_arts bear_blows[MAX_BEAR] =
{
	{ "You claw %s.", 1, 0, 3, 4, MA_STUN, 4 },
	{ "You swat %s.", 4, 0, 4, 4, MA_WOUND, 20 },
	{ "You bite %s.", 9, 2, 4, 4, MA_WOUND, 30 },
	{ "You hug %s.", 15, 5, 6, 4, MA_FULL_SLOW, 0 },
	{ "You swat and rake %s.", 25, 10, 6, 5, MA_STUN | MA_WOUND, 10 },
	{ "You hug and claw %s.", 30, 15, 6, 6, MA_FULL_SLOW | MA_WOUND, 60 },
	{ "You double swat %s.", 35, 20, 9, 7, MA_STUN | MA_WOUND, 20 },
	{ "You double swat and rake %s.", 40, 25, 10, 10, MA_STUN | MA_WOUND, 25 },
};


magic_power mindcraft_powers[MAX_MINDCRAFT_POWERS] =
{
	/* Level gained,  cost,  %fail,  name,  desc */
	{
		/* Det. monsters/traps */
		1, 1, 15,
		"Precognition",
		"Detect monsters, traps and level layout and lights up at higher levels."
	},
	{
		/* ~MM */
		2, 1, 20,
		"Neural Blast",
		"Blast the minds of your foes."
	},
	{
		/* Phase/Between gate */
		3, 2, 25,
		"Minor Displacement",
		"Short distance teleportation"
	},
	{
		/* Tele. Self / All */
		7, 6, 35,
		"Major Displacement",
		"Teleport you and others at high levels."
	},
	{
		9, 7, 50,
		"Domination",
		"Charm monsters"
	},
	{
		/* Telekinetic "bolt" */
		11, 7, 30,
		"Pulverise",
		"Fires a bolt of pure sound."
	},
	{
		/* Psychic/physical defenses */
		13, 12, 50,
		"Character Armour",
		"Sets up physical/elemental shield."
	},
	{
		15, 12, 60,
		"Psychometry",
		"Identifies objects."
	},
	{
		/* Ball -> LOS */
		18, 10, 45,
		"Mind Wave",
		"Projects psi waves to crush the minds of your foes."
	},
	{
		23, 15, 50,
		"Adrenaline Channeling",
		"Heals you, cures you and speeds you."
	},
	{
		/* Convert enemy HP to mana */
		25, 10, 40,
		"Psychic Drain",
		"Drain your foes' life into your mana reserves"
	},
	{
		/* Ball -> LOS */
		28, 20, 45,
		"Telekinetic Wave",
		"Powerful wave of pure telekinetic forces."
	},
};
	
magic_power necro_powers[MAX_NECRO_POWERS] =
		{
			/* Level gained,  cost,  %fail,  name,  desc */
			{
				/* Bolt/beam/ball/LOS of stun/scare */
				1, 2, 10,
				"Horrify",
				"Calls upon the darkness to stun and scare your foes."
			},
	{
		/* Ball */
		5, 6, 20,
		"Raise Dead",
		"Brings back your foes in the form of various undead.  Also, can heal monsters."
	},
	{
		/* Summons weapon */
		12, 20, 25,
		"Necromantic Teeth",
		"Conjures a temporary vampiric weapon."
	},
	{
		/* Heals when killing a monster */
		20, 10, 25,
		"Absorb Soul",
		"Gives back some life for each kill."
	},
	{
		/* Bolt */
		30, 15, 20,
		"Vampirism",
		"Drain the life of your foes into your own."
	},
	{
		/* The Death word, always bolt put your HP to 1 */
		35, 100, 25,
		"Death",
		"Instantly kills your opponent and you, turning yourself into an undead."
	},
};

magic_power mimic_powers[MAX_MIMIC_POWERS] =
{
	/* Level gained,  cost,  %fail,  name */
	{
		/* Use a book of lore */
		1, 2, 0,
		"Mimic",
		"Lets you use the powers of a Cloak of Mimicry."
	},
	{
		/* Invisibility */
		10, 6, 20,
		"Invisibility",
		"Hides you from the sight of mortals."
	},
	{
		/* +1 pair of legs */
		25, 20, 25,
		"Legs Mimicry",
		"Temporarily provides a new pair of legs."
	},
	{
		/* wall form */
		30, 40, 30,
		"Wall Mimicry",
		"Temporarily lets you walk in walls, and ONLY in walls."
	},
	{
		/* +1 pair of arms, +1 weapon */
		35, 100, 40,
		"Arms Mimicry",
		"Temporarily provides a new pair of arms."
	},
};

magic_power symbiotic_powers[MAX_SYMBIOTIC_POWERS] =
{
	/* Level gained,  cost,  %fail,  name */
	{
		1, 1, 0,
		"Hypnotise",
		"Hypnotise a non-moving pet to allow you to enter symbiosis(wear) with it."
	},
	{
		1, 1, 0,
		"Release",
		"Release an hypnotised pet."
	},
	{
		3, 2, 10,
		"Charm Never-Moving",
		"Tries to charm a never-moving monster."
	},
	{
		5, 5, 20,
		"Life Share",
		"Evens out your life with your symbiote."
	},
	{
		10, 10, 20,
		"Use Minor Powers",
		"Allows you to use some of the powers of your symbiote."
	},
	{
		15, 14, 25,
		"Heal Symbiote",
		"Heals your symbiotic monster."
	},
	{
		25, 30, 40,
		"Use major powers",
		"Allows you to use all the powers of your symbiote."
	},
	{
		30, 35, 40,
		"Summon Never-Moving Pet",
		"Summons a never-moving pet."
	},
	{
		40, 60, 70,
		"Force Symbiosis",
		"Allows you to use all the powers of a monster in your line of sight."
	},
};


/*
 * Name and description (max. 10 lines) of the gods.
 * Only the first four lines are printed at birth. 
 */

deity_type deity_info[MAX_GODS] =
{
	{
		{ MODULE_TOME, MODULE_THEME, -1, },
		"Nobody",
		{
			"Atheist",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
		},
	},
	{
		{ MODULE_TOME, MODULE_THEME, -1, },
		"Eru Iluvatar",
		{
			"He is the supreme god, he created the world, and most of its inhabitants.",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
		},
	},
	{
		{ MODULE_TOME, MODULE_THEME, -1, },
		"Manwe Sulimo",
		{
			"He is the king of the Valar, most powerful of them after Melkor.",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
		},
	},
	{
		{ MODULE_TOME, MODULE_THEME, -1, },
		"Tulkas",
		{
			"He is the last of the Valar that came to the world, and the fiercest fighter.",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
		},
	},
	{
		{ MODULE_TOME, MODULE_THEME, -1, },
		"Melkor Bauglir",
		{
			"He is the most powerful of the Valar. He became corrupted and he's now ",
			"the greatest threat of Arda, he is also known as Morgoth, the dark enemy.",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
		},
	},
	{
		{ MODULE_TOME, MODULE_THEME, -1, },
		"Yavanna Kementari",
		{
			"She is the Vala of nature, protectress of the great forests of Middle-earth.",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
		},
	},
	{
		{ MODULE_THEME, -1, },
		"Aule the Smith",
		{
			"Aule is a smith, and the creator of the Dwarves.",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
		},
	},
	{
		{ MODULE_THEME, -1, },
		"Varda Elentari",
		{
			"The Queen of the Stars. In light is her power and joy.",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
		},
	},
	{
		{ MODULE_THEME, -1, },
		"Ulmo",
		{
			"Ulmo is called Lord of Waters, he rules all that is water on Arda.",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
		},
	},
	{
		{ MODULE_THEME, -1, },
		"Mandos",
		{
			"The Doomsman of the Valar and keeper of the slain.",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
		},
	},
};

/* jk - to hit, to dam, to ac, to stealth, to disarm, to saving throw */
/* this concept is taken from Adom, where Thomas Biskup thought it out, */
/* as far as I know. */
tactic_info_type tactic_info[9] =
{
	/* hit  dam   ac  stl  sav */
	{  -10, -10, +15,  +3, +14, "coward" },
	{   -8,  -8, +10,  +2,  +9, "meek" },
	{   -4,  -4,  +5,  +1,  +5, "wary" },
	{   -2,  -2,  +2,  +1,  +2, "careful" },
	{    0,   0,   0,   0,   0, "normal" },
	{    2,   2,  -2,  -1,  -3, "confident" },
	{    4,   4,  -5,  -2,  -7, "aggressive" },
	{    6,   6, -10,  -3, -12, "furious" },
	{    8,  12, -25,  -5, -18, "berserker" }
};

/*
 * Random artifact activations.
 */
activation activation_info[MAX_T_ACT] =
{
	{ "death", 0, ACT_DEATH },
	{ "ruination", 0, ACT_RUINATION },
	{ "destruction", 1000, ACT_DESTRUC },
	{ "stupidity", 0, ACT_UNINT },
	{ "weakness", 0, ACT_UNSTR },
	{ "unhealth", 0, ACT_UNCON },
	{ "ugliness", 0, ACT_UNCHR },
	{ "clumsiness", 0, ACT_UNDEX },
	{ "naivete", 0, ACT_UNWIS },
	{ "stat loss", 0, ACT_STATLOSS },
	{ "huge stat loss", 0, ACT_HISTATLOSS },
	{ "experience loss", 0, ACT_EXPLOSS },
	{ "huge experience loss", 0, ACT_HIEXPLOSS },
	{ "teleportation", 1000, ACT_TELEPORT },
	{ "monster summoning", 5, ACT_SUMMON_MONST },
	{ "paralyzation", 0, ACT_PARALYZE },
	{ "hallucination", 100, ACT_HALLU },
	{ "poisoning", 0, ACT_POISON },
	{ "hunger", 0, ACT_HUNGER },
	{ "stun", 0, ACT_STUN },
	{ "cuts", 0, ACT_CUTS },
	{ "paranoia", 0, ACT_PARANO },
	{ "confusion", 0, ACT_CONFUSION },
	{ "blindness", 0, ACT_BLIND },
	{ "pet summoning", 1010, ACT_PET_SUMMON },
	{ "cure paralyzation", 5000, ACT_CURE_PARA },
	{ "cure hallucination", 1000, ACT_CURE_HALLU },
	{ "cure poison", 1000, ACT_CURE_POIS },
	{ "cure hunger", 1000, ACT_CURE_HUNGER },
	{ "cure stun", 1000, ACT_CURE_STUN },
	{ "cure cut", 1000, ACT_CURE_CUTS },
	{ "cure fear", 1000, ACT_CURE_FEAR },
	{ "cure confusion", 1000, ACT_CURE_CONF },
	{ "cure blindness", 1000, ACT_CURE_BLIND },
	{ "cure light wounds", 500, ACT_CURE_LW },
	{ "cure serious wounds", 750, ACT_CURE_MW },
	{ "cure critical wounds", 1000, ACT_CURE_700 },
	{ "curing", 1100, ACT_CURING },
	{ "genocide", 5000, ACT_GENOCIDE },
	{ "mass genocide", 10000, ACT_MASS_GENO },
	{ "restoration", 2000, ACT_REST_ALL },
	{ "light", 1000, ACT_LIGHT },
	{ "darkness", 0, ACT_DARKNESS },
	{ "teleportation", 1000, ACT_TELEPORT },
	{ "level teleportation", 500, ACT_LEV_TELE },
	{ "acquirement", 30000, ACT_ACQUIREMENT },
	{ "something weird", 50, ACT_WEIRD },
	{ "aggravation", 0, ACT_AGGRAVATE },
	{ "corruption", 100, ACT_MUT },
	{ "cure insanity", 2000, ACT_CURE_INSANITY },
	{ "light absortion", 800, ACT_LIGHT_ABSORBTION },
};

/*
 * Possible movement type.
 */
move_info_type move_info[9] =
{
	/*        speed, searching, stealth, perception */
	{ -10, 17, 4, 20, "slug-like"},
	{ -8, 12, 4, 16, "very slow"},
	{ -6, 8, 3, 10, "slow"},
	{ -3, 4, 2, 6, "leisurely"},
	{ 0, 0, 0, 0, "normal"},
	{ 1, -4, -1, -4, "brisk"},
	{ 2, -6, -4, -8, "fast"},
	{ 3, -10, -7, -14, "very fast"},
	{ 4, -16, -10, -20, "running"}
};

/*
 * Possible inscriptions type.
 */
inscription_info_type inscription_info[MAX_INSCRIPTIONS] =
{
        {       /* Padding; 0 index is used to signify "no inscription" */
		"",
		0,
		0,
	},
	{       /* Light up the room(Adunaic) */
		"ure nimir",   /* sun shine */
		INSCRIP_EXEC_ENGRAVE | INSCRIP_EXEC_WALK | INSCRIP_EXEC_MONST_WALK,
		30,
	},
	{       /* Darkness in room(Adunaic) */
		"lomi gimli",   /* night stars */
		INSCRIP_EXEC_ENGRAVE | INSCRIP_EXEC_WALK | INSCRIP_EXEC_MONST_WALK,
		10,
	},
	{       /* Storm(Adunaic) */
		"dulgi bawiba",   /* black winds */
		INSCRIP_EXEC_ENGRAVE | INSCRIP_EXEC_WALK | INSCRIP_EXEC_MONST_WALK,
		40,
	},
	{       /* Protection(Sindarin) */
		"pedo mellon a minno",   /* say friend and enter */
		INSCRIP_EXEC_MONST_WALK,
		8,
	},
	{       /* Dwarves summoning(Khuzdul) */
		"Baruk Khazad! Khazad aimenu!",   /* Axes of the Dwarves, the Dwarves are upon you! */
		INSCRIP_EXEC_ENGRAVE,
		100,
	},
	{       /* Open Chasm(Nandorin) */
		"dunna hrassa",   /* black precipice */
		INSCRIP_EXEC_MONST_WALK,
		50,
	},
	{       /* Blast of Black Fire(Orcish) */
		"burz ghash ronk",   /* black fire pool */
		INSCRIP_EXEC_ENGRAVE | INSCRIP_EXEC_WALK | INSCRIP_EXEC_MONST_WALK,
		60,
	},
};

/*
 * Inscriptions for pseudo-id
 */
cptr sense_desc[] =
{
	"whoops",
	"cursed",
	"average",
	"good",
	"good",
	"excellent",
	"worthless",
	"terrible",
	"special",
	"broken",
	""
};

/*
 * Flag groups used for art creation, level gaining weapons, ...
 * -----
 * Name,
 * Price,
 * Flags 1,
 * Flags 2,
 * Flags 3,
 * Flags 4,
 * ESP,
 */
extern std::vector<flags_group> const &flags_groups()
{
	static auto *instance = new std::vector<flags_group> {
		flags_group {
			"Fire",
			TERM_L_RED,
			1,
			TR_SLAY_UNDEAD | TR_BRAND_FIRE | TR_RES_FIRE |
	                TR_SH_FIRE | TR_LITE1 | TR_IGNORE_FIRE,
		},
		flags_group {
			"Cold",
			TERM_WHITE,
			1,
			TR_SLAY_DRAGON | TR_SLAY_DEMON | TR_BRAND_COLD | TR_RES_COLD |
	                TR_INVIS | TR_SLOW_DIGEST | TR_IGNORE_COLD,
		},
		flags_group {
			"Acid",
			TERM_GREEN,
			3,
			TR_SLAY_ANIMAL | TR_IMPACT | TR_TUNNEL |
	                TR_BRAND_ACID | TR_RES_ACID | TR_IGNORE_ACID,
		},
		flags_group {
			"Lightning",
			TERM_L_BLUE,
			1,
			TR_SLAY_EVIL | TR_BRAND_ELEC | TR_RES_ELEC |
	                TR_IGNORE_ELEC | TR_SH_ELEC | TR_TELEPORT,
		},
		flags_group {
			"Poison",
			TERM_L_GREEN,
			2,
			TR_CHR | TR_VAMPIRIC | TR_SLAY_ANIMAL | TR_BRAND_POIS |
	                TR_SUST_CHR | TR_RES_POIS | TR_DRAIN_EXP |
	                ESP_TROLL | ESP_GIANT,
		},
		flags_group {
			"Air",
			TERM_BLUE,
			5,
			TR_WIS | TR_STEALTH | TR_INFRA | TR_SPEED |
			TR_RES_LITE | TR_RES_DARK | TR_RES_BLIND | TR_SUST_WIS |
	                TR_FEATHER | TR_SEE_INVIS | TR_BLESSED |
	                ESP_GOOD,
		},
		flags_group {
			"Earth",
			TERM_L_UMBER,
			5,
			TR_STR | TR_CON | TR_TUNNEL | TR_BLOWS | TR_SLAY_TROLL |
			TR_SLAY_GIANT | TR_IMPACT | TR_SUST_STR | TR_SUST_CON |
	                TR_FREE_ACT | TR_RES_FEAR | TR_RES_SHARDS | TR_REGEN |
			ESP_TROLL | ESP_GIANT,
		},
		flags_group {
			"Mind",
			TERM_YELLOW,
			7,
			TR_INT | TR_SUST_INT | TR_RES_CONF | TR_RES_FEAR |
			ESP_ORC | ESP_TROLL | ESP_GIANT | ESP_ANIMAL | ESP_UNIQUE | ESP_SPIDER | ESP_DEMON,
		},
		flags_group {
			"Shield",
			TERM_RED,
			7,
			TR_DEX | TR_SUST_DEX | TR_INVIS | TR_REFLECT |
			TR_HOLD_LIFE | TR_RES_SOUND | TR_RES_NEXUS |
	                TR_REGEN,
		},
		flags_group {
			"Chaos",
			TERM_VIOLET,
			7,
	                TR_CHAOTIC | TR_IMPACT | TR_RES_CHAOS | TR_RES_DISEN | TR_REGEN |
			ESP_ALL,
		},
		flags_group {
			"Magic",
			TERM_L_BLUE,
			10,
			TR_MANA | TR_SPELL | TR_RES_CHAOS | TR_RES_DISEN | TR_WRAITH |
			TR_PRECOGNITION | TR_FLY | TR_CLONE,
		},
		flags_group {
			"Antimagic",
			TERM_L_DARK,
			10,
			TR_VAMPIRIC | TR_CHAOTIC | TR_BLOWS | TR_SPEED | TR_LIFE |
			TR_REFLECT | TR_FREE_ACT | TR_HOLD_LIFE | TR_NO_MAGIC |
			TR_NO_TELE | TR_SEE_INVIS | TR_ANTIMAGIC_50,
		}
	};

	return *instance;
};

/* Powers */
power_type powers_type[POWER_MAX] =
{
	{
		"spit acid",
		"You can spit acid.",
		"You gain the ability to spit acid.",
		"You lose the ability to spit acid.",
		9, 9, A_DEX, 15,
	},
	{
		"fire breath",
		"You can breath fire.",
		"You gain the ability to breathe fire.",
		"You lose the ability to breathe fire.",
		20, 10, A_CON, 18,
	},
	{
		"hypnotic gaze",
		"Your gaze is hypnotic.",
		"Your eyes look mesmerising...",
		"Your eyes look uninteresting.",
		12, 12, A_CHR, 18,
	},
	{
		"telekinesis",
		"You are telekinetic.",
		"You gain the ability to move objects telekinetically.",
		"You lose the ability to move objects telekinetically.",
		9, 9, A_WIS, 14,
	},
	{
		"teleport",
		"You can teleport at will.",
		"You gain the power of teleportation at will.",
		"You lose the power of teleportation at will.",
		7, 7, A_WIS, 15,
	},
	{
		"mind blast",
		"You can mind blast your enemies.",
		"You gain the power of Mind Blast.",
		"You lose the power of Mind Blast.",
		5, 3, A_WIS, 15,
	},
	{
		"emit radiation",
		"You can emit hard radiation at will.",
		"You start emitting hard radiation.",
		"You stop emitting hard radiation.",
		15, 15, A_CON, 14,
	},
	{
		"vampiric drain",
		"You can drain life from a foe.",
		"You become vampiric.",
		"You are no longer vampiric.",
		4, 5, A_CON, 9,
	},
	{
		"smell metal",
		"You can smell nearby precious metal.",
		"You smell a metallic odour.",
		"You no longer smell a metallic odour.",
		3, 2, A_INT, 12,
	},
	{
		"smell monsters",
		"You can smell nearby monsters.",
		"You smell filthy monsters.",
		"You no longer smell filthy monsters.",
		5, 4, A_INT, 15,
	},
	{
		"blink",
		"You can teleport yourself short distances.",
		"You gain the power of minor teleportation.",
		"You lose the power of minor teleportation.",
		3, 3, A_WIS, 12,
	},
	{
		"eat rock",
		"You can consume solid rock.",
		"The walls look delicious.",
		"The walls look unappetising.",
		8, 12, A_CON, 18,
	},
	{
		"swap position",
		"You can switch locations with another being.",
		"You feel like walking a mile in someone else's shoes.",
		"You feel like staying in your own shoes.",
		15, 12, A_DEX, 16,
	},
	{
		"shriek",
		"You can emit a horrible shriek.",
		"Your vocal cords get much tougher.",
		"Your vocal cords get much weaker.",
		4, 4, A_CON, 6,
	},
	{
		"illuminate",
		"You can emit bright light.",
		"You can light up rooms with your presence.",
		"You can no longer light up rooms with your presence.",
		3, 2, A_INT, 10,
	},
	{
		"detect curses",
		"You can feel the danger of evil magic.",
		"You can feel evil magic.",
		"You can no longer feel evil magic.",
		7, 14, A_WIS, 14,
	},
	{
		"berserk",
		"You can drive yourself into a berserk frenzy.",
		"You feel a controlled rage.",
		"You no longer feel a controlled rage.",
		8, 8, A_STR, 14,
	},
	{
		"polymorph",
		"You can polymorph yourself at will.",
		"Your body seems mutable.",
		"Your body seems stable.",
		18, 20, A_CON, 18,
	},
	{
		"Midas touch",
		"You can turn ordinary items to gold.",
		"You gain the Midas touch.",
		"You lose the Midas touch.",
		10, 5, A_INT, 12,
	},
	{
		"grow mold",
		"You can cause mold to grow near you.",
		"You feel a sudden affinity for mold.",
		"You feel a sudden dislike for mold.",
		1, 6, A_CON, 14,
	},
	{
		"resist elements",
		"You can harden yourself to the ravages of the elements.",
		"You feel like you can protect yourself.",
		"You feel like you might be vulnerable.",
		10, 12, A_CON, 12,
	},
	{
		"earthquake",
		"You can bring down the dungeon around your ears.",
		"You gain the ability to wreck the dungeon.",
		"You lose the ability to wreck the dungeon.",
		12, 12, A_STR, 16,
	},
	{
		"eat magic",
		"You can consume magic energy for your own use.",
		"Your magic items look delicious.",
		"Your magic items no longer look delicious.",
		17, 1, A_WIS, 15,
	},
	{
		"weigh magic",
		"You can feel the strength of the magics affecting you.",
		"You feel you can better understand the magic around you.",
		"You no longer sense magic.",
		6, 6, A_INT, 10,
	},
	{
		"sterilise",
		"You can cause mass impotence.",
		"You can give everything around you a headache.",
		"You hear a massed sigh of relief.",
		20, 40, A_CHR, 18,
	},
	{
		"panic hit",
		"You can run for your life after hitting something.",
		"You suddenly understand how thieves feel.",
		"You no longer feel jumpy.",
		10, 12, A_DEX, 14,
	},
	{
		"dazzle",
		"You can emit confusing, blinding radiation.",
		"You gain the ability to emit dazzling lights.",
		"You lose the ability to emit dazzling lights.",
		7, 15, A_CHR, 8,
	},
	{
		"spear of darkness",
		"You can create a spear of darkness.",
		"An illusory spear of darkness appears in your hand.",
		"The spear of darkness disappear.",
		7, 10, A_WIS, 9,
	},
	{
		"recall",
		"You can travel between towns and the depths.",
		"You feel briefly homesick, but it passes.",
		"You feel briefly homesick.",
		17, 50, A_INT, 16,
	},
	{
		"banish evil",
		"You can send evil creatures directly to the Nether Realm.",
		"You feel a holy wrath fill you.",
		"You no longer feel a holy wrath.",
		25, 25, A_WIS, 18,
	},
	{
		"cold touch",
		"You can freeze things with a touch.",
		"Your hands get very cold.",
		"Your hands warm up.",
		2, 2, A_CON, 11,
	},
	{
		"throw object",
		"You can hurl objects with great force.",
		"Your throwing arm feels much stronger.",
		"Your throwing arm feels much weaker.",
		1, 10, A_STR, 6,
	},
	{
		"find secret passages",
		"You can use secret passages.",
		"You suddenly notice lots of hidden ways.",
		"You no longer can use hidden ways.",
		15, 15, A_DEX, 12,
	},
	{
		"detect doors and traps",
		"You can detect hidden doors and traps.",
		"You develop an affinity for traps.",
		"You no longer can detect hidden doors and traps.",
		5, 3, A_WIS, 10,
	},
	{
		"create food",
		"You can create food.",
		"Your cooking skills greatly improve.",
		"Your cooking skills return to a normal level.",
		15, 10, A_INT, 10,
	},
	{
		"remove fear",
		"You can embolden yourself.",
		"You feel your fears lessening.",
		"You feel your fears growing again.",
		3, 5, A_WIS, 8,
	},
	{
		"set explosive rune",
		"You can set explosive runes.",
		"You suddenly understand how explosive runes work.",
		"You suddenly forget how explosive runes work.",
		25, 35, A_INT, 15,
	},
	{
		"stone to mud",
		"You can destroy walls.",
		"You can destroy walls.",
		"You cannot destroy walls anymore.",
		20, 10, A_STR, 12,
	},
	{
		"poison dart",
		"You can throw poisoned darts.",
		"You get an infinite supply of poisoned darts.",
		"You lose your infinite supply of poisoned darts.",
		12, 8, A_DEX, 14,
	},
	{
		"magic missile",
		"You can cast magic missiles.",
		"You suddenly understand the basics of magic.",
		"You forget the basics of magic.",
		2, 2, A_INT, 9,
	},
	{
		"grow trees",
		"You can grow trees.",
		"You feel an affinity for trees.",
		"You no longer feel an affinity for trees.",
		2, 6, A_CHR, 3,
	},
	{
		"cold breath",
		"You can breath cold.",
		"You gain the ability to breathe cold.",
		"You lose the ability to breathe cold.",
		20, 10, A_CON, 18,
	},
	{
		"chaos breath",
		"You can breath chaos.",
		"You gain the ability to breathe chaos.",
		"You lose the ability to breathe chaos.",
		20, 10, A_CON, 18,
	},
	{
		"elemental breath",
		"You can breath the elements.",
		"You gain the ability to breathe the elements.",
		"You lose the ability to breathe the elements.",
		20, 10, A_CON, 18,
	},
	{
		"change the world",
		"You can wreck the world around you.",
		"You gain the ability to wreck the world.",
		"You lose the ability to wreck the world.",
		1, 30, A_CHR, 6,
	},
	{
		"scare monster",
		"You can scare monsters.",
		"You gain the ability to scare monsters.",
		"You lose the ability to scare monsters.",
		4, 3, A_INT, 3,
	},
	{
		"restore life",
		"You can restore lost life forces.",
		"You gain the ability to restore your life force.",
		"You lose the ability to restore your life force.",
		30, 30, A_WIS, 18,
	},
	{
		"summon monsters",
		"You can call upon monsters.",
		"You gain the ability to call upon monsters.",
		"You lose the ability to call upon monsters.",
		0, 0, 0, 0,
	},
	{
		"necromantic powers",
		"You can use the foul necromantic magic.",
		"You gain the ability to use the foul necromantic magic.",
		"You lose the ability to use the foul necromantic magic.",
		0, 0, 0, 0,
	},
	{
		"Rohan Knight's Powers",
		"You can use rohir powers.",
		"You gain the ability to use rohir powers.",
		"You lose the ability to use rohir powers.",
		0, 0, 0, 0,
	},
	{
		"Thunderlord's Powers",
		"You can use thunderlords powers.",
		"You gain the ability to use thunderlords powers.",
		"You lose the ability to use thunderlords powers.",
		0, 0, 0, 0,
	},
	{
		"Death Mold's Powers",
		"You can use the foul deathmold magic.",
		"You gain the ability to use the foul deathmold magic.",
		"You lose the ability to use the foul deathmold magic.",
		0, 0, 0, 0,
	},
	{
		"Hypnotise Pet",
		"You can mystify pets.",
		"You gain the ability to mystify pets.",
		"You lose the ability to mystify pets.",
		0, 0, 0, 0,
	},
	{
		"Awaken Hypnotised Pet",
		"You can wake up a pet.",
		"You gain the ability to wake up a pet.",
		"You lose the ability to wake up a pet.",
		0, 0, 0, 0,
	},
	{
		"Incarnate",
		"You can incarnate into a body.",
		"You feel the need to get a body.",
		"You no longer feel the need for a new body.",
		0, 0, 0, 0,
	},
	{
		"magic map",
		"You can sense what is beyond walls.",
		"You feel you can sense what is beyond walls.",
		"You no longer can sense what is beyond walls.",
		7, 10, A_WIS, 15,
	},
	{
		"lay trap",
		"You can lay monster traps.",
		"You suddenly understand how rogues work.",
		"You no longer understand how rogues work.",
		1, 1, A_DEX, 1,
	},
	{
		"notused", /* Merchant abilities; no longer used, but want to
			    * avoid having to move all potential places where
			    * we're indexing into this table. */
		"notused",
		"notused",
		"notused",
		0, 0, 0, 0,
	},
	{
		"turn pet into companion",
		"You can turn a pet into a companion.",
		"You suddenly gain authority over your pets.",
		"You can no longer convert pets into companions.",
		2, 10, A_CHR, 10,
	},
	{
		"turn into a bear",
		"You can turn into a bear.",
		"You suddenly gain beorning powers.",
		"You can no longer shapeshift into a bear.",
		2, 5, A_CON, 5,
	},
	{
		"sense dodge success",
		"You can sense your dodging success chance.",
		"You suddenly can sense your dodging success chance.",
		"You can no longer sense your dodging success chance.",
		0, 0, 0, 0,
	},
	{
		"turn into a Balrog",
		"You can turn into a Balrog at will.",
		"You feel the fire of Udun burning in you.",
		"You no longer feel the fire of Udun in you.",
		35, 80, A_WIS, 25,
	},
	{
		"invisibility",
		"You are able melt into the shadows to become invisible.",
		"You suddenly become able to melt into the shadows.",
		"You lose your shadow-melting ability.",
		30, 10, A_DEX, 20,
	},
	{
		"web",
		"You are able throw a thick and very resistant spider web.",
		"You suddenly become able to weave webs.",
		"You lose your web-weaving capability.",
		25, 30, A_DEX, 20,
	},
	{
		"control space/time continuum",
		"You are able to control the space/time continuum.",
		"You become able to control the space/time continuum.",
		"You are no more able to control the space/time continuum.",
		1, 10, A_WIS, 10,
	},
};

/*
 * The Quests
 */
quest_type quest[MAX_Q_IDX] =
{
	{
		FALSE,
		"",
		{
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
		},
		QUEST_STATUS_UNTAKEN,
		0,

		NULL,
		NULL,
		{0, 0},
		NULL,
	},
	{
		FALSE,
		"Dol Guldur",
		{
			"The forest of Mirkwood is a very dangerous place to go, mainly due to",
			"the activities of the Necromancer that lurks in Dol Guldur.",
			"Find him, and free Mirkwood from his spells.",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
		},
		QUEST_STATUS_TAKEN,
		70,
	
		&plots[PLOT_MAIN],
		quest_necro_init_hook,
		{0, 0},
		NULL,
	},
	{
		FALSE,
		"Sauron",
		{
			"It is time to take the battle to Morgoth. But, before you can",
			"reach it, you must find and kill Sauron.  Only after defeating",
			"this powerful sorcerer will the stairs leading to Morgoth's",
			"room be opened.",
			"",
			"",
			"",
			"",
			"",
			"",
		},
		QUEST_STATUS_UNTAKEN,
		99,
	
		&plots[PLOT_MAIN],
		quest_sauron_init_hook,
		{0, 0},
		NULL,
	},
	{
		FALSE,
		"Morgoth",
		{
			"Your final quest is the ultimate quest that has always been",
			"required of you. You must enter the fetid depths of Angband, where",
			"Morgoth is waiting. Travel deep, and defeat this source of all our",
			"problems.  Be prepared, be patient, and good luck. May the light",
			"shine on you.",
			"",
			"",
			"",
			"",
			"",
		},
		QUEST_STATUS_UNTAKEN,
		100,
	
		&plots[PLOT_MAIN],
		quest_morgoth_init_hook,
		{0, 0},
		NULL,
	},
	
	/* Bree plot */
	{
		FALSE,
		"Thieves!",
		{
			"There are thieves robbing my people! They live in a small",
			"burrow outside the city walls, but they get inside the walls",
			"with a tunnel to a building here! Your task is to go into",
			"the building and kill these ruffians.",
			"",
			"",
			"",
			"",
			"",
			"",
		},
		QUEST_STATUS_UNTAKEN,
		5,
	
		&plots[PLOT_BREE],
		quest_thieves_init_hook,
		{0, 0},
		NULL,
	},
	
	{
		FALSE,
		"Random Quest",
		{
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
		},
		QUEST_STATUS_UNTAKEN,
		5,
	
		NULL,
		quest_random_init_hook,
		{0, 0},
		quest_random_describe,
	},
	
	{
		FALSE,
		"Lost Hobbit",
		{
			"Merton Proudfoot, a young hobbit, seems to have disappeared.",
			"Last time anyone saw him was near the horrible maze to the south of Bree.",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
		},
		QUEST_STATUS_UNTAKEN,
		25,
	
		&plots[PLOT_OTHER],
		quest_hobbit_init_hook,
		{0, 0},
		NULL,
	},
	
	{
		FALSE,
		"The Dark Horseman",
		{
			"A dark-cloaked horseman has been spotted several times in town.",
			"He carries an aura of fear with him and people seem to get sick",
			"wherever he goes.  Please do something, but be careful...",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
		},
		QUEST_STATUS_UNTAKEN,
		40,
	
		&plots[PLOT_BREE],
		quest_nazgul_init_hook,
		{0, 0},
		NULL,
	},
	
	{
		FALSE,
		"The Trolls Glade",
		{
			"A group of Forest Trolls settled in an abandoned forest in the",
			"south east of our town. They are killing our people.  You must",
			"put an end to this!  It might be best to look for them at night.",
			"Local hobbits claim that the mighty swords Orcrist and Glamdring",
			"can be found there! Bring back one of them as a proof!",
			"",
			"",
			"",
			"",
			"",
		},
		QUEST_STATUS_UNTAKEN,
		30,
	
		&plots[PLOT_BREE],
		quest_troll_init_hook,
		{FALSE, 0},
		NULL,
	},
	
	{
		FALSE,
		"The Wight Grave",
		{
			"The Barrow-Downs hides many mysteries and dangers.",
			"Lately many people, both men and hobbits, have disappeared there.",
			"Please put an end to this threat!",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
		},
		QUEST_STATUS_UNTAKEN,
		30,
	
		&plots[PLOT_BREE],
		quest_wight_init_hook,
		{FALSE, 0},
		NULL,
	},
	
	/* Lorien plot */
	{
		FALSE,
		"Spiders of Mirkwood",
		{
			"Powers lurk deep within Mirkwood. Spiders have blocked the",
			"path through the forest, and Thranduil's folk have been",
			"unable to hold them off. It is your task to drive them",
			"away. Be careful -- many traps have been laid by their",
			"webs, and their venom is dangerous indeed.",
			"",
			"",
			"",
			"",
			"",
		},
		QUEST_STATUS_UNTAKEN,
		25,
	
		&plots[PLOT_LORIEN],
		quest_spider_init_hook,
		{0, 0},
		NULL,
	},
	{
		FALSE,
		"Poisoned Water",
		{
			"A curse has beset Lothlorien. All trees along the shorelines of Nimrodel",
			"are withering away. We fear the blight could spread to the whole forest.",
			"The cause seems to be an unknown poison. You are to go to the West and",
			"travel along Celebrant and Nimrodel until you discover the source of",
			"the poisoning.  Then you must destroy it and drop these potions on",
			"the tainted water.",
			"",
			"",
			"",
			"",
		},
		QUEST_STATUS_UNTAKEN,
		30,
	
		&plots[PLOT_LORIEN],
		quest_poison_init_hook,
		{0, 0},
		NULL,
	},
	/* Other quests */
	{
		FALSE,
		"The Broken Sword",
		{
			"You have found Narsil, a broken sword. It is said that the sword that",
			"was broken shall be reforged... Maybe it is this one.",
			"You should bring it to Aragorn at Minas Anor -- he would know.",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
		},
		QUEST_STATUS_UNTAKEN,
		20,
	
		&plots[PLOT_OTHER],
		quest_narsil_init_hook,
		{0, 0},
		NULL,
	},
	/* Gondolin plot */
	{
		FALSE,
		"Eol the Dark Elf",
		{
			"We have disturbing tidings. Eol the Dark Elf has come seeking his kin in",
			"Gondolin. We cannot let anyone pass the borders of the city without the",
			"King's leave. Go forth to the eastern mountains and apprehend him. If",
			"he resists, use whatever means possible to hinder him from reaching the",
			"city. Be wary -- the mountain caves may have many hidden traps.",
			"",
			"",
			"",
			"",
			"",
		},
		QUEST_STATUS_UNTAKEN,
		30,
	
		&plots[PLOT_GONDOLIN],
		quest_eol_init_hook,
		{0, 0},
		NULL,
	},
	{
		FALSE,
		"Nirnaeth Arnoediad",
		{
			"The fortunes of war in the north turn against us.",
			"Morgoth's treachery has driven our armies back nigh",
			"to the city's walls. Go forth from the city gates",
			"and clear a path for them to retreat. You need not",
			"destroy the troll army, simply drive a path through.",
			"",
			"",
			"",
			"",
			"",
		},
		QUEST_STATUS_UNTAKEN,
		37,
	
		&plots[PLOT_GONDOLIN],
		quest_nirnaeth_init_hook,
		{0, 0},
		NULL,
	},
	{
		FALSE,
		"Invasion of Gondolin",
		{
			"Morgoth is upon us! Dragons and Balrogs have poured over secret",
			"ways of the Echoriath, and are looking for our city. They are",
			"conducted by Maeglin! You must stop him or they will find us.",
			"Do not let Maeglin get to the stairs or everything will be lost!",
			"Go now, be brave.",
			"",
			"",
			"",
			"",
			"",
		},
		QUEST_STATUS_UNTAKEN,
		80,
	
		&plots[PLOT_GONDOLIN],
		quest_invasion_init_hook,
		{0, 0},
		NULL,
	},
	/* Minas Anor Plot*/
	{
		FALSE,
		"The Last Alliance",
		{
			"The armies of Morgoth are closing in on the last remaining strongholds",
			"of resistance against him. We are too far apart to help each other.",
			"The arrival of our new Thunderlord allies has helped, but can only delay",
			"the inevitable. We must be able to stand together and reinforce each other,",
			"or both our kingdoms will fall separately. The Thunderlords have taught us",
			"how to use the Void Jumpgates: we need you to open a Void Jumpgate in our",
			"own city, and that of Gondolin.",
			"Simply travel to Gondolin, but beware of rebel thunderlords.",
			"",
			"",
		},
		QUEST_STATUS_UNTAKEN,
		80,
	
		&plots[PLOT_MINAS],
		quest_between_init_hook,
		{0, 0},
		NULL,
	},
	{
		FALSE,
		"The One Ring",
		{
			"Find the One Ring, then bring it to Mount Doom, in Mordor, to drop",
			"it in the Great Fire where it was once forged.",
			"But beware: *NEVER* use it, or you will be corrupted.",
			"Once it is destroyed you will be able to permanently defeat Sauron.",
			"The ring must be cast back into the fires of Mount Doom!",
			"",
			"",
			"",
			"",
			"",
		},
		QUEST_STATUS_UNTAKEN,
		99,
	
		&plots[PLOT_MAIN],
		quest_one_init_hook,
		{0, 0},
		NULL,
	},
	
	{
		FALSE,
		"Mushroom supplies",
		{
			"Farmer Maggot asked you to bring him back his mushrooms.",
			"Do not harm his dogs.",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
		},
		QUEST_STATUS_UNTAKEN,
		3,
	
		&plots[PLOT_OTHER],
		quest_shroom_init_hook,
		{0, 0},
		NULL,
	},
	
	{
		FALSE,
		"The prisoner of Dol Guldur",
		{
			"You keep hearing distress cries in the dark tower of",
			"Dol Guldur...",
			"Maybe there is someone being held prisoner and tortured!",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
		},
		QUEST_STATUS_UNTAKEN,
		60,
	
		&plots[PLOT_OTHER],
		quest_thrain_init_hook,
		{0, 0},
		NULL,
	},
	
	/* The 2 ultra endings go here */
	{
		FALSE,
		"Falling Toward Apotheosis",
		{
			"You must enter the Void where Melkor spirit lurks to destroy",
			"him forever. Remember however that it is likely to be your own",
			"death that awaits you.",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
		},
		QUEST_STATUS_UNTAKEN,
		150,
	
		&plots[PLOT_MAIN],
		quest_ultra_good_init_hook,
		{0, 0},
		NULL,
	},
	{
		FALSE,
		"Falling Toward Apotheosis",
		{
			"You must now launch an onslaught on Valinor itself to eliminate",
			"once and for all any posible resistance to your dominance of Arda.",
			"Remember however that it is likely to be your own death that awaits you.",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
		},
		QUEST_STATUS_UNTAKEN,
		150,
	
		&plots[PLOT_MAIN],
		quest_ultra_evil_init_hook,
		{0, 0},
		NULL,
	},
	/* More Lorien */
	{
		FALSE,
		"Wolves!",
		{
			"There are wolves pestering my people! They gather in a hut",
			"on the edge of town and menace everyone nearby. Your task",
			"is to go in there and clear them out.",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
		},
		QUEST_STATUS_UNTAKEN,
		15,
	
		&plots[PLOT_LORIEN],
		quest_wolves_init_hook,
		{0, 0},
		NULL,
	},
	/* More Gondolin */
	{
		FALSE,
		"Dragons!",
		{
			"There are dragons pestering my people! They gather in a",
			"building on the edge of town and menace everyone nearby.",
			"Your task is to go into the building and clear them out.",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
		},
		QUEST_STATUS_UNTAKEN,
		25,
	
		&plots[PLOT_GONDOLIN],
		quest_dragons_init_hook,
		{0, 0},
		NULL,
	},
	/* More Minas Anor */
	{
		FALSE,
		"Haunted House!",
		{
			"There are undead pestering my people! They gather in a hut",
			"on the edge of town and menace everyone nearby. Your task",
			"is to go into the building and clear out the beasts.",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
		},
		QUEST_STATUS_UNTAKEN,
		45,
	
		&plots[PLOT_MINAS],
		quest_haunted_init_hook,
		{0, 0},
		NULL,
	},
	/* Khazad-Dum Plot*/
	{
		FALSE,
		"Evil!",
		{
			"We have burrowed too deep, and let out some creatures of",
			"Morgoth's that threaten to kill us all! Your task is to save us",
			"from them!",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
		},
		QUEST_STATUS_UNTAKEN,
		60,
	
		&plots[PLOT_KHAZAD],
		quest_evil_init_hook,
		{0, 0},
		NULL,
	},
	/* Bounty */
	{
		FALSE,
		"Bounty quest",
		{
			"", /* dynamic desc */
		},
		QUEST_STATUS_UNTAKEN,
		-1,
		NULL,
		quest_bounty_init_hook,
		{0, 0, 0, 0},
		quest_bounty_describe,
	},
	/* Fireproofing */
	{
		FALSE,
		"Old Mages quest",
		{
			"", /* dynamic desc */
		},
		QUEST_STATUS_UNTAKEN,
		20,
		NULL,
		quest_fireproof_init_hook,
		{0, 0, 0, 0},
		quest_fireproof_describe,
	},
	/* Library */
	{
		FALSE,
		"Library quest",
		{
			"", /* dynamic desc */
		},
		QUEST_STATUS_UNTAKEN,
		35,
		NULL,
		quest_library_init_hook,
		{ -1, -1, -1, -1 },
		quest_library_describe,
	},
	/* God quest */
	{
		FALSE,
		"God quest",
		{
			"", /* dynamic desc */
		},
		QUEST_STATUS_UNTAKEN,
		-1,
		NULL,
		quest_god_init_hook,
		{ 0 /* quests_given */,
		  0 /* relics_found */,
		  1 /* dun_mindepth */,
		  4 /* dun_maxdepth */,
		  0 /* dun_minplev */,
		  0 /* relic_gen_tries */,
		  FALSE /* relic_generated */,
		  1 /* dung_x */,
		  1 /* dung_y */,
		},
		quest_god_describe,
	},
};


/* List of powers for Symbiants/Powers */
monster_power monster_powers[] =
	{
		{ SF_SHRIEK_IDX, "Aggravate Monster", 1, FALSE },
		{ SF_MULTIPLY_IDX, "Multiply", 10, FALSE },
		{ SF_S_ANIMAL_IDX, "Summon Animal", 30, FALSE },
		{ SF_ROCKET_IDX, "Fire a Rocket", 40, TRUE },
		{ SF_ARROW_1_IDX, "Light Arrow", 1, FALSE },
		{ SF_ARROW_2_IDX, "Minor Arrow", 3, FALSE },
		{ SF_ARROW_3_IDX, "Major Arrow", 7, TRUE },
		{ SF_ARROW_4_IDX, "Great Arrow", 9, TRUE },
		{ SF_BR_ACID_IDX, "Breathe Acid", 10, FALSE },
		{ SF_BR_ELEC_IDX, "Breathe Lightning", 10, FALSE },
		{ SF_BR_FIRE_IDX, "Breathe Fire", 10, FALSE },
		{ SF_BR_COLD_IDX, "Breathe Cold", 10, FALSE },
		{ SF_BR_POIS_IDX, "Breathe Poison", 15, TRUE },
		{ SF_BR_NETH_IDX, "Breathe Nether", 30, TRUE },
		{ SF_BR_LITE_IDX, "Breathe Light", 20, TRUE },
		{ SF_BR_DARK_IDX, "Breathe Dark", 20, TRUE },
		{ SF_BR_CONF_IDX, "Breathe Confusion", 15, TRUE },
		{ SF_BR_SOUN_IDX, "Breathe Sound", 30, TRUE },
		{ SF_BR_CHAO_IDX, "Breathe Chaos", 30, TRUE },
		{ SF_BR_DISE_IDX, "Breathe Disenchantment", 30, TRUE },
		{ SF_BR_NEXU_IDX, "Breathe Nexus", 30, TRUE },
		{ SF_BR_TIME_IDX, "Breathe Time", 30, TRUE },
		{ SF_BR_INER_IDX, "Breathe Inertia", 30, TRUE },
		{ SF_BR_GRAV_IDX, "Breathe Gravity", 30, TRUE },
		{ SF_BR_SHAR_IDX, "Breathe Shards", 30, TRUE },
		{ SF_BR_PLAS_IDX, "Breathe Plasma", 30, TRUE },
		{ SF_BR_WALL_IDX, "Breathe Force", 30, TRUE },
		{ SF_BR_MANA_IDX, "Breathe Mana", 40, TRUE },
		{ SF_BA_NUKE_IDX, "Nuke Ball", 30, TRUE },
		{ SF_BR_NUKE_IDX, "Breathe Nuke", 40, TRUE },
		{ SF_BA_CHAO_IDX, "Chaos Ball", 30, TRUE },
		{ SF_BR_DISI_IDX, "Breathe Disintegration", 40, TRUE },
		{ SF_BA_ACID_IDX, "Acid Ball", 8, FALSE },
		{ SF_BA_ELEC_IDX, "Lightning Ball", 8, FALSE },
		{ SF_BA_FIRE_IDX, "Fire Ball", 8, FALSE },
		{ SF_BA_COLD_IDX, "Cold Ball", 8, FALSE },
		{ SF_BA_POIS_IDX, "Poison Ball", 20, TRUE },
		{ SF_BA_NETH_IDX, "Nether Ball", 20, TRUE },
		{ SF_BA_WATE_IDX, "Water Ball", 20, TRUE },
		{ SF_BA_MANA_IDX, "Mana Ball", 50, TRUE },
		{ SF_BA_DARK_IDX, "Darkness Ball", 20, TRUE },
		{ SF_CAUSE_1_IDX, "Cause Light Wounds", 20, FALSE },
		{ SF_CAUSE_2_IDX, "Cause Medium Wounds", 30, FALSE },
		{ SF_CAUSE_3_IDX, "Cause Critical Wounds", 35, TRUE },
		{ SF_CAUSE_4_IDX, "Cause Mortal Wounds", 45, TRUE },
		{ SF_BO_ACID_IDX, "Acid Bolt", 5, FALSE },
		{ SF_BO_ELEC_IDX, "Lightning Bolt", 5, FALSE },
		{ SF_BO_FIRE_IDX, "Fire Bolt", 5, FALSE },
		{ SF_BO_COLD_IDX, "Cold Bolt", 5, FALSE },
		{ SF_BO_POIS_IDX, "Poison Bolt", 10, TRUE },
		{ SF_BO_NETH_IDX, "Nether Bolt", 15, TRUE },
		{ SF_BO_WATE_IDX, "Water Bolt", 20, TRUE },
		{ SF_BO_MANA_IDX, "Mana Bolt", 25, TRUE },
		{ SF_BO_PLAS_IDX, "Plasma Bolt", 20, TRUE },
		{ SF_BO_ICEE_IDX, "Ice Bolt", 20, TRUE },
		{ SF_MISSILE_IDX, "Magic Missile", 1, FALSE },
		{ SF_SCARE_IDX, "Scare", 4, FALSE },
		{ SF_BLIND_IDX, "Blindness", 6, FALSE },
		{ SF_CONF_IDX, "Confusion", 7, FALSE },
		{ SF_SLOW_IDX, "Slowness", 10, FALSE },
		{ SF_HOLD_IDX, "Paralyse", 10, FALSE },
		{ SF_HASTE_IDX, "Haste Self", 50, FALSE },
		{ SF_HAND_DOOM_IDX, "Hand of Doom", 30, TRUE },
		{ SF_HEAL_IDX, "Healing", 60, FALSE },
		{ SF_S_ANIMALS_IDX, "Summon Animals", 60, TRUE },
		{ SF_BLINK_IDX, "Phase Door", 2, FALSE },
		{ SF_TPORT_IDX, "Teleport", 10, FALSE },
		{ SF_TELE_TO_IDX, "Teleport To", 20, TRUE },
		{ SF_TELE_AWAY_IDX, "Teleport Away", 20, FALSE },
		{ SF_TELE_LEVEL_IDX, "Teleport Level", 20, TRUE },
		{ SF_DARKNESS_IDX, "Darkness", 3, FALSE },
		{ SF_RAISE_DEAD_IDX, "Raise the Dead", 400, TRUE },
		{ SF_S_THUNDERLORD_IDX, "Summon Thunderlords", 90, TRUE },
		{ SF_S_KIN_IDX, "Summon Kin", 80, FALSE },
		{ SF_S_HI_DEMON_IDX, "Summon Greater Demons", 90, TRUE },
		{ SF_S_MONSTER_IDX, "Summon Monster", 50, FALSE },
		{ SF_S_MONSTERS_IDX, "Summon Monsters", 60, TRUE },
		{ SF_S_ANT_IDX, "Summon Ants", 30, FALSE },
		{ SF_S_SPIDER_IDX, "Summon Spider", 30, FALSE },
		{ SF_S_HOUND_IDX, "Summon Hound", 50, TRUE },
		{ SF_S_HYDRA_IDX, "Summon Hydra", 40, TRUE },
		{ SF_S_ANGEL_IDX, "Summon Angel", 60, TRUE },
		{ SF_S_DEMON_IDX, "Summon Demon", 60, TRUE },
		{ SF_S_UNDEAD_IDX, "Summon Undead", 70, TRUE },
		{ SF_S_DRAGON_IDX, "Summon Dragon", 70, TRUE },
		{ SF_S_HI_UNDEAD_IDX, "Summon High Undead", 90, TRUE },
		{ SF_S_HI_DRAGON_IDX, "Summon High Dragon", 90, TRUE },
		{ SF_S_WRAITH_IDX, "Summon Wraith", 90, TRUE },
	};


/*
 * A list of tvals and their textual names
 */
tval_desc tvals[] =
{
        { TV_SWORD, "Sword" },
        { TV_POLEARM, "Polearm" },
        { TV_HAFTED, "Hafted Weapon" },
        { TV_AXE, "Axe" },
        { TV_BOW, "Bow" },
        { TV_BOOMERANG, "Boomerang" },
        { TV_ARROW, "Arrows" },
        { TV_BOLT, "Bolts" },
        { TV_SHOT, "Shots" },
        { TV_SHIELD, "Shield" },
        { TV_CROWN, "Crown" },
        { TV_HELM, "Helm" },
        { TV_GLOVES, "Gloves" },
        { TV_BOOTS, "Boots" },
        { TV_CLOAK, "Cloak" },
        { TV_DRAG_ARMOR, "Dragon Scale Mail" },
        { TV_HARD_ARMOR, "Hard Armor" },
        { TV_SOFT_ARMOR, "Soft Armor" },
        { TV_RING, "Ring" },
        { TV_AMULET, "Amulet" },
        { TV_LITE, "Lite" },
        { TV_POTION, "Potion" },
        { TV_POTION2, "Potion" },
        { TV_SCROLL, "Scroll" },
        { TV_WAND, "Wand" },
        { TV_STAFF, "Staff" },
        { TV_ROD_MAIN, "Rod" },
        { TV_ROD, "Rod Tip" },
        { TV_BOOK, "Schools Spellbook", },
        { TV_SYMBIOTIC_BOOK, "Symbiotic Spellbook", },
        { TV_DRUID_BOOK, "Elemental Stone" },
        { TV_MUSIC_BOOK, "Music Book" },
        { TV_DAEMON_BOOK, "Daemon Book" },
        { TV_SPIKE, "Spikes" },
        { TV_DIGGING, "Digger" },
        { TV_CHEST, "Chest" },
        { TV_FOOD, "Food" },
        { TV_FLASK, "Flask" },
        { TV_MSTAFF, "Mage Staff" },
        { TV_PARCHMENT, "Parchment" },
        { TV_INSTRUMENT, "Musical Instrument" },
        { TV_JUNK, "Junk" },
        { 0, NULL }
};

/* Tval descriptions */
tval_desc tval_descs[] =
{
	{
		TV_MSTAFF,
		"Mage Staves are the spellcaster's weapons of choice.  "
		"They all reduce spellcasting time to 80% of "
		"normal time and some will yield even greater powers."
	},
	{
		TV_PARCHMENT,
		"Parchments can contain useful information ... or useless "
		"junk."
	},
	{
		TV_EGG,
		"Eggs are laid by some monsters.  If they hatch in your "
		"inventory the monster will be your friend."
	},
	{
		TV_TOOL,
		"Tools can be digging implements, climbing equipment and such. "
		"They have their own slot in your inventory."
	
	},
	{
		TV_INSTRUMENT,
		"Musical instruments can be used with the Music skill to play "
		"magical songs. Some of them can also be activated."
	},
	{
		TV_BOOMERANG,
		"Boomerangs can be used instead of bows or slings.  They "
		"are more like melee weapons than bows."
	},
	{
		TV_SHOT,
		"Shots are small, hard balls.  They are the standard ammunition "
		"for slings.  You can carry them in your quiver if you have a sling "
		"equipped."
	},
	{
		TV_ARROW,
		"Arrows are the standard ammunition for bows.  You can carry "
		"them in your quiver if you have a bow equipped."
	},
	{
		TV_BOLT,
		"Bolts are the standard ammunition for crossbows.  You can "
		"carry them in your quiver if you have a crossbow equipped."
	},
	{
		TV_BOW,
		"Slings, bows and crossbows are used to attack monsters "
		"from a distance."
	},
	{
		TV_DIGGING,
		"Tools can be digging implements, climbing equipment and such.  "
		"They have their own slot in your inventory."
	},
	{
		TV_HAFTED,
		"Hafted weapons are melee weapons.  Eru followers can use them "
		"without penalties."
	},
	{
		TV_SWORD,
		"Swords are melee weapons."
	},
	{
		TV_AXE,
		"Axes are melee weapons."
	},
	{
		TV_POLEARM,
		"Polearms are melee weapons."
	},
	{
		TV_DRAG_ARMOR,
		"Dragon armour is made from the scales of dead dragons. "
		"These mighty sets of armour usually yield great power to "
		"their wearer."
	},
	{
		TV_LITE,
		"Lights allow you to read things and see from afar. Some of "
		"them need to be fueled but some do not."
	},
	{
		TV_AMULET,
		"Amulets are fine pieces of jewelry, usually imbued with "
		"arcane magics."
	},
	{
		TV_RING,
		"Rings are fine pieces of jewelry, usually imbued with "
		"arcane magics."
	},
	{
		TV_STAFF,
		"Staves are objects imbued with mystical powers."
	},
	{
		TV_WAND,
		"Wands are like small staves and usually have a targeted "
		"effect."
	},
	{
		TV_ROD,
		"Rod tips are the physical bindings of powerful "
		"spells.  Zap (attach) them to a rod to get a fully "
		"functional rod. Each spell takes some mana from the rod "
		"it is attached to to work."
	},
	{
		TV_ROD_MAIN,
		"Rods contain mana reserves used to cast spells in rod "
		"tips.  Zap (attach) a rod tip to them to get a fully "
		"functional rod. Each spell takes some mana from the rod "
		"it is attached to to work."
	},
	{
		TV_SCROLL,
		"Scrolls are magical parchments imbued with magic spells. "
		"Some are good, some...are not.  When a scroll is read, its "
		"magic is released and the scroll is destroyed."
	},
	{
		TV_POTION,
		"Potions are magical liquids.  Some of them are "
		"beneficial...some not."
	},
	{
		TV_POTION2,
		"Potions are magical liquids.  Some of them are "
		"beneficial...some not."
	},
	{
		TV_FLASK,
		"Flasks of oil can be used to refill lanterns."
	},
	{
		TV_FOOD,
		"Everybody needs to eat, even you."
	},
	{
		TV_HYPNOS,
		"This monster seems to be hypnotised and friendly."
	},
	{
		TV_RANDART,
		"Those objects are only known of by rumours.  It is said that "
		"they can be activated for great or strange effects..."
	},
	{
		TV_JUNK,
		"Junk is usually worthless, though experienced archers can "
		"create ammo with them."
	},
	{
		TV_SKELETON,
		"It looks dead..."
	},
	{
		TV_BOTTLE,
		"An empty bottle."
	},
	{
		TV_SPIKE,
		"Spikes can be used to jam doors."
	},
	{
		TV_CORPSE,
		"It looks dead..."
	},
	{
		TV_BOOTS,
		"Boots can help your armour rating.  Some of these are magical."
	},
	{
		TV_GLOVES,
		"Handgear is used to protect hands, but nonmagical ones "
		"can sometimes hinder spellcasting."
	},
	{
		TV_HELM,
		"Headgear will protect your head."
	},
	{
		TV_CROWN,
		"Headgear will protect your head."
	},
	{
		TV_SHIELD,
		"Shields will help improve your defence rating, but you "
		"cannot use them with two handed weapons."
	},
	{
		TV_CLOAK,
		"Cloaks can shield you from damage.  Sometimes they also "
		"provide magical powers."
	},
	{
		TV_SOFT_ARMOR,
		"Soft armour is light, and will not hinder your combat much."
	},
	{
		TV_HARD_ARMOR,
		"Hard armour provides much more protection than soft "
		"armour but also hinders combat much more."
	},
	{
		TV_SYMBIOTIC_BOOK,
		"This mystical book is used by symbiants to extend their "
		"symbiosis."
	},
	{
		TV_MUSIC_BOOK,
		"This song book is used by bards to play songs."
	},
	{
		TV_DRUID_BOOK,
		"This mystical book is used by druids to call upon the "
		"powers of nature."
	},
	{
		TV_DAEMON_BOOK,
		"This unholy demon equipment is used with the Demonology skill to control "
		"the school of demon power."
	},
	{0, ""},
};

/*
 * List of the between exits
 *       s16b corresp;           Corresponding between gate
 *       bool_ dungeon;           Do we exit in a dungeon or in the wild ?
 *
 *       s16b wild_x, wild_y;    Wilderness spot to land onto
 *       s16b p_ptr->px, p_ptr->py;            Location of the map
 *
 *       s16b d_idx;             Dungeon to land onto
 *       s16b level;
 */
between_exit between_exits[MAX_BETWEEN_EXITS] =
{
	{
		1,
		FALSE,
		49, 11,
		119, 25,
		0, 0
	},
	{
		0,
		FALSE,
		60, 56,
		10, 35,
		0, 0
	},
	/* Theme: Minas Tirith -> Gondolin link */
	{
		0,
		FALSE,
		3, 11,
		119, 25,
		0, 0
	},
};

/*
 * max body parts
 */
int max_body_part[BODY_MAX] =
{
	3,        /* Weapon */
	1,        /* Torso */
	3,        /* Arms */
	6,        /* Finger */
	2,        /* Head */
	2,        /* Legs */
};

/*
 * Description of GF_FOO
 */
gf_name_type gf_names[] =
{
	{ GF_ELEC, "electricity" },
	{ GF_POIS, "poison" },
	{ GF_ACID, "acid" },
	{ GF_COLD, "cold" },
	{ GF_FIRE, "fire" },
	{ GF_UNBREATH, "asphyxiating gas" },
	{ GF_CORPSE_EXPL, "corpse explosion" },
	{ GF_MISSILE, "missile" },
	{ GF_ARROW, "arrow" },
	{ GF_PLASMA, "plasma" },
	{ GF_WAVE, "a tidal wave" },
	{ GF_WATER, "water" },
	{ GF_LITE, "light" },
	{ GF_DARK, "darkness" },
	{ GF_LITE_WEAK, "weak light" },
	{ GF_DARK_WEAK, "weak darkness" },
	{ GF_SHARDS, "shards" },
	{ GF_SOUND, "sound" },
	{ GF_CONFUSION, "confusion" },
	{ GF_FORCE, "force" },
	{ GF_INERTIA, "inertia" },
	{ GF_MANA, "pure mana" },
	{ GF_METEOR, "meteor" },
	{ GF_ICE, "ice" },
	{ GF_CHAOS, "chaos" },
	{ GF_NETHER, "nether" },
	{ GF_DISENCHANT, "disenchantment" },
	{ GF_NEXUS, "nexus" },
	{ GF_TIME, "time" },
	{ GF_GRAVITY, "gravity" },
	{ GF_KILL_WALL, "wall destruction" },
	{ GF_KILL_DOOR, "door destruction" },
	{ GF_MAKE_WALL, "wall creation" },
	{ GF_MAKE_DOOR, "door creation" },
	{ GF_OLD_CLONE, "clone" },
	{ GF_OLD_POLY, "polymorph" },
	{ GF_OLD_HEAL, "healing" },
	{ GF_OLD_SPEED, "speed" },
	{ GF_OLD_SLOW, "slowness" },
	{ GF_OLD_CONF, "confusion" },
	{ GF_OLD_SLEEP, "sleep" },
	{ GF_OLD_DRAIN, "drain life" },
	{ GF_AWAY_UNDEAD, "teleport away undead" },
	{ GF_AWAY_EVIL, "teleport away evil" },
	{ GF_AWAY_ALL, "teleport away" },
	{ GF_TURN_UNDEAD, "scare undead" },
	{ GF_TURN_EVIL, "scare evil" },
	{ GF_TURN_ALL, "scare" },
	{ GF_DISP_UNDEAD, "dispel undead" },
	{ GF_DISP_EVIL, "dispel evil" },
	{ GF_DISP_ALL, "dispel" },
	{ GF_DISP_DEMON, "dispel demons" },
	{ GF_DISP_LIVING, "dispel living creatures" },
	{ GF_ROCKET, "rocket" },
	{ GF_NUKE, "nuke" },
	{ GF_MAKE_GLYPH, "glyph creation" },
	{ GF_STASIS, "stasis" },
	{ GF_STONE_WALL, "stone wall creation" },
	{ GF_DEATH_RAY, "death ray" },
	{ GF_STUN, "stunning" },
	{ GF_HOLY_FIRE, "holy fire" },
	{ GF_HELL_FIRE, "hellfire" },
	{ GF_DISINTEGRATE, "disintegration" },
	{ GF_CHARM, "charming" },
	{ GF_CONTROL_UNDEAD, "undead control" },
	{ GF_CONTROL_ANIMAL, "animal control" },
	{ GF_PSI, "psionic energy" },
	{ GF_PSI_DRAIN, "psionic drain" },
	{ GF_TELEKINESIS, "telekinesis" },
	{ GF_JAM_DOOR, "door jamming" },
	{ GF_DOMINATION, "domination" },
	{ GF_DISP_GOOD, "dispel good" },
	{ GF_RAISE, "raise dead" },
	{ GF_STAR_IDENTIFY, "*identification*" },
	{ GF_DESTRUCTION, "destruction" },
	{ GF_STUN_CONF, "stunning and confusion" },
	{ GF_STUN_DAM, "stunning and damage" },
	{ GF_CONF_DAM, "confusion and damage" },
	{ GF_STAR_CHARM, "*charming*" },
	{ GF_IMPLOSION, "implosion" },
	{ GF_LAVA_FLOW, "lava" },
	{ GF_FEAR, "fear" },
	{ GF_BETWEEN_GATE, "jumpgate creation" },
	{ GF_WINDS_MANA, "" },
	{ GF_DEATH, "death" },
	{ GF_CONTROL_DEMON, "control demon" },
	{ GF_RAISE_DEMON, "raise demon" },
	{ GF_TRAP_DEMONSOUL, "*control demon*" },
	{ GF_ATTACK, "projected melee attacks" },
	{ -1, NULL },
};

/**
 * Modules
 */
module_type modules[MAX_MODULES] =
{
	{ 
		{ "ToME",
		  { 2, 4, 0 },
		  { "DarkGod", "darkgod@t-o-m-e.net" },
		  "The Tales of Middle-earth, the standard and official game.\n"
		  "You are set on a quest to investigate the old tower of Dol Guldur.\n"
		  "But who knows what will happen...",
		  "ToME",
		  NULL /* default dir */,
		},
		/* Randarts: */
		{ 30, 20, 20 },
		/* Skills: */
		{ 6, 4, },
		/* Intro function */
		tome_intro,
		/* Race status function: ToME requires no special handling */
		NULL
	},

	{
		{ "Theme",
		  { 1, 2, 0 },
		  { "furiosity", "furiosity@gmail.com" },
		  "A module that goes back to Tolkien roots, though by no means canonical.\n"
		  "A new wilderness map, new monsters, objects, artifacts, uniques, ego items,\n"
		  "terrain features, gods, races, subraces, and classes. Have fun. :-)",
		  "Theme",
		  "theme",
		},
		/* Randarts: */
		{ 30, 30, 30 },
		/* Skill overage: */
		{ 6, 5, },
		/* Intro function */
		theme_intro,
		/* Race status function */
		theme_race_status
	}

};

