#pragma once

#include "spell_type_fwd.hpp"
#include "h-basic.h"
#include "types_fwd.h"

extern s32b NOXIOUSCLOUD;
extern s32b AIRWINGS;
extern s32b INVISIBILITY;
extern s32b POISONBLOOD;
extern s32b THUNDERSTORM;
extern s32b STERILIZE;

casting_result  air_noxious_cloud(int);
const char     *air_noxious_cloud_info();
casting_result  air_wings_of_winds(int);
const char     *air_wings_of_winds_info();
casting_result  air_invisibility(int);
const char     *air_invisibility_info();
casting_result  air_poison_blood(int);
const char     *air_poison_blood_info();
casting_result  air_thunderstorm(int);
const char     *air_thunderstorm_info();
casting_result  air_sterilize(int);
const char     *air_sterilize_info();

extern s32b BLINK;
extern s32b DISARM;
extern s32b TELEPORT;
extern s32b TELEAWAY;
extern s32b RECALL;
extern s32b PROBABILITY_TRAVEL;

casting_result  convey_blink(int);
const char     *convey_blink_info();
casting_result  convey_disarm(int);
casting_result  convey_teleport(int);
const char     *convey_teleport_info();
casting_result  convey_teleport_away(int);
casting_result  convey_recall(int);
const char     *convey_recall_info();
casting_result  convey_probability_travel(int);
const char     *convey_probability_travel_info();

extern s32b DEMON_BLADE;
extern s32b DEMON_MADNESS;
extern s32b DEMON_FIELD;
extern s32b DOOM_SHIELD;
extern s32b UNHOLY_WORD;
extern s32b DEMON_CLOAK;
extern s32b DEMON_SUMMON;
extern s32b DISCHARGE_MINION;
extern s32b CONTROL_DEMON;

casting_result  demonology_demon_blade(int);
const char     *demonology_demon_blade_info();
casting_result  demonology_demon_madness(int);
const char     *demonology_demon_madness_info();
casting_result  demonology_demon_field(int);
const char     *demonology_demon_field_info();
casting_result  demonology_doom_shield(int);
const char     *demonology_doom_shield_info();
casting_result  demonology_unholy_word(int);
const char     *demonology_unholy_word_info();
casting_result  demonology_demon_cloak(int);
const char     *demonology_demon_cloak_info();
casting_result  demonology_summon_demon(int);
const char     *demonology_summon_demon_info();
casting_result  demonology_discharge_minion(int);
const char     *demonology_discharge_minion_info();
casting_result  demonology_control_demon(int);
const char     *demonology_control_demon_info();

extern s32b STARIDENTIFY;
extern s32b IDENTIFY;
extern s32b VISION;
extern s32b SENSEHIDDEN;
extern s32b REVEALWAYS;
extern s32b SENSEMONSTERS;

casting_result  divination_greater_identify(int);
casting_result  divination_identify(int);
const char     *divination_identify_info();
casting_result  divination_vision(int);
casting_result  divination_sense_hidden(int);
const char     *divination_sense_hidden_info();
casting_result  divination_reveal_ways(int);
const char     *divination_reveal_ways_info();
casting_result  divination_sense_monsters(int);
const char     *divination_sense_monsters_info();

extern s32b STONESKIN;
extern s32b DIG;
extern s32b STONEPRISON;
extern s32b STRIKE;
extern s32b SHAKE;

casting_result  earth_stone_skin(int);
const char     *earth_stone_skin_info();
casting_result  earth_dig(int);
casting_result  earth_stone_prison(int);
casting_result  earth_strike(int);
const char     *earth_strike_info();
casting_result  earth_shake(int);
const char     *earth_shake_info();

extern s32b ERU_SEE;
extern s32b ERU_LISTEN;
extern s32b ERU_UNDERSTAND;
extern s32b ERU_PROT;

casting_result  eru_see_the_music(int);
const char     *eru_see_the_music_info();
casting_result  eru_listen_to_the_music(int);
casting_result  eru_know_the_music(int);
casting_result  eru_lay_of_protection(int);
const char     *eru_lay_of_protection_info();

