#ifndef H_e7e01ebf_e19f_439d_b88d_cad51446a7a0
#define H_e7e01ebf_e19f_439d_b88d_cad51446a7a0

#include "spell_type_fwd.h"

#include "h-type.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Forward declarations
 */
struct device_allocation;
struct range_type;

/*
 * Casting type
 */
typedef enum { USE_SPELL_POINTS, USE_PIETY } casting_type;

/*
 * Does the spell appear on spell random books?
 */
typedef enum { RANDOM, NO_RANDOM } random_type;

/*
 * Spell functions
 */

void spell_type_init(spell_type *spell, cptr name);
void spell_type_init_music(spell_type *spell,
			   s16b minimum_pval,
			   char* (*info_func)(),
			   casting_result (*effect_func)(int o_idx));
void spell_type_init_music_lasting(spell_type *spell,
				   s16b minimum_pval,
				   char* (*info_func)(),
				   casting_result (*effect_func)(int o_idx),
				   int (*lasting_func)());
void spell_type_init_mage(spell_type *spell,
			  random_type random_type,
			  s32b school_idx,
			  char* (*info_func)(),
			  casting_result (*effect_func)(int o_idx));
void spell_type_init_priest(spell_type *spell,
			    s32b school_idx,
			    char* (*info_func)(),
			    casting_result (*effect_func)(int o_idx));
void spell_type_init_device(spell_type *spell,
			    char* (*info_func)(),
			    casting_result (*effect_func)(int o_idx));
void spell_type_init_demonology(spell_type *spell,
				char* (*info_func)(),
				casting_result (*effect_func)(int o_idx));
void spell_type_init_geomancy(spell_type *spell,
			      char* (*info_func)(),
			      casting_result (*effect_func)(int o_idx),
			      bool_ (*depend_func)());

void spell_type_set_activation_timeout(spell_type *spell, cptr timeout_s);
void spell_type_set_inertia(spell_type *spell, s32b difficulty, s32b delay);
void spell_type_set_difficulty(spell_type *spell, byte skill_level, s32b failure_rate);
void spell_type_set_mana(spell_type *spell, s32b min, s32b max);
void spell_type_set_castable_while_blind(spell_type *spell, bool_ value);
void spell_type_set_castable_while_confused(spell_type *spell, bool_ value);
void spell_type_describe(spell_type *spell, cptr line);

void spell_type_add_school(spell_type *spell, s32b school_idx);

void spell_type_set_device_charges(spell_type *spell, cptr charges_s);
void spell_type_add_device_allocation(spell_type *spell, struct device_allocation *a);

spell_type *spell_type_new(cptr name);

int spell_type_produce_effect_lasting(spell_type *spell);
casting_result spell_type_produce_effect(spell_type *spell, int o_idx);
cptr spell_type_name(spell_type *spell);
int spell_type_skill_level(spell_type *spell);
void spell_type_description_foreach(spell_type *spell, void (*callback)(void *data, cptr text), void *data);
long spell_type_roll_charges(spell_type *spell);
void spell_type_activation_description(spell_type *spell, char *buf);
int spell_type_activation_roll_timeout(spell_type *spell);
struct device_allocation *spell_type_device_allocation(spell_type *spell, byte tval);
bool_ spell_type_uses_piety_to_cast(spell_type *spell);
bool_ spell_type_castable_while_blind(spell_type *spell);
bool_ spell_type_castable_while_confused(spell_type *spell);
s16b spell_type_minimum_pval(spell_type *spell);
s16b spell_type_random_type(spell_type *spell);
bool_ spell_type_school_foreach(spell_type *spell, bool_ (*callback)(void *data, s32b school_idx), void *data);
bool_ spell_type_inertia(spell_type *spell, s32b *difficulty, s32b *delay);
s32b spell_type_failure_rate(spell_type *spell);
s16b spell_type_casting_stat(spell_type *spell);
cptr spell_type_info(spell_type *spell);
void spell_type_mana_range(spell_type *spell, struct range_type *range);
bool_ spell_type_dependencies_satisfied(spell_type *spell);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
