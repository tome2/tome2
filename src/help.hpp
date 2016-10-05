#pragma once

#include "h-basic.h"

#include <string>

extern void init_hooks_help();
extern void help_race(std::string const &race);
extern void help_subrace(std::string const &subrace);
extern void help_class(std::string const &klass);
extern void help_god(cptr god);
extern void help_skill(cptr skill);
extern void help_ability(cptr ability);