extern s32b GLOBELIGHT;
extern s32b FIREFLASH;
extern s32b FIERYAURA;
extern s32b FIREWALL;
extern s32b FIREGOLEM;

casting_result  fire_globe_of_light(int);
const char     *fire_globe_of_light_info();
casting_result  fire_fireflash(int);
const char     *fire_fireflash_info();
casting_result  fire_fiery_shield(int);
const char     *fire_fiery_shield_info();
casting_result  fire_firewall(int);
const char     *fire_firewall_info();
casting_result  fire_golem(int);
const char     *fire_golem_info();

extern s32b CALL_THE_ELEMENTS;
extern s32b CHANNEL_ELEMENTS;
extern s32b ELEMENTAL_WAVE;
extern s32b VAPORIZE;
extern s32b GEOLYSIS;
extern s32b DRIPPING_TREAD;
extern s32b GROW_BARRIER;
extern s32b ELEMENTAL_MINION;

casting_result  geomancy_call_the_elements(int);
const char     *geomancy_call_the_elements_info();
casting_result  geomancy_channel_elements(int);
casting_result  geomancy_elemental_wave(int);
casting_result  geomancy_vaporize(int);
const char     *geomancy_vaporize_info();
bool_           geomancy_vaporize_depends();
casting_result  geomancy_geolysis(int);
const char     *geomancy_geolysis_info();
bool_           geomancy_geolysis_depends();
casting_result  geomancy_dripping_tread(int);
const char     *geomancy_dripping_tread_info();
bool_           geomancy_dripping_tread_depends();
casting_result  geomancy_grow_barrier(int);
bool_           geomancy_grow_barrier_depends();
casting_result  geomancy_elemental_minion(int);
const char     *geomancy_elemental_minion_info();

extern s32b MANATHRUST;
extern s32b DELCURSES;
extern s32b RESISTS;
extern s32b MANASHIELD;

casting_result  mana_manathrust(int);
const char     *mana_manathrust_info();
casting_result  mana_remove_curses(int);
casting_result  mana_elemental_shield(int);
const char     *mana_elemental_shield_info();
casting_result  mana_disruption_shield(int);
const char     *mana_disruption_shield_info();

extern s32b MANWE_SHIELD;
extern s32b MANWE_AVATAR;
extern s32b MANWE_BLESS;
extern s32b MANWE_CALL;

casting_result  manwe_wind_shield(int);
const char     *manwe_wind_shield_info();
casting_result  manwe_avatar(int);
const char     *manwe_avatar_info();
casting_result  manwe_blessing(int);
const char     *manwe_blessing_info();
casting_result  manwe_call(int);
const char     *manwe_call_info();

extern s32b MELKOR_CURSE;
extern s32b MELKOR_CORPSE_EXPLOSION;
extern s32b MELKOR_MIND_STEAL;

void do_melkor_curse(int m_idx);

casting_result  melkor_curse(int);
casting_result  melkor_corpse_explosion(int);
const char     *melkor_corpse_explosion_info();
casting_result  melkor_mind_steal(int);
const char     *melkor_mind_steal_info();

extern s32b RECHARGE;
extern s32b SPELLBINDER;
extern s32b DISPERSEMAGIC;
extern s32b TRACKER;
extern s32b INERTIA_CONTROL;
extern timer_type *TIMER_INERTIA_CONTROL;

casting_result  meta_recharge(int);
const char     *meta_recharge_info();
casting_result  meta_spellbinder(int);
const char     *meta_spellbinder_info();
casting_result  meta_disperse_magic(int);
casting_result  meta_tracker(int);
casting_result  meta_inertia_control(int);
const char     *meta_inertia_control_info();

void meta_inertia_control_timer_callback();
void meta_inertia_control_calc_mana(int *msp);
void meta_inertia_control_hook_birth_objects();

extern s32b CHARM;
extern s32b CONFUSE;
extern s32b ARMOROFFEAR;
extern s32b STUN;

casting_result mind_charm(int);
const char    *mind_charm_info();
casting_result mind_confuse(int);
const char    *mind_confuse_info();
casting_result mind_armor_of_fear(int);
const char    *mind_armor_of_fear_info();
casting_result mind_stun(int);
const char    *mind_stun_info();

