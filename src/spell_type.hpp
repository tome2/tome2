#pragma once

#include "spell_type_fwd.h"
#include <string>
#include <functional>

void spell_type_description_foreach(spell_type *spell, std::function<void (std::string const &text)>);
