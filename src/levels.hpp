#pragma once

#include "h-basic.h"

extern bool_ get_dungeon_generator(char *buf);
extern bool_ get_level_desc(char *buf);
extern void get_level_flags(void);
extern bool_ get_dungeon_name(char *buf);
extern bool_ get_dungeon_special(char *buf);
extern int get_branch(void);
extern int get_fbranch(void);
extern int get_flevel(void);
extern bool_ get_dungeon_save(char *buf);
