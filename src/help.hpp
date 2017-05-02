#pragma once

#include "h-basic.h"

#include <string>

void init_hooks_help();
void help_race(std::string const &race);
void help_subrace(std::string const &subrace);
void help_class(std::string const &klass);
void help_god(cptr god);
void help_skill(std::string const &skill);
void help_ability(std::string const &ability);
