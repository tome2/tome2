#include <snowhouse/snowhouse.h>
using namespace snowhouse;
#include "tests.h"


template <typename T>
void SequenceContainerActual()
{
  const char* ExpectedActual = "\nActual: [ 1, 2, 3, 5, 8 ]";

  T container;
  container.clear();
  container.push_back(1);
  container.push_back(2);
  container.push_back(3);
  container.push_back(5);
  container.push_back(8);

  std::cout << "ShouldHandleAllOperator" << std::endl;
  {
    Assert::That(container, Has().All().GreaterThan(1).Or().LessThan(4));
  }

  std::cout << "ShouldHandleFailingAllOperator" << std::endl;
  {
    AssertTestFails(Assert::That(container, Has().All().GreaterThan(4)), std::string("Expected: all greater than 4") + ExpectedActual);
  }

  std::cout << "SHouldHandleInvalidExpressionAfterAllOperator" << std::endl;
  {
    AssertTestFails(Assert::That(container, Has().All().Not()), "The expression contains a not operator without any operand");
  }

  std::cout << "ShouldHandleNoExpressionAfterAllOperator" << std::endl;
  {
    AssertTestFails(Assert::That(container, Has().All()), "The expression after \"all\" operator does not yield any result");
  }

  std::cout << "ShouldHandleAtLeastOperator" << std::endl;
  {
    Assert::That(container, Has().AtLeast(1).LessThan(5));
  }

  std::cout << "ShouldHandleFailingAtLeastOperator" << std::endl;
  {
    AssertTestFails(Assert::That(container, Has().AtLeast(2).LessThan(2)), std::string("Expected: at least 2 less than 2") + ExpectedActual);
  }

  std::cout << "ShouldHandleExactlyOperator" << std::endl;
  {
    Assert::That(container, Has().Exactly(1).EqualTo(3));
  }

  std::cout << "ShouldHandleFailingExactlyOperator" << std::endl;
  {
    AssertTestFails(Assert::That(container, Has().Exactly(2).EqualTo(3)), std::string("Expected: exactly 2 equal to 3") + ExpectedActual);
  }

  std::cout << "ShouldHandleAtMostOperator" << std::endl;
  {
    Assert::That(container, Has().AtMost(1).EqualTo(5));
  }

  std::cout << "ShouldHandleFailingAtMostOperator" << std::endl;
  {
    AssertTestFails(Assert::That(container, Has().AtMost(1).EqualTo(3).Or().EqualTo(5)), std::string("Expected: at most 1 equal to 3 or equal to 5") + ExpectedActual);
  }

  std::cout << "ShouldHandleNoneOperator" << std::endl;
  {
    Assert::That(container, Has().None().EqualTo(666));
  }

  std::cout << "ShouldHandleFailingNoneOperator" << std::endl;
  {
    AssertTestFails(Assert::That(container, Has().None().EqualTo(5)), std::string("Expected: none equal to 5") + ExpectedActual);
  }

  std::cout << "ShouldHandleContaining" << std::endl;
  {
    Assert::That(container, Contains(3));
  }

  std::cout << "ShouldDetectFailingContains" << std::endl;
  {
    AssertTestFails(Assert::That(container, Contains(99)), std::string("contains 99") + ExpectedActual);
  }

  std::cout << "ShouldHandleOfLength" << std::endl;
  {
    Assert::That(container, HasLength(5));
  }

  std::cout << "ShouldHandleFailingOfLength" << std::endl;
  {
    AssertTestFails(Assert::That(container, HasLength(7)), std::string("of length 7") + ExpectedActual);
  }

  std::cout << "ShouldHandleContaining_ExpressionTemplates" << std::endl;
  {
    Assert::That(container, Contains(3));
  }

  std::cout << "ShouldDetectFailingContains_ExpressionTemplates" << std::endl;
  {
    AssertTestFails(Assert::That(container, Contains(99)), std::string("contains 99") + ExpectedActual);
  }

  std::cout << "ShouldHandleOfLength_ExpressionTemplates" << std::endl;
  {
    Assert::That(container, HasLength(5));
  }

  std::cout << "ShouldHandleFailingOfLengthForVectors" << std::endl;
  {
    AssertTestFails(Assert::That(container, HasLength(7)), std::string("of length 7") + ExpectedActual);
  }

  std::cout << "ShouldHandleIsEmpty" << std::endl;
  {
    T is_empty;

    Assert::That(is_empty, IsEmpty());
  }

  std::cout << "ShouldHandleFailingIsEmpty" << std::endl;
  {
    AssertTestFails(Assert::That(container, IsEmpty()), "of length 0");
  }

  std::cout << "ShouldHandleFluentIsEmpty" << std::endl;
  {
    T is_empty;

    Assert::That(is_empty, Is().Empty());
  }

  std::cout << "ShouldHandleFailingFluentIsEmpty" << std::endl;
  {
    AssertTestFails(Assert::That(container, Is().Empty()), "of length 0");
  }

  std::cout << "ShouldHandlerEqualsContainer" << std::endl;
  {
    std::list<int> expected;
    expected.assign(container.begin(), container.end());

    AssertThat(container, EqualsContainer(expected));
  }

  std::cout << "ShouldHandleEqualsContainer_Fluent" << std::endl;
  {
    std::list<int> expected;
    expected.assign(container.begin(), container.end());

    AssertThat(container, Is().EqualToContainer(expected));
  }

  std::cout << "ShouldHandleFailingEqualsContainer" << std::endl;
  {
    const int e[] = {4, 2, 4};
    std::list<int> expected(e, e + sizeof(e) / sizeof(e[0]));

    AssertTestFails(Assert::That(container, EqualsContainer(expected)), "Expected: [ 4, 2, 4 ]");
  }

  std::cout << "ShouldHandleFailingEqualsContainer_Fluent" << std::endl;
  {
    const int e[] = {4, 2, 4};
    std::list<int> expected(e, e + sizeof(e) / sizeof(e[0]));

    AssertTestFails(Assert::That(container, Is().EqualToContainer(expected)), "Expected: [ 4, 2, 4 ]");
  }
}

void SequenceContainerTests()
{
  std::cout << "================================================" << std::endl;
  std::cout << "   SequenceContainerTests(vector)" << std::endl;
  std::cout << "================================================" << std::endl;
  SequenceContainerActual<std::vector<int> >();

  std::cout << "================================================" << std::endl;
  std::cout << "   SequenceContainerTests(list)" << std::endl;
  std::cout << "================================================" << std::endl;
  SequenceContainerActual<std::list<int> >();

  std::cout << "================================================" << std::endl;
  std::cout << "   SequenceContainerTests(deque)" << std::endl;
  std::cout << "================================================" << std::endl;
  SequenceContainerActual<std::deque<int> >();
}
