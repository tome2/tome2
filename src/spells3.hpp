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

casting_result  air_noxious_cloud();
const char     *air_noxious_cloud_info();
casting_result  air_wings_of_winds();
const char     *air_wings_of_winds_info();
casting_result  air_invisibility();
const char     *air_invisibility_info();
casting_result  air_poison_blood();
const char     *air_poison_blood_info();
casting_result  air_thunderstorm();
const char     *air_thunderstorm_info();
casting_result  air_sterilize();
const char     *air_sterilize_info();

extern s32b BLINK;
extern s32b DISARM;
extern s32b TELEPORT;
extern s32b TELEAWAY;
extern s32b RECALL;
extern s32b PROBABILITY_TRAVEL;

casting_result  convey_blink();
const char     *convey_blink_info();
casting_result  convey_disarm();
casting_result  convey_teleport();
const char     *convey_teleport_info();
casting_result  convey_teleport_away();
casting_result  convey_recall();
const char     *convey_recall_info();
casting_result  convey_probability_travel();
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

casting_result  demonology_demon_blade();
const char     *demonology_demon_blade_info();
casting_result  demonology_demon_madness();
const char     *demonology_demon_madness_info();
casting_result  demonology_demon_field();
const char     *demonology_demon_field_info();
casting_result  demonology_doom_shield();
const char     *demonology_doom_shield_info();
casting_result  demonology_unholy_word();
const char     *demonology_unholy_word_info();
casting_result  demonology_demon_cloak();
const char     *demonology_demon_cloak_info();
casting_result  demonology_summon_demon();
const char     *demonology_summon_demon_info();
casting_result  demonology_discharge_minion();
const char     *demonology_discharge_minion_info();
casting_result  demonology_control_demon();
const char     *demonology_control_demon_info();

extern s32b STARIDENTIFY;
extern s32b IDENTIFY;
extern s32b VISION;
extern s32b SENSEHIDDEN;
extern s32b REVEALWAYS;
extern s32b SENSEMONSTERS;

casting_result  divination_greater_identify();
casting_result  divination_identify();
const char     *divination_identify_info();
casting_result  divination_vision();
casting_result  divination_sense_hidden();
const char     *divination_sense_hidden_info();
casting_result  divination_reveal_ways();
const char     *divination_reveal_ways_info();
casting_result  divination_sense_monsters();
const char     *divination_sense_monsters_info();

extern s32b STONESKIN;
extern s32b DIG;
extern s32b STONEPRISON;
extern s32b STRIKE;
extern s32b SHAKE;

casting_result  earth_stone_skin();
const char     *earth_stone_skin_info();
casting_result  earth_dig();
casting_result  earth_stone_prison();
casting_result  earth_strike();
const char     *earth_strike_info();
casting_result  earth_shake();
const char     *earth_shake_info();

extern s32b ERU_SEE;
extern s32b ERU_LISTEN;
extern s32b ERU_UNDERSTAND;
extern s32b ERU_PROT;

casting_result  eru_see_the_music();
const char     *eru_see_the_music_info();
casting_result  eru_listen_to_the_music();
casting_result  eru_know_the_music();
casting_result  eru_lay_of_protection();
const char     *eru_lay_of_protection_info();

extern s32b GLOBELIGHT;
extern s32b FIREFLASH;
extern s32b FIERYAURA;
extern s32b FIREWALL;
extern s32b FIREGOLEM;

casting_result  fire_globe_of_light();
const char     *fire_globe_of_light_info();
casting_result  fire_fireflash();
const char     *fire_fireflash_info();
casting_result  fire_fiery_shield();
const char     *fire_fiery_shield_info();
casting_result  fire_firewall();
const char     *fire_firewall_info();
casting_result  fire_golem();
const char     *fire_golem_info();

extern s32b CALL_THE_ELEMENTS;
extern s32b CHANNEL_ELEMENTS;
extern s32b ELEMENTAL_WAVE;
extern s32b VAPORIZE;
extern s32b GEOLYSIS;
extern s32b DRIPPING_TREAD;
extern s32b GROW_BARRIER;
extern s32b ELEMENTAL_MINION;

casting_result  geomancy_call_the_elements();
const char     *geomancy_call_the_elements_info();
casting_result  geomancy_channel_elements();
casting_result  geomancy_elemental_wave();
casting_result  geomancy_vaporize();
const char     *geomancy_vaporize_info();
bool_           geomancy_vaporize_depends();
casting_result  geomancy_geolysis();
const char     *geomancy_geolysis_info();
bool_           geomancy_geolysis_depends();
casting_result  geomancy_dripping_tread();
const char     *geomancy_dripping_tread_info();
bool_           geomancy_dripping_tread_depends();
casting_result  geomancy_grow_barrier();
bool_           geomancy_grow_barrier_depends();
casting_result  geomancy_elemental_minion();
const char     *geomancy_elemental_minion_info();

