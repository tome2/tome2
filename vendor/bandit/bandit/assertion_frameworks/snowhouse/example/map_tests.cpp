#include <snowhouse/snowhouse.h>
using namespace snowhouse;
#include "tests.h"

void MapTests()
{
  std::cout << "================================================" << std::endl;
  std::cout << "   MapTests" << std::endl;
  std::cout << "================================================" << std::endl;

  std::map<std::string, int> ages;
  ages["joakim"] = 38;
  ages["maria"] = 36;
  ages["hanna"] = 6;
  ages["moa"] = 4;

  std::cout << "ContainingShouldDetermineIfKeyExists" << std::endl;
  {
    Assert::That(ages, Is().Containing("joakim"));
  }
  
  std::cout << "ShouldGiveAProperMessageWhenContainingFails" << std::endl;
  {
    AssertTestFails(Assert::That(ages, Is().Not().Containing("hanna")), 
        "Expected: not contains hanna"); 
  }

  std::cout << "ContainingShouldDetermineIfKeyExists" << std::endl;
  {
    Assert::That(ages, Contains("joakim"));
  }
  
  std::cout << "ShouldGiveAProperMessageWhenContainingFails" << std::endl;
  {
    AssertTestFails(Assert::That(ages, !Contains("hanna")), 
        "Expected: not contains hanna"); 
  }
}
