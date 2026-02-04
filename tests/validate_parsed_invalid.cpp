#include <cassert>
#include <string>

#include <vix/validation/Pipe.hpp>
#include <vix/validation/ValidationError.hpp>

using namespace vix::validation;

int main()
{
  std::string input = "abc";

  auto res = validate_parsed<int>("age", input)
                 .between(18, 120)
                 .result("age must be a number");

  assert(!res.ok());
  assert(res.errors.size() >= 1);
  assert(res.errors.all()[0].field == "age");
  assert(res.errors.all()[0].code == ValidationErrorCode::Format);

  return 0;
}
