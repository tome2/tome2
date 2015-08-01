#include <snowhouse/snowhouse.h>
using namespace snowhouse;
#include "tests.h"

void StringTests()
{
  std::cout << "================================================" << std::endl;
  std::cout << "   StringTests" << std::endl;
  std::cout << "================================================" << std::endl;

  std::cout << "ShouldHandleStringContainsConstraint" << std::endl;
  {
    Assert::That("abcdef", Contains("bcde"));
  }

  std::cout << "StringConstraintShouldHandleMatchAtBeginningOfString" << std::endl;
  {
    Assert::That("abcdef", Contains("a"));
  }  

  std::cout << "ShouldDetectFailingContains" << std::endl;
  {
    AssertTestFails(Assert::That("abcdef", Contains("hello")), "contains hello");
  }

  std::cout << "ShouldHandleStringStartingWithConstraint" << std::endl;
  {
    Assert::That("abcdef", StartsWith("abc"));
  }

  std::cout << "ShouldHandleStringEndingWithConstraint" << std::endl;
  {
    Assert::That("abcdef", EndsWith("def"));
  }

  std::cout << "ShouldHandleOperatorsForStrings" << std::endl;
  {
    Assert::That("abcdef", StartsWith("ab") && EndsWith("ef"));
  }

  std::cout << "ShouldHandleStringsWithMultipleOperators" << std::endl;
  {
    Assert::That("abcdef", StartsWith("ab") && !EndsWith("qwqw"));
  }

  std::cout << "ShouldHandleOfLength" << std::endl;
  {
    Assert::That("12345", HasLength(5));
  }

  std::cout << "ShouldHandleWeirdLongExpressions" << std::endl;
  {
    Assert::That("12345", HasLength(5) && StartsWith("123") && !EndsWith("zyxxy"));
  }

  std::cout << "ShouldHandleStdStrings" << std::endl;
  {
    Assert::That("12345", Contains(std::string("23")));
  }

  std::cout << "ShouldHandleSimpleChar" << std::endl;
  {
    Assert::That("12345", StartsWith('1'));
  }
}
