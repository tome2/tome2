#pragma once

/*
 * Maximum number of "normal" pack slots, and the index of the "overflow"
 * slot, which can hold an item, but only temporarily, since it causes the
 * pack to "overflow", dropping the "last" item onto the ground.  Since this
 * value is used as an actual slot, it must be less than "INVEN_WIELD" (below).
 * Note that "INVEN_PACK" is probably hard-coded by its use in savefiles, and
 * by the fact that the screen can only show 23 items plus a one-line prompt.
 */
#define INVEN_PACK              23

/*
 * Indexes used for various "equipment" slots (hard-coded by savefiles, etc).
 */
#define INVEN_WIELD     24 /* 3 weapons -- WEAPONS */
#define INVEN_BOW       27 /* 1 bow -- WEAPON */
#define INVEN_RING      28 /* 6 rings -- FINGER */
#define INVEN_NECK      34 /* 2 amulets -- HEAD */
#define INVEN_LITE      36 /* 1 lite -- TORSO */
#define INVEN_BODY      37 /* 1 body -- TORSO */
#define INVEN_OUTER     38 /* 1 cloak -- TORSO */
#define INVEN_ARM       39 /* 3 arms -- ARMS */
#define INVEN_HEAD      42 /* 2 heads -- HEAD */
#define INVEN_HANDS     44 /* 3 hands -- ARMS */
#define INVEN_FEET      47 /* 2 feets -- LEGS */
#define INVEN_CARRY     49 /* 1 carried monster -- TORSO */
#define INVEN_AMMO      50 /* 1 quiver -- TORSO */
#define INVEN_TOOL      51 /* 1 tool -- ARMS */

/*
 * Total number of inventory slots (hard-coded).
 */
#define INVEN_TOTAL     52
#define INVEN_EQ        (INVEN_TOTAL - INVEN_WIELD)
