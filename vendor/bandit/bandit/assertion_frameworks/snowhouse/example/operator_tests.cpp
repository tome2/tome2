#include <snowhouse/snowhouse.h>
using namespace snowhouse;
#include "tests.h"

void OperatorTests()
{
  std::cout << "================================================" << std::endl;
  std::cout << "   OperatorTests" << std::endl;
  std::cout << "================================================" << std::endl;

  std::cout << "ShouldHandleAndOperatorExpressionTemplates" << std::endl;
  {
    Assert::That(5, IsLessThan(6) && IsGreaterThan(4));
  } 

  std::cout << "ShouldHandleAndOperator" << std::endl;
  {
    Assert::That(5, Is().LessThan(6).And().GreaterThan(4));
  }

  std::cout << "ShouldHandleAndOperatorFailExpressionTemplates" << std::endl;
  {
    AssertTestFails(Assert::That(5, IsLessThan(7) && IsGreaterThan(5)), 
        "less than 7 and greater than 5");
  } 

  std::cout << "ShouldHandleAndOperatorFail" << std::endl;
  {
    AssertTestFails(Assert::That(5, Is().LessThan(7).And().GreaterThan(5)), 
        "less than 7 and greater than 5");
  }

  std::cout << "ShouldHandleOrOperator" << std::endl;
  {
    Assert::That(12, Is().LessThan(7).Or().GreaterThan(5));
  } 

  std::cout << "ShouldHandleOrOperatorExpressionTemplates" << std::endl;
  {
    Assert::That(12, IsLessThan(7) || IsGreaterThan(5));
  }

  std::cout << "ShouldHandleOrOperatorFails" << std::endl;
  {
    AssertTestFails(Assert::That(67, Is().LessThan(12).Or().GreaterThan(99)), 
        "less than 12 or greater than 99");
  }
   
  std::cout << "ShouldHandleOrOperatorFailsExpressionTemplates" << std::endl;
  {
    AssertTestFails(Assert::That(67, IsLessThan(12) || IsGreaterThan(99)), 
        "less than 12 or greater than 99");
  }
  
  std::cout << "ShouldHandleNotOperators" << std::endl;
  {
    Assert::That(5, Is().Not().EqualTo(4));
  }

  std::cout << "ShouldHandleNotOperatorsExpressionTemplates" << std::endl;
  {
    Assert::That(5, !Equals(4));
  }

  std::cout << "ShouldHandleNotOperatorsFails" << std::endl;
  {
    AssertTestFails(Assert::That(12, Is().Not().EqualTo(12)), "not equal to 12");
  } 

  std::cout << "ShouldHandleNotOperatorsFailsExpressionTemplates" << std::endl;
  {
    AssertTestFails(Assert::That(12, !Equals(12)), "not equal to 12");
  }

  std::cout << "ShouldHandleNotOperatorsForStrings" << std::endl;
  {
    Assert::That("joakim", Is().Not().EqualTo("harry"));
  } 

  std::cout << "ShouldHandleNotOperatorsForStringsExpressionTemplates" << std::endl;
  {
    Assert::That("joakim", !Equals("harry"));
  }

  std::cout << "ShouldHandleBothLeftAndRightAssociativeOperators" << std::endl;
  {
    Assert::That(5, Is().GreaterThan(4).And().Not().LessThan(3));
  } 

  std::cout << "ShouldHandleBothLeftAndRightAssociativeOperatorsExpressionTemplates" << std::endl;
  {
    Assert::That(5, IsGreaterThan(4)&& !IsLessThan(3));
  }
   
  std::cout << "MalformedExpressionYieldsError" << std::endl;
  {
    AssertTestFails(Assert::That(4, Is().Not()), 
        "The expression contains a not operator without any operand");
  }

  std::cout << 
    "EqualsWithDeltaOperator_should_fail_for_actual_larger_than_delta" 
    << std::endl;
  {
    AssertTestFails(Assert::That(3.9, EqualsWithDelta(3, 0.5)), 
        "Expected: equal to 3 (+/- 0.5)");
  }

  std::cout << "EqualsWithDeltaOperator_should_fail_for_actual_less_than_delta" << std::endl;
  {
    AssertTestFails(Assert::That(2.49, EqualsWithDelta(3, 0.5)), 
        "Expected: equal to 3 (+/- 0.5)");
  }

  std::cout << "EqualsWithDeltaOperator_should_succeed" << std::endl;
  {
    Assert::That(2, EqualsWithDelta(1.9, 0.1));
  }

  std::cout << "Fluent_equals_with_delta_should_fail_for_actual_larger_than_delta" << std::endl;
  {
    AssertTestFails(Assert::That(3.9, Is().EqualToWithDelta(3, 0.5)), 
        "Expected: equal to 3 (+/- 0.5)");
  }

  std::cout << "Fluent_EqualsWithDeltaOperator_should_fail_for_actual_less_than_delta" << std::endl;
  {
    AssertTestFails(Assert::That(2.49, Is().EqualToWithDelta(3, 0.5)), 
        "Expected: equal to 3 (+/- 0.5)");
  }

  std::cout << "Fluent_EqualsWithDeltaOperator_should_succeed" << std::endl;
  {
    Assert::That(2, Is().EqualToWithDelta(1.9, 0.1));
  }

}
