#pragma once

#include "ego_flag_set.hpp"
#include "h-basic.h"
#include "object_type_fwd.hpp"
#include "obj_theme_fwd.hpp"

#include <boost/optional.hpp>

typedef enum { OPTIMIZE, NO_OPTIMIZE } optimize_flag;
typedef enum { DESCRIBE, NO_DESCRIBE } describe_flag;

extern void inc_stack_size(int item, int delta);
extern void inc_stack_size_ex(int item, int delta, optimize_flag opt, describe_flag desc);
extern object_type *get_object(int item);
extern s32b calc_total_weight(void);
extern void add_random_ego_flag(object_type *o_ptr, ego_flag_set const &fego, bool_ *limit_blows);
extern bool init_match_theme(obj_theme const &theme);
extern bool_ kind_is_artifactable(int k_idx);
extern bool_ kind_is_legal(int k_idx);
extern void inven_item_charges(int item);
extern void inven_item_describe(int item);
extern void inven_item_increase(int item, int num);
extern bool_ inven_item_optimize(int item);
extern void floor_item_charges(int item);
extern void floor_item_describe(int item);
extern void floor_item_increase(int item, int num);
extern void floor_item_optimize(int item);
extern bool_ inven_carry_okay(object_type const *o_ptr);
extern s16b inven_carry(object_type *o_ptr, bool_ final);
extern s16b inven_takeoff(int item, int amt, bool_ force_drop);
extern void inven_drop(int item, int amt, int dy, int dx, bool_ silent);
extern void excise_object_idx(int o_idx);
extern void delete_object_idx(int o_idx);
extern void delete_object(int y, int x);
extern void compact_objects(int size);
extern void wipe_o_list(void);
extern s16b o_pop(void);
extern errr get_obj_num_prep(void);
extern s16b get_obj_num(int level);
extern void object_known(object_type *o_ptr);
extern bool object_known_p(object_type const *o_ptr);
extern void object_aware(object_type *o_ptr);
extern bool object_aware_p(object_type const *o_ptr);
extern void object_tried(object_type *o_ptr);
extern bool object_tried_p(object_type const *o_ptr);
extern s32b object_value(object_type const *o_ptr);
extern s32b object_value_real(object_type const *o_ptr);
extern bool_ object_similar(object_type const *o_ptr, object_type const *j_ptr);
extern void object_absorb(object_type *o_ptr, object_type *j_ptr);
extern s16b lookup_kind(int tval, int sval);
extern void object_wipe(object_type *o_ptr);
extern void object_prep(object_type *o_ptr, int k_idx);
extern void object_copy(object_type *o_ptr, object_type *j_ptr);
extern void apply_magic(object_type *o_ptr, int lev, bool_ okay, bool_ good, bool_ great, boost::optional<int> force_power = boost::none);
extern bool_ make_object(object_type *j_ptr, bool_ good, bool_ great, obj_theme const &theme);
extern void place_object(int y, int x, bool_ good, bool_ great, int where);
extern bool_ make_gold(object_type *j_ptr);
extern void place_gold(int y, int x);
extern s16b drop_near(object_type *o_ptr, int chance, int y, int x);
extern void acquirement(int y1, int x1, int num, bool_ great, bool_ known);
extern void combine_pack(void);
extern void reorder_pack(void);
extern void random_artifact_resistance (object_type * o_ptr);
extern s16b floor_carry(int y, int x, object_type *j_ptr);
extern void pack_decay(int item);
extern void floor_decay(int item);
extern s16b m_bonus(int max, int level);
extern s32b flag_cost(object_type const *o_ptr, int plusses);