extern s32b MAGELOCK;
extern s32b SLOWMONSTER;
extern s32b ESSENCESPEED;
extern s32b BANISHMENT;

casting_result  tempo_magelock(int);
casting_result  tempo_slow_monster(int);
const char     *tempo_slow_monster_info();
casting_result  tempo_essence_of_speed(int);
const char     *tempo_essence_of_speed_info();
casting_result  tempo_banishment(int);
const char     *tempo_banishment_info();

extern s32b TULKAS_AIM;
extern s32b TULKAS_WAVE;
extern s32b TULKAS_SPIN;

casting_result  tulkas_divine_aim(int);
const char     *tulkas_divine_aim_info();
casting_result  tulkas_wave_of_power(int);
const char     *tulkas_wave_of_power_info();
casting_result  tulkas_whirlwind(int);

extern s32b DRAIN;
extern s32b GENOCIDE;
extern s32b WRAITHFORM;
extern s32b FLAMEOFUDUN;

int udun_in_book(s32b sval, s32b pval);
int levels_in_book(s32b sval, s32b pval);

casting_result  udun_drain(int);
casting_result  udun_genocide(int);
casting_result  udun_wraithform(int);
const char     *udun_wraithform_info();
casting_result  udun_flame_of_udun(int);
const char     *udun_flame_of_udun_info();

extern s32b TIDALWAVE;
extern s32b ICESTORM;
extern s32b ENTPOTION;
extern s32b VAPOR;
extern s32b GEYSER;

casting_result  water_tidal_wave(int);
const char     *water_tidal_wave_info();
casting_result  water_ice_storm(int);
const char     *water_ice_storm_info();
casting_result  water_ent_potion(int);
const char     *water_ent_potion_info();
casting_result  water_vapor(int);
const char     *water_vapor_info();
casting_result  water_geyser(int);
const char     *water_geyser_info();

extern s32b YAVANNA_CHARM_ANIMAL;
extern s32b YAVANNA_GROW_GRASS;
extern s32b YAVANNA_TREE_ROOTS;
extern s32b YAVANNA_WATER_BITE;
extern s32b YAVANNA_UPROOT;

casting_result  yavanna_charm_animal(int);
const char     *yavanna_charm_animal_info();
casting_result  yavanna_grow_grass(int);
const char     *yavanna_grow_grass_info();
casting_result  yavanna_tree_roots(int);
const char     *yavanna_tree_roots_info();
casting_result  yavanna_water_bite(int);
const char     *yavanna_water_bite_info();
casting_result  yavanna_uproot(int);
const char     *yavanna_uproot_info();

extern s32b GROWTREE;
extern s32b HEALING;
extern s32b RECOVERY;
extern s32b REGENERATION;
extern s32b SUMMONANNIMAL;
extern s32b GROW_ATHELAS;

casting_result  nature_grow_trees(int);
const char     *nature_grow_trees_info();
casting_result  nature_healing(int);
const char     *nature_healing_info();
casting_result  nature_recovery(int);
casting_result  nature_regeneration(int);
const char     *nature_regeneration_info();
casting_result  nature_summon_animal(int);
const char     *nature_summon_animal_info();
casting_result  nature_grow_athelas(int);

extern s32b DEVICE_HEAL_MONSTER;
extern s32b DEVICE_SPEED_MONSTER;
extern s32b DEVICE_WISH;
extern s32b DEVICE_SUMMON;
extern s32b DEVICE_MANA;
extern s32b DEVICE_NOTHING;
extern s32b DEVICE_HOLY_FIRE;
extern s32b DEVICE_THUNDERLORDS;

casting_result  device_heal_monster(int);
const char     *device_heal_monster_info();
casting_result  device_haste_monster(int);
const char     *device_haste_monster_info();
casting_result  device_wish(int);
casting_result  device_summon_monster(int);
casting_result  device_mana(int);
const char     *device_mana_info();
casting_result  device_nothing(int);
casting_result  device_holy_fire(int);
const char     *device_holy_fire_info();
casting_result  device_thunderlords(int);

