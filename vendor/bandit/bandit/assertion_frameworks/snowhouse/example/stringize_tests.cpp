#include <snowhouse/snowhouse.h>
using namespace snowhouse;
#include "tests.h"

namespace
{
  // No overload for operator<<(std::ostream&) or specialization of igloo::Stringizer
  struct WithoutStreamOperator
  {
    WithoutStreamOperator(int id)
    : m_id(id)
    {
    }

    bool operator==(const WithoutStreamOperator& rhs) const
    {
      return m_id == rhs.m_id;
    }

    int m_id;
  };

  // Has operator<<(std::ostream&)
  struct WithStreamOperator : public WithoutStreamOperator
  {
    WithStreamOperator(int id)
    : WithoutStreamOperator(id)
    {
    }
  };
   
  std::ostream& operator<<(std::ostream& stream, const WithStreamOperator& a)
  {
    stream << a.m_id;
    return stream;
  }

  // Has no operator<<(std::ostream&), but a specialization of igloo::Stringizer
  struct WithoutStreamOperatorButWithStringizer : public WithoutStreamOperator
  {
    WithoutStreamOperatorButWithStringizer(int id)
    : WithoutStreamOperator(id)
    {
    }
  };
}

namespace snowhouse {

  template<>
  struct Stringizer< WithoutStreamOperatorButWithStringizer >
  {
    static std::string ToString(const WithoutStreamOperatorButWithStringizer& value)
    {
      return snowhouse::Stringize(value.m_id);
    }
  };
}

void StringizeTests()
{
  std::cout << "================================================" << std::endl;
  std::cout << "   StringizeTests" << std::endl;
  std::cout << "================================================" << std::endl;

  std::cout << "ShouldHandleTypesWithStreamOperators" << std::endl;
  {
    WithStreamOperator a(12);
    WithStreamOperator b(13);
    AssertTestFails(Assert::That(a, Is().EqualTo(b)), "Expected: equal to 13\nActual: 12");
  }

  std::cout << "ShouldHandleTypesWithoutStreamOperators" << std::endl;
  {
    WithoutStreamOperator a(12);
    WithoutStreamOperator b(13);
    AssertTestFails(Assert::That(a, Is().EqualTo(b)), "Expected: equal to [unsupported type]\nActual: [unsupported type]");
  }

  std::cout << "ShouldHandleTypesWithTraits" << std::endl;
  {
    WithoutStreamOperatorButWithStringizer a(12);
    WithoutStreamOperatorButWithStringizer b(13);
    AssertTestFails(Assert::That(a, Is().EqualTo(b)), "Expected: equal to 13\nActual: 12");
  }

  std::cout << "================================================" << std::endl;
  std::cout << "   StringizeTestsExpressionTemplates" << std::endl;
  std::cout << "================================================" << std::endl;

  std::cout << "ShouldHandleTypesWithStreamOperators" << std::endl;
  {
    WithStreamOperator a(12);
    WithStreamOperator b(13);
    AssertTestFails(Assert::That(a, Equals(b)), "Expected: equal to 13\nActual: 12");
  }

  std::cout << "ShouldHandleTypesWithoutStreamOperators" << std::endl;
  {
    WithoutStreamOperator a(12);
    WithoutStreamOperator b(13);
    AssertTestFails(Assert::That(a, Equals(b)), "Expected: equal to [unsupported type]\nActual: [unsupported type]");
  }

  std::cout << "ShouldHandleTypesWithTraits" << std::endl;
  {
    WithoutStreamOperatorButWithStringizer a(12);
    WithoutStreamOperatorButWithStringizer b(13);
    AssertTestFails(Assert::That(a, Is().EqualTo(b)), "Expected: equal to 13\nActual: 12");
  }
} 
