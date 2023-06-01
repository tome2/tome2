#pragma once

#include "h-basic.hpp"
#include "object_type_fwd.hpp"
#include "monster_race_fwd.hpp"
#include "monster_power_fwd.hpp"

#include <string>
#include <vector>

bool is_magestaff();
void do_cmd_browse_aux(object_type *o_ptr);
void do_cmd_browse();
void fetch(int dir, int wgt, bool require_los);
void do_poly_self();
std::string symbiote_name(bool capitalize);
int use_symbiotic_power(int r_idx, bool great);
void use_monster_power(int r_idx, bool great);
bool is_ok_spell(s32b spell_idx, s32b pval);
s32b get_school_spell(const char *do_what, s16b force_book);
void do_cmd_copy_spell();
void cast_school_spell();
std::vector<monster_power const *> extract_monster_powers(monster_race const *r_ptr, bool great);
