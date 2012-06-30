#ifndef H_ce003b12_cf58_444f_a927_5451f6dd8af1
#define H_ce003b12_cf58_444f_a927_5451f6dd8af1

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
} // extern "C"
#endif

#endif
