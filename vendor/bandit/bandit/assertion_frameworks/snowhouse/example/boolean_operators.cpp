#include <snowhouse/snowhouse.h>
using namespace snowhouse;
#include "tests.h"

void BooleanOperators()
{
  std::cout << "================================================" << std::endl;
  std::cout << "   Boolean operators" << std::endl;
  std::cout << "================================================" << std::endl;

  std::cout << "ShouldHandleIsFalseOperator" << std::endl;
  {
    Assert::That(false, IsFalse());
  }

  std::cout << "ShouldHandleWhenIsFalseFails" << std::endl;
  {
    AssertTestFails(Assert::That(true, IsFalse()), "Expected: false");
  }

  std::cout << "ShouldHandleIsTrueOperator" << std::endl;
  {
    Assert::That(true, IsTrue());
  }

  std::cout << "ShouldHandleWhenIsTrueFails" << std::endl;
  {
    AssertTestFails(Assert::That(false, IsTrue()), "Expected: true");
  }

  std::cout << "ShouldHandleFluentIsTrue" << std::endl;
  {
    Assert::That(true, Is().True());
    AssertTestFails(Assert::That(false, Is().True()), "Expected: true");
  }

  std::cout << "ShouldHandleFluentIsFalse" << std::endl;
  {
    Assert::That(false, Is().False());
    AssertTestFails(Assert::That(true, Is().False()), "Expected: false");
  }

  std::cout << "ShouldTreatAssertWithoutConstraintAsBooleanConstrains" << std::endl;
  {
    Assert::That(true);
  }
}