extern s32b MANATHRUST;
extern s32b DELCURSES;
extern s32b RESISTS;
extern s32b MANASHIELD;

casting_result  mana_manathrust();
const char     *mana_manathrust_info();
casting_result  mana_remove_curses();
casting_result  mana_elemental_shield();
const char     *mana_elemental_shield_info();
casting_result  mana_disruption_shield();
const char     *mana_disruption_shield_info();

extern s32b MANWE_SHIELD;
extern s32b MANWE_AVATAR;
extern s32b MANWE_BLESS;
extern s32b MANWE_CALL;

casting_result  manwe_wind_shield();
const char     *manwe_wind_shield_info();
casting_result  manwe_avatar();
const char     *manwe_avatar_info();
casting_result  manwe_blessing();
const char     *manwe_blessing_info();
casting_result  manwe_call();
const char     *manwe_call_info();

extern s32b MELKOR_CURSE;
extern s32b MELKOR_CORPSE_EXPLOSION;
extern s32b MELKOR_MIND_STEAL;

void do_melkor_curse(int m_idx);

casting_result  melkor_curse();
casting_result  melkor_corpse_explosion();
const char     *melkor_corpse_explosion_info();
casting_result  melkor_mind_steal();
const char     *melkor_mind_steal_info();

extern s32b RECHARGE;
extern s32b SPELLBINDER;
extern s32b DISPERSEMAGIC;
extern s32b TRACKER;
extern s32b INERTIA_CONTROL;
extern timer_type *TIMER_INERTIA_CONTROL;

casting_result  meta_recharge();
const char     *meta_recharge_info();
casting_result  meta_spellbinder();
const char     *meta_spellbinder_info();
casting_result  meta_disperse_magic();
casting_result  meta_tracker();
casting_result  meta_inertia_control();
const char     *meta_inertia_control_info();

void meta_inertia_control_timer_callback();
void meta_inertia_control_calc_mana(int *msp);
void meta_inertia_control_hook_birth_objects();

extern s32b CHARM;
extern s32b CONFUSE;
extern s32b ARMOROFFEAR;
extern s32b STUN;

casting_result mind_charm();
const char    *mind_charm_info();
casting_result mind_confuse();
const char    *mind_confuse_info();
casting_result mind_armor_of_fear();
const char    *mind_armor_of_fear_info();
casting_result mind_stun();
const char    *mind_stun_info();

extern s32b MAGELOCK;
extern s32b SLOWMONSTER;
extern s32b ESSENCESPEED;
extern s32b BANISHMENT;

casting_result  tempo_magelock();
casting_result  tempo_slow_monster();
const char     *tempo_slow_monster_info();
casting_result  tempo_essence_of_speed();
const char     *tempo_essence_of_speed_info();
casting_result  tempo_banishment();
const char     *tempo_banishment_info();

extern s32b TULKAS_AIM;
extern s32b TULKAS_WAVE;
extern s32b TULKAS_SPIN;

casting_result  tulkas_divine_aim();
const char     *tulkas_divine_aim_info();
casting_result  tulkas_wave_of_power();
const char     *tulkas_wave_of_power_info();
casting_result  tulkas_whirlwind();

extern s32b DRAIN;
extern s32b GENOCIDE;
extern s32b WRAITHFORM;
extern s32b FLAMEOFUDUN;

int udun_in_book(s32b sval, s32b pval);
int levels_in_book(s32b sval, s32b pval);

casting_result  udun_drain();
casting_result  udun_genocide();
casting_result  udun_wraithform();
const char     *udun_wraithform_info();
casting_result  udun_flame_of_udun();
const char     *udun_flame_of_udun_info();

extern s32b TIDALWAVE;
extern s32b ICESTORM;
extern s32b ENTPOTION;
extern s32b VAPOR;
extern s32b GEYSER;

casting_result  water_tidal_wave();
const char     *water_tidal_wave_info();
casting_result  water_ice_storm();
const char     *water_ice_storm_info();
casting_result  water_ent_potion();
const char     *water_ent_potion_info();
casting_result  water_vapor();
const char     *water_vapor_info();
casting_result  water_geyser();
const char     *water_geyser_info();

extern s32b YAVANNA_CHARM_ANIMAL;
extern s32b YAVANNA_GROW_GRASS;
extern s32b YAVANNA_TREE_ROOTS;
extern s32b YAVANNA_WATER_BITE;
extern s32b YAVANNA_UPROOT;

casting_result  yavanna_charm_animal();
const char     *yavanna_charm_animal_info();
casting_result  yavanna_grow_grass();
const char     *yavanna_grow_grass_info();
casting_result  yavanna_tree_roots();
const char     *yavanna_tree_roots_info();
casting_result  yavanna_water_bite();
const char     *yavanna_water_bite_info();
casting_result  yavanna_uproot();
const char     *yavanna_uproot_info();

extern s32b GROWTREE;
extern s32b HEALING;
extern s32b RECOVERY;
extern s32b REGENERATION;
extern s32b SUMMONANNIMAL;
extern s32b GROW_ATHELAS;

