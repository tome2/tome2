#pragma once

/*
 * Spell effect function result
 */
typedef enum {
	NO_CAST,             /* Spell not cast; user aborted */
	CAST,                /* Spell was cast */
} casting_result;

/*
 * Forward declaration of the spell_type
 */
typedef struct spell_type spell_type;
struct spell_type;
