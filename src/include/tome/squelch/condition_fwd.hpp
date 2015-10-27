#ifndef H_1122f873_e83d_4af8_b6a8_a9e8db195473
#define H_1122f873_e83d_4af8_b6a8_a9e8db195473

#include <functional>
#include <memory>

namespace squelch {

enum class match_type : int;
class Condition;
typedef std::function< std::shared_ptr<Condition> () > ConditionFactory;

} // namespace

#endif
