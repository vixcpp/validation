#include <cassert>
#include <iostream>
#include <string>

#include <vix/validation/Pipe.hpp>
#include <vix/validation/ValidationError.hpp>

using vix::validation::validate_parsed;
using vix::validation::ValidationErrorCode;

int main()
{
  // -------------------------
  // Valid parsed value
  // -------------------------
  {
    std::string age = "25";
    auto res = validate_parsed<int>("age", age)
                   .between(18, 120)
                   .result();

    assert(res.ok());
  }

  // -------------------------
  // Parsed value out of range
  // -------------------------
  {
    std::string age = "10";
    auto res = validate_parsed<int>("age", age)
                   .between(18, 120, "age out of range")
                   .result();

    assert(!res.ok());
    assert(res.errors.size() == 1);
    assert(res.errors.all()[0].field == "age");
    assert(res.errors.all()[0].code == ValidationErrorCode::Between);
  }

  // -------------------------
  // Parsing failure (format)
  // -------------------------
  {
    std::string age = "abc";
    auto res = validate_parsed<int>("age", age)
                   .between(18, 120)
                   .result("age must be a number");

    assert(!res.ok());
    assert(res.errors.size() == 1);

    const auto &err = res.errors.all()[0];
    assert(err.field == "age");
    assert(err.code == ValidationErrorCode::Format);
    assert(err.meta.count("conversion_code") == 1);
    assert(err.meta.count("position") == 1);
  }

  // -------------------------
  // Overflow parsing failure
  // -------------------------
  {
    std::string age = "999999999999999999999";
    auto res = validate_parsed<int>("age", age)
                   .max(120)
                   .result("invalid age");

    assert(!res.ok());
    assert(res.errors.size() == 1);
    assert(res.errors.all()[0].code == ValidationErrorCode::Format);
  }

  std::cout << "[validation] validate_parsed smoke tests passed\n";
  return 0;
}