extern s32b MUSIC_STOP;
extern s32b MUSIC_HOLD;
extern s32b MUSIC_CONF;
extern s32b MUSIC_STUN;
extern s32b MUSIC_LITE;
extern s32b MUSIC_HEAL;
extern s32b MUSIC_HERO;
extern s32b MUSIC_TIME;
extern s32b MUSIC_MIND;
extern s32b MUSIC_BLOW;
extern s32b MUSIC_WIND;
extern s32b MUSIC_YLMIR;
extern s32b MUSIC_AMBARKANTA;

casting_result  music_stop_singing_spell(int);
int             music_holding_pattern_lasting();
casting_result  music_holding_pattern_spell(int);
const char     *music_holding_pattern_info();
int             music_illusion_pattern_lasting();
casting_result  music_illusion_pattern_spell(int);
const char     *music_illusion_pattern_info();
int             music_stun_pattern_lasting();
casting_result  music_stun_pattern_spell(int);
const char     *music_stun_pattern_info();
int             music_song_of_the_sun_lasting();
casting_result  music_song_of_the_sun_spell(int);
int             music_flow_of_life_lasting();
casting_result  music_flow_of_life_spell(int);
const char     *music_flow_of_life_info();
int             music_heroic_ballad_lasting();
casting_result  music_heroic_ballad_spell(int);
int             music_hobbit_melodies_lasting();
casting_result  music_hobbit_melodies_spell(int);
const char     *music_hobbit_melodies_info();
int             music_clairaudience_lasting();
casting_result  music_clairaudience_spell(int);
const char     *music_clairaudience_info();
casting_result  music_blow_spell(int);
const char     *music_blow_info();
casting_result  music_gush_of_wind_spell(int);
const char     *music_gush_of_wind_info();
casting_result  music_horns_of_ylmir_spell(int);
const char     *music_horns_of_ylmir_info();
casting_result  music_ambarkanta_spell(int);

extern s32b AULE_FIREBRAND;
extern s32b AULE_ENCHANT_WEAPON;
extern s32b AULE_ENCHANT_ARMOUR;
extern s32b AULE_CHILD;

casting_result  aule_firebrand_spell(int);
const char     *aule_firebrand_info();
casting_result  aule_enchant_weapon_spell(int);
const char     *aule_enchant_weapon_info();
casting_result  aule_enchant_armour_spell(int);
const char     *aule_enchant_armour_info();
casting_result  aule_child_spell(int);
const char     *aule_child_info();

extern s32b MANDOS_TEARS_LUTHIEN;
extern s32b MANDOS_SPIRIT_FEANTURI;
extern s32b MANDOS_TALE_DOOM;
extern s32b MANDOS_CALL_HALLS;

casting_result  mandos_tears_of_luthien_spell(int);
const char     *mandos_tears_of_luthien_info();
casting_result  mandos_spirit_of_the_feanturi_spell(int);
const char     *mandos_spirit_of_the_feanturi_info();
casting_result  mandos_tale_of_doom_spell(int);
const char     *mandos_tale_of_doom_info();
casting_result  mandos_call_to_the_halls_spell(int);
const char     *mandos_call_to_the_halls_info();

extern s32b ULMO_BELEGAER;
extern s32b ULMO_DRAUGHT_ULMONAN;
extern s32b ULMO_CALL_ULUMURI;
extern s32b ULMO_WRATH;

casting_result  ulmo_song_of_belegaer_spell(int);
const char     *ulmo_song_of_belegaer_info();
casting_result  ulmo_draught_of_ulmonan_spell(int);
const char     *ulmo_draught_of_ulmonan_info();
casting_result  ulmo_call_of_the_ulumuri_spell(int);
const char     *ulmo_call_of_the_ulumuri_info();
casting_result  ulmo_wrath_of_ulmo_spell(int);
const char     *ulmo_wrath_of_ulmo_info();

extern s32b VARDA_LIGHT_VALINOR;
extern s32b VARDA_CALL_ALMAREN;
extern s32b VARDA_EVENSTAR;
extern s32b VARDA_STARKINDLER;

casting_result  varda_light_of_valinor_spell(int);
const char     *varda_light_of_valinor_info();
casting_result  varda_call_of_almaren_spell(int);
casting_result  varda_evenstar_spell(int);
casting_result  varda_star_kindler_spell(int);
const char     *varda_star_kindler_info();
