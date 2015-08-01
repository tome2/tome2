
//          Copyright Joakim Karlsson & Kim Gräsman 2010-2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <snowhouse/snowhouse.h>
using namespace snowhouse;
#include "tests.h"

struct IsEvenNumberNoStreamOperator
{
  bool Matches(const int actual) const
  {
    return (actual % 2) == 0; 
  }
};

struct IsEvenNumberWithStreamOperator
{
  bool Matches(const int actual) const
  {
    return (actual % 2) == 0; 
  }

  friend std::ostream& operator<<(std::ostream& stm, 
      const IsEvenNumberWithStreamOperator& );
};

std::ostream& operator<<(std::ostream& stm, 
    const IsEvenNumberWithStreamOperator& )
{
  stm << "An even number";
  return stm;
}

void CustomMatchers()
{
  std::cout << "================================================" << std::endl;
  std::cout << "   CustomMatchersNoStreamOperator" << std::endl;
  std::cout << "================================================" << std::endl;

  std::cout << "CanHandleCustomMatcher" << std::endl;
  {
    Assert::That(2, Fulfills(IsEvenNumberNoStreamOperator()));
  }

  std::cout << "CustomMatcherWithFluent" << std::endl;
  {
    Assert::That(2, Is().Fulfilling(IsEvenNumberNoStreamOperator()));
  }

  std::cout << "OutputsCorrectMessageWhenFails" << std::endl;
  {
    AssertTestFails(Assert::That(3, Fulfills(IsEvenNumberNoStreamOperator())),
        "Expected: [unsupported type]\nActual: 3");
  }


  std::cout << "================================================" << std::endl;
  std::cout << "CustomMatcherWithStreamOperator" << std::endl;
  std::cout << "================================================" << std::endl;

  std::cout << "ErrorMessageUsesCustomStreamOperatorIfAvailable" << std::endl;
  {
    AssertTestFails(Assert::That(3, Fulfills(IsEvenNumberWithStreamOperator())), 
        "Expected: An even number\nActual: 3");
  }
}
