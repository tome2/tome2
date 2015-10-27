#include <stdexcept>
#include <sstream>
#include <snowhouse/snowhouse.h>
using namespace snowhouse;
#include "tests.h"

void throwRuntimeError() {
   throw std::runtime_error("This is expected");
}

struct IgnoreErrors {
   template <class ExpectedType, class ActualType>
   static void Handle(const ExpectedType&, const ActualType&, const char*, int)
   {
   }

   static void Handle(const std::string&)
   {
   }
};

void BasicAssertions()
{
  std::cout << "================================================" << std::endl;
  std::cout << "    ASSERTIONS " << std::endl;
  std::cout << "================================================" << std::endl;

  std::cout << "ShouldHandleIntegerEquality" << std::endl;  
	{
		Assert::That(5, Is().EqualTo(5));		
	}
	
  std::cout << "ShouldDetectIntegerInequality" << std::endl;
	{
		AssertTestFails(Assert::That(5, Is().EqualTo(4)), "equal to 4");
	}
	
  std::cout << "ShouldDetectIfNotFails" << std::endl;
	{
		AssertTestFails(Assert::That(5, Is().Not().EqualTo(5)), "Expected: not equal to 5\nActual: 5\n");
	}
	
  std::cout << "ShouldHandleStrings" << std::endl;
	{
		Assert::That(std::string("joakim"), Is().EqualTo(std::string("joakim")));
	}
	
  std::cout << "ShouldHandleStringsWithoutExplicitTemplateSpecialization" << std::endl;
	{
		Assert::That("kim", Is().EqualTo("kim"));
	}
	
  std::cout << "ShouldHandleGreaterThan" << std::endl;
	{
		Assert::That(5, Is().GreaterThan(4));
	}
	
  std::cout << "ShouldDetectWhenGreaterThanFails" << std::endl;
	{
		AssertTestFails(Assert::That(5, Is().GreaterThan(5)),
        "Expected: greater than 5\nActual: 5\n");
	}
	
  std::cout << "ShouldHandleLessThan" << std::endl;
	{
		Assert::That(5, Is().LessThan(6));
	}
	
  std::cout << "ShouldDetectWhenLessThanFails" << std::endl;
	{
		AssertTestFails(Assert::That(6, Is().LessThan(5)),
        "Expected: less than 5\nActual: 6\n");
	}
	
  std::cout << "ShouldThrowExplicitFailureMessage" << std::endl;
	{
		AssertTestFails(Assert::Failure("foo"), "foo");
	}

  std::cout << "Should contain location information" << std::endl;
  {
    int line;
    std::string file;

    try
    {
      Assert::That(5, Equals(2), "filename", 32);
    }
    catch(const AssertionException& e)
    {
      line = e.GetLineNumber();
      file = e.GetFilename();
    }

    Assert::That(line, Equals(32));
    Assert::That(file, Equals("filename"));
  }

  std::cout << "ShouldEnsureExceptionIsThrown" << std::endl;  
    {
          
      AssertThrows(std::runtime_error, throwRuntimeError());
    }

  std::cout << "ShouldIgnoreTheError" << std::endl;  
    {
      ConfigurableAssert<IgnoreErrors>::That(1, Equals(2));
    }

  std::cout << "================================================" << std::endl;
  std::cout << "    ASSERTIONS EXPRESSION TEMPLATES" << std::endl;
  std::cout << "================================================" << std::endl;

  std::cout << "ShouldHandleIntegerEquality" << std::endl;
	{
		Assert::That(5, Equals(5));
	}                                                            
	
  std::cout << "ShouldDetectIntegerInequality" << std::endl;
	{
		AssertTestFails(Assert::That(5, Equals(4)), "equal to 4");
	}
	
  std::cout << "ShouldDetectIfNotFails" << std::endl;
	{
		AssertTestFails(Assert::That(5, !Equals(5)), 
        "Expected: not equal to 5\nActual: 5\n");
	}
	
  std::cout << "ShouldHandleStrings" << std::endl;
	{
		Assert::That(std::string("joakim"), Equals(std::string("joakim")));
	}
	
  std::cout << "ShouldHandleStringsWithoutExplicitTemplateSpecialization" 
    << std::endl;
	{
		Assert::That("kim", Equals("kim"));
	}
	
  std::cout << "ShouldHandleGreaterThan" << std::endl;
	{
		Assert::That(5, IsGreaterThan(4));
	}

  std::cout << "ShouldHandleGreaterThanOrEqualTo" << std::endl;
	{
		Assert::That(4, IsGreaterThanOrEqualTo(4));
		Assert::That(5, IsGreaterThanOrEqualTo(4));
	}
	
  std::cout << "ShouldDetectWhenGreaterThanFails" << std::endl;
	{
		AssertTestFails(Assert::That(5, IsGreaterThan(5)),
        "Expected: greater than 5\nActual: 5\n");
	}

  std::cout << "ShouldDetectWhenGreaterThanOrEqualToFails" << std::endl;
	{
		AssertTestFails(Assert::That(4, IsGreaterThanOrEqualTo(5)),
        "Expected: greater than or equal to 5\nActual: 4\n");
	}

  std::cout << "ShouldHandleLessThan" << std::endl;
	{
		Assert::That(5, IsLessThan(6));
	}

  std::cout << "ShouldHandleLessThanOrEqualTo" << std::endl;
	{
		Assert::That(5, IsLessThanOrEqualTo(6));
		Assert::That(6, IsLessThanOrEqualTo(6));
	}
	
  std::cout << "ShouldDetectWhenLessThanFails" << std::endl;
	{
		AssertTestFails(Assert::That(6, IsLessThan(5)),
        "Expected: less than 5\nActual: 6\n");
	}

  std::cout << "ShouldDetectWhenLessThanOrEqualToFails" << std::endl;
	{
		AssertTestFails(Assert::That(6, IsLessThanOrEqualTo(5)),
			"Expected: less than or equal to 5\nActual: 6\n");
	}

#if __cplusplus > 199711L
  std::cout << "ShouldHandleNull" << std::endl;
    {
       Assert::That(nullptr, IsNull());
    }

  std::cout << "ShouldHandleNull" << std::endl;
    {
       Assert::That(nullptr, Is().Null());
    }

  std::cout << "ShouldHandleNotNull" << std::endl;
    {
       int anInt = 0;
       Assert::That(&anInt, ! IsNull());
    }

  std::cout << "ShouldDetectWhenIsNullFails" << std::endl;
    {
       int anInt = 0;
       std::ostringstream message;
       message << "Expected: equal to nullptr\nActual: " << &anInt << "\n";
       AssertTestFails(Assert::That(&anInt, IsNull()), message.str());
    }

  std::cout << "ShouldDetectWhenIsNullFails" << std::endl;
    {
       int anInt = 0;
       std::ostringstream message;
       message << "Expected: equal to nullptr\nActual: " << &anInt << "\n";
       AssertTestFails(Assert::That(&anInt, Is().Null()), message.str());
    }

  std::cout << "ShouldDetectWhenIsNotNullFails" << std::endl;
    {
       std::ostringstream message;
       message << "Expected: not equal to nullptr\nActual: nullptr\n";

       AssertTestFails(Assert::That(nullptr, ! IsNull()), message.str());
    }
#endif
}
