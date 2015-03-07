#pragma once

#include "angband.h"

extern byte get_item_letter_color(object_type *o_ptr);
extern void object_pickup(int this_o_idx);
extern bool_ apply_set(s16b a_idx, s16b set_idx);
extern bool_ takeoff_set(s16b a_idx, s16b set_idx);
extern bool_ wield_set(s16b a_idx, s16b set_idx, bool_ silent);
extern bool_ verify(cptr prompt, int item);
extern void flavor_init(void);
extern void reset_visuals(void);
extern int object_power(object_type *o_ptr);
extern bool_ object_flags_no_set;
extern void object_flags(object_type *o_ptr, u32b *f1, u32b *f2, u32b *f3, u32b *f4, u32b *f5, u32b *esp);
extern void object_flags_known(object_type *o_ptr, u32b *f1, u32b *f2, u32b *f3, u32b *f4, u32b *f5, u32b *esp);
extern void object_desc(char *buf, object_type *o_ptr, int pref, int mode);
extern void object_desc_store(char *buf, object_type *o_ptr, int pref, int mode);
extern bool_ object_out_desc(object_type *o_ptr, FILE *fff, bool_ trim_down, bool_ wait_for_it);
extern char index_to_label(int i);
extern s16b wield_slot_ideal(object_type *o_ptr, bool_ ideal);
extern s16b wield_slot(object_type *o_ptr);
extern cptr describe_use(int i);
extern bool_ item_tester_okay(object_type *o_ptr);
extern void display_inven(void);
extern void display_equip(void);
extern void show_inven();
extern void show_equip();
extern void toggle_inven_equip(void);
extern bool_ (*get_item_extra_hook)(int *cp);
extern bool_ get_item(int *cp, cptr pmt, cptr str, int mode);
extern cptr item_activation(object_type *o_ptr,byte num);
extern void py_pickup_floor(int pickup);
extern void object_gain_level(object_type *o_ptr);
