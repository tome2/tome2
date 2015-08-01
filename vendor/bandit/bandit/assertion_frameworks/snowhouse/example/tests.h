#ifndef SNOWHOUSE_EXAMPLES_TEST_H
#define SNOWHOUSE_EXAMPLES_TEST_H

#define AssertTestFails(assertion, expected_error_text) \
  std::string IGLOO_INTERNAL_expected_error = "Test did not fail"; \
  try \
  { \
    assertion; \
  }  \
  catch(const AssertionException& exception_from_igloo_assertion)  \
  {  \
  IGLOO_INTERNAL_expected_error = exception_from_igloo_assertion.GetMessage();  \
  }  \
  Assert::That(IGLOO_INTERNAL_expected_error, Is().Containing(expected_error_text));

#endif
