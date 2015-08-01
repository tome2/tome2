#include <snowhouse/snowhouse.h>
using namespace snowhouse;
#include "tests.h"

void StringLineTests()
{
  std::cout << "================================================" << std::endl;
  std::cout << "   StringLineTests" << std::endl;
  std::cout << "================================================" << std::endl;

  std::cout << "CanAssertThatAtLeastOneLineInAStreamMatches" << std::endl;
  {
    Assert::That("First line\n", Has().AtLeast(1).EqualTo("First line"));
  }

  std::cout << "CanDetectWhenAssertionFails" << std::endl;
  {
    AssertTestFails(Assert::That("First line\n", Has().AtLeast(1).EqualTo("Second line")), "Expected: at least 1 equal to Second line");
  }

  std::cout << "CanHandleLineMissingNewline" << std::endl;
  {
    Assert::That("First line", Has().AtLeast(1).EqualTo("First line"));
  }

  std::cout << "CanHandleSeveralLines" << std::endl;
  {
    std::string lines = "First line\nSecond line";
    Assert::That(lines, Has().Exactly(2).EndingWith("line"));
  }

  std::cout << "CanHandleWindowsLineEndings" << std::endl;
  {
    std::string lines = "First line\r\nSecond line\r\nThird line";
    Assert::That(lines, Has().Exactly(3).EndingWith("line"));
  }

  std::cout << "CanMatchBeginningOfLinesWithWindowsLineEndings" << std::endl;
  {
    std::string lines = "First line\nSecond line\r\nThird line";
    Assert::That(lines, Has().Exactly(1).StartingWith("Second"));
  }

  std::cout << "CanHandleEmptyLinesWhenUsingWindowsLineEndings" << std::endl;
  {
    std::string lines = "\r\nSecond line\r\n\r\n";
    Assert::That(lines, Has().Exactly(2).OfLength(0));
  }

  std::cout << "CanHandleLastLineMissingNewlineForWindowsLineEndings" << std::endl;
  {
    std::string lines = "First line\r\nSecond line";
    Assert::That(lines, Has().Exactly(2).EndingWith("line"));
  }

  std::cout << "CanHandleAllEmptyLines" << std::endl;
  {
    Assert::That("\n\n\n\n\n\n", Has().Exactly(6).OfLength(0));
  }

  std::cout << "CanHandleAllEmptyLinesWithWindowsLineEndings" << std::endl;
  {
    Assert::That("\r\n\r\n\r\n", Has().Exactly(3).OfLength(0));
  }


  std::cout << "================================================" << std::endl;
  std::cout << "   StringLineParserTests" << std::endl;
  std::cout << "================================================" << std::endl;


  std::cout << "CanParseEmptyString" << std::endl;
  {
    std::vector<std::string> res;

    StringLineParser::Parse("", res);

    Assert::That(res, HasLength(0));
  }

  std::cout << "CanParseSingleLine" << std::endl;
  {
    std::vector<std::string> res;

    StringLineParser::Parse("Simple line", res);

    Assert::That(res, HasLength(1));
    Assert::That(res, Has().Exactly(1).EqualTo("Simple line"));
  }

  std::cout << "CanParseTwoLines" << std::endl;
  {
    std::vector<std::string> res;

    StringLineParser::Parse("One line\nTwo lines", res);

    Assert::That(res, HasLength(2));
    Assert::That(res, Has().Exactly(1).EqualTo("One line"));
    Assert::That(res, Has().Exactly(1).EqualTo("Two lines"));
  }

  std::cout << "CanParseThreeLines" << std::endl;
  {
    std::vector<std::string> res;

    StringLineParser::Parse("One line\nTwo lines\nThree lines", res);

    Assert::That(res, HasLength(3));
    Assert::That(res, Has().Exactly(1).EqualTo("One line"));
    Assert::That(res, Has().Exactly(1).EqualTo("Two lines"));
    Assert::That(res, Has().Exactly(1).EqualTo("Three lines"));
  }

  std::cout << "CanHandleStringEndingWithNewline" << std::endl;
  {
    std::vector<std::string> res;
    StringLineParser::Parse("One line\n", res);
    Assert::That(res, HasLength(1));
    Assert::That(res, Has().Exactly(1).EqualTo("One line"));
  }

  std::cout << "CanHandleSingleLineWithWindowsLineEnding" << std::endl;
  {
    std::vector<std::string> res;
    StringLineParser::Parse("One line\r\n", res);
    Assert::That(res, HasLength(1));
    Assert::That(res, Has().Exactly(1).EqualTo("One line"));
  }

  std::cout << "CanHandleTwoLinesWithWindowsLineEndings" << std::endl;
  {
    std::vector<std::string> res;
    StringLineParser::Parse("One line\r\nTwo lines", res);
    Assert::That(res, HasLength(2));
    Assert::That(res, Has().Exactly(1).EqualTo("One line"));
    Assert::That(res, Has().Exactly(1).EqualTo("Two lines"));
  }

  std::cout << "CanHandleEmptyLineWithNewline" << std::endl;
  {
    std::vector<std::string> res;
    StringLineParser::Parse("\n", res);
    Assert::That(res, Is().OfLength(1).And().Exactly(1).OfLength(0));
  }

  std::cout << "CanHandleTwoEmptyLines" << std::endl;
  {
    std::vector<std::string> res;
    StringLineParser::Parse("\n\n", res);
    Assert::That(res, HasLength(2));
    Assert::That(res, Has().Exactly(2).OfLength(0));
  }

  std::cout << "CanHandleTwoEmptyLinesWithWindowsLineEndings" << std::endl;
  {
    std::vector<std::string> res;
    StringLineParser::Parse("\r\n\r\n", res);
    Assert::That(res, HasLength(2));
    Assert::That(res, Has().Exactly(2).OfLength(0));
  }

  std::cout << "CanHandleCarriageReturnOnly" << std::endl;
  {
    std::vector<std::string> res;
    StringLineParser::Parse("One line\rTwo lines", res);
    Assert::That(res, HasLength(2));
    Assert::That(res, Has().Exactly(1).EqualTo("One line"));
    Assert::That(res, Has().Exactly(1).EqualTo("Two lines"));
  }

  std::cout << "CanHandleCarriageReturnOnlyAtEndOfString" << std::endl;
  {
    std::vector<std::string> res;
    StringLineParser::Parse("One line\r\nTwo lines\r", res);
    Assert::That(res, HasLength(2));
    Assert::That(res, Has().Exactly(1).EqualTo("One line"));
    Assert::That(res, Has().Exactly(1).EqualTo("Two lines"));
  }
}