casting_result  nature_grow_trees();
const char     *nature_grow_trees_info();
casting_result  nature_healing();
const char     *nature_healing_info();
casting_result  nature_recovery();
casting_result  nature_regeneration();
const char     *nature_regeneration_info();
casting_result  nature_summon_animal();
const char     *nature_summon_animal_info();
casting_result  nature_grow_athelas();

extern s32b DEVICE_HEAL_MONSTER;
extern s32b DEVICE_SPEED_MONSTER;
extern s32b DEVICE_WISH;
extern s32b DEVICE_SUMMON;
extern s32b DEVICE_MANA;
extern s32b DEVICE_NOTHING;
extern s32b DEVICE_HOLY_FIRE;
extern s32b DEVICE_THUNDERLORDS;

casting_result  device_heal_monster();
const char     *device_heal_monster_info();
casting_result  device_haste_monster();
const char     *device_haste_monster_info();
casting_result  device_wish();
casting_result  device_summon_monster();
casting_result  device_mana();
const char     *device_mana_info();
casting_result  device_nothing();
casting_result  device_holy_fire();
const char     *device_holy_fire_info();
casting_result  device_thunderlords();

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

casting_result  music_stop_singing_spell();
int             music_holding_pattern_lasting();
casting_result  music_holding_pattern_spell();
const char     *music_holding_pattern_info();
int             music_illusion_pattern_lasting();
casting_result  music_illusion_pattern_spell();
const char     *music_illusion_pattern_info();
int             music_stun_pattern_lasting();
casting_result  music_stun_pattern_spell();
const char     *music_stun_pattern_info();
int             music_song_of_the_sun_lasting();
casting_result  music_song_of_the_sun_spell();
int             music_flow_of_life_lasting();
casting_result  music_flow_of_life_spell();
const char     *music_flow_of_life_info();
int             music_heroic_ballad_lasting();
casting_result  music_heroic_ballad_spell();
int             music_hobbit_melodies_lasting();
casting_result  music_hobbit_melodies_spell();
const char     *music_hobbit_melodies_info();
int             music_clairaudience_lasting();
casting_result  music_clairaudience_spell();
const char     *music_clairaudience_info();
casting_result  music_blow_spell();
const char     *music_blow_info();
casting_result  music_gush_of_wind_spell();
const char     *music_gush_of_wind_info();
casting_result  music_horns_of_ylmir_spell();
const char     *music_horns_of_ylmir_info();
casting_result  music_ambarkanta_spell();

extern s32b AULE_FIREBRAND;
extern s32b AULE_ENCHANT_WEAPON;
extern s32b AULE_ENCHANT_ARMOUR;
extern s32b AULE_CHILD;

casting_result  aule_firebrand_spell();
const char     *aule_firebrand_info();
casting_result  aule_enchant_weapon_spell();
const char     *aule_enchant_weapon_info();
casting_result  aule_enchant_armour_spell();
const char     *aule_enchant_armour_info();
casting_result  aule_child_spell();
const char     *aule_child_info();

extern s32b MANDOS_TEARS_LUTHIEN;
extern s32b MANDOS_SPIRIT_FEANTURI;
extern s32b MANDOS_TALE_DOOM;
extern s32b MANDOS_CALL_HALLS;

casting_result  mandos_tears_of_luthien_spell();
const char     *mandos_tears_of_luthien_info();
casting_result  mandos_spirit_of_the_feanturi_spell();
const char     *mandos_spirit_of_the_feanturi_info();
casting_result  mandos_tale_of_doom_spell();
const char     *mandos_tale_of_doom_info();
casting_result  mandos_call_to_the_halls_spell();
const char     *mandos_call_to_the_halls_info();

extern s32b ULMO_BELEGAER;
extern s32b ULMO_DRAUGHT_ULMONAN;
extern s32b ULMO_CALL_ULUMURI;
extern s32b ULMO_WRATH;

casting_result  ulmo_song_of_belegaer_spell();
const char     *ulmo_song_of_belegaer_info();
casting_result  ulmo_draught_of_ulmonan_spell();
const char     *ulmo_draught_of_ulmonan_info();
casting_result  ulmo_call_of_the_ulumuri_spell();
const char     *ulmo_call_of_the_ulumuri_info();
casting_result  ulmo_wrath_of_ulmo_spell();
const char     *ulmo_wrath_of_ulmo_info();

extern s32b VARDA_LIGHT_VALINOR;
extern s32b VARDA_CALL_ALMAREN;
extern s32b VARDA_EVENSTAR;
extern s32b VARDA_STARKINDLER;

casting_result  varda_light_of_valinor_spell();
const char     *varda_light_of_valinor_info();
casting_result  varda_call_of_almaren_spell();
casting_result  varda_evenstar_spell();
casting_result  varda_star_kindler_spell();
const char     *varda_star_kindler_info();
