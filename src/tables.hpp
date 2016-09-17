#pragma once

#include "angband.h"
#include "activation.hpp"
#include "between_exit.hpp"
#include "body.hpp"
#include "cli_comm_fwd.hpp"
#include "flags_group.hpp"
#include "gf_name_type.hpp"
#include "inscription_info_type.hpp"
#include "magic_power.hpp"
#include "martial_arts.hpp"
#include "module_type.hpp"
#include "monster_power.hpp"
#include "move_info_type.hpp"
#include "option_type.hpp"
#include "player_defs.hpp"
#include "power_type.hpp"
#include "powers.hpp"
#include "quest_type.hpp"
#include "tactic_info_type.hpp"
#include "tval_desc.hpp"

#include <vector>

extern s16b ddd[9];
extern s16b ddx[10];
extern s16b ddy[10];
extern s16b ddx_ddd[9];
extern s16b ddy_ddd[9];
extern byte adj_mag_mana[];
extern byte adj_mag_fail[];
extern byte adj_mag_stat[];
extern byte adj_chr_gold[];
extern byte adj_wis_sav[];
extern byte adj_dex_dis[];
extern byte adj_int_dis[];
extern byte adj_dex_ta[];
extern byte adj_str_td[];
extern byte adj_dex_th[];
extern byte adj_str_th[];
extern byte adj_str_wgt[];
extern byte adj_str_hold[];
extern byte adj_str_dig[];
extern byte adj_str_blow[];
extern byte adj_dex_blow[];
extern byte adj_dex_safe[];
extern byte adj_con_fix[];
extern byte adj_con_mhp[];
extern byte blows_table[12][12];
extern byte extract_energy[300];
extern s32b player_exp[PY_MAX_LEVEL];
extern cptr color_names[16];
extern cptr stat_names[6];
extern cptr stat_names_reduced[6];
extern cptr window_flag_desc[32];
extern martial_arts bear_blows[MAX_BEAR];
extern martial_arts ma_blows[MAX_MA];
extern magic_power mindcraft_powers[MAX_MINDCRAFT_POWERS];
extern magic_power necro_powers[MAX_NECRO_POWERS];
extern magic_power mimic_powers[MAX_MIMIC_POWERS];
extern magic_power symbiotic_powers[MAX_SYMBIOTIC_POWERS];
extern move_info_type move_info[9];
extern tactic_info_type tactic_info[9];
extern activation activation_info[MAX_T_ACT];
extern inscription_info_type inscription_info[MAX_INSCRIPTIONS];
extern cptr sense_desc[];
extern std::vector<flags_group> const &flags_groups();
extern power_type powers_type[POWER_MAX];
extern cptr artifact_names_list;
extern monster_power monster_powers[MONSTER_POWERS_MAX];
extern tval_desc tvals[];
extern tval_desc tval_descs[];
extern between_exit between_exits[MAX_BETWEEN_EXITS];
extern int month_day[9];
extern cptr month_name[9];
extern cli_comm *cli_info;
extern int cli_total;
extern quest_type quest[MAX_Q_IDX];
extern int max_body_part[BODY_MAX];
extern gf_name_type gf_names[];
extern module_type modules[MAX_MODULES];
