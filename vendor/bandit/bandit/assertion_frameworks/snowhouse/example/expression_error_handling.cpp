#include <snowhouse/snowhouse.h>
using namespace snowhouse;
#include "tests.h"

void ExpressionErrorHandling()
{
  std::cout << "================================================" << std::endl;
  std::cout << "   ExpressionErrorHandling" << std::endl;
  std::cout << "================================================" << std::endl;

  std::vector<int> collection;
  collection.push_back(1);
  collection.push_back(2);
  collection.push_back(3);
  
  std::cout << "AnInvalidAllOperationShouldBeReportedProperly" << std::endl;
  {
    AssertTestFails(Assert::That(collection, Has().All()), 
        "The expression after \"all\" operator does not yield any result");
  }
  
  std::cout << "AnInvalidAtLeastOperationShouldBeReportedProperly" << std::endl;
  {
    AssertTestFails(Assert::That(collection, Has().AtLeast(2)),
        "The expression after \"at least 2\" operator does not yield any result");
  }
  
}
