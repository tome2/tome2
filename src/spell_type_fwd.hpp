#pragma once

/*
 * Spell effect function result
 */
typedef enum {
	NO_CAST,             /* Spell not cast; user aborted */
	CAST_OBVIOUS,        /* Cast; caster discovers effect (devices) */
	CAST_HIDDEN          /* Cast; caster does NOT discover effect (devices) */
} casting_result;

/*
 * Forward declaration of the spell_type
 */
typedef struct spell_type spell_type;
struct spell_type;
