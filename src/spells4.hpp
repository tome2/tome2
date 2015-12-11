#pragma once

#include "h-basic.h"
#include "object_type_fwd.hpp"
#include "school_book_fwd.hpp"

extern s32b SCHOOL_AIR;
extern s32b SCHOOL_AULE;
extern s32b SCHOOL_CONVEYANCE;
extern s32b SCHOOL_DEMON;
extern s32b SCHOOL_DEVICE;
extern s32b SCHOOL_DIVINATION;
extern s32b SCHOOL_EARTH;
extern s32b SCHOOL_ERU;
extern s32b SCHOOL_FIRE;
extern s32b SCHOOL_GEOMANCY;
extern s32b SCHOOL_MANA;
extern s32b SCHOOL_MANDOS;
extern s32b SCHOOL_MANWE;
extern s32b SCHOOL_MELKOR;
extern s32b SCHOOL_META;
extern s32b SCHOOL_MIND;
extern s32b SCHOOL_MUSIC;
extern s32b SCHOOL_NATURE;
extern s32b SCHOOL_TEMPORAL;
extern s32b SCHOOL_TULKAS;
extern s32b SCHOOL_UDUN;
extern s32b SCHOOL_ULMO;
extern s32b SCHOOL_VARDA;
extern s32b SCHOOL_WATER;
extern s32b SCHOOL_YAVANNA;

void print_spell_desc(int s, int y);
void init_school_books();
school_book *school_books_at(int sval);
void school_book_add_spell(school_book *school_book, s32b spell_idx);
void random_book_setup(s16b sval, s32b spell_idx);
int print_spell(cptr label, byte color, int y, s32b s);
int school_book_length(int sval);
int spell_x(int sval, int spell_idx, int i);
bool_ school_book_contains_spell(int sval, s32b spell_idx);
void lua_cast_school_spell(s32b spell_idx, bool_ no_cost);
