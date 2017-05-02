#pragma once

#include "h-basic.h"

bool_ get_dungeon_generator(char *buf);
bool_ get_level_desc(char *buf);
void get_level_flags();
bool_ get_dungeon_name(char *buf);
bool_ get_dungeon_special(char *buf);
int get_branch();
int get_fbranch();
int get_flevel();
bool_ get_dungeon_save(char *buf);
