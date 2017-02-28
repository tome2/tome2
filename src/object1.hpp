#pragma once

#include "h-basic.h"
#include "object_filter.hpp"
#include "object_flag_set.hpp"

#include <boost/optional.hpp>
#include <functional>

typedef std::function<boost::optional<int>(object_filter_t const &filter)> select_by_name_t;

extern byte get_item_letter_color(object_type const *o_ptr);
extern void object_pickup(int this_o_idx);
extern void apply_set(s16b a_idx, s16b set_idx);
extern bool_ takeoff_set(s16b a_idx, s16b set_idx);
extern bool_ wield_set(s16b a_idx, s16b set_idx, bool_ silent);
extern bool_ verify(cptr prompt, int item);
extern void flavor_init(void);
extern void reset_visuals(void);
extern int object_power(object_type *o_ptr);
extern bool_ object_flags_no_set;
extern object_flag_set object_flags(object_type const *o_ptr);
extern object_flag_set object_flags_known(object_type const *o_ptr);

extern s32b calc_object_need_exp(object_type const *o_ptr);
extern void object_desc(char *buf, object_type const *o_ptr, int pref, int mode);
extern void object_desc_store(char *buf, object_type *o_ptr, int pref, int mode);
extern bool_ object_out_desc(object_type *o_ptr, FILE *fff, bool_ trim_down, bool_ wait_for_it);
extern char index_to_label(int i);
extern s16b wield_slot_ideal(object_type const *o_ptr, bool_ ideal);
extern s16b wield_slot(object_type const *o_ptr);
extern cptr describe_use(int i);
extern void display_inven(void);
extern void display_equip(void);
extern void show_inven_full();
extern void show_equip_full();
extern void toggle_inven_equip(void);
extern bool_ get_item(int *cp, cptr pmt, cptr str, int mode, object_filter_t const &filter = object_filter::True(), select_by_name_t const &select_by_name = select_by_name_t());
extern cptr item_activation(object_type *o_ptr);
extern void py_pickup_floor(int pickup);
extern void object_gain_level(object_type *o_ptr);
extern byte object_attr(object_type const *o_ptr);
extern byte object_attr_default(object_type *o_ptr);
extern char object_char(object_type const *o_ptr);
extern char object_char_default(object_type const *o_ptr);
extern bool artifact_p(object_type const *o_ptr);
extern bool ego_item_p(object_type const *o_ptr);
extern bool is_ego_p(object_type const *o_ptr, s16b ego);
extern bool cursed_p(object_type const *o_ptr);
