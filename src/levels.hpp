#pragma once

#include "h-basic.h"

bool_ get_dungeon_generator(char *buf);
bool_ get_level_desc(char *buf);
void get_level_flags(void);
bool_ get_dungeon_name(char *buf);
bool_ get_dungeon_special(char *buf);
int get_branch(void);
int get_fbranch(void);
int get_flevel(void);
bool_ get_dungeon_save(char *buf);
