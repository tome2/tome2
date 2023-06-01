#pragma once

#include "h-basic.hpp"
#include "object_filter.hpp"
#include "object_flag_set.hpp"

#include <boost/optional.hpp>
#include <functional>

typedef std::function<boost::optional<int>(object_filter_t const &filter)> select_by_name_t;

byte get_item_letter_color(object_type const *o_ptr);
void object_pickup(int this_o_idx);
void apply_set(s16b a_idx, s16b set_idx);
bool takeoff_set(s16b a_idx, s16b set_idx);
bool wield_set(s16b a_idx, s16b set_idx, bool silent);
bool verify(const char *prompt, int item);
void flavor_init();
void reset_visuals();
boost::optional<int> object_power(object_type *o_ptr);
extern bool object_flags_no_set;
object_flag_set object_flags(object_type const *o_ptr);
object_flag_set object_flags_known(object_type const *o_ptr);

s32b calc_object_need_exp(object_type const *o_ptr);
void object_desc(char *buf, object_type const *o_ptr, int pref, int mode);
void object_desc_store(char *buf, object_type *o_ptr, int pref, int mode);
bool object_out_desc(object_type *o_ptr, FILE *fff, bool trim_down, bool wait_for_it);
char index_to_label(int i);
s16b wield_slot_ideal(object_type const *o_ptr, bool ideal);
s16b wield_slot(object_type const *o_ptr);
const char *describe_use(int i);
void display_inven();
void display_equip();
void show_inven_full();
void show_equip_full();
void toggle_inven_equip();
bool get_item(int *cp, const char *pmt, const char *str, int mode, object_filter_t const &filter = object_filter::True(), select_by_name_t const &select_by_name = select_by_name_t());
const char *item_activation(object_type *o_ptr);
void py_pickup_floor(int pickup);
void object_gain_level(object_type *o_ptr);
byte object_attr(object_type const *o_ptr);
byte object_attr_default(object_type const *o_ptr);
char object_char(object_type const *o_ptr);
char object_char_default(object_type const *o_ptr);
bool artifact_p(object_type const *o_ptr);
bool ego_item_p(object_type const *o_ptr);
bool is_ego_p(object_type const *o_ptr, s16b ego);
bool cursed_p(object_type const *o_ptr);
