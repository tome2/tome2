#pragma once

#include "ego_flag_set.hpp"
#include "h-basic.hpp"
#include "object_kind_fwd.hpp"
#include "object_type_fwd.hpp"
#include "obj_theme_fwd.hpp"

#include <boost/optional.hpp>

typedef enum { OPTIMIZE, NO_OPTIMIZE } optimize_flag;
typedef enum { DESCRIBE, NO_DESCRIBE } describe_flag;

void inc_stack_size(int item, int delta);
void inc_stack_size_ex(int item, int delta, optimize_flag opt, describe_flag desc);
object_type *get_object(int item);
s32b calc_total_weight();
void add_random_ego_flag(object_type *o_ptr, ego_flag_set const &fego, bool *limit_blows);
bool init_match_theme(obj_theme const &theme);
bool kind_is_artifactable(object_kind const *);
bool kind_is_legal(object_kind const *);
void inven_item_charges(int item);
void inven_item_describe(int item);
void inven_item_increase(int item, int num);
void inven_item_optimize(int item);
void floor_item_charges(int item);
void floor_item_describe(int item);
void floor_item_increase(int item, int num);
void floor_item_optimize(int item);
bool inven_carry_okay(object_type const *o_ptr);
s16b inven_carry(object_type *o_ptr, bool final);
s16b inven_takeoff(int item, int amt, bool force_drop);
void inven_drop(int item, int amt, int dy, int dx, bool silent);
void excise_object_idx(int o_idx);
void delete_object_idx(int o_idx);
void delete_object(int y, int x);
void compact_objects(int size);
void rescue_artifact(object_type *o_ptr);
void wipe_o_list();
s16b o_pop();
errr get_obj_num_prep();
s16b get_obj_num(int level);
void object_known(object_type *o_ptr);
bool object_known_p(object_type const *o_ptr);
void object_aware(object_type *o_ptr);
bool object_aware_p(object_type const *o_ptr);
s32b object_value(object_type const *o_ptr);
s32b object_value_real(object_type const *o_ptr);
bool object_similar(object_type const *o_ptr, object_type const *j_ptr);
void object_absorb(object_type *o_ptr, object_type *j_ptr);
s16b lookup_kind(int tval, int sval);
void object_wipe(object_type *o_ptr);
void object_prep(object_type *o_ptr, int k_idx);
void object_copy(object_type *o_ptr, object_type *j_ptr);
void apply_magic(object_type *o_ptr, int lev, bool okay, bool good, bool great, boost::optional<int> force_power = boost::none);
bool make_object(object_type *j_ptr, bool good, bool great, obj_theme const &theme);
void place_object(int y, int x, bool good, bool great, int where);
bool make_gold(object_type *j_ptr);
void place_gold(int y, int x);
s16b drop_near(object_type *o_ptr, int chance, int y, int x);
void acquirement(int y1, int x1, int num, bool great);
void combine_pack();
void reorder_pack();
void random_artifact_resistance(object_type * o_ptr);
s16b floor_carry(int y, int x, object_type *j_ptr);
void pack_decay(int item);
void floor_decay(int item);
s16b m_bonus(int max, int level);
s32b flag_cost(object_type const *o_ptr, int plusses);
