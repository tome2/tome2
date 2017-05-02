#pragma once

#include "h-basic.h"
#include "object_type_fwd.hpp"
#include "monster_race_fwd.hpp"
#include "monster_power_fwd.hpp"

#include <string>
#include <vector>

bool_ is_magestaff(void);
void do_cmd_browse_aux(object_type *o_ptr);
void do_cmd_browse(void);
void fetch(int dir, int wgt, bool_ require_los);
void do_poly_self(void);
std::string symbiote_name(bool capitalize);
int use_symbiotic_power(int r_idx, bool great);
void use_monster_power(int r_idx, bool great);
bool_ is_ok_spell(s32b spell_idx, s32b pval);
s32b get_school_spell(cptr do_what, s16b force_book);
void do_cmd_copy_spell(void);
void cast_school_spell(void);
std::vector<monster_power const *> extract_monster_powers(monster_race const *r_ptr, bool great);
